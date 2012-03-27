#ifndef VTREE_H
#define VTREE_H

#include <assert.h>
#include <vector>

// Virtual-container array
template<class Base, size_t MaxSize>
class varray {
private:
	typedef unsigned char byte;
	struct Container {
		byte a[MaxSize];
	};
	std::vector<Container> m;
	bool mAllocAllowed;
public:
	varray();
	//~varray();

	template<class T> T& add(const T&);
	template<class T> T& alloc();
	void clear();
	Base& operator[](size_t i);
	size_t size() const { return m.size(); }
};

template<class Base, size_t MaxSize>
varray<Base, MaxSize>::varray() {
	clear();
}

// the reference returned by this function is valid until clear() is called.
template<class Base, size_t MaxSize>
Base& varray<Base, MaxSize>::operator[](size_t i) {
	mAllocAllowed = false;
	return *(Base*)&m[i];
}

// the reference returned by this function is valid until clear() or alloc() is called.
template<class Base, size_t MaxSize> template<class T>
T& varray<Base, MaxSize>::alloc() {
	assert(mAllocAllowed);
	assert(sizeof(T) <= MaxSize);
	m.resize(m.size()+1);
	Base* b((T*)&m[m.size()-1]);	// Make sure that T is a subclass of Base.
	T* t = (T*)b;
	new (t) T;	// Invoke T's default constructor.
	return *t;
}

// the reference returned by this function is valid until clear() or alloc() is called.
template<class Base, size_t MaxSize> template<class T>
T& varray<Base, MaxSize>::add(const T& t) {
	assert(mAllocAllowed);
	assert(sizeof(T) <= MaxSize);
	m.resize(m.size()+1);
	Base* b((T*)&m[m.size()-1]);	// Make sure that T is a subclass of Base.
	T* tp = (T*)b;
	new (tp) T(t);	// Invoke T's copy constructor.
	return *tp;
}

template<class Base, size_t MaxSize>
void varray<Base, MaxSize>::clear() {
	mAllocAllowed = true;
	m.resize(0);
}

#endif	//VTREE_H
