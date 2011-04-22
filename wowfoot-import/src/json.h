#ifndef JSON_H
#define JSON_H

#include <string>

class JsonArray;
class JsonObject;

class JsonValue {
public:
	virtual ~JsonValue() {}
	// exactly one of these is valid.
	virtual const JsonArray* toArray() const { return NULL; }
	virtual const JsonObject* toObject() const { return NULL; }
	virtual const std::string& toString() const { static const std::string e(""); return e; }
};

class JsonArray : public JsonValue {
public:
	virtual size_t size() const = 0;
	virtual const JsonValue& operator[](size_t index) const = 0;
};

class JsonObject : public JsonValue {
public:
	virtual const JsonValue* operator[](const std::string& key) const = 0;
};

JsonValue* parseJson(const char* text);

#endif	//JSON_H
