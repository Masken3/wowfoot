#ifndef TDB_H
#define TDB_H

#include "tdb_raw.h"
#include <assert.h>
#include <unordered_map>
#include <unordered_set>

using namespace std;

template<class T>
class TDB {
private:
	typedef unordered_map<int, T> Map;
	typedef unordered_set<T, T> Set;
	static void* tableFetchMap(int entry);
	static void tableFetchSet();
	static Map* sMap;
	static Set* sSet;
	static T sT;
public:
	static void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
		Map&);
	static void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
		Set&);
};

template<class T>
void TDB<T>::fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	Map& map)
{
	assert(sMap == NULL);

#if 0
	// sanity check
	size_t size = 0;
	for(size_t i=0; i<nCol; i++) {
		switch(cf[i].type) {
		case CDT_INT:
			size += sizeof(int);
			break;
		case CDT_STRING:
			size += sizeof(string);
			break;
		case CDT_FLOAT:
			size += sizeof(float);
			break;
		}
	}
	printf("%lu == %lu\n", size, sizeof(T));
	assert(size == sizeof(T));
#endif

	sMap = &map;
	::fetchTable(tableName, cf, nCol, tableFetchMap);
	sMap = NULL;
}

template<class T> std::unordered_map<int, T>* TDB<T>::sMap;

template<class T>
void* TDB<T>::tableFetchMap(int entry) {
	assert(sMap != NULL);
	pair<typename Map::iterator, bool> res = sMap->insert(pair<int, T>(entry, T()));
	assert(res.second);
	T* ptr = &res.first->second;	// assignment provides compile-time type safety.
	return ptr;
}


template<class T>
void TDB<T>::fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	Set& set)
{
	assert(sSet == NULL);
	sSet = &set;
	::fetchTable(tableName, cf, nCol, tableFetchSet, &sT);
	sSet = NULL;
}

template<class T> std::unordered_set<T, T>* TDB<T>::sSet;
template<class T> T TDB<T>::sT;

template<class T>
void TDB<T>::tableFetchSet() {
	assert(sSet != NULL);
	pair<typename Set::iterator, bool> res = sSet->insert(sT);
	assert(res.second);
}

#endif	//TDB_H
