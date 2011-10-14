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
	while(Row row = res.fetch_row()) {
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
	if(gConn.errnum()) {
		printf("Error fetching row: %s\n", gConn.error());
		throw logic_error("Error fetching row");
	}
}
