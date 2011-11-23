#include "dbcChrRaces.h"
#include "chrRaces.h"
#include "chtmlUtil.h"

#define DEBUG_DATA 0

string chrRaces(int mask) {
	if(mask == -1)
#if DEBUG_DATA
		return "(-1)";
#else
		return "";
#endif
	gChrRaces.load();
	ChrRaces::citr itr = gChrRaces.begin();
	string s;
	bool full = true;
	for(; itr != gChrRaces.end(); ++itr) {
		const ChrRace& c(itr->second);
		if(mask & (1 << (itr->first - 1))) {
			if(!s.empty())
				s += ", ";
			s += c.name;
		} else {
			full = false;
		}
	}
	if(s.empty())
		s = "Unknown race mask: " + toString(mask);
	else {
#define ALL_RACES 0x6ff
		if(full || (mask & ALL_RACES) == ALL_RACES)
			s.clear();
#if DEBUG_DATA
		s += " (" + toStringHex(mask) + ")";
#endif
	}
	return s;
}
