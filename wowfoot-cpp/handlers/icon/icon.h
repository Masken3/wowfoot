#ifndef ICON_H
#define ICON_H

#include <string>
#include "dllHelpers.h"

using namespace std;

// returns path to the image file.
// this path should be suitable for pasting into HTML.
string getIcon(const char* name) VISIBLE;

#endif	//ICON_H
