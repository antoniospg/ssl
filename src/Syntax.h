#ifndef SYNTAX_H
#define SYNTAX_H

#include <vector>

#include "Lex.h"

class Syntax {
 public:
  std::vector<Token> token_stream;
  Syntax(std::vector<Token> token_stream);
  bool err;
  void error(int line, string message);
};

#endif
