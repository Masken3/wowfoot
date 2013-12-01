#define __STDC_FORMAT_MACROS
#include "wowVersion.h"
#include "faction.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "db_creature_onkill_reputation.h"
#include "db_quest.h"
#include "item_shared.h"
#include "FactionTemplate.index.h"
#include "util/numof.h"
#include "util/arraySize.h"

using namespace std;

static Tab* members(int factionId);
static Tab* quests(int factionId);
static Tab* enemies(int factionId);
static Tab* items(int factionId);

enum FactionTableRowId {
	REP = UTILITY+1,
};

void factionChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcReps.load();
	gFactions.load();
	FactionTemplateIndex::load();
	gNpcs.load();
	gQuests.load();
	//gItems.load();

	int id = toInt(urlPart);
	a = gFactions.find(id);
	if(a) {
		mTitle = a->name;

		//members
		mTabs.push_back(members(id));
		//quests that modify reputation
		mTabs.push_back(quests(id));
		//npcs that can be killed for reputation
		mTabs.push_back(enemies(id));
		//items that require reputation to buy
		//mTabs.push_back(items(id));
		items(id);

		mTabs.push_back(getComments("faction", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

void factionChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* members(int factionId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "members";
	t.title = "Members";
	npcColumns(t);
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	FactionTemplateIndex::FactionPair ftp = FactionTemplateIndex::findFaction(factionId);
	for(; ftp.first != ftp.second; ++ftp.first) {
		int f = ftp.first->second->first;
		//printf("FactionTemplate %i\n", f);
		auto p = gNpcs.findFaction(f);
		for(; p.first != p.second; ++p.first) {
			const Npc& npc(*p.first->second);
			Row r;
			npcRow(r, npc);
			r[SPAWN_COUNT] = toString(npc.spawnCount);
			// todo: add rep loss for killing.
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	return &t;
}

static Tab* quests(int factionId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "quests";
	t.title = "Quests";
	t.columns.push_back(Column(NAME, "Title", ENTRY, "quest"));
	t.columns.push_back(Column(UTILITY, "Reputation"));
#if (CONFIG_WOW_VERSION > 30000)
	Quests::RewardFactionIdPair p = gQuests.findRewardFactionId(factionId);
#else
	auto p = gQuests.findRewRepFaction(factionId);
#endif
	for(; p.first != p.second; ++p.first) {
		const Quest& q(*p.first->second);
		Row r;
		r[ENTRY] = toString(q.id);
		r[NAME] = q.title;
		for(size_t i=0; i<ARRAY_SIZE(q.rewardRepFaction); i++) {
			if(q.rewardRepFaction[i] == factionId) {
				r[UTILITY] = toString(q.rewardRepValue[i]);
			}
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* enemies(int factionId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "enemies";
	t.title = "Enemies";
	npcColumns(t);
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	t.columns.push_back(Column(REP, "Reputation", Column::NoEscape));
	auto p = gNpcReps.findRewOnKillRepFaction(factionId);
	for(; p.first != p.second; ++p.first) {
		const NpcRep& rep(*p.first->second);
		const Npc* npc = gNpcs.find(rep.creature_id);
		Row r;
		if(npc) {
			npcRow(r, *npc);
			r[SPAWN_COUNT] = toString(npc->spawnCount);
		} else {
			r[ENTRY] = toString(rep.creature_id);
		}

		// calculate/format value
		ostringstream o;
		const char* sides[2] = { "Alliance", "Horde" };
		const char* standings[] = {
			"Hated",
			"Hostile",
			"Unfriendly",
			"Neutral",
			"Friendly",
			"Honored",
			"Revered",
			"Exalted",
		};
		bool isEnemy = true;
		for(int i=0; i<2; i++) {
			int repVal = rep.rewOnKillRepValue[i];
			int repFaction = rep.rewOnKillRepFaction[i];
			if(repVal == 0 && repFaction == 0)
				continue;
			if(repVal < 0 && repFaction == factionId) {
				isEnemy = false;
				break;
			}
			o << (repVal > 0 ? "+" : "")<<repVal<<" with ";
			streamNameLinkById(o, gFactions, repFaction);
			unsigned s = rep.maxStanding[i];
			if(repVal > 0) {
				o << " until ";
				if(s < NUMOF(standings)) {
					o << standings[s];
				} else {
					o << "[invalid standing code "<<s<<"]";
				}
			}
			if(rep.teamDependent) {
				EASSERT(rep.teamDependent == 1);
				o << " for "<< sides[i];
			} else {
			}
			o << "<br>";
		}
		if(!isEnemy)
			continue;
		r[REP] = o.str();

		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* items(int factionId) {
	return NULL;
}
