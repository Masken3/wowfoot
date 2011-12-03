@singular = 'Area'
@plural = @dbcName = 'AreaTable'
@upperCase = 'AREA_TABLE'

@id = 0
@struct = [
m(:int, 'map', 1),
m(:int, 'parent', 2),
m(:int, 'level', 10),
m(:string, 'name', 11),	#encoding: utf-8
]

#@preRow = 'printf("number 0\n");'
#@midRow = 'printf("area %i: map %i, parent %i, \"%s\"\n", id, s.map, s.parent, s.name);'
