#ifndef ITR_PAIR_H
#define ITR_PAIR_H

template<class T> class ItrPair {
public:
	virtual bool hasNext() const = 0;
	virtual const T& next() = 0;
	virtual ~ItrPair() {}
};

template<class T, class Itr> class ItrPairImpl : public ItrPair<T> {
public:
	typedef std::pair<Itr, Itr> Pair;
	ItrPairImpl(Pair p) : m(p) {}
	virtual bool hasNext() const {
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

#endif	//ITR_PAIR_H
