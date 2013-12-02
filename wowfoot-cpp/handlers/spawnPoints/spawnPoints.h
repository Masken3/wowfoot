#ifndef SPAWN_POINTS_H
#define SPAWN_POINTS_H

#include "dllHelpers.h"
#include "db_spawn.h"
#include "mapSize.h"
#include "coord2d.h"
#include <vector>
#include <unordered_map>

using namespace std;

class spawnPointsChtml : public mapSizeChtml {
public:
	spawnPointsChtml() VISIBLE;

	enum Type {
		eYellow,
		eRed,
		eBlue,
		eGreen,
	};

	// call this before calling run().
	// linkBase may be NULL.
	void addSpawns(Spawns::IntPair, Type t=eYellow, const char* linkBase=NULL) VISIBLE;

	void addSpawn(int map, float x, float y, Type t=eYellow, const char* linkBase=NULL, int id=0) VISIBLE;

	int run(ostream& stream) VISIBLE;
private:
	void prepare();
	const char* typeString(Type t);

	struct SpawnCoord {
		Coord2D c;
		Type t;
		const char* linkBase;
		int id;
	};

	struct Zone {
		const char* name;
		vector<SpawnCoord> coords;
	};
	// key is zone id
	unordered_map<int, Zone> mZones;
	size_t mSpawnCount;
	int mMainArea;
};

void spawnPointsPrepare() VISIBLE;
int zoneFromCoords(int map, float x, float y) VISIBLE;

#endif	//SPAWN_POINTS_H
