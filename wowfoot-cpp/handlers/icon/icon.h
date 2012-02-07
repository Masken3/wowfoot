#ifndef ICON_H
#define ICON_H

#include <string>
#include "dllHelpers.h"

using namespace std;

// returns path to the image file.
// this path should be suitable for pasting into HTML.
// \a name is name of MPQ file in "Interface\Icons\".
string getIcon(const char* name) VISIBLE;

// \a name is full path of MPQ file.
string getIconRaw(const char* name) VISIBLE;

#endif	//ICON_H
