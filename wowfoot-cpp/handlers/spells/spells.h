#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcSpell.h"
#include "dbcSpellIcon.h"
#include "itrPair.h"

class spellsChtml : public PageContext {
public:
	spellsChtml();
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~spellsChtml();
private:
	ItrPair<Spell>* mPair;
};
