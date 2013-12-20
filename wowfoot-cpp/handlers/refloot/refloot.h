#include "pageContext.h"
#include "tabs.h"

class reflootChtml : public tabsChtml, public PageContext {
public:
	reflootChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	bool mFound;
};
