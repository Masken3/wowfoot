#include "pageContext.h"
#include "chtmlUtil.h"
#include "db_quest.h"
#include "dbcFaction.h"
#include "itrPair.h"

class questsChtml : public PageContext {
public:
	questsChtml();
	void title(std::ostream&);
	int runPage(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~questsChtml();
private:
	ItrPair<Quest>* mPair;
};
