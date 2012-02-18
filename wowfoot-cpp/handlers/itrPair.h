#ifndef ITR_PAIR_H
#define ITR_PAIR_H

template<class T> class ItrPair {
public:
	virtual bool hasNext() = 0;
	virtual const T& next() = 0;
	virtual ~ItrPair() {}
};

template<class T, class Itr> class ItrPairImpl : public ItrPair<T> {
public:
	typedef std::pair<Itr, Itr> Pair;
	ItrPairImpl(Pair p) : m(p) {}
	virtual bool hasNext() {
		return m.first != m.second;
	}
	const T& next() {
		const T& t(*m.first->second);
		++m.first;
		return t;
	}
	virtual ~ItrPairImpl() {}
private:
	Pair m;
};

template<class Map, class Test>
class SimpleItrPair : public ItrPair<typename Map::valueType> {
public:
	typedef typename Map::citr citr;
	typedef typename Map::valueType value;
private:
	citr mItr;
	const citr mEnd;
public:
	SimpleItrPair(citr b, citr e) : mItr(b), mEnd(e) {}
	bool hasNext() {
		while(mItr != mEnd) {
			if(Test::have(mItr->second))
				break;
			++mItr;
		}
		return mItr != mEnd;
	}
	const value& next() {
		const value& s(mItr->second);
		++mItr;
		return s;
	}
	virtual ~SimpleItrPair() {}
};

#endif	//ITR_PAIR_H
