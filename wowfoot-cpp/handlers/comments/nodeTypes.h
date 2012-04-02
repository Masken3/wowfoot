#ifndef NODETYPES_H
#define NODETYPES_H

#define TAG_TYPES(m) \
	m(NO_TYPE, true, NULL)\
	m(LIST, true, NULL)\
	m(LIST_ITEM, true, NULL)\
	m(TABLE, true, NULL)\
	m(ANCHOR, false, "a")\
	m(BOLD, false, NULL)\
	m(ITALIC, false, NULL)\
	m(UNDERLINE, false, "span")\
	m(SMALL, false, "span")\
	m(COLOR, false, "span")\

enum TagType {
#define _TAG_TYPE_ENUM(name, allowMultiple, endTag) name,
	TAG_TYPES(_TAG_TYPE_ENUM)
	_TAG_TYPE_COUNT
};

#endif	//NODETYPES_H
