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

	// call this before calling run().
	void addSpawns(Spawns::IdPair) VISIBLE;

	int run(ostream& stream) VISIBLE;
private:
	void prepare();

	struct Zone {
		const char* name;
		vector<Coord2D> coords;
	};
	// key is zone id
	unordered_map<int, Zone> mZones;
	size_t mSpawnCount;
	int mMainArea;
};

void spawnPointsPrepare() VISIBLE;
int zoneFromCoords(int map, float x, float y) VISIBLE;

#endif	//SPAWN_POINTS_H
