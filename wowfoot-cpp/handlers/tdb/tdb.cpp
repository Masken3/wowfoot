#include <mysql++.h>
#include "config.h"
#include "tdb_raw.h"
#include <sstream>
#include <stdio.h>

using namespace mysqlpp;
using namespace std;

Connection gConn(TDB_WORLD, TDB_SERVER, TDB_USER, TDB_PASS);

void fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	TableFetchCallback tfc)
{
	printf("fetching table %s...\n", tableName);
	ostringstream oss;
	oss << "SELECT ";
	for(size_t i=0; i<nCol; i++) {
		if(i != 0)
			oss << ", ";
		oss << cf[i].name;
	}
	oss << " FROM " << tableName;
	Query query = gConn.query(oss.str());
	UseQueryResult res = query.use();
	if(!res) {
		throw logic_error("Query.use failed");
	}
	int count = 0;
	while(Row row = res.fetch_row()) {
		count++;
		if(count % 1000 == 0) {
			printf("%i\n", count);
		}
		assert(strcmp(cf[0].name, "entry") == 0);
		int entry = row["entry"];
		char* dst = (char*)tfc(entry);

		//entry
		*(int*)dst = entry;
		char* ptr = dst + sizeof(int);

		for(size_t i=1; i<nCol; i++) {
			switch(cf[i].type) {
			case CDT_INT:
				*(int*)ptr = row[cf[i].name];
				ptr += sizeof(int);
				break;
			case CDT_STRING:
				*(string*)ptr = string(row[cf[i].name]);
				ptr += sizeof(string);
				break;
			case CDT_FLOAT:
				*(float*)ptr = row[cf[i].name];
				ptr += sizeof(float);
				break;
			}
		}
	}
	printf("%i\n", count);
	if(gConn.errnum()) {
		printf("Error fetching row: %s\n", gConn.error());
		throw logic_error("Error fetching row");
	}
}
