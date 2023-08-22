#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <variant>

using namespace std;

class Callable;

// Varinant to encode possible object values
typedef variant<monostate, string, int, char, bool> Obj;

string to_string(const Obj &val);
void report(int line, string where, string message);

#endif
