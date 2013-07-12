#include "skillShared.h"
#include "lockEnums.h"
#include <string.h>

// returns NULL on parse error.
const SkillLine* parseSkillId(ostream& os, const char* urlPart) {
	// format: <category>.<skill>
	int categoryId, skillId;
	uint len;
	int res = sscanf(urlPart, "%i.%i%n", &categoryId, &skillId, &len);
	EASSERT(res == 2);
	//printf("category: %i. skill: %i. len: %i\n", categoryId, skillId, len);
	EASSERT(len == strlen(urlPart));
	const SkillLine* sl = gSkillLines.find(skillId);
	if(!sl) {
		os << "Skill not found.\n";
		return NULL;
	}
	if(sl->category != categoryId) {
		os << "Skill not found in category.\n";
		return NULL;
	}
	return sl;
}

// returns 0 if no lock type.
int lockTypeFromSkill(int skillId) {
	switch(skillId) {
	case 633: return LOCKTYPE_PICKLOCK;
	case 182: return LOCKTYPE_HERBALISM;
	case 186: return LOCKTYPE_MINING;
	case 356: return LOCKTYPE_FISHING;
	default: return 0;
	//default: FAIL("skill is not associated with locks.");
	}
}
