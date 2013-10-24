#include "npc.chtml.h"
#include "db_creature_template.h"
#include "db_creature_template_spells.h"
#include "db_spawn.h"
#include "questrelation.h"
#include "comments.h"
#include "item_shared.h"
#include "db_npc_vendor.h"
#include "db_npc_trainer.h"
#include "db_item.h"
#include "dbcSpell.h"
#include "Spell.index.h"
#include "money.h"
#include "dbcSkillLine.h"
#include "db_loot_template.h"
#include "db_quest.h"
#include "questObjectiveT.h"
#include "util/arraySize.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

static Tab* sells(int npcId);
static Tab* teaches(int npcId);
static Tab* spawnedBy(int npcId);
static Tab* usesSpells(int npcId);
static Tab* drops(int npcId);
static Tab* pickpocketLoot(int npcId);
static Tab* questObjective(int npcId);

void npcChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcs.load();
	gCreatureSpawns.load();
	gFactions.load();
	gFactionTemplates.load();
	gNpcVendors.load();
	gNpcTrainers.load();
	gNpcSpells.load();
	gItems.load();
	gSpells.load();
	SpellIndex::load();
	gSkillLines.load();
	gQuests.load();
	gPickpocketingLoots.load();
	gCreatureLoots.load();
	spawnPointsPrepare();

	int id = toInt(urlPart);
	a = gNpcs.find(id);
	if(a) {
		mTitle = a->name.c_str();

		mTabs.push_back(spawnedBy(id));
		mTabs.push_back(getQuestRelations("startQuest", "Starts quest", gCreatureQuestGivers, id));
		mTabs.push_back(getQuestRelations("endQuest", "Ends quest", gCreatureQuestFinishers, id));
		mTabs.push_back(sells(id));
		mTabs.push_back(teaches(id));
		mTabs.push_back(usesSpells(id));
		mTabs.push_back(drops(id));
		mTabs.push_back(pickpocketLoot(id));
		mTabs.push_back(questObjective(id));
		mTabs.push_back(getComments("npc", id));

		mSpawnPointsChtml.addSpawns(gCreatureSpawns.findId(id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

void npcChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* simpleLoot(int npcId, const char* id, const char* title, const Loots& loots) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = id;
	t.title = title;
	itemColumns(t);
	lootColumns(t);
	Loots::IntPair res = loots.findEntry(npcId);
	for(; res.first != res.second; ++res.first) {
		const Loot& loot(*res.first->second);
		Row r;
		const Item* item = gItems.find(loot.item);
		if(item) {
			itemRow(r, *item);
		} else {
			r[ENTRY] = toString(loot.item);
		}
		lootRow(r, loot);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* drops(int npcId) {
	return simpleLoot(npcId, "drops", "Drops", gCreatureLoots);
}

static Tab* pickpocketLoot(int npcId) {
	return simpleLoot(npcId, "pickpocketLoot", "Pickpocket", gPickpocketingLoots);
}

static Tab* questObjective(int npcId) {
	return questObjectiveT(npcId, "questObjective", "Quest objective",
		&Quests::findReqCreatureOrGOId, &Quest::Objective::reqCreatureOrGOId, &Quest::Objective::reqCreatureOrGOCount);
}

static Tab* sells(int npcId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "sells";
	t.title = "Sells";
	itemColumns(t);
	t.columns.push_back(Column(STOCK, "Stock"));
	auto res = gNpcVendors.findEntry(npcId);
	for(; res.first != res.second; ++res.first) {
		const NpcVendor& nv(*res.first->second);
		Row r;
		itemRow(r, gItems[nv.item]);
		if(nv.maxcount == 0)
			r[STOCK] = "∞";
		else
			r[STOCK] = toString(nv.maxcount);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static void setName(Row& r, int column, int spellId) {
	const Spell* s = gSpells.find(spellId);
	if(s) {
		r[column] = s->name;
		if(s->rank) if(s->rank[0]) {
			r[column] += std::string(" (") + s->rank + std::string(")");
		}
	} else {
		r[column] = "Unknown spell "+toString(spellId);
	}
}

static Tab* teaches(int npcId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "teaches";
	t.title = "Teaches";
	t.columns.push_back(Column(NAME, "Teaching", ENTRY, "spell"));
	t.columns.push_back(Column(UTILITY, "Taught", Column::NoEscape));
	t.columns.push_back(Column(COST, "Cost", Column::NoEscape));
	t.columns.push_back(Column(RESTRICTIONS, "Requirements"));
	t.columns.push_back(Column(CLEVEL, "cLevel"));
	auto res = gNpcTrainers.findEntry(npcId);
	for(; res.first != res.second; ++res.first) {
		const NpcTrainer& nt(*res.first->second);
		Row r;
		r[ENTRY] = toString(nt.spell);
		setName(r, NAME, nt.spell);
		if(const Spell* s = gSpells.find(nt.spell)) {
			if(s->effect[0].id == 36) {	//SPELL_EFFECT_LEARN
				setName(r, UTILITY, s->effect[0].triggerSpell);
				ostringstream oss;
				oss << "<a href=\"spell="<<s->effect[0].triggerSpell<<"\">"<<r[UTILITY]<<"</a>";
				r[UTILITY] = oss.str();
			}
		}
		{
			ostringstream oss;
			moneyHtml(oss, nt.spellCost);
			r[COST] = oss.str();
		}
		const SkillLine* sl = gSkillLines.find(nt.reqSkill);
		if(sl) {
			r[RESTRICTIONS] = sl->name;
			if(nt.reqSkillValue > 0) {
				r[RESTRICTIONS] += " " + toString(nt.reqSkillValue);
			}
		}
		r[CLEVEL] = toString(nt.reqLevel);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* spawnedBy(int npcId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "spawnedBy";
	t.title = "Spawned by";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "spell"));
	auto res = SpellIndex::findSpawnCreature(npcId);
	for(; res.first != res.second; ++res.first) {
		const Spell& s(*res.first->second);
		Row r;
		r[ENTRY] = toString(s.id);
		setName(r, NAME, s.id);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* usesSpells(int npcId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "usesSpells";
	t.title = "Uses spells";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "spell"));
	const NpcSpell* ns = gNpcSpells.find(npcId);
	if(ns) for(size_t i=0; i<ARRAY_SIZE(ns->spell); i++) {
		int spellId = ns->spell[i];
		if(spellId != 0) {
			Row r;
			r[ENTRY] = toString(spellId);
			setName(r, NAME, spellId);
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	return &t;
}
