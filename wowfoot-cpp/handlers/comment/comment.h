#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabTable.h"

class commentChtml : public tabsChtml, public PageContext {
public:
	int run(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	int mId;
};
