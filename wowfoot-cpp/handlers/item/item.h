#include "wowVersion.h"
#include "db_quest.h"
#include "pageContext.h"
#include "tabTable.h"
#include "db_item.h"
#if (CONFIG_WOW_VERSION > 30000)
#include "dbcTotemCategory.h"
#define HAVE_EXTENDED_COST 1
#define HAVE_TOTEM_CATEGORY 1
#else
#define HAVE_EXTENDED_COST 0
#define HAVE_TOTEM_CATEGORY 0
#endif
#include "dbcSpell.h"
#include "chrClasses.h"
#include "chrRaces.h"
#include "dbcItemSet.h"
#include "dbcItemDisplayInfo.h"
#include "item_shared.h"
#include "icon.h"
#include "chtmlUtil.h"
#include "dbcSkillLine.h"
#include "util/arraySize.h"
#include "win32.h"

class itemChtml : public tabsChtml, public PageContext {
public:
	itemChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const Item* a;
	float mDps;

	struct Resistance {
		int Item::* value;
		const char* name;
	};
	static const Resistance mResistances[];
	static const int mnResistances;

	struct Quality {
		const char* color;
		const char* name;
	};

	static const Quality& ITEM_QUALITY(int id);
	static const char* ITEM_EQUIP(int id);
	static const char* ITEM_BONDING(int id);
	static const char* ITEM_MATERIAL(int id);
	static const char* ITEM_DAMAGE_TYPE(int id);
	static const char* ITEM_STAT(int id);
	static const char* SPELLTRIGGER(int id);

	struct Flag {
		int flag;
		const char* name;
	};
	static const Flag ITEM_FLAGS[];
	static const int nITEM_FLAGS;

	static const Flag ITEM_BAG_FAMILY[];
	static const int nITEM_BAG_FAMILY;
};

#define FOR(i, n) for(int i=0; i<n; i++)
