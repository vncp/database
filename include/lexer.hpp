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
  Token prev;

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

  Token nextToken() {
    Token token;

    // skip whitespace
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
      readChar();

    switch(ch) {
      case ';':
        token = {token_type::SEMICOLON, ch};
        break;
      case '(':
        token = {token_type::LPAREN, ch};
        break;
      case ')':
        token = {token_type::RPAREN, ch};
        break;
      case ',':
        token = {token_type::COMMA, ch};
        break;
      case '.':
        token = {token_type::COMMAND, ch};
        break;
      case 0:
        token = Token{token_type::ENDOFFILE, ""};
      default:
        if (isLetter(ch)) {
          token.literal = readIdentifier();
          token.type = token_type::lookUpIdentifier(token.literal);
          return token;
        } 
        else if (isDigit(ch)) {
          token.type = token_type::INT;
          token.literal = readNumber();
          return token;
        }
        else {
          token = Token(token_type::ILLEGAL, ch);
        }
    }
    readChar();
    return token;
  }

  bool isLetter(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
  }

  bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
  }

  string readNumber() {
    int curr_pos = position;
    while (isDigit(ch)) {
      readChar();
    }
    return string(input.begin() + curr_pos, input.begin() + position);
  }

  string readIdentifier() {
    int curr_pos = position;
    if (isLetter(ch)) {
      readChar();
    }
    while (isLetter(ch) || isDigit(ch)) {
      readChar();
    }
    return string(input.begin() + curr_pos, input.begin() + position);
  }
};

#endif /* __LEXER_HPP__ */