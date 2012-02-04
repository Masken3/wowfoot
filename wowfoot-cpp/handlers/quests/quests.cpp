#define __STDC_FORMAT_MACROS
#include "quests.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void questsChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
}

#if 0
class QuestItrMaxRep : public Quests::citr {
private:
	typedef Quests::citr super;
	super mEnd;
public:
	QuestItrMaxRep(super begin, super end) : super(begin), mEnd(end) {}
	QuestItrMaxRep& operator++() {
		do {
			super::operator++();
		} while((*this)->second.requiredMaxRepFaction != 0 && *this != mEnd);
		return *this;
	}
};
#endif

class QuestMaxRepItrPair : public ItrPair<Quest> {
private:
	mutable Quests::citr mItr;
	const Quests::citr mEnd;
public:
	QuestMaxRepItrPair(Quests::citr b, Quests::citr e) : mItr(b), mEnd(e) {}
	bool hasNext() const {
		if(mItr == mEnd)
			return false;
		while(mItr != mEnd) {
			if(mItr->second.requiredMaxRepFaction != 0)
				break;
			++mItr;
		}
		return mItr != mEnd;
	}
	const Quest& next() {
		const Quest& q(mItr->second);
		++mItr;
		return q;
	}
	virtual ~QuestMaxRepItrPair() {}
};

void questsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gQuests.load();
	gFactions.load();

	getArguments(drd);

	mPair = new QuestMaxRepItrPair(gQuests.begin(), gQuests.end());
#if 0
	Quests::citr b = gQuests.begin();
	Quests::citr e = gQuests.end();
	mPair = new ItrPairImpl<Quest, QuestItrMaxRep>(
		pair<QuestItrMaxRep, QuestItrMaxRep>(QuestItrMaxRep(b, e), QuestItrMaxRep(e, e)));
#endif
#if 0
	if(mItemClass >= 0) {
		gItems.load();
		if(mSubClass < 0) {
			mPair = new ItrPairImpl<Item, Items::ClassItr>(gItems.findClass(mItemClass));
		} else {
			mPair = new ItrPairImpl<Item, Items::ClassSubclassItr>(gItems.findClassSubclass(mItemClass, mSubClass));
		}
	} else if(mSubClass >= 0) {
		throw Exception("subClass without itemClass");
	}
#endif

#if 0
	if(mPair) {
		int count = 0;
		while(mPair->hasNext()) {
			const Item& i(mPair->next());
			printf("Item %i (%s)\n", i.entry, i.name.c_str());
			count++;
		}
		printf("count: %i\n", count);
		Items::citr itr = gItems.begin();
		while(count < 10) {
			const Item& i(itr->second);
			printf("Item %i class %i (%s)\n", i.entry, i.class_, i.name.c_str());
			count++;
			++itr;
		}
	}
#endif

	drd->code = run(os);
}

questsChtml::questsChtml() : mPair(NULL) {}

questsChtml::~questsChtml() {
	if(mPair)
		delete mPair;
}
