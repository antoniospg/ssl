#ifndef SYNTAX_H
#define SYNTAX_H

#include <unordered_map>
#include <vector>

#include "Lex.h"
#include "Util.h"

class Syntax {
 public:
  std::vector<Token> token_stream;
  std::unordered_map<int, Obj> constants;
  Syntax(std::vector<Token> token_stream);
  bool err;
  void error(int line, string message);
};

#endif
