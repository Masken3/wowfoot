#define __STDC_FORMAT_MACROS
#include "spell.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "db_gameobject_template.h"
#include "money.h"
#include "dbcSpell.h"
//#include "dbcSpellEffectNames.h"
#include "db_item.h"
#include "db_creature_template_spells.h"
#include "dbcSpellItemEnchantment.h"
#include "util/arraySize.h"

using namespace std;

static Tab* usedByItem(int id);
static Tab* usedByGameobject(int id);
static Tab* usedByCreature(int id);

enum TableRowId {
	NAME = ENTRY+1,
	SPAWN_COUNT = ENTRY+2,
};
#define MAX_COUNT 100

void spellChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gSpellItemEnchantments.load();
	gSpellIcons.load();
	gNpcs.load();
	gItems.load();
	gSpells.load();
	//gSpellEffectNames.load();
	gSpellMechanics.load();
	gSpellDurations.load();
	gSpellRanges.load();
	SkillLineAbilityIndex::load();
	gNpcSpells.load();

#if 0
	printf("SpellMechanics:\n");
	for(auto itr = gSpellMechanics.begin(); itr != gSpellMechanics.end(); ++itr) {
		printf("%i: %s\n", itr->first, itr->second.name);
	}
#endif

	int id = toInt(urlPart);
	a = gSpells.find(id);
	if(a) {
		mTitle = a->name;

		mTabs.push_back(usedByItem(id));
		mTabs.push_back(usedByGameobject(id));
		mTabs.push_back(usedByCreature(id));
		/*mTabs.push_back(taughtByNpc(id));
		mTabs.push_back(taughtByItem(id));*/

		mTabs.push_back(getComments("spell", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

static Tab* usedByItem(int id) {
	tabTableChtml& t(*new tabTableChtml);
	t.id = "item";
	t.title = "Used by item";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "item"));
	Items::IntPair p = gItems.findSpellId(id);
	for(; p.first != p.second && t.array.size() < MAX_COUNT;
		++p.first)
	{
		const Item& s(*p.first->second);
		Row r;
		r[ENTRY] = toString(s.entry);
		r[NAME] = s.name;
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* usedByGameobject(int id) {
	tabTableChtml& t(*new tabTableChtml);
	t.id = "object";
	t.title = "Used by object";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "object"));
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	auto p = gObjects.findSpell(id);
	for(; p.first != p.second && t.array.size() < MAX_COUNT;
		++p.first)
	{
		const Object& o(*p.first->second);
		Row r;
		r[ENTRY] = toString(o.entry);
		r[NAME] = o.name;
		r[SPAWN_COUNT] = toString(o.spawnCount);
		// todo: link "linkedTrapId", if any.
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* usedByCreature(int id) {
	tabTableChtml& t(*new tabTableChtml);
	t.id = "creature";
	t.title = "Used by NPC";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "npc"));
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	auto p = gNpcSpells.findSpell(id);
	for(; p.first != p.second && t.array.size() < MAX_COUNT;
		++p.first)
	{
		const Npc& o(gNpcs[p.first->second->entry]);
		Row r;
		r[ENTRY] = toString(o.entry);
		r[NAME] = o.name;
		r[SPAWN_COUNT] = toString(o.spawnCount);
		// todo: link "linkedTrapId", if any.
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

void spellChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

template<class T>
static void streamEscape(ostream& s, const T t) {
	s << t;
}

#if 0
//template<>
static void streamEscape(ostream& s, const string& t) {
	streamEscape(streamHtmlEscape, s, t);
}
#endif

//template<>
static void streamEscape(ostream& s, const char* t) {
	streamEscape(streamHtmlEscape, s, t);
}

void spellChtml::streamPlainMembers(ostream& stream) {
	stream << "<p>";
#define STREAM_PLAIN_MEMBER(name) stream << " " #name ": ";\
	streamEscape(stream, a->name);
	SPELL_PLAIN_MEMBERS(STREAM_PLAIN_MEMBER);
	stream << "</p>\n";
}

void spellChtml::streamPowerName(ostream& stream, int type) {
	switch(type) {
	case 0: stream << "Mana"; break;
	case 1: stream << "Rage"; break;
	case 2: stream << "Focus"; break;
	case 3: stream << "Energy"; break;
	case 4: stream << "Happiness"; break;
	case -2: stream << "Health"; break;
	default: stream << "Unknown power type ("<<type<<")";
	}
}

void spellChtml::streamEffects(ostream& stream) {
	bool hasEffect = false;
	for(int i=0; i<3; i++) {
		const Spell::Effect& e(a->effect[i]);
		if(e.id != 0) {
			if(!hasEffect) {
				hasEffect = true;
				stream << "<p>Effects:</p>\n";
			}
			stream << "<p>"<<i<<": ";
			const char* name = spellEffectName(e.id);
			const char* auraName = NULL;
			if(e.id == 6) {	// Apply Aura
				auraName = spellAuraName(e.applyAuraName);
			}
			if(name) {
				stream << "<span class=\"itemDescription\">" << name;
				if(auraName) {
					stream << " ("<<auraName<<")";
				}
				stream << "</span>: ";
			}
#define STREAM_EFFECT_MEMBER(m) stream << " " #m ": "<<e.m;
			SPELL_EFFECT_MEMBERS(STREAM_EFFECT_MEMBER);
			stream << "</p>\n";
			if(e.id == 53) {	// Enchant Item Permanent
				const auto& en = gSpellItemEnchantments[e.miscValue];
				stream << "<p>Enchantment:<br>\n";
				stream << "description: "<<en.description<<"<br>\n";
				stream << "auraId: "<<en.auraId<<"<br>\n";
				stream << "flags: "<<en.flags<<"<br>\n";
				for(size_t j=0; j<ARRAY_SIZE(en.effect); j++) {
					const auto& ef(en.effect[j]);
					stream << "effect "<<j<<": type "<<ef.type<<", amount "<<ef.amount<<", "
						" <a href=\"spell="<<ef.spellId<<"\">spell "<<ef.spellId<<"</a><br>\n";
				}
			}
		}
	}
	if(!hasEffect) {
		stream << "<p>No known effects.</p>\n";
	}
}

// the two Learning spells have different data.
//    uint32 spellToLearn = (m_spellInfo->Id == 483 || m_spellInfo->Id == 55884) ? damage : m_spellInfo->Effects[effIndex].TriggerSpell;
