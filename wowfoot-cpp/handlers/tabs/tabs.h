#ifndef TABS_H
#define TABS_H

#include "dllHelpers.h"
#include "chtmlBase.h"

#include <vector>

using namespace std;

class Tab : public ChtmlContext {
public:
	const char* id;
	const char* title;
	int count;
};

class VISIBLE tabsChtml : public ChtmlContext {
public:
	int run(ostream& stream);

	// deleted at the end of run().
	vector<Tab*> mTabs;
};

#endif	//TABS_H
