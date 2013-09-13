#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <math.h>
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

#define EACH(vec, i, m)  auto m(&vec[0]); for(size_t i=0; m = &vec[i], i<vec.size(); i++)

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
	virtual void questAreaObjective(const AreaTrigger& at) {
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

struct Location {
	int map;
	float x, y, z;
};

template<class T> Location makeLocation(const T& t) {
	Location l;
	l.map = t.map;
	l.x = t.x;
	l.y = t.y;
	l.z = t.z;
	return l;
}
static Location makeLocation(const Spawn& t) {
	Location l;
	l.map = t.map;
	l.x = t.position_x;
	l.y = t.position_y;
	l.z = t.position_z;
	return l;
}

struct QuestWithLocations : QuestPointListener {
	int id;
	const Quest& quest;
	vector<Location> givers, finishers;
	vector<vector<Location> > objectiveLocations;

	explicit QuestWithLocations(int _id) : id(_id), quest(gQuests[_id]) {
		getQuestLocations(quest, *this);
	}
	virtual void questGivers(Spawns::IdPair sp) {
		for(; sp.first != sp.second; ++sp.first) {
			const Spawn& s(*sp.first->second);
			givers.push_back(makeLocation(s));
		}
	}
	virtual void questFinishers(Spawns::IdPair sp) {
		for(; sp.first != sp.second; ++sp.first) {
			const Spawn& s(*sp.first->second);
			finishers.push_back(makeLocation(s));
		}
	}
	virtual void questObjectives(Spawns::IdPair sp) {
		vector<Location> o;
		for(; sp.first != sp.second; ++sp.first) {
			const Spawn& s(*sp.first->second);
			o.push_back(makeLocation(s));
		}
		objectiveLocations.push_back(o);
	}
	virtual void questAreaObjective(const AreaTrigger& at) {
		vector<Location> o;
		o.push_back(makeLocation(at));
		objectiveLocations.push_back(o);
	}
};

class QWLs : public ConstMap<int, QuestWithLocations> {
public:
	void load() {
		for(auto itr = gQuests.begin(); itr != gQuests.end(); ++itr) {
			const Quest& q(itr->second);
			insert(pair<int, QuestWithLocations>(q.id, QuestWithLocations(q.id)));
			//insert(*this, q.id, QuestWithLocations(q.id));
		}
	}
};

static QWLs sQWLs;

struct QuestObjective {
	bool done;
	vector<Location> locations;
};
struct QuestInProgress {
	explicit QuestInProgress(int _id);
	int id;
	vector<QuestObjective> objectives;
	bool allObjectivesDone();
};
// Available or Completed.
enum AC { Givers, Finishers };
struct QuestAC {
	explicit QuestAC(int _id, AC);
	int id;
	vector<Location> locations;
};

QuestAC::QuestAC(int _id, AC ac) : id(_id),
	locations(ac == Givers ? sQWLs[_id].givers : sQWLs[_id].finishers)
{
}
QuestInProgress::QuestInProgress(int _id) : id(_id) {
	EACH(sQWLs[_id].objectiveLocations, i, ol) {
		QuestObjective o = { false, *ol };
		objectives.push_back(o);
	}
}

bool QuestInProgress::allObjectivesDone() {
	EACH(objectives, i, o) {
		if(!o->done)
			return false;
	}
	return true;
}

static float dist(const Location& a, const Location& b) {
	if(a.map != b.map) {
		assert(false);
	}
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	float square = dx*dx + dy*dy + dz*dz;
	return sqrt(square);
}

static const Location sStartPoint = { 0, -6240, 331, 382 };
static const int sStartLevel = 1;
static const int sIgnoredZones[] = {
	12,	// elwynn
	141,	// teldrassil
	148,	// darkshore
};

class Router {
	// find optimal way to complete all quests, given:
	// a start point,
	// starting character level,
	// a list of zones to ignore.
	// For this example, we'll use a simple, greedy algo, level 1, and the Dun Morogh startpoint.
	bool findNearest(const vector<QuestAC>& quests, size_t& qi, size_t& li, float radius) const {
		float minDist = radius;
		EACH(quests, i, q) {
			EACH(q->locations, j, f) {
				float d = dist(mCurrentLocation, *f);
				if(d < minDist) {
					minDist = d;
					qi = i;
					li = j;
				}
			}
		}
		return minDist < radius;
	}
	bool findNearest(const vector<QuestInProgress>& qips, size_t& qi, size_t& oi, size_t& li, float radius) const {
		float minDist = radius;
		EACH(qips, i, q) {
			EACH(q->objectives, j, o) if(!o->done) {
				EACH(o->locations, k, f) {
					float d = dist(mCurrentLocation, *f);
					if(d < minDist) {
						minDist = d;
						qi = i;
						oi = j;
						li = k;
					}
				}
			}
		}
		return minDist < radius;
	}
	void acquireQuest(vector<QuestAC>& avail, size_t qi, vector<QuestInProgress>& qips, vector<QuestAC>& qc) {
		int id = avail[qi].id;
		const Quest& q(gQuests[id]);
		printf("Acquired %s (%i)\n", q.title.c_str(), id);
		mAcquiredCount++;
		avail.erase(avail.begin()+qi);
		if(q.objectives.empty()) {
			QuestAC c(id, Finishers);
			qc.push_back(c);
		} else {
			QuestInProgress qip(id);
			qips.push_back(qip);
		}
	}
	void finishQuest(vector<QuestAC>& qc, size_t qi) {
		int id = qc[qi].id;
		printf("Finished %s (%i)\n", gQuests[id].title.c_str(), id);
		qc.erase(qc.begin()+qi);
		mFinishedCount++;
	}
	void completeObjective(vector<QuestInProgress>& qips, size_t qi, size_t oi, vector<QuestAC>& qc) {
		QuestInProgress& qip(qips[qi]);
		qip.objectives[oi].done = true;
		printf("Completed objective %" PRIuPTR " of quest %s (%i)\n", oi, gQuests[qip.id].title.c_str(), qip.id);
		if(qip.allObjectivesDone()) {
			printf("Quest complete.\n");
			QuestAC c(qip.id, Finishers);
			qc.push_back(c);
			qips.erase(qips.begin()+qi);
		}
	}
public:
	Location mCurrentLocation;
	int mAcquiredCount, mFinishedCount;
	void printResults() {
		const Location& l(mCurrentLocation);
		printf("a %i, f %i, loc %i %.2f %.2f %.2f\n",
			mAcquiredCount, mFinishedCount,
			l.map, l.x, l.y, l.z);
	}
	void go(const Location& target) {
		printf("travel %.2f yards\n", dist(mCurrentLocation, target));
		mCurrentLocation = target;
	}
	void route() {
		// let's try the greedy algorithm:
		// can do any quests available to your faction, including class and profession.
		// finish any quests within 100 yards.
		// pick up any quests within 100 yards.
		// work on any quest whose objective is in 100 yards.
		// if no quests are found, double radius and try again.
		// stop once radius is greater than the size of the world.
		// consider portals to other maps.
		// radius in other map = baseRadius - distanceToPortal.
		// boats and zeppelins count as portals between continents.
		// should also consider server teleport-points.

		// for now, stop after finishing 10 quests or picking up 20.

		mCurrentLocation = sStartPoint;
		mAcquiredCount = mFinishedCount = 0;
		vector<QuestInProgress> questsInProgress;
		vector<QuestAC> questsCompleted, questsAvailable;

		printf("Calculating quest locations...\n");
		sQWLs.load();
		printf("Calculated %" PRIuPTR " quests...\n", sQWLs.size());

		printf("Setting up available quests...\n");
		for(auto itr = sQWLs.begin(); itr != sQWLs.end(); ++itr) {
			const QuestWithLocations& qwl(itr->second);
			bool hasOnlyEasternGivers = true;
			EACH(qwl.givers, i, g) {
				if(g->map != 0) {
					hasOnlyEasternGivers = false;
				}
			}
			if(hasOnlyEasternGivers) {
				QuestAC q(qwl.id, Givers);
				questsAvailable.push_back(q);
			}
		}
		printf("%" PRIuPTR " quests available.\n", questsAvailable.size());

newRound:
		if(mAcquiredCount >= 200 || mFinishedCount >= 100 || questsAvailable.empty())
			return;

		float radius = 100;
		while(1) {
			size_t qi, oi, li;
			if(findNearest(questsCompleted, qi, li, radius)) {
				go(questsCompleted[qi].locations[li]);
				finishQuest(questsCompleted, qi);
				goto newRound;
			}

			if(findNearest(questsAvailable, qi, li, radius)) {
				go(questsAvailable[qi].locations[li]);
				acquireQuest(questsAvailable, qi, questsInProgress, questsCompleted);
				goto newRound;
			}

			if(findNearest(questsInProgress, qi, oi, li, radius)) {
				go(questsInProgress[qi].objectives[oi].locations[li]);
				completeObjective(questsInProgress, qi, oi, questsCompleted);
				goto newRound;
			}
			radius *= 2;
		}
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

	Router r;
	printf("Router start...\n");
	r.route();
	printf("Router done.\n");
	r.printResults();

#if 0
	Analyzer a;
	printf("Analyzer start...\n");
	a.analyze();
	printf("Analyzer done.\n");
#endif
}
