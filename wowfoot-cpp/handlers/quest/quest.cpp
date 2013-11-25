#define __STDC_FORMAT_MACROS
#include "wowVersion.h"
#include "quest.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "dbcSpell.h"
#include "db_item.h"
#include "db_loot_template.h"
#if CONFIG_WOW_VERSION > 30000
#include "dbcQuestFactionReward.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include "win32.h"
#include "util/minmax.h"
#include "util/stl_map_insert.h"

using namespace std;

void questChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gMaps.load();
	gAreaQuestObjectives.load();
	gAreaTriggers.load();
	gCreatureQuestGivers.load();
	gObjectQuestGivers.load();
	printf("Loading finishers...\n");
	gCreatureQuestFinishers.load();
	gObjectQuestFinishers.load();
	printf("Finishers loaded.\n");
	gFactions.load();
#if CONFIG_WOW_VERSION > 30000
	gQuestFactionRewards.load();
#endif
	gNpcs.load();
	gItems.load();
	gSpells.load();
	gQuests.load();
	gObjects.load();
	spawnPointsPrepare();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gGameobjectLoots.load();

	int id = toInt(urlPart);
	a = gQuests.find(id);
	if(a) {
		mTitle = a->title;

		// todo: fix this for version 1.12.
		int rewMoney = a->rewardMoneyMaxLevel;
		if(a->level >= 65)
			mRewXp = rewMoney / 6;
		else if(a->level == 64)
			mRewXp = int(rewMoney / 4.8);
		else if(a->level == 63)
			mRewXp = int(rewMoney / 3.6);
		else if(a->level == 62)
			mRewXp = int(rewMoney / 2.4);
		else if(a->level == 61)
			mRewXp = int(rewMoney / 1.2);
		else //if(a->level <= 60)
			mRewXp = int(rewMoney / 0.6);

		mTabs.push_back(getComments("quest", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

#if CONFIG_WOW_VERSION > 30000
int questChtml::rewRepValue(int index) {
	if(a->rewardFactionValueIdOverride[index] == 0) {
		int id = a->rewardFactionValueId[index];
		EASSERT(abs(id) <= 9);
		if(id >= 0)
			return gQuestFactionRewards[1].rep[id].value;
		else	// (id < 0)
			return gQuestFactionRewards[2].rep[-id].value;
	} else {
		return a->rewardFactionValueIdOverride[index] / 100;
	}
}
#endif

void questChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

void questChtml::streamEncodedObjectives(ostream& stream) {
	for(uint i=0; i<ARRAY_SIZE(a->objective); i++) {
		const Quest::Objective& o(a->objective[i]);
		bool empty = true;
		// objective.text is optional, used only for scripted objectives.
		if(!o.text.empty()) {
			empty = false;
			streamWowFormattedText(stream, o.text);
			stream << ". ";
		}
		if(o.reqSourceCount != 0) {
			empty = false;
			stream << "Source item: ";
			NAMELINK(gItems, o.reqSourceId);
			stream << " x"<<o.reqSourceCount<<"\n";
		}
		if(o.reqItemCount != 0) {
			empty = false;
			stream << "Item: ";
			NAMELINK(gItems, o.reqItemId);
			stream << " x"<<o.reqItemCount<<"\n";
		}
		if(o.reqSpellCast) {
			empty = false;
			stream << "Cast ";
			NAMELINK(gSpells, o.reqSpellCast);
			if(o.reqCreatureOrGOCount != 0) {
				stream << "on ";
			}
		} else if(o.reqCreatureOrGOId > 0) {
			empty = false;
			stream << "Kill ";
		} else if(o.reqCreatureOrGOId < 0) {
			empty = false;
			stream << "Use ";
		}
		if(o.reqCreatureOrGOId > 0) {
			stream << "creature ";
			NAMELINK(gNpcs, o.reqCreatureOrGOId);
			stream << " "<<o.reqCreatureOrGOCount<<" times.\n";
		} else if(o.reqCreatureOrGOId < 0) {
			stream << "object ";
			NAMELINK(gObjects, -o.reqCreatureOrGOId);
			stream << " "<<o.reqCreatureOrGOCount<<" times.\n";
		}
		if(!empty) {
			stream << "<br>\n";
		}
	}
	// todo: spawnPoints.
	for(auto p = gAreaQuestObjectives.findQuest(a->id); p.first != p.second; ++p.first) {
		const AreaTrigger& at(gAreaTriggers[p.first->second->id]);
		const Map& map(gMaps[at.map]);
		stream << "Visit POI "<<at.id<<": map "<<at.map<<" ("<<map.name<<"), "<<at.x<<" "<<at.y<<" "<<at.z<<", radius "<<at.radius<<"<br>\n";
	}

	getQuestLocations(*a, *this);
}

void questChtml::questGivers(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eBlue);
}
void questChtml::questFinishers(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eYellow);
}
void questChtml::questObjectives(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eRed);
}
void questChtml::questAreaObjective(const AreaTrigger& at) {
	mSpawnPoints.addSpawn(at.map, at.x, at.y, spawnPointsChtml::eGreen);
}

struct QuestInfo {
	int visitCount;
	bool hasRootVisit;

	// distance from root quest.
	// may be negative, positive or zero.
	int maxDistance;

	QuestInfo() {
		memset(this, 0, sizeof(*this));
	}
};

class QuestChainWalker {
private:
	unordered_map<int, QuestInfo*> mInfo;
public:
	~QuestChainWalker() {
		for(auto itr=mInfo.begin(); itr != mInfo.end(); ++itr) {
			delete itr->second;
		}
	}
protected:
	enum Direction { eNone, eUp, eDown, eBoth };

	virtual void parent(const QuestInfo&, int qid, Direction d) = 0;
	virtual void child(const QuestInfo&, int qid, Direction d) = 0;
	virtual QuestInfo* create(const Quest&) = 0;

	QuestInfo& get(const Quest& q) {
		QuestInfo* info(mInfo[q.id]);
		if(!info) {
			info = mInfo[q.id] = create(q);
		}
		return *info;
	}
	void doParent(QuestInfo& info, int id, Direction d) {
		parent(info, id, d);
		QuestInfo* res = visit(id, d, info.maxDistance);
		if(!res) return;
	}
	void doChild(QuestInfo& info, int id, Direction d) {
		child(info, id, d);
		QuestInfo* res = visit(id, d, info.maxDistance);
		if(!res) return;
	}
	void process(const Quest& q, QuestInfo& info, Direction d) {
		Direction up=eNone, down=eNone;
		if(d != eNone) {
			info.hasRootVisit = true;
			if(d == eUp || d == eBoth)
				up = eUp;
			if(d == eDown || d == eBoth)
				down = eDown;
		}

		// parents
		doParent(info, q.prevQuestId, up);
		for(auto p = gQuests.findNextQuestId(q.id); p.first != p.second; ++p.first) {
			doParent(info, p.first->second->id, up);
		}
		for(auto p = gQuests.findNextQuestInChain(q.id); p.first != p.second; ++p.first) {
			doParent(info, p.first->second->id, up);
		}

		// children
		doChild(info, q.nextQuestId, down);
		if(q.nextQuestInChain != q.nextQuestId) {
			doChild(info, q.nextQuestInChain, down);
		}
		for(auto p = gQuests.findPrevQuestId(q.id); p.first != p.second; ++p.first) {
			doChild(info, p.first->second->id, down);
		}
	};
	QuestInfo* visit(int id, Direction d, int maxDist) {
		if(id == 0)
			return 0;
		if(id < 0)
			id *= -1;
		const Quest* qp = gQuests.find(id);
		if(!qp) {
			printf("Quest %i not found!\n", id);
			return 0;
		}
		QuestInfo& info(get(*qp));
		if(d == eUp)
			info.maxDistance = MIN(maxDist - 1, info.maxDistance);
		if(d == eDown)
			info.maxDistance = MAX(maxDist + 1, info.maxDistance);
		info.visitCount++;
		if(info.visitCount == 1 || (!info.hasRootVisit && d != eNone)) {
			process(*qp, info, d);
			return &info;
		} else {
			return 0;
		}
	}
};

struct GraphNode : QuestInfo {
	// quest dependencies, prerequisites.
	vector<GraphNode*> out, in;
	const Quest* q;

	bool used;
};

class GraphQuestChainWalker : QuestChainWalker {
private:
	void relative(const QuestInfo& qi, int id, vector<GraphNode*> GraphNode::* chain) {
		GraphNode& gn = (GraphNode&)qi;
		if(!id) {
			return;
		}
		const Quest* pp = gQuests.find(id);
		if(!pp) {
			printf("relative %i not found!\n", id);
			return;
		}
		(gn.*chain).push_back((GraphNode*)&get(*pp));
	}
protected:
	virtual void parent(const QuestInfo& qi, int parentId, Direction d) {
		relative(qi, parentId, &GraphNode::in);
	}
	virtual void child(const QuestInfo& qi, int childId, Direction d) {
		relative(qi, childId, &GraphNode::out);
	}
	virtual QuestInfo* create(const Quest& q) {
		GraphNode* gn = new GraphNode;
		gn->q = &q;
		gn->maxDistance = 0;
		gn->used = false;
		return gn;
	}
public:
	// return value becomes invalid when this is deleted.
	GraphNode* walk(const Quest& q) {
		QuestInfo& info(get(q));
		process(q, info, eBoth);
		return (GraphNode*)&info;
	}
};

static void addToTree(GraphNode* node, multimap<int, GraphNode*>& sortedTree) {
	if(node->used)
		return;
	insert(sortedTree, node->maxDistance, node);
	node->used = true;
	for(size_t i=0; i<node->out.size(); i++)
		addToTree(node->out[i], sortedTree);
	for(size_t i=0; i<node->in.size(); i++)
		addToTree(node->in[i], sortedTree);
}

// The difficulty of this is equivalent to graphviz.
void questChtml::streamQuestChain(ostream& stream) {
	// construct graph. throw error on loops.
	GraphQuestChainWalker w;
	GraphNode* root = w.walk(*a);
	//EASSERT(root->level == 0);	// enable once we've fixed the bugs.
	// sort by y-level: maximum distance from root quest, exclusiveGroup priority.
	multimap<int, GraphNode*> sortedTree;
	addToTree(root, sortedTree);
	// generate HTML
	int level = sortedTree.begin()->first;
	stream << level << ": ";
	for(auto itr=sortedTree.begin(); itr != sortedTree.end(); ++itr) {
		if(itr->first != level) {
			stream << "<br>\n";
			level = itr->first;
			stream << level << ": ";
		}
		const Quest& q(*itr->second->q);
		if(q.id == a->id) {
			stream << q.title;
		} else {
			NAMELINK(gQuests, q.id);
		}
		stream << "\n";
	}
}
