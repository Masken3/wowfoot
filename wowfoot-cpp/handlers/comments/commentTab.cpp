#include "commentTab.h"
#include "patch.h"
#include <time.h>
#include "win32.h"
#include <stdexcept>

using namespace std;

void commentTabChtml::indentHtml(ostream& stream, int indent) {
}

void commentTabChtml::indentEndHtml(ostream& stream) {
}

const char* commentTabChtml::patch(const char* date) {
	// parse string date to time_t
	// search through patch version list
	struct tm tm;
	const char* res = strptime(date, "%Y/%m/%d %T", &tm);
	if(!res)
		throw logic_error("commentTabChtml::patch: strptime failed");
	time_t t = timegm(&tm);
	for(int i=1; i<gnPatchVersions; i++) {
		if(t < gPatchVersions[i].time)
			return gPatchVersions[i-1].name;
	}
	throw logic_error("commentTabChtml::patch: invalid date");
}
