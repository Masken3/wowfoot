#ifndef VTREE_H
#define VTREE_H

#include <assert.h>
#include <vector>
#include <string.h>
#include "util/exception.h"

// Virtual-container array
template<class Base, size_t MaxSize>
class varray {
protected:
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

// the reference returned by this function is valid until clear(), add() or alloc() is called.
template<class Base, size_t MaxSize>
Base& varray<Base, MaxSize>::operator[](size_t i) {
	//mAllocAllowed = false;
	EASSERT(i < m.size());
	return *(Base*)&m[i];
}

// the reference returned by this function is valid until clear(), add() or alloc() is called.
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

#if 0
#define my_static_assert(a) \
	static char const static_assertion[(a) ? 1 : -1] = {'!'}
#else
#define my_static_assert(a) static_assert(a, #a)
#endif

// the reference returned by this function is valid until clear() or alloc() is called.
template<class Base, size_t MaxSize> template<class T>
T& varray<Base, MaxSize>::add(const T& t) {
	assert(mAllocAllowed);
	my_static_assert(sizeof(T) <= MaxSize);
	m.resize(m.size()+1);
	Base* b((T*)&m[m.size()-1]);	// Make sure that T is a subclass of Base.
	T* tp = (T*)b;
	//new (tp) T(t);	// Invoke T's copy constructor.
	// copy T data. warning: this will cause non-POD objects to become unstable and cause crashes.
	memcpy(tp, &t, MaxSize);
	return *tp;
}

template<class Base, size_t MaxSize>
void varray<Base, MaxSize>::clear() {
	mAllocAllowed = true;
	m.resize(0);
}

template<class Base, size_t MaxSize>
class vvector : public varray<Base, MaxSize> {
private:
public:
	class vref {
	private:
		int index;
		vvector& v;
	public:
		bool isValid() const { return index >= 0; }
		Base* operator ->() const { assert(isValid); return &v[index]; }
		vref(vvector& _v, int i) : v(_v), index(i) {}

		vref& operator=(const vref& o) {
			assert(&v == &o.v);
			index = o.index;
		}
	};

	const vref Null;

	vref operator[](size_t i) { return vref(*this, i); }

	vvector() : Null(*this, -2) {}
};

#endif	//VTREE_H
