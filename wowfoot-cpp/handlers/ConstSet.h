#ifndef CONST_SET_H
#define CONST_SET_H

#include "util/exception.h"
#include <unordered_set>
#include <stdexcept>
#include <assert.h>
using namespace std;

// Syntactic sugar for accessing a const unordered_set.
template<class Key>
class ConstSet : protected unordered_set<Key, Key> {
protected:
	typedef unordered_set<Key, Key> super;
public:
	typedef const Key* ptr;
	typedef const Key& ref;
	typedef typename super::const_iterator citr;

	citr begin() const { return this->super::begin(); }
	citr end() const { return this->super::end(); }

	// returns NULL if key is not found.
	ptr find(const Key&) const;

	// throws an exception if key is not found.
	ref operator[](const Key&) const;
};

template<class Key>
typename ConstSet<Key>::ptr
ConstSet<Key>::find(const Key& key) const {
	ASSERTE(!super::empty());
	citr itr = this->super::find(key);
	if(itr != this->super::end())
		return &itr->second;
	else
		return NULL;
}

template<class Key>
typename ConstSet<Key>::ref
ConstSet<Key>::operator[](const Key& key) const {
	ASSERTE(!super::empty());
	citr itr = super::find(key);
	if(itr != super::end())
		return itr->second;
	else
		throw Exception("ConstSet[]");
}

#endif	//CONST_SET_H
