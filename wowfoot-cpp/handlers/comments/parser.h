#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include "nodeTypes.h"
#include "vtree.h"
#include <stack>

class Node;

#define REF(i) mArray[i]
#define VALID(i) (i >= 0)
#define INVALID (-1)

class NodeAdder {
public:
	typedef varray<Node, 96> Array;
	typedef int Ref;
protected:
	Ref mFirstNode;
	Ref mPreviousNode;
	Array mArray;
private:
	void add(const Node& n);
	Ref findStartTag(const Node& endTag);
public:
	void addLinebreakNode();

	// tag: pointer to tag in source data.
	// len: length of tag in source data.
	// tLen: length of basic tag (without attributes or whitespace)
	// dst: static string; HTML representation of tag (without <>, so it can be used with attributes).
	// end: static string; HTMP end tag. NULL if this is an end tag or a tag that has no end.
	void addTagNode(TagType type, const char* tag, size_t len, size_t tLen, const char* dst, const char* end);

	void addColorTag(const char* id, size_t len);
	void addWowfootUrlNode(const char* path, size_t len);
	void addWowpediaUrlNode(const char* path, size_t len);
	void addUrlNode(const char* url, size_t len);
	void addStaticTextNode(const char* text);
	void addTextNode(const char* text, size_t len);
	void addUrlEndNode();
	template<class Map> void addPageNode(Map&, const char* type, const char* id, size_t len);
};

class Parser : protected NodeAdder {
public:
	void parse(const char* src);
private:
	const char* mNodeStart;

	void parseTag(const char* tag, size_t len);
	void parseUrl(const char* url, size_t len);
	const char* parseUnescapedUrl(const char* ptr);
	void flush(const char* end);

	template<class Map>
	bool pageTag(const char* type, size_t typeLen, const char* tag, size_t tagLen,
		Map& map);
};

#endif	//PARSER_H