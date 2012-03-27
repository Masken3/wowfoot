#ifndef NODE_H
#define NODE_H

#include <ostream>

#define _DECLARE_MEMBER(type, name) type name;

#define _DECLARE_ARGUMENT_F(type, name) type _##name
#define _DECLARE_ARGUMENT_S(type, name) ,type _##name
#define _INITIALIZE_MEMBER_F(type, name) name(_##name)
#define _INITIALIZE_MEMBER_S(type, name) ,name(_##name)

#define _DEFINE_CONSTRUCTOR(m, name)\
	name(m(_DECLARE_ARGUMENT_F, _DECLARE_ARGUMENT_S)) :\
	m(_INITIALIZE_MEMBER_F, _INITIALIZE_MEMBER_S) {}

#define _DECLARE_ALL(m, name)\
	m(_DECLARE_MEMBER, _DECLARE_MEMBER)\
	_DEFINE_CONSTRUCTOR(m, name)\
	void print(std::ostream&) const;\

class Node {
public:
	virtual void print(std::ostream&) const = 0;
};

class TagNode : public Node {
public:
#define _TAG_NODE(f, m)\
	f(const char*, tag)\
	m(size_t, len)\
	m(const char*, dst)\

_DECLARE_ALL(_TAG_NODE, TagNode)
};

class LinebreakNode : public Node {
	void print(std::ostream&) const;
};

#define _TEXT_LEN(f, m)\
	f(const char*, text)\
	m(size_t, len)\

class TextNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, TextNode)
};

class StaticTextNode : public Node {
public:
#define _STATIC_TEXT_NODE(f, m)\
	f(const char*,text)\

_DECLARE_ALL(_STATIC_TEXT_NODE, StaticTextNode)
};

class ColorNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, ColorNode)
};

class UrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, UrlNode)
};

class WowfootUrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, WowfootUrlNode)
};

class WowpediaUrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, WowpediaUrlNode)
};

template<class Map>
class PageNode : public Node {
public:
#define _PAGE_NODE(f, m)\
	f(Map&, map)\
	m(const char*, type)\
	m(const char*, idString)\
	m(size_t, idLen)\

_DECLARE_ALL(_PAGE_NODE, PageNode)
};

#endif	//NODE_H
