#ifndef STL_MAP_INSERT_H
#define STL_MAP_INSERT_H

template<class Map, class Key, class Value>
void
insert(Map& m, const Key& k, const Value& v) {
	m.insert(std::pair<Key, Value>(k, v));
}

#endif	//STL_MAP_INSERT_H
