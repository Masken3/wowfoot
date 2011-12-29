#ifndef ITEM_SHARED_H
#define ITEM_SHARED_H

#include "db_item.h"
#include <ostream>

void streamAllCostHtml(std::ostream&, const Item&) VISIBLE;
void streamItemClassHtml(std::ostream&, const Item&) VISIBLE;

#endif	//ITEM_SHARED_H
