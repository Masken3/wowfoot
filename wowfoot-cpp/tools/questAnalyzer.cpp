#include <unordered_map>
#include <stdio.h>
#include "db_quest.h"
#include "db_gameobject_template.h"
#include "db_creature_template.h"
#include "db_loot_template.h"
#include "db_item.h"
#include "db_questrelation.h"
#include "db_spawn.h"
#include "dbcSpell.h"
#include "dbcMap.h"
#include "dbcAreaTrigger.h"
#include "questPoints.h"
#include "util/exception.h"
#include "util/minmax.h"
#include "util/arraySize.h"

using namespace std;

struct QuestInfo {
	int visitCount;
	int childChainLength;
};

// analyze quest dependency graph
class Analyzer : QuestPointListener {
private:
	QuestInfo mMax;
	// qid, count; number of times a quest has been seen by the analyzer.
	unordered_map<int, QuestInfo> mInfo;
	int mostPopular;
	int longestChain;
	FILE* mDotFile;
	FILE* mMapFile;
	int mMaxGivers, mMG;
	int mMaxFinishers, mMF;

public:
	void analyze() {
		mDotFile = fopen("build/quests.dot", "w");
		if(!mDotFile)
			throwERRNO();
		mMapFile = fopen("build/questmap.txt", "w");
		if(!mMapFile)
			throwERRNO();
		fprintf(mDotFile, "strict digraph quests {\n");
		mMax = QuestInfo();
		// visit each quest:
		// if it has been visited, don't visit it again.
		// visit all related quests (required, opened, following)
		// if a quest has no requirements, it is a root. count descendants.
		// table "conditions" is required for a few special cases.
		int distinct = 0;
		int count = 0;
		mMaxGivers = mMaxFinishers = 0;
		for(auto itr = gQuests.begin(); itr != gQuests.end(); ++itr) {
			const Quest& q(itr->second);
			count++;
			fprintf(mDotFile, "n%i [label=\"%s\" URL=\"http://localhost:3002/quest=%i\" target=\"_blank\"];\n",
				q.id, q.title.c_str(), q.id);
			dumpQuestLocations(q);
			if(!preVisit(q.id))
				continue;
			distinct++;
			process(q);
		}
		fprintf(mDotFile, "}\n");
		fclose(mDotFile);
		fclose(mMapFile);
		printf("Looked at %i distinct chains out of %i quests.\n", distinct, count);
		printf("Max visitCount: %i (quest=%i)\n", mMax.visitCount, mostPopular);
		printf("Max chainLength: %i (quest=%i)\n", mMax.childChainLength, longestChain);
		printf("Max givers: %i (quest=%i)\n", mMaxGivers, mMG);
		printf("Max finishers: %i (quest=%i)\n", mMaxFinishers, mMF);
	}
private:
	int mGiverCount, mFinisherCount, mObjectiveCount;
	virtual void questGivers(Spawns::IdPair sp) {
		mGiverCount += dumpQuestLocationPair(sp);
	}
	virtual void questFinishers(Spawns::IdPair sp) {
		if(mFinisherCount == -1) {
			fprintf(mMapFile, " finisher");
			mFinisherCount = 0;
		}
		mFinisherCount += dumpQuestLocationPair(sp);
	}
	virtual void questObjectives(Spawns::IdPair sp) {
		if(mObjectiveCount == -1) {
			fprintf(mMapFile, "\ng%i f%i", mGiverCount, mFinisherCount);
			mObjectiveCount = 0;
		}
		mObjectiveCount += dumpQuestLocationPair(sp);
	}
	void questAreaObjective(const AreaTrigger& at) {
		questObjectives(Spawns::IdPair());
		fprintf(mMapFile, " at %i (map %i, %f,%f,%f, r%f)", at.id, at.map, at.x, at.y, at.z, at.radius);
	}
	// returns count
	int dumpQuestLocationPair(Spawns::IdPair sp) {
		int count = 0;
		for(; sp.first != sp.second; ++sp.first) {
			const Spawn& s(*sp.first->second);
			count++;
			fprintf(mMapFile, " map %i (%f,%f,%f)", s.map, s.position_x, s.position_y, s.position_z);
		}
		return count;
	}
	void dumpQuestLocations(const Quest& q) {
		fprintf(mMapFile, "%i (%s) giver", q.id, q.title.c_str());
		mGiverCount = 0;
		mFinisherCount = -1;
		mObjectiveCount = -1;
		getQuestLocations(q, *this);
		fprintf(mMapFile, "\n");

		// skip "CLUCK!"
		if(mMaxGivers < mGiverCount && q.id != 3861) {
			mMaxGivers = mGiverCount;
			mMG = q.id;
		}
		if(mMaxFinishers < mFinisherCount && q.id != 3861) {
			mMaxFinishers = mFinisherCount;
			mMF = q.id;
		}
	}

	void parent(int cid, int oid) {
		if(oid == 0)
			return;
		if(oid < 0)
			oid *= -1;
		fprintf(mDotFile, "n%i -> n%i;\n", oid, cid);
	}
	void child(int cid, int oid) {
		if(oid == 0)
			return;
		if(oid < 0)
			oid *= -1;
		fprintf(mDotFile, "n%i -> n%i;\n", cid, oid);
	}
	// returns max child chain length, including self.
	int process(const Quest& q) {
		QuestInfo& info(mInfo[q.id]);
		int res;

		parent(q.id, q.prevQuestId);
		visit(q.prevQuestId);
		child(q.id, q.nextQuestId);
		res = visit(q.nextQuestId);
		info.childChainLength = MAX(info.childChainLength, res);
		if(q.nextQuestInChain != q.nextQuestId)
			child(q.id, q.nextQuestInChain);
		res = visit(q.nextQuestInChain);
		info.childChainLength = MAX(info.childChainLength, res);
		if(info.childChainLength > mMax.childChainLength) {
			longestChain = q.id;
			mMax.childChainLength = info.childChainLength;
		}
		return info.childChainLength+1;
	}
	bool preVisit(int id) {
		QuestInfo& info(mInfo[id]);
		info.visitCount++;
		if(info.visitCount > mMax.visitCount) {
			mostPopular = id;
			mMax.visitCount = info.visitCount;
		}
		return info.visitCount == 1;
	}
	int visit(int id) {
		if(id == 0)
			return 0;
		if(id < 0)
			id *= -1;
		if(!preVisit(id))
			return 0;
		const Quest* qp = gQuests.find(id);
		if(!qp) {
			printf("Quest %i not found!\n", id);
			return 0;
		}
		return process(*qp);
	}
};

int main() {
	gQuests.load();
	gObjects.load();
	gNpcs.load();
	gItems.load();
	gCreatureQuestGivers.load();
	gCreatureQuestFinishers.load();
	gObjectQuestGivers.load();
	gObjectQuestFinishers.load();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gGameobjectLoots.load();
	gMaps.load();
	gAreaQuestObjectives.load();
	gAreaTriggers.load();
	gCreatureSpawns.load();
	gGameobjectSpawns.load();

	Analyzer a;
	printf("Analyzer start...\n");
	a.analyze();
	printf("Analyzer done.\n");
}
