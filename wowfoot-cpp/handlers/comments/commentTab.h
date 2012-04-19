#ifndef COMMENT_TAB_H
#define COMMENT_TAB_H

#include <string>
#include <vector>
#include "tabs.h"

using namespace std;

struct Comment {
	string user;
	string body;
	string originalBody;
	int rating;
	string date;
	int indent;
	int id;
};

class commentTabChtml : public Tab {
public:
	int run(ostream&);

	vector<Comment> mComments;

protected:
	void indentHtml(ostream&, int indent);
	void indentEndHtml(ostream&);
	const char* patch(const char* date);
};

#endif	//COMMENT_TAB_H
