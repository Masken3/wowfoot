#define __STDC_FORMAT_MACROS
#include "skills.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void skillsChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gSkillLines.load();
	gSkillLineCategories.load();
	gSpellIcons.load();
}

void skillsChtml::title(ostream& o) {
}

skillsChtml::~skillsChtml() {
}
