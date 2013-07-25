@singular = 'ChrRace'
@plural = @dbcName = 'ChrRaces'
@upperCase = 'DBC_CHR_RACES'

if(CONFIG_WOW_VERSION < 20000)
	nameIdx = 17
else
	nameIdx = 14
end

@id = 0
@struct = [
m(:string, 'name', nameIdx),
]
