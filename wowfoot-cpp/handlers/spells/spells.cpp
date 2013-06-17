#define __STDC_FORMAT_MACROS
#include "spells.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <dbcSkillLine.h>

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

class SkillLinePair : public ItrPair<Spell> {
private:
	SkillLineAbilityIndex::SpellPair mPair;
public:
	SkillLinePair(int skillId) {
		mPair = SkillLineAbilityIndex::findSkill(skillId);
	}
	virtual bool hasNext() {
		return mPair.first != mPair.second;
	}
	virtual const Spell& next() {
		const Spell& s(gSpells[mPair.first->second->spell]);
		++mPair.first;
		return s;
	}
	virtual ~SkillLinePair() {}
};

void spellsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gSpells.load();
	gSkillLines.load();
	gSpellIcons.load();
	SkillLineAbilityIndex::load();
	gItems.load();
	gItemDisplayInfos.load();

	printf("urlPart: %s\n", urlPart);
	getArguments(drd);

	if(urlPart[0] == 0) {
		printf("default: list spells with 2 icons.\n");
		mPair = new SimpleItrPair<Spells, SpellActiveIconTest>(gSpells.begin(), gSpells.end());
	} else {
		// otherwise: list category spells.
		// format: <category>.<skill>
		int categoryId, skillId;
		uint len;
		int res = sscanf(urlPart, "%i.%i%n", &categoryId, &skillId, &len);
		EASSERT(res == 2);
		printf("category: %i. skill: %i. len: %i\n", categoryId, skillId, len);
		EASSERT(len == strlen(urlPart));
		const SkillLine* sl = gSkillLines.find(skillId);
		if(!sl) {
			mTitle = urlPart;
			os << "Skill not found.\n";
			return;
		}
		if(sl->category != categoryId) {
			mTitle = urlPart;
			os << "Skill not found in category.\n";
			return;
		}
		mTitle = sl->name;
		mPair = new SkillLinePair(skillId);
	}
}

void spellsChtml::title(ostream& stream) {
	stream << mTitle;
}

spellsChtml::spellsChtml() : PageContext("Spells"), mPair(NULL) {}

spellsChtml::~spellsChtml() {
	if(mPair)
		delete mPair;
}

void spellsChtml::streamMultiItem(ostream& stream, int id, int count) {
	if(id != 0) {
		stream << "<a href=\"item="<<id<<"\">";
		const Item* item = gItems.find(id);
		if(item) {
			const ItemDisplayInfo* di = gItemDisplayInfos.find(item->displayId);
			if(di) {
				stream << "<img src=\"";
				ESCAPE_URL(getIcon(di->icon));
				stream << "\" alt=\""<<item->name<<"\">";
			} else {
				stream << "Warning: invalid display id ("<<item->displayId<<")";
			}
		} else {
			stream << "Warning: invalid item id ("<<id<<")";
		}
		stream << count;
		stream << "</a>\n";
	}
}

#if 0
<td style="padding: 0px;"><div style="width: 44px; margin: 0px auto;"><div style="float: left;" class="iconmedium"><ins style="background-image: url(&quot;http://wow.zamimg.com/images/wow/icons/medium/inv_fabric_linen_01.jpg&quot;);"></ins><del></del><a href="http://www.wowhead.com/item=2589"></a><span class="glow q1" style="position: absolute; right: 0px; bottom: 0px;"><div style="position: absolute; white-space: nowrap; left: -1px; top: -1px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: -1px; top: 0px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: -1px; top: 1px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: 0px; top: -1px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: 0px; top: 0px; z-index: 4;">2</div><div style="position: absolute; white-space: nowrap; left: 0px; top: 1px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: 1px; top: -1px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: 1px; top: 0px; color: black; z-index: 2;">2</div><div style="position: absolute; white-space: nowrap; left: 1px; top: 1px; color: black; z-index: 2;">2</div><span style="visibility: hidden;">2</span></span></div></div></td>
#endif
