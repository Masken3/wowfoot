#include "lockStrings.h"

const char* lockSkillType(int lt) {
	switch(lt) {
	case LOCKTYPE_PICKLOCK: return "Lockpicking";
	case LOCKTYPE_HERBALISM: return "Herbalism";
	case LOCKTYPE_MINING: return "Mining";
	case LOCKTYPE_DISARM_TRAP: return "Disarm Trap";
	case LOCKTYPE_OPEN: return "Open";
	case LOCKTYPE_TREASURE: return "Treasure";
	case LOCKTYPE_CALCIFIED_ELVEN_GEMS: return "Calcified Elven Gems";
	case LOCKTYPE_CLOSE: return "Close";
	case LOCKTYPE_ARM_TRAP: return "Arm Trap";
	case LOCKTYPE_QUICK_OPEN: return "Quick Open";
	case LOCKTYPE_QUICK_CLOSE: return "Quick Close";
	case LOCKTYPE_OPEN_TINKERING: return "Open(Tinkering)";
	case LOCKTYPE_OPEN_KNEELING: return "Open(Kneeling)";
	case LOCKTYPE_OPEN_ATTACKING: return "Open(Attacking)";
	case LOCKTYPE_GAHZRIDIAN: return "Gahzridian";
	case LOCKTYPE_BLASTING: return "Blasting";
	case LOCKTYPE_SLOW_OPEN: return "Slow Open";
	case LOCKTYPE_SLOW_CLOSE: return "Slow Close";
	case LOCKTYPE_FISHING: return "Fishing";
	default: return 0;
	}
}
