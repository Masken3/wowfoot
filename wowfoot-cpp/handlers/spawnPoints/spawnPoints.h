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
	Spawns::IdPair mSpawns;
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

#endif	//SPAWN_POINTS_H
