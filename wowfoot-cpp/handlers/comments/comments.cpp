#include "comments.h"
#include "commentTab.h"
#include <sqlite3.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

typedef unsigned char byte;

static sqlite3* sDB;

#define SQLT(func) do { int _res = (func); if(_res != SQLITE_OK) {\
	sqlite3_finalize(stmt); stmt = NULL;\
	printf("%s:%i: %s\n", __FILE__, __LINE__, sqlite3_errmsg(sDB));\
	throw logic_error("sqlite"); } } while(0)

static void closeDb() {
	if(sDB) {
		sqlite3_stmt* stmt = NULL;	//dummy
		SQLT(sqlite3_close(sDB));
	}
}

Tab* getComments(const char* type, int id) {
	sqlite3_stmt* stmt = NULL;
	if(!sDB) {
		SQLT(sqlite3_open("../wowfoot-import/imports.db", &sDB));
		atexit(&closeDb);
	}

	char query[1024];
	sprintf(query, "SELECT user, body, rating, date, indent"
		" FROM comments"
		" INNER JOIN %s_comments on commentId = id"
		" WHERE %s_comments.entry = %i",
		type, type, id);

	SQLT(sqlite3_prepare_v2(sDB, query, -1, &stmt, NULL));

	int res;
	commentTabChtml* ct = new commentTabChtml();
	while((res = sqlite3_step(stmt)) == SQLITE_ROW) {
		Comment c;
		c.user = (const char*)sqlite3_column_text(stmt, 0);
		c.body = (const char*)sqlite3_column_text(stmt, 1);
		c.rating = sqlite3_column_int(stmt, 2);
		c.date = (const char*)sqlite3_column_text(stmt, 3);
		c.indent = sqlite3_column_int(stmt, 4);
		ct->mComments.push_back(c);
	}
	if(res != SQLITE_DONE) {
		SQLT(res);
	}
	SQLT(sqlite3_finalize(stmt));
	ct->id = "comments";
	ct->title = "Comments";
	ct->count = ct->mComments.size();
	return ct;
}
