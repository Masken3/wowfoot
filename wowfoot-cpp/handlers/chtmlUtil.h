#include "dllHelpers.h"

#include <ostream>
#include <string>
using namespace std;

// throws an exception if the string cannot be fully converted.
int toInt(const char*) VISIBLE;

string toupper(string s) VISIBLE;

string htmlEscape(const string& src) VISIBLE;

#define VECTOR_EACH(etype, vector, element) \
for(size_t element##_i=0; element##_i<vector.size(); element##_i++) { etype& element(vector[element##_i]);
