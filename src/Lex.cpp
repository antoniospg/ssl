#include "Lex.h"

#include <iostream>

#include "Util.h"

using namespace std;

std::map<TokenType, string> tok_to_string;

// ----------------------------------------
// Token function definitions
Token::Token(TokenType type, string lexeme, Obj literal, int line,
             int snd_token)
    : type(type),
      lexeme(lexeme),
      line(line),
      literal(literal),
      snd_token(snd_token) {}

string Token::show_val() {
  return string("type: '" + tok_to_string[type] +
                "', snd_token: " + to_string(snd_token) + ", lexeme: '" +
                lexeme + "', literal: '" + to_string(literal) + "'");
}

// ----------------------------------------
// Lexer function definitions
Lexer::Lexer(string src) : err(false), src(src), start(0), current(0), line(1) {
  // Keywords
  keywords["array"] = ARRAY;
  keywords["boolean"] = BOOLEAN;
  keywords["break"] = BREAK;
  keywords["char"] = CHAR;
  keywords["continue"] = CONTINUE;
  keywords["do"] = DO;
  keywords["else"] = ELSE;
  keywords["false"] = FALSE;
  keywords["function"] = FUNCTION;
  keywords["if"] = IF;
  keywords["integer"] = INTEGER;
  keywords["of"] = OF;
  keywords["string"] = STRING;
  keywords["struct"] = STRUCT;
  keywords["true"] = TRUE;
  keywords["type"] = TYPE;
  keywords["var"] = VAR;
  keywords["while"] = WHILE;

  // Enum To string
  tok_to_string[ARRAY] = "ARRAY";
  tok_to_string[BOOLEAN] = "BOOLEAN";
  tok_to_string[BREAK] = "BREAK";
  tok_to_string[CHAR] = "CHAR";
  tok_to_string[CONTINUE] = "CONTINUE";
  tok_to_string[DO] = "DO";
  tok_to_string[ELSE] = "ELSE";
  tok_to_string[FALSE] = "FALSE";
  tok_to_string[FUNCTION] = "FUNCTION";
  tok_to_string[IF] = "IF";
  tok_to_string[INTEGER] = "INTEGER";
  tok_to_string[OF] = "OF";
  tok_to_string[STRING] = "STRING";
  tok_to_string[STRUCT] = "STRUCT";
  tok_to_string[TRUE] = "TRUE";
  tok_to_string[TYPE] = "TYPE";
  tok_to_string[VAR] = "VAR";
  tok_to_string[WHILE] = "WHILE";
  tok_to_string[COLON] = "COLON";
  tok_to_string[SEMI_COLON] = "SEMI_COLON";
  tok_to_string[COMMA] = "COMMA";
  tok_to_string[EQUALS] = "EQUALS";
  tok_to_string[LEFT_SQUARE] = "LEFT_SQUARE";
  tok_to_string[RIGHT_SQUARE] = "RIGHT_SQUARE";
  tok_to_string[LEFT_BRACES] = "LEFT_BRACES";
  tok_to_string[RIGHT_BRACES] = "RIGHT_BRACES";
  tok_to_string[LEFT_PARENTHESIS] = "LEFT_PARENTHESIS";
  tok_to_string[RIGHT_PARENTHESIS] = "RIGHT_PARENTHESIS";
  tok_to_string[AND] = "AND";
  tok_to_string[OR] = "OR";
  tok_to_string[LESS_THAN] = "LESS_THAN";
  tok_to_string[GREATER_THAN] = "GREATER_THAN";
  tok_to_string[LESS_OR_EQUAL] = "LESS_OR_EQUAL";
  tok_to_string[GREATER_OR_EQUAL] = "GREATER_OR_EQUAL";
  tok_to_string[NOT_EQUAL] = "NOT_EQUAL";
  tok_to_string[EQUAL_EQUAL] = "EQUAL_EQUAL";
  tok_to_string[PLUS] = "PLUS";
  tok_to_string[PLUS_PLUS] = "PLUS_PLUS";
  tok_to_string[MINUS] = "MINUS";
  tok_to_string[MINUS_MINUS] = "MINUS_MINUS";
  tok_to_string[TIMES] = "TIMES";
  tok_to_string[DIVIDE] = "DIVIDE";
  tok_to_string[DOT] = "DOT";
  tok_to_string[NOT] = "NOT";
  tok_to_string[CHARACTER] = "CHARACTER";
  tok_to_string[NUMERAL] = "NUMERAL";
  tok_to_string[STRINGVAL] = "STRINGVAL";
  tok_to_string[ID] = "ID";
  tok_to_string[UNKNOWN] = "UNKNOWN";
}

void Lexer::error(int line, string message) {
  err = true;
  report(line, "", message);
}

bool Lexer::isDigit(char c) { return c <= '9' && c >= '0'; }

bool Lexer::isAlpha(char c) {
  return (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c == '_');
}

bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

inline char Lexer::consume() { return src[current++]; }

inline bool Lexer::srcEnd() { return current >= src.size(); }

inline char Lexer::lookahead(int offset) {
  if (current + offset >= src.size()) return '\0';
  return src[current + offset];
}

bool Lexer::match(char expect) {
  if (srcEnd()) return false;

  if (src[current] == expect) {
    current++;
    return true;
  }

  return false;
}

void Lexer::addToken(TokenType type) { addToken(type, ""); }

void Lexer::addToken(TokenType type, string literal) {
  // Convert string to Obj variant
  // Default is monostate
  Obj value;

  switch (type) {
    case STRINGVAL:
      value = literal;
      break;
    case CHARACTER:
      value = literal[0];
      break;
    case NUMERAL:
      value = stoi(literal);
      break;
    default:
      break;
  }

  if (type == ID) {
    // Search if the snd token exists, otherwise, create a new one
    if (snd_tokens.find(literal) == snd_tokens.end())
      snd_tokens[literal] = ++last_snd_token;

    tokens.push_back(Token(type, src.substr(start, current - start), value,
                           line, snd_tokens[literal]));
  } else {
    tokens.push_back(
        Token(type, src.substr(start, current - start), value, line));
  }
}

void Lexer::scanString() {
  // Consume chars until reach last '"'
  while (lookahead(0) != '"' && !srcEnd()) {
    if (lookahead(0) == '\n') line++;

    consume();
  }

  // If reach end, fail
  if (srcEnd()) {
    error(line, "Expect end of string");
    exit(1);
  }

  // Consume last '"'
  consume();
}

void Lexer::scanNum() {
  while (isDigit(lookahead(0))) consume();
}

void Lexer::scanIdentifier() {
  while (isAlphaNumeric(lookahead(0))) consume();
}

void Lexer::scanChar() {
  // Consume char until reach last '''
  if (lookahead(0) != '\'' && !srcEnd()) consume();

  // If reach end, fail
  if (srcEnd()) {
    error(line, "Expect end of string");
    exit(1);
  }

  // Consume last '"'
  consume();
}

void Lexer::consumeToken() {
  char c = consume();

  switch (c) {
    // Single char tokens
    case '(':
      addToken(LEFT_PARENTHESIS);
      break;
    case ')':
      addToken(RIGHT_PARENTHESIS);
      break;
    case '[':
      addToken(LEFT_SQUARE);
      break;
    case ']':
      addToken(RIGHT_SQUARE);
      break;
    case '{':
      addToken(LEFT_BRACES);
      break;
    case '}':
      addToken(RIGHT_BRACES);
      break;
    case ',':
      addToken(COMMA);
      break;
    case '.':
      addToken(DOT);
      break;
    case ';':
      addToken(SEMI_COLON);
      break;
    case '*':
      addToken(TIMES);
      break;
    case ':':
      addToken(COLON);
      break;
    case '&':
      addToken(AND);
      break;
    case '|':
      addToken(OR);
      break;
    case ' ':
      break;
    case '\n':
      line++;
      break;

    // 2 char tokens
    case '!':
      addToken(match('=') ? NOT_EQUAL : NOT);
      break;
    case '=':
      addToken(match('=') ? EQUAL_EQUAL : EQUALS);
      break;
    case '<':
      addToken(match('=') ? LESS_OR_EQUAL : LESS_THAN);
      break;
    case '>':
      addToken(match('=') ? GREATER_OR_EQUAL : GREATER_THAN);
      break;
    case '+':
      addToken(match('+') ? PLUS_PLUS : PLUS);
      break;
    case '-':
      addToken(match('-') ? MINUS_MINUS : MINUS);
      break;

    // Slashes
    case '/':
      if (match('/'))
        while (lookahead(0) != '\n' && !srcEnd()) consume();
      else
        addToken(DIVIDE);
      break;

    // String literal
    case '"':
      scanString();
      addToken(STRINGVAL, src.substr(start + 1, current - start - 2));
      break;

    // Char literal
    case '\'':
      scanChar();
      addToken(CHARACTER, src.substr(start + 1, current - start - 2));
      break;

    // Special cases
    default:
      // Check if it's a digit
      if (isDigit(c)) {
        scanNum();
        addToken(NUMERAL, src.substr(start, current - start));
        // Check if it's a identifier
      } else if (isAlpha(c)) {
        scanIdentifier();

        // Check if it's a keyword
        string identifierText = src.substr(start, current - start);
        TokenType tokType = (keywords.find(identifierText) == keywords.end())
                                ? ID
                                : keywords[identifierText];
        addToken(tokType, identifierText);
        // Error
      } else {
        error(line, string("Unexpected character: ") + c);
        err = true;
      }
      break;
  }
}

void Lexer::getTokens() {
  while (!srcEnd()) {
    consumeToken();
    start = current;
  }

  addToken(UNKNOWN);
}
