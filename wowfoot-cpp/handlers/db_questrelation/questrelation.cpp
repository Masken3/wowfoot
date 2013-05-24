#include "questrelation.h"
#include "tabTable.h"
#include "db_quest.h"
#include "chtmlUtil.h"

enum QrRowId {
	NAME = ENTRY+1,
};


Tab* getQuestRelations(const char* title, QuestRelations& set, int id) {
	set.load();
	gQuests.load();

	tabTableChtml& t = *new tabTableChtml();
	t.id = title;
	t.title = title;
	t.columns.push_back(Column(NAME, "Title", ENTRY, "quest"));
	auto p = set.findId(id);
	for(; p.first != p.second; ++p.first) {
		const QuestRelation& qr(*p.first->second);
		const Quest* q = gQuests.find(qr.quest);
		Row r;
		r[ENTRY] = toString(qr.quest);
		if(q)
			r[NAME] = q->title;
		else
			r[NAME] = "Unknown quest "+r[ENTRY];
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
