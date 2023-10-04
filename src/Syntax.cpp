#include "Syntax.h"

#include <iostream>
#include <iterator>
#include <stack>

#include "ParserTables.h"
#include "Semantics.h"
#include "Util.h"

typedef std::string string;

#define IS_SHIFT(p) ((p) > 0)
#define IS_REDUCTION(p) ((p) < 0)
#define RULE(p) (-(p))

void print_stack(std::stack<int> st) {
  std::cout << "print_stack: ";
  while (!st.empty()) {
    std::cout << st.top() << " ";
    st.pop();
  }
  std::cout << std::endl;
}

// Gambiarra, nao mudar os enums. Y deve ser o ultimo
int inc_term_tok(TokenType tok) { return tok + _Y + 1; }

Syntax::Syntax(std::vector<Token> token_stream) : token_stream(token_stream) {
  Semantics *sema = new Semantics();
  std::stack<int> stack;
  stack.push(0);

  int last_snd_token = -1;
  int last_const_idx = -1;
  auto tok_iter = token_stream.begin();
  do {
    if (tok_iter->snd_token != -1) last_snd_token = tok_iter->snd_token;
    if (tok_iter->const_idx != -1) {
      last_const_idx = tok_iter->const_idx;
      constants[last_const_idx] = tok_iter->literal;
    }

    //std::cout << "CUR TOK: lexeme: " << tok_iter->lexeme
    //          << " | snd_token: " << tok_iter->snd_token << std::endl;

    int act = action[stack.top()][inc_term_tok(tok_iter->type)];
    if (IS_SHIFT(act)) {
      stack.push(act);
      std::advance(tok_iter, 1);
    } else {
      if (IS_REDUCTION(act)) {
        int r = RULE(act) - 1;
        //std::cout << "REDUCTION: " << r << std::endl;
        // Reducao pela regra 0 significa que terminou
        if (r == 0) break;
        sema->addRule(r, last_snd_token, last_const_idx, tok_iter->line,
                      constants);
        if (sema->err) break;

        for (int i = 0; i < aux[r][0]; i++) stack.pop();
        stack.push(action[stack.top()][aux[r][1]]);
      } else {
        // Error
        error(tok_iter->line, string("Syntax error :("));
        break;
      }
    }
  } while (stack.size() > 1 || tok_iter->type != UNKNOWN);

  delete sema;
}

void Syntax::error(int line, string message) {
  err = true;
  report(line, "", message);
}
