#define __STDC_FORMAT_MACROS
#include "spells.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void spellsChtml::httpArgument(const char* key, const char* value) {
	if(*value == 0)
		return;
	printf("p %s: %s\n", key, value);
}

struct SpellActiveIconTest {
	static bool have(const Spell& s) {
		return s.activeIconID != 0;
	}
};

#if 0
class SpellActiveIconItrPair : public ItrPair<Quest> {
private:
	mutable Spells::citr mItr;
	const Spells::citr mEnd;
public:
	SpellActiveIconItrPair(Spells::citr b, Spells::citr e) : mItr(b), mEnd(e) {}
	bool hasNext() const {
		while(mItr != mEnd) {
			if(mItr->second.activeIconID != 0)
				break;
			++mItr;
		}
		return mItr != mEnd;
	}
	const Spell& next() {
		const Spell& s(mItr->second);
		++mItr;
		return s;
	}
	virtual ~SpellActiveIconItrPair() {}
};
#endif

void spellsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gSpells.load();

	getArguments(drd);

	mPair = new SimpleItrPair<Spells, SpellActiveIconTest>(gSpells.begin(), gSpells.end());
}

void spellsChtml::title(ostream& stream) {
}

spellsChtml::spellsChtml() : PageContext("Spells"), mPair(NULL) {}

spellsChtml::~spellsChtml() {
	if(mPair)
		delete mPair;
}
