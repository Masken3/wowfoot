#define __STDC_FORMAT_MACROS
#include "node.h"
#include "pageTags.h"
#include "chtmlUtil.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

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

void LinebreakNode::printEndTag(std::ostream& o) const {
}

void LinebreakNode::doDump() const {
	printf("Linebreak %svisible\n", visible ? "" : "in");
}

void TextNode::print(std::ostream& o) const {
	streamEscape(streamHtmlEscape, o, text, len);
}

void TextNode::printEndTag(std::ostream& o) const {
}

void TextNode::doDump() const {
	printf("Text: '%.*s'\n", (int)len, text);
}

bool TextNode::isSpace() const {
	for(size_t i=0; i<len; i++) {
		if(!isspace(text[i]))
			return false;
	}
	return true;
}

void StaticTextNode::print(std::ostream& o) const {
	o << text;
}

void StaticTextNode::printEndTag(std::ostream& o) const {
}

void StaticTextNode::doDump() const {
	printf("StaticText: '%s'\n", text);
}

void ListItemNode::print(std::ostream& o) const {
	o << "<li";
	if(styleNone) {
		o << " style=\"list-style: none\">";
	} else {
		o << " value=\""<<value<<"\">";
	}
}

void ListItemNode::printEndTag(std::ostream& o) const {
	o << "</li>";
}

void ListItemNode::doDump() const {
	printf("ListItemNode: ");
	if(styleNone) {
		printf("styleNone\n");
	} else {
		printf("%i\n", value);
	}
}

bool ListItemNode::hasEndTag(const char* e) const {
	return strcmp(e, "/li") == 0;
}

struct FormattingInfo {
	const char* span;
	const char* spanEnd;
	const char* div;
	TagType tagType;
};

static const FormattingInfo sFormattingInfos[] = {
#define _FORMATTING_INFO(name, span, div) {(*span ? span : NULL), (*span ? "/" span : NULL), div, _##name},
	FORMATTING_TYPES(_FORMATTING_INFO)
};

TagType FormattingNode::tagType() const {
	const FormattingInfo& fi(sFormattingInfos[type]);
	return fi.tagType;
}

void FormattingNode::print(std::ostream& o) const {
	const FormattingInfo& fi(sFormattingInfos[type]);
	if(div) {
		o << "<div class=\"" << fi.div << "\">";
	} else if(fi.span) {
		o << "<" << fi.span << ">";
	} else {
		o << "<span class=\"" << fi.div << "\">";
	}
}

void FormattingNode::printEndTag(std::ostream& o) const {
	const FormattingInfo& fi(sFormattingInfos[type]);
	if(div) {
		o << "</div>";
	} else if(fi.span) {
		o << "</" << fi.span << ">";
	} else {
		o << "</span>";
	}
}

void FormattingNode::doDump() const {
	const FormattingInfo& fi(sFormattingInfos[type]);
	printf("Format: %s%s\n", fi.div, div ? " div" : "");
}

bool FormattingNode::hasEndTag(const char* e) const {
	if(*e == '/')
		e++;
	else
		return false;
	//printf("hasEndTag(%s, %s, %" PRIuPTR ")\n", tag, e, tLen);
	if(strncmp(tag, e, tLen) != 0)
		return false;
	return e[tLen] == 0;
}

bool FormattingNode::isStructureTag() const {
	return div;
}

void ColorNode::print(std::ostream& o) const {
	if(div)
		o << "<div class=\"";
	else
		o << "<span class=\"";
	o.write(text, len);
	o << "\">";
}

void ColorNode::printEndTag(std::ostream& o) const {
	if(div)
		o << "</div>";
	else
		o << "</span>";
}

void ColorNode::doDump() const {
	printf("Color: '%.*s'%s\n", (int)len, text, div ? " div" : "");
}

bool ColorNode::hasEndTag(const char* e) const {
	return strcmp(e, "/color") == 0;
}

void UrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	streamEscape(streamUrlEscape, o, text, len);
	o << "\">";
}

void UrlNode::printEndTag(std::ostream& o) const {
	o << "</a>";
}

void UrlNode::doDump() const {
	printf("Url: '%.*s'\n", (int)len, text);
}

bool UrlNode::hasEndTag(const char* e) const {
	return strcmp(e, "/url") == 0;
}

void WowfootUrlNode::print(std::ostream& o) const {
	o << "<a href=\"";
	streamEscape(streamUrlEscape, o, text, len);
	o << "\">";
}

void WowfootUrlNode::doDump() const {
	printf("WowfootUrl: '%.*s'\n", (int)len, text);
}

void WowpediaUrlNode::print(std::ostream& o) const {
	o << "<a href=\"http://www.wowpedia.org/";
	streamEscape(streamUrlEscape, o, text, len);
	o << "\">";
}

void WowpediaUrlNode::doDump() const {
	printf("WowpediaUrl: '%.*s'\n", (int)len, text);
}

void TagNode::print(std::ostream& o) const {
	o << "<" << dst;
	if(type == TABLE)
		o << " summary=\"table\"";
	o << ">";
}

void TagNode::printEndTag(std::ostream& o) const {
	o << "<" << end << ">";
}

void TagNode::doDump() const {
	printf("Tag: '%.*s'\n", (int)len, tag);
}

bool TagNode::hasEndTag(const char* e) const {
	if(*e == '/')
		e++;
	else
		return false;
	if(strncmp(tag, e, tLen) != 0)
		return false;
	return e[tLen] == 0;
}

bool TagNode::isStructureTag() const {
	return type == LIST || type == TABLE;
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
void PageNode<Map>::printEndTag(std::ostream& o) const {
}

template<class Map>
void PageNode<Map>::doDump() const {
	printf("Tag: '%.*s'\n", (int)idLen, idString);
}

#define INSTANTIATE_PAGENODE(name, map)\
	template class PageNode<typeof(map)>;

PAGE_TAGS(INSTANTIATE_PAGENODE);
