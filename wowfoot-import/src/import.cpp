#include "config.h"
#include <sqlite3.h>
#include <string>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <fstream>
#include "json.h"

using namespace std;

// static config
static const char* const DB_FILENAME = "imports.db";

// helpers
static string parseCommentLine(const string& entry, const char* line);
static void dumpJsonValue(const JsonValue&);
static string escape(const string&);

static bool beginsWith(const string& s, const string& with) {
	return strncmp(s.c_str(), with.c_str(), with.length()) == 0;
}

static void handleErrno() __attribute((noreturn));
static void handleErrno() {
	printf("Errno: %i (%s)\n", errno, strerror(errno));
	abort();
}

#define SQLT(func) do { int _res = (func); if(_res != SQLITE_OK) { if(db) { sqlite3_close(db); } return 1; } } while(0)

// main program

int main() {
	// open db
	sqlite3* db;
	SQLT(sqlite3_open(DB_FILENAME, &db));
	
	// create tables
	{
		ifstream f("create-db.sql");
		if(!f) {
			handleErrno();
		}
		while(f.good()) {
			string s;
			getline(f, s);
			char* err;
			SQLT(sqlite3_exec(db, s.c_str(), NULL, NULL, &err));
		}
	}
	
	// list files in src dir
	DIR* dir = opendir(CONFIG_SRCDIR);
	if(!dir) {
		handleErrno();
	}
	string query = "BEGIN;\n";
	printf("listing %s ...\n", CONFIG_SRCDIR);
	while(dirent* de = readdir(dir)) {
		printf("%s\n", de->d_name);
		if(de->d_name[0] == '.')
			continue;
		static const char QUEST[] = "quest=";
		assert(strncmp(de->d_name, QUEST, strlen(QUEST)) == 0);
		const char* entry = de->d_name + strlen(QUEST);
		
		ifstream f((string(CONFIG_SRCDIR "/") + de->d_name).c_str());
		if(!f) {
			handleErrno();
		}
		int c = 1;
		while(f.good()) {
			string line;
			getline(f, line);
			//printf("%i: %s\n", c++, line.c_str());
			c++;
			static const string COMMENT_LINE_BEGIN = "var lv_comments0 = ";
			if(beginsWith(line, COMMENT_LINE_BEGIN)) {
				//printf("got it.\n");
				//printf("%i: %s\n", c++, line.c_str());
				query += parseCommentLine(entry, line.c_str() + COMMENT_LINE_BEGIN.length());
			}
		}
		//break;
	}
	query += "COMMIT;\n";
	printf("Query complete. saving...\n");

	ofstream("query.sql").write(query.c_str(), query.length());
	char* err;
	printf("executing...\n");
	if(SQLITE_OK != sqlite3_exec(db, query.c_str(), NULL, NULL, &err)) {
		printf("Error: %s\n", err);
		abort();
	}
	
	printf("closing...\n");
	SQLT(sqlite3_close(db));
	printf("done.\n");
}

static string parseCommentLine(const string& entry, const char* line) {
	JsonValue* root = parseJson(line);
	assert(root);
	//dumpJsonValue(*root);
	//printf("\n");
	const JsonArray* arr = root->toArray();
	assert(arr);
	string query;
	//printf("size: %zu\n", arr->size());
	for(size_t i=0; i<arr->size(); i++) {
		const JsonObject* hp = (*arr)[i].toObject();
		assert(hp);
		const JsonObject& h(*hp);
		query += "INSERT INTO 'quest_comments' VALUES ("+entry+", "+
			h["id"].toString()+");\n";
		query += "INSERT INTO 'comments' VALUES ("+h["id"].toString()+", '"+
			escape(h["user"].toString())+"', '"+
			escape(h["body"].toString())+"', "+
			h["rating"].toString()+", '"+
			escape(h["date"].toString())+"', "+
			h["indent"].toString()+");\n";
	}
	delete root;
	return query;
}

static string escape(const string& s) {
	string n;
	n.reserve(s.length()*2);
	for(size_t i=0; i<s.length(); i++) {
		if(s[i] == '\'') {
			n += '\'';
		}
		n += s[i];
	}
	return n;
}

static void dumpJsonValue(const JsonValue& val) {
	static int level = 0;
	level++;
	assert(level < 10);
	if(const JsonArray* arr = val.toArray()) {
		printf("[");
		//printf("%p", arr);
		for(size_t i=0; i<arr->size(); i++) {
			if(i != 0)
				printf(", ");
			dumpJsonValue((*arr)[i]);
		}
		printf("]");
	} else if(const JsonObject* obj = val.toObject()) {
		printf("{%p", obj);
#if 0
		for(JsonObject::iterator itr = obj->begin(); itr != obj->end(); ++itr) {
			if(itr != obj->begin())
				printf(", ");
			printf("%s:", itr->first.c_str());
			dumpJsonValue(*itr->second);
		}
#endif
		printf("}");
	} else {
		printf("%s", val.toString().c_str());
	}
	level--;
}
