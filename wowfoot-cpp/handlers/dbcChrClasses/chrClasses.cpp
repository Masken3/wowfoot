#include "dbcChrClasses.h"
#include "chrClasses.h"
#include "chtmlUtil.h"

#define DEBUG_DATA 0

string chrClasses(int mask) {
	if(mask == -1)
#if DEBUG_DATA
		return "(-1)";
#else
		return "";
#endif
	gChrClasses.load();
	ChrClasses::citr itr = gChrClasses.begin();
	string s;
	bool full = true;
	for(; itr != gChrClasses.end(); ++itr) {
		const ChrClass& c(itr->second);
		if(mask & (1 << (itr->first - 1))) {
			if(!s.empty())
				s += ", ";
			s += c.name;
		} else {
			full = false;
		}
	}
	if(s.empty())
		s = "Unknown class mask: " + toString(mask);
	else {
		if(full)
			s.clear();
#if DEBUG_DATA
		s += " (" + toStringHex(mask) + ")";
#endif
	}
	return s;
}
