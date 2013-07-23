#define __STDC_FORMAT_MACROS
#include "quests.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "util/minmax.h"

void questsChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
}

struct QuestMaxRepTest {
	static bool have(const Quest& q) {
		return q.requiredMaxRepFaction != 0;
	}
};

struct QuestInfo {
	int visitCount;
	int childChainLength;
};

// analyze quest dependency graph
class Analyzer {
private:
	QuestInfo mMax;
	// qid, count; number of times a quest has been seen by the analyzer.
	unordered_map<int, QuestInfo> mInfo;
	int mostPopular;
	int longestChain;
	FILE* mFile;

public:
	void analyze() {
		mFile = fopen("build/quests.dot", "w");
		if(!mFile)
			throwERRNO();
		fprintf(mFile, "strict digraph quests {\n");
		mMax = QuestInfo();
		// visit each quest:
		// if it has been visited, don't visit it again.
		// visit all related quests (required, opened, following)
		// if a quest has no requirements, it is a root. count descendants.
		// table "conditions" is required for a few special cases.
		int distinct = 0;
		int count = 0;
		for(auto itr = gQuests.begin(); itr != gQuests.end(); ++itr) {
			const Quest& q(itr->second);
			count++;
			fprintf(mFile, "n%i [label=\"%s\"];\n", q.id, q.title.c_str());
			if(!preVisit(q.id))
				continue;
			distinct++;
			process(q);
		}
		fprintf(mFile, "}\n");
		fclose(mFile);
		printf("Looked at %i distinct chains out of %i quests.\n", distinct, count);
		printf("Max visitCount: %i (quest=%i)\n", mMax.visitCount, mostPopular);
		printf("Max chainLength: %i (quest=%i)\n", mMax.childChainLength, longestChain);
	}
private:
	void parent(int cid, int oid) {
		if(oid == 0)
			return;
		if(oid < 0)
			oid *= -1;
		fprintf(mFile, "n%i -> n%i;\n", oid, cid);
	}
	void child(int cid, int oid) {
		if(oid == 0)
			return;
		if(oid < 0)
			oid *= -1;
		fprintf(mFile, "n%i -> n%i;\n", cid, oid);
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

void questsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gQuests.load();
	gFactions.load();

	getArguments(drd);

	mPair = new SimpleItrPair<Quests, QuestMaxRepTest>(gQuests.begin(), gQuests.end());

	Analyzer a;
	a.analyze();
}

void questsChtml::title(std::ostream& os) {
}

questsChtml::questsChtml() : PageContext("Quests"), mPair(NULL) {}

questsChtml::~questsChtml() {
	if(mPair)
		delete mPair;
}
