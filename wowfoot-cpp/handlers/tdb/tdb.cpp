// mysql prerequisites
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#endif
#include <mysql.h>

#include "config.h"
#include "tdb_raw.h"
#include "util/exception.h"
#include "util/criticalSection.h"
#include <sstream>
#include <stdio.h>
#include <stdexcept>

using namespace std;

static MYSQL* sMysql;
static const char* sColName;
static CriticalSection sCS;

static void error(const char* funcName) __attribute__((noreturn));
static void error(const char* funcName) {
	printf("%s: %s\n", funcName, mysql_error(sMysql));
	mysql_close(sMysql);
	sMysql = NULL;
	throw Exception(funcName);
}

static void init() {
	LOCK(sCS);
	static bool sInit = false;
	if(sInit)
		return;
	sInit = true;

	// Required for strtod() to work properly on systems whose default locale
	// uses non-dot (.) radix characters.
	// Example: Sweden uses comma (,).
	setlocale(LC_NUMERIC, "C");
}

static int safe_atoi(const char* str, unsigned long len) {
	if(!str)
		return 0;
	char* end;
	int i = strtol(str, &end, 10);
	if(end != str + len) {
		printf("%s: %s\n", sColName, str);
	}
	assert(end == str + len);
	return i;
}

static float safe_atof(const char* str, unsigned long len) {
	if(!str)
		return 0;
	char* end;
	double d = strtod(str, &end);
	if(end != str + len) {
		printf("%s: %s\n", sColName, str);
	}
	assert(end == str + len);
	return (float)d;
}

template<class T>
void fetchTableBase(const char* tableName, const ColumnFormat* cf, size_t nCol,
	char* (*dstGet)(const ColumnFormat&, MYSQL_ROW, unsigned long* lengths, T), T t,
	size_t ColCountStart, void (*PostCallback)())
{
	LOCK(sCS);
	init();

	sMysql = mysql_init(NULL);
	if(!mysql_real_connect(sMysql, TDB_SERVER, TDB_USER, TDB_PASS, TDB_WORLD, 0,NULL,0)) {
		error("mysql_real_connect");
	}

	printf("fetching table %s...\n", tableName);
	ostringstream oss;
	oss << "SELECT ";
	for(size_t i=0; i<nCol; i++) {
		if(i != 0)
			oss << ", ";
		oss << cf[i].name;
	}
	oss << " FROM " << tableName;
	string s = oss.str();
	if(mysql_real_query(sMysql, s.c_str(), s.length()))
		error("mysql_real_query");
	MYSQL_RES* res = mysql_use_result(sMysql);
	if(!res)
		error("mysql_use_result");
	int count = 0;
	MYSQL_ROW row;
	while((row = mysql_fetch_row(res)) != NULL) {
		count++;
		if(count % 10000 == 0) {
			printf("%i\n", count);
		}

		assert(mysql_num_fields(res) == nCol);
		unsigned long* lengths = mysql_fetch_lengths(res);
		assert(lengths);

		// get destination struct
		char* dst = dstGet(cf[0], row, lengths, t);

		for(size_t i=ColCountStart; i<nCol; i++) {
			sColName = cf[i].name;
			char* ptr = dst + cf[i].offset;
			switch(cf[i].type) {
			case CDT_INT:
				*(int*)ptr = safe_atoi(row[i], lengths[i]);
				break;
			case CDT_STRING:
				*(string*)ptr = string(row[i], lengths[i]);
				break;
			case CDT_FLOAT:
				*(float*)ptr = safe_atof(row[i], lengths[i]);
				break;
			}
		}
		PostCallback();
	}
	printf("%i\n", count);
	if(mysql_errno(sMysql))
		error("mysql_fetch_row");
	mysql_free_result(res);
	mysql_close(sMysql);
}

static char* dstGetMap(const ColumnFormat& cf, MYSQL_ROW row, unsigned long* lengths, TableFetchMap tfm) {
	// get destination struct
	//assert(strcmp(cf.name, "entry") == 0);
	int entry = safe_atoi(row[0], lengths[0]);
	char* dst = (char*)tfm(entry);

	// store entry in struct
	*(int*)(dst + cf.offset) = entry;
	return dst;
}
static void postDummy() {}

void fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	TableFetchMap tfm)
{
	fetchTableBase<TableFetchMap>(tableName, cf, nCol, dstGetMap, tfm, 1, postDummy);
}

static char* dstGetSet(const ColumnFormat&, MYSQL_ROW, unsigned long*, void* vdst) {
	return (char*)vdst;
}

void fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	TableFetchSet tfs, void* vdst)
{
	fetchTableBase<void*>(tableName, cf, nCol, dstGetSet, vdst, 0, tfs);
}
