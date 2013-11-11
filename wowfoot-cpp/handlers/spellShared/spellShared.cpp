#define __STDC_FORMAT_MACROS

#include "spellShared.h"
#include "tabTable.h"
#include "Spell.index.h"
#include <sstream>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "dbcSkillLine.h"
#include "db_npc_trainer.h"
#include "db_creature_template.h"
#include "skillShared.h"
#include "win32.h"
#include "tabs.h"
#include "db_item.h"
#include "item_shared.h"
#include "dbcSpellIcon.h"
#include "icon.h"
#include "util/arraySize.h"
#include "SkillLineAbility.index.h"
#include "dbcItemDisplayInfo.h"

Tab* spellsTab(const char* tabId, const char* tabTitle, ItrPair<Spell>& spells,
	bool haveSkillName)
{
	gNpcTrainers.load();
	gSpellIcons.load();
	gSkillLines.load();
	SkillLineAbilityIndex::load();

	tabTableChtml& t = *new tabTableChtml();
	t.id = tabId;
	t.title = tabTitle;
	t.columns.push_back(Column(ICON, "Icon/item", Column::NoEscape));
	t.columns.push_back(Column(NAME, "Name", Column::NoEscape));
	t.columns.push_back(Column(REAGENTS, "Reagents", Column::NoEscape));
	t.columns.push_back(Column(SOURCE, "Source", Column::NoEscape));
	t.columns.push_back(Column(SKILL, "Skill", Column::NoEscape));

	// it's expensive to construct this object. let's keep one instance.
	std::ostringstream stream;

	while(spells.hasNext()) {
		const Spell& s(spells.next());
		const Spell::Effect& e(s.effect[0]);
		const Quality* itemQuality = NULL;
		Row r;
		r[ENTRY] = toString(s.id);

		// ICON
		stream.str("");
		if(e.itemType) {
			itemQuality = streamMultiItem(stream, e.itemType, e.basePoints + e.dieSides);
		} else {
			const SpellIcon* si = gSpellIcons.find(s.spellIconID);
			if(si) {
				stream <<"<img src=\"";
				ESCAPE_URL(getIconRaw(si->icon));
				stream <<"\" alt=\"";
				ESCAPE(si->icon);
				stream <<"\">";
			} else if(s.spellIconID != 0) {
				stream << "Warning: invalid icon id ("<<s.spellIconID<<")";
			}
		}
		r[ICON] = stream.str();

		// NAME
		stream.str("");
		stream <<"<a href=\"spell="<<s.id<<"\">";
		if(itemQuality) {
			stream <<"<span style=\"color:#"<<itemQuality->color<<";\">";
		}
		ESCAPE(s.name);
		if(s.rank) if(s.rank[0]) {
			stream <<" ("<<s.rank<<")";
		}
		if(itemQuality) {
			stream <<"</span>";
		}
		stream <<"</a>";
		r[NAME] = stream.str();

		// REAGENTS
		stream.str("");
		for(size_t i=0; i<ARRAY_SIZE(s.reagent); i++) {
			const Spell::Reagent& re(s.reagent[i]);
			streamMultiItem(stream, re.id, re.count);
		}
		r[REAGENTS] = stream.str();

		// SOURCE
		stream.str("");
		int requiredSkillLevel = streamSource(stream, s.id);
		r[SOURCE] = stream.str();

		// SKILL
		stream.str("");
		auto slas = SkillLineAbilityIndex::findSpell(s.id);
		for(; slas.first != slas.second; ++slas.first) {
			const SkillLineAbility* sla = slas.first->second;
			stream << "<!--";
			if(haveSkillName) {
				stream << gSkillLines[sla->skill].name;
			}
			{
				char buf[32];
				sprintf(buf, "%04i", sla->minValue);
				stream <<buf<<"-->";
			}
			if(haveSkillName) {
				stream <<"<a href=\"spells="<<sla->skill<<"\">"<<gSkillLines[sla->skill].name<<"</a>\n";
			}
			if(requiredSkillLevel >= 0) {
				stream <<"<span class=\"skillRed\">"<<requiredSkillLevel<<"</span>\n";
			}
			stream <<"<span class=\"skillYellow\">"<<sla->minValue<<"</span>\n";
			stream <<"<span class=\"skillGreen\">"<<
				(sla->maxValue - (sla->maxValue - sla->minValue) / 2)<<"</span>\n";
			stream <<"<span class=\"skillGray\">"<<sla->maxValue<<"</span>\n";
		}
		r[SKILL] = stream.str();

		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

const Quality* streamMultiItem(ostream& stream, int id, int count) {
	const Quality* q = NULL;
	if(id != 0) {
		stream << "<a href=\"item="<<id<<"\" style=\"white-space:nowrap\">";
		const Item* item = gItems.find(id);
		if(item) {
			q = &ITEM_QUALITY(item->quality);
			stream << "<span style=\"color:#"<<q->color<<";\">";
			const ItemDisplayInfo* di = gItemDisplayInfos.find(item->displayId);
			if(di) {
				stream << "<img src=\"";
				ESCAPE_URL(getIcon(di->icon));
				stream << "\" alt=\""<<item->name<<"\" title=\""<<item->name<<"\">";
			} else {
				stream << "displayId ("<<item->displayId<<")";
			}
		} else {
			stream << "item ("<<id<<")";
		}
		stream << count;
		if(item)
			stream << "</span>";
		stream << "</a>\n";
	}
	return q;
}

static void streamItemSource(ostream& stream, const Item& item) {
	stream << "<a href=\"item="<<item.entry<<"\">";
	stream << "<span style=\"color:#"<<ITEM_QUALITY(item.quality).color<<";\">";
	streamName(stream, item);
	stream << "</span>";
	stream << "</a>\n";
	// todo: determine if the thing is sold or dropped, and if so, by how many.
}

// returns trainerCount.
static int streamNpcTrainers(ostream& stream, int id, int& requiredSkillLevel, bool pass) {
	int trainerCount = 0;
	for(auto p = SpellIndex::findLearnSpell(id); p.first != p.second; ++p.first) {
		for(auto t = gNpcTrainers.findSpell(p.first->second->id); t.first != t.second; ++t.first) {
			const NpcTrainer& nt(*t.first->second);
			if(nt.reqSkill) {
				requiredSkillLevel = nt.reqSkillValue;
			}
			if(!gNpcs.find(nt.entry)) {
				int count = 0;
				for(auto n = gNpcTrainers.findSpell(-nt.entry); n.first != n.second; ++n.first) {
					const NpcTrainer& nn(*n.first->second);
					if(pass) {
						if(trainerCount + count > 0)
							stream << ", ";
						NAMELINK(gNpcs, nn.entry);
					}
					count++;
				}
				EASSERT(count > 0);
				trainerCount += count;
			} else {
				if(pass) {
					if(trainerCount > 0)
						stream << ", ";
					NAMELINK(gNpcs, nt.entry);
				}
				trainerCount++;
			}
		}
	}
	return trainerCount;
}

int streamSource(ostream& stream, int id) {
	int requiredSkillLevel = -1;
	// Find spells that teach this spell, and items that use the teacher spell.
	for(auto p = SpellIndex::findLearnSpell(id); p.first != p.second; ++p.first) {
		const Spell& s(*p.first->second);
		for(auto r = gItems.findSpellId(s.id); r.first != r.second; ++r.first) {
			const Item& item(*r.first->second);
			streamItemSource(stream, item);
			if(item.requiredSkill) {
				requiredSkillLevel = item.requiredSkillRank;
			}
		}
	}
	// Find items that teach this spell directly (SPELLTRIGGER 6).
	for(auto r = gItems.findSpellId(id); r.first != r.second; ++r.first) {
		const Item& item(*r.first->second);
		for(uint i=0; i<ARRAY_SIZE(item.spellTrigger); i++) {
			if(item.spellTrigger[i] == 6) {
				streamItemSource(stream, item);
				if(item.requiredSkill) {
					requiredSkillLevel = item.requiredSkillRank;
				}
			}
		}
	}

	// todo: some spells are learned as part of starting the skill line.

	int trainerCount = streamNpcTrainers(stream, id, requiredSkillLevel, false);
	if(trainerCount > 0) {
		if(trainerCount < 5) {
			streamNpcTrainers(stream, id, requiredSkillLevel, true);
			stream << " ("<<trainerCount<<" trainers)\n";
		} else {
			stream << trainerCount<<" trainers\n";
		}
	}
	return requiredSkillLevel;
}

int slaYellow(int id) {
	auto slas = SkillLineAbilityIndex::findSpell(id);
	for(; slas.first != slas.second; ++slas.first) {
		const SkillLineAbility* sla = slas.first->second;
		return sla->minValue;
	}
	return -1;
}
