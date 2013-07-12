@singular = @dbcName = 'Lock'
@plural = 'Locks'
@upperCase = 'LOCKS'

@id = 0
@struct = [
m(:int, 'id', 0),
as('Entry', 'e', [m(:int, 'type', 1), m(:int, 'index', 9), m(:int, 'skill', 17)], 8),
]
