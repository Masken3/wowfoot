#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "db_item.h"
#include "dbcItemSubClass.h"
#include "dbcItemClass.h"

class itemsChtml : public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
};
