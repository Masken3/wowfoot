#include "money.h"

ostream& moneyHtml(ostream& stream, int total) {
	int copper = total % 100;
	total /= 100;
	int silver = total % 100;
	total /= 100;
	int gold = total;
	if(gold > 0)
		stream << gold<<"<img src=\"output/Interface/MoneyFrame/UI-GoldIcon.png\" alt=\"gold\">";
	if(gold > 0 && silver > 0)
		stream << ' ';
	if(silver > 0)
		stream << silver<<"<img src=\"output/Interface/MoneyFrame/UI-SilverIcon.png\" alt=\"silver\">";
	if(copper > 0 && silver > 0)
		stream << ' ';
	if(copper > 0)
		stream << copper<<"<img src=\"output/Interface/MoneyFrame/UI-CopperIcon.png\" alt=\"copper\">";
	return stream;
}
