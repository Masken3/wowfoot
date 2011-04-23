#include "json.h"
#include <vector>
#include <assert.h>

using namespace std;

// Classes

class JsonValueImpl : public JsonValue {
public:
	string mValue;
public:
	const string& toString() const { return mValue; }
};

class JsonArrayImpl : public JsonArray {
public:
	vector<JsonValue*> mVec;
public:
	virtual ~JsonArrayImpl() {
		for(size_t i=0; i<mVec.size(); i++) {
			delete mVec[i];
		}
	}
	const JsonArray* toArray() const { return this; }
	virtual size_t size() const { return mVec.size(); }
	virtual const JsonValue& operator[](size_t i) const { return *mVec[i]; }
};

class JsonObjectImpl : public JsonObject {
public:
	Map mMap;
public:
	virtual ~JsonObjectImpl() {
		for(Map::iterator itr=mMap.begin(); itr != mMap.end(); ++itr) {
			delete itr->second;
		}
	}
	const JsonObject* toObject() const { return this; }
	virtual const JsonValue& operator[](const string& key) const {
		Map::const_iterator itr = mMap.find(key);
		if(itr == mMap.end())
			abort();
		else
			return *itr->second;
	}
	
	virtual iterator begin() const { return mMap.begin(); }
	virtual iterator end() const { return mMap.end(); }
};

// Statics
const char* sText;

// Functions
static JsonValue* parseValue();
static JsonArray* parseArray();
static JsonObject* parseObject();
static JsonValue* parseInt();
static JsonValue* parseString(char delim);
static void error() __attribute((noreturn));
static void skipWhitespace();
static void token(char);
static bool isJsonDigit(char);
static bool nextInList(char breaker);

JsonValue* parseJson(const char* text) {
	sText = text;
	return parseValue();
}

static JsonValue* parseValue() {
	skipWhitespace();
	char c = *sText;
	if(c == '[')
		return parseArray();
	if(c == '{')
		return parseObject();
	if(c == '\'' || c == '"')
		return parseString(c);
	if(isJsonDigit(c))
		return parseInt();
	error();
}

static JsonValue* parseString(char delim) {
	token(delim);
	string s;
	while(*sText != delim) {
		if(*sText == '\\')
			sText++;
		s += *sText;
		sText++;
	}
	sText++;
	JsonValueImpl* v = new JsonValueImpl;
	v->mValue = s;
	return v;
}

static JsonValue* parseInt() {
	string s;
	while(isJsonDigit(*sText)) {
		s += *sText;
		sText++;
	}
	JsonValueImpl* v = new JsonValueImpl;
	v->mValue = s;
	return v;
}
static int level = 0;
static JsonArray* parseArray() {
	level++;
	assert(level < 10);
	token('[');
	JsonArrayImpl* arr = new JsonArrayImpl;
	while(true) {
		if(*sText == ']')
			break;
		arr->mVec.push_back(parseValue());
		if(nextInList(']'))
			continue;
		else
			break;
	}
	level--;
	return arr;
}

static JsonObject* parseObject() {
	level++;
	assert(level < 10);
	token('{');
	JsonObjectImpl* obj = new JsonObjectImpl;
	while(true) {
		if(*sText == '}')
			break;
		skipWhitespace();
		string name;
		while(isalpha(*sText)) {
			name += *sText;
			sText++;
		}
		token(':');
		obj->mMap[name] = parseValue();
		
		if(nextInList('}'))
			continue;
		else
			break;
	}
	level--;
	return obj;
}

static bool isJsonDigit(char c) {
	return isdigit(c) || c == '-';
}

static void error() {
	printf("Json parse error. current text: (%i) %s\n", *sText, sText);
	abort();
}

static void skipWhitespace() {
	while(isspace(*sText)) {
		sText++;
	}
}

static void token(char c) {
	skipWhitespace();
	if(*sText != c)
		error();
	sText++;
}

static bool nextInList(char breakToken) {
	char c = *sText;
	sText++;
	if(c == ',')
		return true;
	if(c == breakToken)
		return false;
	error();
}
