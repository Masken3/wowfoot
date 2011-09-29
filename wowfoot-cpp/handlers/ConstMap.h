#ifndef CONST_MAP_H
#define CONST_MAP_H

#include <unordered_map>
#include <stdexcept>
using namespace std;

// Syntactic sugar for accessing a const unordered_map.
template<class Key, class Value>
class ConstMap : protected unordered_map<Key, Value> {
protected:
	typedef unordered_map<Key, Value> super;
	typedef typename super::const_iterator citr;
public:
	typedef const Value* ptr;
	typedef const Value& ref;

	// returns NULL if key is not found.
	ptr find(const Key&) const;

	// throws an exception if key is not found.
	ref operator[](const Key&) const;
};

template<class Key, class Value>
typename ConstMap<Key, Value>::ptr
ConstMap<Key, Value>::find(const Key& key) const {
	citr itr = this->super::find(key);
	if(itr != this->super::end())
		return &itr->second;
	else
		return NULL;
}

template<class Key, class Value>
typename ConstMap<Key, Value>::ref
ConstMap<Key, Value>::operator[](const Key& key) const {
	citr itr = super::find(key);
	if(itr != super::end())
			return itr->second;
		else
			throw logic_error("ConstMap[]");
	}
#endif	//CONST_MAP_H
