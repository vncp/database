#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <string>
#include <tokens.hpp>
using namespace std;

class Lexer {
public:
  string input;
  int position = 0;
  int nextPosition = 0;
  char ch;

  Lexer(string input) :
    input(input) {
      readChar();
    }

  void readChar() {
    if (nextPosition >= input.size())
      ch = 0;
    else
      ch = input[nextPosition];
    position = nextPosition++;
  }

  // First version (single symbols)
  Token nextToken() {
    Token token;

    switch(ch) {
      case ';':
        token = Token(token_type::SEMICOLON, ch);
        break;
      case '(':
        token = Token{token_type::LPAREN, ch};
        break;
      case ')':
        token = Token{token_type::RPAREN, ch};
        break;
      case ',':
        token = Token{token_type::COMMA, ch};
        break;
      case 0:
        token = Token{token_type::ENDOFFILE, ""};
    }

    readChar();
    return token;
  }
};

#endif /* __LEXER_HPP__ */