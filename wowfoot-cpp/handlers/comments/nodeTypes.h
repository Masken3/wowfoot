#ifndef NODETYPES_H
#define NODETYPES_H

#define TAG_TYPES(m) \
	m(NO_TYPE, true, NULL)\
	m(LIST, true, NULL)\
	m(LIST_ITEM, true, NULL)\
	m(TABLE, true, NULL)\
	m(ANCHOR, false, "a")\
	m(COLOR, false, "span")\

#define FORMATTING_TYPES(m) \
	m(BOLD, "b", "bold")\
	m(ITALIC, "i", "italic")\
	m(UNDERLINED, "", "underlined")\
	m(SMALL, "", "small")\
	m(CODE, "", "code")\
	m(QUOTE, "", "quote")\

enum TagType {
#define _TAG_TYPE_ENUM(name, allowMultiple, endTag) name,
	TAG_TYPES(_TAG_TYPE_ENUM)
#define _FORMATTING_TAG_TYPE_ENUM(name, span, div) _##name,
	FORMATTING_TYPES(_FORMATTING_TAG_TYPE_ENUM)
	_TAG_TYPE_COUNT
};

enum FormattingType {
#define _FORMATTING_TYPE_ENUM(name, span, div) name,
	FORMATTING_TYPES(_FORMATTING_TYPE_ENUM)
	_FORMATTING_TYPE_COUNT
};

#endif	//NODETYPES_H
