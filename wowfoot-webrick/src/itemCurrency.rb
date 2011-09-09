require '../wowfoot-ex/output/ItemExtendedCost.rb'

# Hash of Hashes.
# itemId => { extendedCostId => true }
ITEM_CURRENCY = {}

ITEM_EXTENDED_COST.each do |cId, data|
	data[:item].each do |item|
		ITEM_CURRENCY[item.id] = {} if(!ITEM_CURRENCY[item.id])
		ITEM_CURRENCY[item.id][cId] = true
	end
end
