#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "TotemCategory.h"

class itemChtml : public tabsChtml {
public:
	int run(ostream& stream);
	const char* mTitle;
	const Item* a;
	float dps;

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
	static const char* ITEM_CLASS(int id);
	static const char* ITEM_SUBCLASS(int c, int sc);
	static const char* ITEM_EQUIP(int id);
	static const char* ITEM_BONDING(int id);
	static const char* ITEM_MATERIAL(int id);
	static const char* ITEM_DAMAGE_TYPE(int id);
	static const char* ITEM_STAT(int id);

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
