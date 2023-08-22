#ifndef LEX_H
#define LEX_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Util.h"

typedef std::string string;

enum TokenType {
  // palavras reservadas
  ARRAY,
  BOOLEAN,
  BREAK,
  CHAR,
  CONTINUE,
  DO,
  ELSE,
  FALSE,
  FUNCTION,
  IF,
  INTEGER,
  OF,
  STRING,
  STRUCT,
  TRUE,
  TYPE,
  VAR,
  WHILE,
  // simbolos
  COLON,
  SEMI_COLON,
  COMMA,
  EQUALS,
  LEFT_SQUARE,
  RIGHT_SQUARE,
  LEFT_BRACES,
  RIGHT_BRACES,
  LEFT_PARENTHESIS,
  RIGHT_PARENTHESIS,
  AND,
  OR,
  LESS_THAN,
  GREATER_THAN,
  LESS_OR_EQUAL,
  GREATER_OR_EQUAL,
  NOT_EQUAL,
  EQUAL_EQUAL,
  PLUS,
  PLUS_PLUS,
  MINUS,
  MINUS_MINUS,
  TIMES,
  DIVIDE,
  DOT,
  NOT,
  // tokens regulares
  CHARACTER,
  NUMERAL,
  STRINGVAL,
  ID,
  // token deconhecido
  UNKNOWN
};

class Token {
public:
  int line;
  TokenType type;
  string lexeme;
  Obj literal;
  int snd_token;

  Token(TokenType type, string lexeme, Obj literal, int line, int snd_token = -1);
  string show_val();
};

class Lexer {
public:
  std::vector<Token> tokens;
  std::unordered_map<string, int> snd_tokens;
  int last_snd_token = -1;
  bool err;

  Lexer(string src);
  bool isDigit(char c);
  bool isAlpha(char c);
  bool isAlphaNumeric(char c);
  void getTokens();
  inline char consume();
  inline bool srcEnd();
  inline char lookahead(int offset);
  bool match(char expect);
  void consumeToken();
  void scanString();
  void scanNum();
  void scanIdentifier();
  void scanChar();
  void addToken(TokenType type);
  void addToken(TokenType type, string literal);
  void error(int line, string message);

private:
  string src;
  uint start, current, line;
  std::map<string, TokenType> keywords;
};

#endif
