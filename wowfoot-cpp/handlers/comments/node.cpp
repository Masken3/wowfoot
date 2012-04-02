#include "node.h"
#include "pageTags.h"
#include <string.h>

void Node::dump(int level) const {
	//printf("%i (%p): %p, %p\n", _i, this, child, next);
	printf("%3i:", _i);
	if(level)
		printf("%*c", level, ' ');
	doDump();
}

void LinebreakNode::print(std::ostream& o) const {
	if(visible)
		o << "<br>";
	o << "\n";
}

void LinebreakNode::doDump() const {
	printf("Linebreak %svisible\n", visible ? "" : "in");
}

void TextNode::print(std::ostream& o) const {
	//o.write(text, len);
	for(size_t i=0; i<len; i++) {
		char c = text[i];
		if(c == '&') {
			o << "&amp;";
		} else if(c == '<') {
			o << "&lt;";
		} else if(c == '>') {
			o << "&gt;";
		} else {
			o << c;
		}
	}
}

void TextNode::doDump() const {
	printf("Text: '%.*s'\n", (int)len, text);
}

void StaticTextNode::print(std::ostream& o) const {
	o << text;
}

void StaticTextNode::doDump() const {
	printf("StaticText: '%s'\n", text);
}

void ColorNode::print(std::ostream& o) const {
	o << "<span class=\"";
	o.write(text, len);
	o << "\">";
}

void ColorNode::doDump() const {
	printf("Color: '%.*s'\n", (int)len, text);
}

void UrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	o.write(text, len);
	o << "\">";
}

void UrlNode::doDump() const {
	printf("Url: '%.*s'\n", (int)len, text);
}

void WowfootUrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	o.write(text, len);
	o << "\">";
}

void WowfootUrlNode::doDump() const {
	printf("WowfootUrl: '%.*s'\n", (int)len, text);
}

void WowpediaUrlNode::print(std::ostream& o) const {
	o << "<a href=\"http://www.wowpedia.org/";
	o.write(text, len);
	o << "\">";
}

void WowpediaUrlNode::doDump() const {
	printf("WowpediaUrl: '%.*s'\n", (int)len, text);
}

void TagNode::print(std::ostream& o) const {
	o << "<" << dst << ">";
}

void TagNode::doDump() const {
	printf("Tag: '%.*s'\n", (int)len, tag);
}

bool TagNode::isEndTagOf(const Node& n) const {
	printf("isEndTagOf '%s' (%s)\n", dst, n.endTag());
	if(!n.endTag())
		return false;
	return (strcmp(dst, n.endTag()) == 0);
}


template<class T> void streamName(ostream& o, const T& t) {
	o << t.name;
}
template<> void streamName<Quest>(ostream& o, const Quest& t) {
	o << t.title;
}

template<class Map>
void PageNode<Map>::print(std::ostream& o) const {
	o << "<a href=\""<<type;
	o.write(idString, idLen);
	o << "\">";
	char* end;
	errno = 0;
	int id = strtol(idString, &end, 10);
	const auto* s = map.find(id);
	if(id < 0 || errno != 0 || end != idString + idLen || !s) {
		// error parsing or invalid id.
		o << "["<<type;
		o.write(idString, idLen);
		o << "]";
	} else {
		streamName(o, *s);
	}
	o << "</a>";
}

template<class Map>
void PageNode<Map>::doDump() const {
	printf("Tag: '%.*s'\n", (int)idLen, idString);
}

#define INSTANTIATE_PAGENODE(name, map)\
	template class PageNode<typeof(map)>;

PAGE_TAGS(INSTANTIATE_PAGENODE);
