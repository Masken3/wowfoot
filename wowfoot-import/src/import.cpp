//#include "config.h"
#include <sqlite3.h>
#include <string>
#include <string.h>
#include <fstream>
#include "json.h"

using namespace std;

// static config
static const char* const DB_FILENAME = "imports.db";
static const char COMMENT_LINE_BEGIN[] = "var lv_comments0 = ";

// helpers
/*static bool beginsWith(const string& s, const string& with) {
	return strncmp(s.c_str(), with.c_str(), with.length()) == 0;
}*/

#define SQLT(func) do { int _res = (func); if(_res != SQLITE_OK) { if(db) { sqlite3_close(db); } return 1; } } while(0)

// main program

int main() {
	// open db
	sqlite3* db;
	SQLT(sqlite3_open(DB_FILENAME, &db));
	
	// create tables
	ifstream f("create-db.sql");
	if(!f) {
		return 1;
	}
	while(f.good()) {
		string s;
		getline(f, s);
		char* err;
		SQLT(sqlite3_exec(db, s.c_str(), NULL, NULL, &err));
	}
}
#if 0
def parseCommentLine(entry, line)
	pos = COMMENT_LINE_BEGIN.length
	open('file.txt', 'w').write("#{entry}, #{line}")
	if(false)	// json standard requires that key strings be quoted, so we can't use this parser.
		line = line[pos, line.length-(pos+2)]
		line.gsub!("'", '"')
		#p line
		hashArray = JSON.parse(line)
	else
		#p line[pos, line.length]
		hashArray = JsonParser.parse(line, pos)
	end
	p hashArray
	query = ''
	hashArray.each do |h|
		query += "INSERT INTO 'quest_comments' VALUES (#{entry}, #{h[:id]});\n"
		query += "INSERT INTO 'comments' VALUES (#{h[:id]}, '#{h[:user]}', '#{h[:body]}',"+
			" #{h[:rating]}, '#{h[:date]}', #{h[:indent]}});\n"
	end
	return query
end

query = "BEGIN;\n"
Dir[CONFIG_SRCDIR + '/*'].each do |filename|
	p filename
	entry = File.basename(filename)['quest='.length, filename.length]
	open(filename, 'r') do |qfile|
		while(line = qfile.gets)
			if(line.beginsWith(COMMENT_LINE_BEGIN))
				query += parseCommentLine(entry, line)
			end
		end
	end
end
query += "COMMIT;\n"

open('query.sql', 'w').write(query)
db.execute query
#endif
			