@dbcName = 'CharTitles'
@singular = 'Title'
@plural = 'Titles'
@upperCase = 'TITLES'

@id = 0
@struct = [
m(:int, 'id', 0),
m(:int, 'unknown', 1),
m(:string, 'name', 2),	# male
#m(:string, 'female', 3),	# same as 2 in all known instances.
m(:int, 'maskId', 4),
m(:int, 'cataclysm', 5),
]
