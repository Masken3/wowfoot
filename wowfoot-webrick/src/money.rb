def moneyHtml(total)
	copper = total % 100
	total /= 100
	silver = total % 100
	total /= 100
	gold = total
	html = ''
	html << "#{gold}<img src=\"output/Interface/MoneyFrame/UI-GoldIcon.png\" alt=\"gold\">" if(gold > 0)
	html << ' ' if(gold > 0 && silver > 0)
	html << "#{silver}<img src=\"output/Interface/MoneyFrame/UI-SilverIcon.png\" alt=\"silver\">" if(silver > 0)
	html << ' ' if(copper > 0 && silver > 0)
	html << "#{copper}<img src=\"output/Interface/MoneyFrame/UI-CopperIcon.png\" alt=\"copper\">" if(copper > 0)
	return html
end
