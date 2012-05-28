#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"

class commentChtml : public tabsChtml, public PageContext {
public:
	commentChtml();
	void title(std::ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	int mId;
};
