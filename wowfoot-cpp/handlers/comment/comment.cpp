#define __STDC_FORMAT_MACROS
#include "comment.chtml.h"
#include "comments.h"
#include "tabs.h"

using namespace std;

void commentChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	mId = toInt(urlPart);
	mTabs.push_back(getComment(mId));

	drd->code = run(os);
}
