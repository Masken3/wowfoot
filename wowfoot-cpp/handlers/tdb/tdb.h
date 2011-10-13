#ifndef TDB_H
#define TDB_H

#include "tdb_raw.h"
#include <assert.h>

template<class T>
class TDB {
private:
	typedef std::unordered_map<int, T> Map;
	static void* tableFetchCallback(int entry);
	static Map* sMap;
public:
	static void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
		Map&);
};

template<class T>
void TDB<T>::fetchTable(const char* tableName, const ColumnFormat* cf, size_t nCol,
	Map& map)
{
	assert(sMap == NULL);

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

	sMap = &map;
	::fetchTable(tableName, cf, nCol, tableFetchCallback);
	sMap = NULL;
}

template<class T> std::unordered_map<int, T>* TDB<T>::sMap;

template<class T>
void* TDB<T>::tableFetchCallback(int entry) {
	assert(sMap != NULL);
	pair<typename Map::iterator, bool> res = sMap->insert(pair<int, T>(entry, T()));
	assert(res.second);
	T* ptr = &res.first->second;	// assignment provides compile-time type safety.
	return ptr;
}

#endif	//TDB_H
