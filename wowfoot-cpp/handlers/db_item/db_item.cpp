#include "db_item.h"
#include "tdb.h"
#include "db_item.format.h"

static bool sLoaded = false;

void Items::load() {
	if(sLoaded)
		return;
	TDB<Item>::fetchTable("item_template", sItemFormats, sizeof(sItemFormats) / sizeof(ColumnFormat), (super&)*this);
	sLoaded = true;
}

Items gItems;
