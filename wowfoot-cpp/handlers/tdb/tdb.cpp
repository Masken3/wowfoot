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
#include <sstream>
#include <stdio.h>
#include <stdexcept>

using namespace std;

static MYSQL* sMysql;

static void error(const char* funcName) {
	printf("%s: %s\n", funcName, mysql_error(sMysql));
	throw logic_error(funcName);
}

static void init() {
	static bool sInit = false;
	if(sInit)
		return;
	sInit = true;

	// Required for strtod() to work properly on systems whose default locale
	// uses non-dot (.) radix characters.
	// Example: Sweden uses comma (,).
	setlocale(LC_NUMERIC, "C");

	sMysql = mysql_init(NULL);
	if(!mysql_real_connect(sMysql, TDB_SERVER, TDB_USER, TDB_PASS, TDB_WORLD, 0,NULL,0)) {
		error("mysql_real_connect");
	}
}

static int safe_atoi(const char* str, unsigned long len) {
	int i;
	unsigned int n;
	int res = sscanf(str, "%i%n", &i, &n);
	assert(res == 1);
	assert(len == n);
	return i;
}

static int safe_atof(const char* str, unsigned long len) {
	float f;
	unsigned int n;
	int res = sscanf(str, "%f%n", &f, &n);
	if(len != n) {
		printf("atof: '%s' len %lu, n %i\n", str, len, n);
	}
	assert(res == 1);
	assert(len == n);
	return f;
}

void fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	TableFetchCallback tfc)
{
	init();

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
		if(count % 1000 == 0) {
			printf("%i\n", count);
		}

		assert(mysql_num_fields(res) == nCol);
		unsigned long* lengths = mysql_fetch_lengths(res);
		assert(lengths);

		// get destination struct
		assert(strcmp(cf[0].name, "entry") == 0);
		int entry = safe_atoi(row[0], lengths[0]);
		char* dst = (char*)tfc(entry);

		// store entry in struct
		*(int*)(dst + cf[0].offset) = entry;

		for(size_t i=1; i<nCol; i++) {
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
	}
	printf("%i\n", count);
	if(mysql_errno(sMysql))
		error("mysql_fetch_row");
	mysql_free_result(res);
}
