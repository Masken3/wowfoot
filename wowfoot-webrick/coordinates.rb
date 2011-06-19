require './coordinate-constants.rb'

def Compute(x, y, center_offset, size, center_val)
	x_offset = (x - center_offset)/size
	y_offset = (y - center_offset)/size
	
	x_val = x_offset + center_val + 0.5
	y_val = y_offset + center_val + 0.5
	return x_val.to_i, y_val.to_i
end

def ComputeGridPair(x, y)
	return Compute(x, y, CENTER_GRID_OFFSET, SIZE_OF_GRIDS, CENTER_GRID_ID)
end

def ComputeCellPair(x, y)
	cx, cy = Compute(x, y, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL, CENTER_GRID_CELL_ID)
	cx %= MAX_NUMBER_OF_CELLS
	cy %= MAX_NUMBER_OF_CELLS
	return cx, cy
end

def zoneFromCoords(map, x, y)
	percentages = {}
	# find areaId
	
	# compute grid and cell coordinates
	#gridX, gridY = ComputeGridPair(x, y)
	#cellX, cellY = ComputeCellPair(y, x)
	globalCellX = ((MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS/2) - (y/SIZE_OF_GRID_CELL)).to_i
	globalCellY = ((MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS/2) - (x/SIZE_OF_GRID_CELL)).to_i
	gridX = globalCellX / MAX_NUMBER_OF_CELLS
	gridY = globalCellY / MAX_NUMBER_OF_CELLS
	cellX = globalCellX % MAX_NUMBER_OF_CELLS
	cellY = globalCellY % MAX_NUMBER_OF_CELLS
	
	puts "Grid: [#{gridX}][#{gridY}]"
	puts "Cell: [#{cellX}][#{cellY}]"
	puts "Pixel: [#{gridX*MAX_NUMBER_OF_CELLS + cellX}][#{gridY*MAX_NUMBER_OF_CELLS + cellY}]"
	grid = AREA_MAP[map][gridY * MAX_NUMBER_OF_GRIDS + gridX]
	raise hell if(!grid)
	areaId = grid[cellY + cellX * MAX_NUMBER_OF_CELLS]
	raise hell if(!areaId)
	
	# find zoneId
	zoneId = areaId
	while(AREA_TABLE[zoneId][:parent] != 0)
		zoneId = AREA_TABLE[zoneId][:parent]
	end
	wma = WORLD_MAP_AREA[zoneId]
	area = AREA_TABLE[areaId]
	puts "Match: #{map}[#{x}, #{y}] = Zone #{zoneId} #{wma ? wma[:name] : nil}, Area #{areaId} #{area ? area[:name] : nil}"
	if(!(wma[:map] == map &&
		wma[:a][:x] >= x && wma[:a][:y] >= y &&
		wma[:b][:x] <= x && wma[:b][:y] <= y))
		raise hell
	end
	#puts "Match: #{map}[#{x}, #{y}] = Zone #{zoneId} #{wma[:name]}, Area #{areaId} #{AREA_TABLE[areaId][:name]}"
	width = wma[:a][:x] - wma[:b][:x]
	percentages[:x] = 1.0 - ((x - wma[:b][:x]) / width)
	height = wma[:a][:y] - wma[:b][:y]
	percentages[:y] = 1.0 - ((y - wma[:b][:y]) / height)
	#p width, height
	#p wma[:a][:y], wma[:b][:y]
	#p percentages
	return zoneId, percentages
end

require 'rubygems'
require 'RMagick'

# set up zone colors
COLORS = [
	'red',
	'green',
	'blue',
	'cyan',
	'yellow',
	'magenta',
	'brown',
	'purple',
]
BLACK = Magick::Pixel.from_color('black')
i = 0
AREA_TABLE.each do |id, hash|
	next unless(hash[:parent] == 0)
	hash[:color] = Magick::Pixel.from_color(COLORS[i])
	i += 1
	i = 0 if(i == COLORS.size)
end

# construct a square grid. save as png.
# I'm thinking 64x64 grids, 8x8 pixels per grid.
def dumpMapImage(map)
	grids = AREA_MAP[map]
	puts "dump: #{MAP[map]}"
	STDOUT.flush

	nPix = MAX_NUMBER_OF_GRIDS * MAX_NUMBER_OF_CELLS
	img = Magick::Image.new(nPix, nPix)
	draw = Magick::Draw.new
	text = Magick::Draw.new
	text.fill('white')
	y=0
	while(y<MAX_NUMBER_OF_GRIDS)
		x=0
		while(x<MAX_NUMBER_OF_GRIDS)
			grid = grids[y*MAX_NUMBER_OF_GRIDS + x]
			if(grid)
				#draw.fill('white')
				cy=0
				while(cy<MAX_NUMBER_OF_CELLS)
					cx=0
					while(cx<MAX_NUMBER_OF_CELLS)
						# set pixel color to cell color
						# 1. find cell color
						# each zone has a color
						# 1a. first find the zone id of the cell
						aid = grid[cx * MAX_NUMBER_OF_CELLS + cy]
						px = x*MAX_NUMBER_OF_CELLS + cx
						py = y*MAX_NUMBER_OF_CELLS + cy
						if(aid == 0 || !AREA_TABLE[aid])
							puts "#{aid}: #{x}x#{y}, #{cx}x#{cy}" if(aid != 0)
							color = BLACK
						else
						while(AREA_TABLE[aid][:parent] != 0)
							aid = AREA_TABLE[aid][:parent]
							#puts "#{aid}: #{x}x#{y}, #{cx}x#{cy}" if(!AREA_TABLE[aid])
						end
						area = AREA_TABLE[aid]
						# 1b. get the color
						color = area[:color]
						# annotate
						if(!area[:annotated])
							area[:annotated] = true
							text.text(px, py, area[:name])
						end
						end
						# 2. set the color
						img.pixel_color(px, py, color)
						cx += 1
					end
					cy += 1
				end
			else
				draw.fill('black')
				draw.rectangle(x*MAX_NUMBER_OF_CELLS, y*MAX_NUMBER_OF_CELLS,
					(x+1)*MAX_NUMBER_OF_CELLS - 1, (y+1)*MAX_NUMBER_OF_CELLS - 1)
			end
			x+=1
		end
		y+=1
	end
	draw.draw(img)
	text.draw(img)
	img.write("output/#{MAP[map]}.png")
end

if(false)
WORLD_MAP_CONTINENT.each do |id,hash|
	dumpMapImage(hash[:map])
end
puts "Experimental mode, exiting..."
exit(42)
end
