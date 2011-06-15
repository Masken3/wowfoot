
# Grids
SIZE_OF_GRIDS = 533.33333
CENTER_GRID_OFFSET = (SIZE_OF_GRIDS/2)
MAX_NUMBER_OF_GRIDS = 64
CENTER_GRID_ID = (MAX_NUMBER_OF_GRIDS/2)

# Cells
MAX_NUMBER_OF_CELLS = 8
SIZE_OF_GRID_CELL = (SIZE_OF_GRIDS/MAX_NUMBER_OF_CELLS)
CENTER_GRID_CELL_ID = (MAX_NUMBER_OF_CELLS*MAX_NUMBER_OF_GRIDS/2)
CENTER_GRID_CELL_OFFSET = (SIZE_OF_GRID_CELL/2)

def Compute(x, y, center_offset, size, center_val)
	x_offset = (x - center_offset)/size
	y_offset = (y - center_offset)/size
	
	x_val = x_offset + center_val + 0.5
	y_val = y_offset + center_val + 0.5
	return x_val, y_val
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
	gridX, gridY = ComputeGridPair(x, y)
	cellX, cellY = ComputeCellPair(x, y)
	grid = AREA_MAP[map][gridY][gridX]
	raise hell if(!grid)
	areaId = grid[cellY][cellX]
	raise hell if(!areaId)
	
	# find zoneId
	zoneId = areaId
	while(AREA_TABLE[zoneId][:parent] != 0)
		zoneId = AREA_TABLE[zoneId][:parent]
	end
	wma = WORLD_MAP_AREA[areaId]
	puts "Match: #{map}[#{x}, #{y}] = Zone #{zoneId} #{wma[:name]}, Area #{areaId} #{AREA_TABLE[areaId][:name]}"
	if(!(wma[:map] == map &&
		wma[:a][:x] >= x && wma[:a][:y] >= y &&
		wma[:b][:x] <= x && wma[:b][:y] <= y))
		STDOUT.flush
		raise hell
	end
	#puts "Match: #{map}[#{x}, #{y}] = Zone #{zoneId} #{wma[:name]}, Area #{areaId} #{AREA_TABLE[areaId][:name]}"
	width = wma[:a][:x] - wma[:b][:x]
	percentages[:x] = 1.0 - ((x - wma[:b][:x]) / width)
	height = hash[:a][:y] - hash[:b][:y]
	percentages[:y] = 1.0 - ((y - wma[:b][:y]) / height)
	return zoneId, percentages
end

require 'rubygems'
require 'RMagick'

# construct a square grid. save as png.
# I'm thinking 64x64 grids, 8x8 pixels per grid.
def dumpMapImage(map)
	grids = AREA_MAP[map]
	puts "dump: #{map}"
	STDOUT.flush
	nPix = MAX_NUMBER_OF_GRIDS * MAX_NUMBER_OF_CELLS
	img = Magick::Image.new(nPix, nPix)
	draw = Magick::Draw.new
	y=0
	while(y<MAX_NUMBER_OF_GRIDS)
		x=0
		while(x<MAX_NUMBER_OF_GRIDS)
			if(grids[y][x])
				draw.fill('white')
			else
				draw.fill('black')
			end
			draw.rectangle(x*MAX_NUMBER_OF_CELLS, y*MAX_NUMBER_OF_CELLS,
				(x+1)*MAX_NUMBER_OF_CELLS - 1, (y+1)*MAX_NUMBER_OF_CELLS - 1)
			x+=1
		end
		y+=1
	end
	draw.draw(img)
	img.write("output/#{map}.png")
end

WORLD_MAP_CONTINENT.each do |id,hash|
	dumpMapImage(hash[:map])
end
