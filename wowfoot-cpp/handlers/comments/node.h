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
	void doDump() const;\

class Node {
public:
	virtual void print(std::ostream&) const = 0;
	virtual bool isTag() const = 0;
	virtual bool isEndTagOf(const Node&) const { return false; }
	// returns the value of the tag required to close this node, if it is a start tag.
	// otherwise, returns NULL.
	virtual const char* endTag() const { return NULL; }
	void dump(int level) const;
	Node* next;
	Node* child;
	int _i;
	Node() : next((Node*)-1), child((Node*)-1), _i(-1) {}
protected:
	virtual void doDump() const = 0;
};

class TagNode : public Node {
public:
#define _TAG_NODE(f, m)\
	f(const char*, tag)\
	m(size_t, len)\
	m(const char*, dst)\
	m(const char*, end)\

_DECLARE_ALL(_TAG_NODE, TagNode)
	bool isTag() const { return true; }
	bool isEndTagOf(const Node&) const;
	const char* endTag() const { return end; }
};

class LinebreakNode : public Node {
	void print(std::ostream&) const;
	void doDump() const;
	bool isTag() const { return false; }
};

#define _TEXT_LEN(f, m)\
	f(const char*, text)\
	m(size_t, len)\

class TextNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, TextNode)
	bool isTag() const { return false; }
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
};

class WowfootUrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, WowfootUrlNode)
	bool isTag() const { return true; }
	const char* endTag() const { return "/a"; }
};

class WowpediaUrlNode : public Node {
public:
_DECLARE_ALL(_TEXT_LEN, WowpediaUrlNode)
	bool isTag() const { return true; }
	const char* endTag() const { return "/a"; }
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
};

#endif	//NODE_H
