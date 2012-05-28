#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "mapSize.h"

class zoneChtml : public PageContext {
public:
	zoneChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* urlPart;
	string mTitle;
};
