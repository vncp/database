#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <string>
#include <iostream>
#include <tokens.hpp>
using namespace std;

/**
 * @brief Lexer class for finding all the tokens
 */
class Lexer
{
private:
  string input;
  int position = 0;
  int nextPosition = 0;
  char ch;

public:
  /**
   * @brief Begins input processing
   * 
   * @param input input string provided by user
   */
  Lexer(string input) : input(input)
  {
    readChar();
  }

  /**
   * @brief Reads the current char and sets positions and next position as necessary
   */
  void readChar()
  {
    if (nextPosition >= input.length())
      ch = 0;
    else
      ch = input[nextPosition];
    position = nextPosition++;
  }

  /**
   * @brief Looks at the enxt char without updating position
   * 
   * @return The next char
   */
  char peekChar()
  {
    if (nextPosition >= input.length())
      return 0;
    else
      return input[nextPosition];
  }

  /**
   * @brief Processes the next token and restores the position states prior
   * 
   * @return Token The next token
   */
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

  /**
   * @brief Progresses to the next token
   * 
   * @return Gives back the current token
   */
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
    case '\'':
      token = {token_type::QUOTE, ch};
      break;
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
        token.literal = readNumber(&token);
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

  /**
   * @brief Reads number and changes the passed token type to int or float depending on input
   * 
   * @param token the token to change the type
   * @return string The literal string input by the user
   */
  string readNumber(Token *token)
  {
    int curr_pos = position;
    while (isDigit(ch))
    {
      readChar();
    }
    token->type = token_type::INT;
    // If we find a decimal, then the token type is a float and we read the decimal integers
    if (ch == '.') {
      token->type = token_type::FLOAT;
      readChar();
      while (isDigit(ch))
      {
        readChar();
      }
    }
    return string(input.begin() + curr_pos, input.begin() + position);
  }

  /**
   * @brief Reads a token to determin whether its an identifier or keyword
   * 
   * @return string The token literal
   */
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