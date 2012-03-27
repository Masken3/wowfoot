#include "node.h"
#include "pageTags.h"

void LinebreakNode::print(std::ostream& o) const {
	o << "<br>\n";
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

void StaticTextNode::print(std::ostream& o) const {
	o << text;
}

void ColorNode::print(std::ostream& o) const {
	o << "<span class=\"";
	o.write(text, len);
	o << "\">";
}

void UrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	o.write(text, len);
	o << "\">";
}

void WowfootUrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	o.write(text, len);
	o << "\">";
}

void WowpediaUrlNode::print(std::ostream& o) const {
	o << "<a href=\"http://www.wowpedia.org/";
	o.write(text, len);
	o << "\">";
}

void TagNode::print(std::ostream& o) const {
	o << "<" << dst << ">";
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

#define INSTANTIATE_PAGENODE(name, map)\
	template class PageNode<typeof(map)>;

PAGE_TAGS(INSTANTIATE_PAGENODE);
