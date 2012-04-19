#include "comments.h"
#include "commentTab.h"
#include "util/exception.h"
#include "formatter.h"
#include <sqlite3.h>
#include <stdio.h>

static sqlite3* sDB;

#define SQLT(func) do { int _res = (func); if(_res != SQLITE_OK) {\
	sqlite3_finalize(stmt); stmt = NULL;\
	printf("%s:%i: %s\n", __FILE__, __LINE__, sqlite3_errmsg(sDB));\
	throw Exception("sqlite"); } } while(0)

static void closeDb() {
	if(sDB) {
		sqlite3_stmt* stmt = NULL;	//dummy
		SQLT(sqlite3_close(sDB));
	}
}

static Tab* getComments(const char* query) {
	sqlite3_stmt* stmt = NULL;
	if(!sDB) {
		SQLT(sqlite3_open("../wowfoot-import/imports.db", &sDB));
		atexit(&closeDb);
	}

	SQLT(sqlite3_prepare_v2(sDB, query, -1, &stmt, NULL));

	int res;
	commentTabChtml* ct = new commentTabChtml();
	Formatter f;
	try {
	while((res = sqlite3_step(stmt)) == SQLITE_ROW) {
		Comment c;
		c.user = (const char*)sqlite3_column_text(stmt, 0);
		c.originalBody = (const char*)sqlite3_column_text(stmt, 1);
		for(size_t i=0; i<c.originalBody.size(); ) {
			// blank out invalid utf-8 sequences.
			wchar_t w;
			int rs = mbtowc(&w, c.originalBody.c_str() + i, c.originalBody.size() - i);
			if(rs <= 0) {
				c.originalBody[i] = ' ';
				i++;
				continue;
			}
			// transform '-' to avoid the HTML end-comment combo "-->".
			if(c.originalBody[i] == '-')
				c.originalBody[i] = '_';
			// transform EOL for readability.
			if(c.originalBody[i] == 'n' && c.originalBody[i-1] == '\\')
				c.originalBody[i] = '\n';
			i += rs;
		}
		c.rating = sqlite3_column_int(stmt, 2);
		c.date = (const char*)sqlite3_column_text(stmt, 3);
		c.indent = sqlite3_column_int(stmt, 4);
		c.id = sqlite3_column_int(stmt, 5);
		printf("Comment %i\n", c.id);
		c.body = f.formatComment((const char*)sqlite3_column_text(stmt, 1));
		ct->mComments.push_back(c);
	}
	if(res != SQLITE_DONE) {
		SQLT(res);
	}
	} catch(Exception& e) {
		SQLT(sqlite3_finalize(stmt));
		throw e;
	}
	SQLT(sqlite3_finalize(stmt));
	ct->id = "comments";
	ct->title = "Comments";
	ct->count = ct->mComments.size();
	return ct;
}

Tab* getComments(const char* type, int id) {
	char query[1024];
	sprintf(query, "SELECT user, body, rating, date, indent, id"
		" FROM comments"
		" INNER JOIN %s_comments on commentId = id"
		" WHERE %s_comments.entry = %i",
		type, type, id);
	return getComments(query);
}

Tab* getComment(int id) {
	char query[1024];
	sprintf(query, "SELECT user, body, rating, date, indent, id"
		" FROM comments"
		" WHERE id = %i",
		id);
	return getComments(query);
}
