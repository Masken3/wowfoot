#ifndef NODE_H
#define NODE_H

#include <ostream>
#include "nodeTypes.h"

#define _DECLARE_MEMBER(type, name) type name;

#define _DECLARE_ARGUMENT_F(type, name) type _##name
#define _DECLARE_ARGUMENT_S(type, name) ,type _##name
#define _INITIALIZE_MEMBER_F(type, name) name(_##name)
#define _INITIALIZE_MEMBER_S(type, name) ,name(_##name)
#define _MEMBER_ARG_F(type, name) _##name
#define _MEMBER_ARG_S(type, name) ,_##name

#define _DEFINE_CONSTRUCTOR(m, name)\
	name(m(_DECLARE_ARGUMENT_F, _DECLARE_ARGUMENT_S)) :\
	m(_INITIALIZE_MEMBER_F, _INITIALIZE_MEMBER_S) {}

#define _DEFINE_INHERIT_CONSTRUCTOR(m, name, base)\
	name(m(_DECLARE_ARGUMENT_F, _DECLARE_ARGUMENT_S)) :\
	base(m(_MEMBER_ARG_F, _MEMBER_ARG_S)) {}

#define _DECLARE_FUNCTIONS(m, name)\
	void print(std::ostream&) const;\
	void doDump() const;\

#define _DECLARE_ALL(m, name)\
	m(_DECLARE_MEMBER, _DECLARE_MEMBER)\
	_DEFINE_CONSTRUCTOR(m, name)\
	_DECLARE_FUNCTIONS(m, name)\

#define _DECLARE_INHERIT(m, name, base)\
	_DEFINE_INHERIT_CONSTRUCTOR(m, name, base)\
	_DECLARE_FUNCTIONS(m, name)\

class Node {
public:
	virtual void print(std::ostream&) const = 0;
	virtual bool isTag() const = 0;
	virtual TagType tagType() const { return NO_TYPE; }
	virtual bool isEndTagOf(const Node&) const { return false; }
	// returns the value of the tag required to close this node, if it is a start tag.
	// otherwise, returns NULL.
	virtual const char* endTag() const { return NULL; }
	virtual bool hasUrl() const { return false; }
	virtual bool isLinebreak() const { return false; }
	virtual bool isSpace() const { return false; }
	bool isEndTag() const { return isTag() && !endTag(); }
	void dump(int level) const;
	int next;
	int child;
	int _i;	// for debugging. todo: add #if.
	Node() : next(-2), child(-2), _i(-1) {}
protected:
	virtual void doDump() const = 0;
};

class TagNode : public Node {
public:
#define _TAG_NODE(f, m)\
	f(const char*, tag)\
	m(size_t, len)\
	m(TagType, type)\
	m(const char*, dst)\
	m(const char*, end)\

_DECLARE_ALL(_TAG_NODE, TagNode)
	bool isTag() const { return true; }
	TagType tagType() const { return type; }
	bool isEndTagOf(const Node&) const;
	const char* endTag() const { return end; }
};

class LinebreakNode : public Node {
public:
#define _LINEBREAK_NODE(f, m)\
	f(bool, visible)\

_DECLARE_ALL(_LINEBREAK_NODE, LinebreakNode)
	bool isTag() const { return false; }
	bool isLinebreak() const { return true; }
};

#define _TEXT_LEN(f, m)\
	f(const char*, text)\
	m(size_t, len)\

class TextNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, TextNode)
	bool isTag() const { return false; }
	bool isSpace() const;
};

class StaticTextNode : public Node {
public:
#define _STATIC_TEXT_NODE(f, m)\
	f(const char*,text)\

_DECLARE_ALL(_STATIC_TEXT_NODE, StaticTextNode)
	bool isTag() const { return false; }
};

class ColorNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, ColorNode)
	bool isTag() const { return true; }
	const char* endTag() const { return "/span"; }
};

class UrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, UrlNode)
	bool isTag() const { return true; }
	const char* endTag() const { return "/a"; }
	bool hasUrl() const { return true; }
	virtual TagType tagType() const { return ANCHOR; }
};

class WowfootUrlNode : public UrlNode {
public:
_DECLARE_INHERIT(_TEXT_LEN, WowfootUrlNode, UrlNode)
};

class WowpediaUrlNode : public UrlNode {
public:
_DECLARE_INHERIT(_TEXT_LEN, WowpediaUrlNode, UrlNode)
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
	bool isTag() const { return false; }
	bool hasUrl() const { return true; }
};

#endif	//NODE_H
