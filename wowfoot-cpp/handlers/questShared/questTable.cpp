#include "questTable.h"
#include "../item/item_shared.h"

void questColumns(tabTableChtml& t) {
	t.columns.push_back(Column(NAME, "Title", ENTRY, "quest"));
	t.columns.push_back(Column(SIDE, "Side"));
}

void questRow(Row& r, const Quest& q) {
	r[ENTRY] = toString(q.id);
	r[NAME] = q.title;
	char buf[32];
	sprintf(buf, "r%x c%x", q.requiredRaces, q.requiredClasses);
	r[SIDE] = buf;
}

Tab* getQuestsTab(const char* id, const char* title, int entry, Quests::IntPair (Quests::*finder)(int)const) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "quests";
	t.title = "Quests";
	questColumns(t);
	auto res = (gQuests.*finder)(entry);
	for(; res.first != res.second; ++res.first) {
		const Quest& q(*res.first->second);
		Row r;
		questRow(r, q);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
