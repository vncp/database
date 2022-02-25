#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <string>
#include <iostream>
#include <tokens.hpp>
using namespace std;

class Lexer
{
private:
  string input;
  int position = 0;
  int nextPosition = 0;
  char ch;

public:
  Lexer(string input) : input(input)
  {
    readChar();
  }

  void readChar()
  {
    if (nextPosition >= input.length())
      ch = 0;
    else
      ch = input[nextPosition];
    position = nextPosition++;
  }

  char peekChar()
  {
    if (nextPosition >= input.length())
      return 0;
    else
      return input[nextPosition];
  }

  Token peekToken()
  {
    int old_position = position;
    int old_nextPosition = nextPosition;
    char old_ch = ch;
    Token next = nextToken();
    position = move(old_position);
    nextPosition = move(nextPosition);
    ch = move(old_ch);
    return next;
  }

  Token nextToken()
  {
    Token token;

    // skip whitespace
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
    {
      readChar();
    }
    switch (ch)
    {
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
    case '!':
      if (peekChar() == '=')
      {
        token = {token_type::NE, "!="};
        readChar();
      }
      else
      {
        token = {token_type::BANG, ch};
      }
      break;
    case '=':
      token = {token_type::EQ, ch};
      break;
    case '<':
      token = {token_type::LT, ch};
      break;
    case '>':
      token = {token_type::GT, ch};
      break;
    case '+':
      token = {token_type::PLUS, ch};
      break;
    case '-':
      // Skip Comment
      if (peekChar() == '-') {
        while(ch != '\0') {
          readChar();
        }
        readChar();
      }
      token = {token_type::MINUS, ch};
      break;
    case '/':
      token = {token_type::SLASH, ch};
      break;
    case '*':
      token = {token_type::ASTERISK, ch};
      break;
    case '\0':
      token = Token{token_type::ENDOFFILE, ""};
      break;
    default:
      if (isLetter(ch))
      {
        token.literal = readIdentifier();
        token.type = token_type::lookUpIdentifier(token.literal);
        return token;
      }
      else if (isDigit(ch))
      {
        token.type = token_type::INT;
        token.literal = readNumber();
        return token;
      }
      else
      {
        token = Token(token_type::ILLEGAL, ch);
      }
    }
    readChar();
    return token;
  }

  bool isLetter(char ch)
  {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
  }

  bool isDigit(char ch)
  {
    return '0' <= ch && ch <= '9';
  }

  string readNumber()
  {
    int curr_pos = position;
    while (isDigit(ch))
    {
      readChar();
    }
    return string(input.begin() + curr_pos, input.begin() + position);
  }

  string readIdentifier()
  {
    int curr_pos = position;
    if (isLetter(ch))
    {
      readChar();
    }
    while (isLetter(ch) || isDigit(ch))
    {
      readChar();
    }
    return string(input.begin() + curr_pos, input.begin() + position);
  }
};

#endif /* __LEXER_HPP__ */