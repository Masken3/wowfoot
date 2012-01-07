#define __STDC_FORMAT_MACROS
#include "quest.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "dbcSpell.h"
#include "db_item.h"

using namespace std;

void questChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcs.load();
	gItems.load();
	gSpells.load();
	gQuests.load();

	int id = toInt(urlPart);
	a = gQuests.find(id);
	if(a) {
		mTitle = a->title;

		// todo:

		mTabs.push_back(getComments("quest", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}

void questChtml::streamQuestText(ostream& os, const string& src) {
	for(size_t i=0; i<src.size(); i++) {
		if(src[i] == '$' && toupper(src[i+1]) == 'B') {
			os << "<br>\n";
			i++;
			continue;
		}
		streamHtmlEscape(os, src[i]);
	}
}
