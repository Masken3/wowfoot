#include <string>
using namespace std;

#include "db_quest.struct.h"
#include "chtmlUtil.h"

template<> void streamName<Quest>(ostream& os, const Quest& t) {
	streamEscape(streamHtmlEscape, os, t.title);
}
