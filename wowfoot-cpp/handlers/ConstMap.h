#ifndef CONST_MAP_H
#define CONST_MAP_H

#include "util/exception.h"
#include <unordered_map>
#include <stdexcept>
#include <assert.h>
using namespace std;

// Syntactic sugar for accessing a const unordered_map.
template<class Key, class Value>
class ConstMap : protected unordered_map<Key, Value> {
protected:
	typedef unordered_map<Key, Value> super;
	typedef pair<typename unordered_map<Key, Value>::iterator, bool> ires;
public:
	typedef const Value* ptr;
	typedef const Value& ref;
	typedef typename super::const_iterator citr;
	typedef Value value;
	typedef Key key;

	citr begin() const { return this->super::begin(); }
	citr end() const { return this->super::end(); }

	// returns NULL if key is not found.
	ptr find(const Key&) const;

	// throws an exception if key is not found.
	ref operator[](const Key&) const;
};

template<class Key, class Value>
typename ConstMap<Key, Value>::ptr
ConstMap<Key, Value>::find(const Key& key) const {
	EASSERT(!super::empty());
	citr itr = this->super::find(key);
	if(itr != this->super::end())
		return &itr->second;
	else
		return NULL;
}

template<class Key, class Value>
typename ConstMap<Key, Value>::ref
ConstMap<Key, Value>::operator[](const Key& key) const {
	EASSERT(!super::empty());
	citr itr = super::find(key);
	if(itr != super::end())
		return itr->second;
	else
		throw Exception("ConstMap[]");
}

#endif	//CONST_MAP_H
