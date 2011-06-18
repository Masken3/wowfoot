require './coordinate-constants.rb'

class IO
	def readBlob(length)
		string = read(length)
		raise hell unless(string.length == length)
		return string
	end
	def readInt
		string = readBlob(4)
		res = 0
		i = 0
		while(i<4)
			res |= string[i] << i*8
			i += 1
		end
		return res
	end
end

NGRIDS = MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS
NCELLS = MAX_NUMBER_OF_CELLS*MAX_NUMBER_OF_CELLS

def loadAreaMap
	puts "loading AreaMap..."
	map = {}
	file = open('../wowfoot-ex/output/AreaMap.bin', 'rb')
	nMaps = file.readInt
	puts "#{nMaps} maps."
	while(!file.eof)
		#puts "Tell: #{file.tell}"
		mapId = file.readInt
		#puts "Loading map #{mapId}"
		#puts "Tell: #{file.tell}"
		raise hell if(MAP[mapId] == nil)
		raise hell unless(map[mapId] == nil)
		grids = Array.new(NGRIDS)
		map[mapId] = grids
		# read grid validity
		i = 0
		validGridCount = 0
		while(i < NGRIDS)
			blob = file.readbyte
			j = 0
			while(j < 8)
				if(blob & (1 << j) != 0)
					grids[i] = true
					validGridCount += 1
				end
				j += 1
				i += 1
			end
		end
		#puts "#{validGridCount} valid grids"
		#puts "Tell: #{file.tell}"
		# read cell data
		gridIndex = 0
		loadedGridCount = 0
		while(true)
			while(!grids[gridIndex])
				gridIndex += 1
				break if(gridIndex == NGRIDS)
			end
			break if(gridIndex == NGRIDS)
			cellIndex = 0
			grids[gridIndex] = file.readBlob(NCELLS * 4).unpack('L')
			loadedGridCount += 1
			gridIndex += 1
		end
		raise hell unless(gridIndex == NGRIDS)
		raise hell unless(loadedGridCount == validGridCount)
	end
	# nMaps is not actually valid, because of some WDTs that can't be loaded.
	#raise hell unless(map.size == nMaps)
	return map
end

AREA_MAP = loadAreaMap
