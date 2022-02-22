/*
 * Define Strings as tokens for simplicity. 
 * - To be improved by setting tokens as bytes later
 */
#include <string>

#ifndef __TOKENS_HPP__
#define __TOKENS_HPP__

namespace token_type {
  // set TokenType so we can change the type later
  using TokenType = std::string;

  // Define tokens with strings for debugging
  const TokenType ILLEGAL = "ILLEGAL"; // Unknown Char
  const TokenType ENDOFFILE = "EOF";

  // Values/ID
  const TokenType IDENTIFIER = "IDENTIFIER";
  const TokenType INT = "INT";

  // Types
  const TokenType INT_TYPE = "INT_TYPE";
  const TokenType CHAR_TYPE = "CHAR_TYPE";
  const TokenType VARCHAR_TYPE = "VARCHAR_TYPE";
  const TokenType FLOAT_TYPE = "FLOAT_TYPE";

  // Delimiters
  const TokenType COMMA = ",";
  const TokenType SEMICOLON = ";";

  // Scope Symbols
  const TokenType LPAREN = "(";
  const TokenType RPAREN = ")";

  // Schema Types
  const TokenType TABLE = "TABLE";
  const TokenType DATABASE = "DATABSE";

  // Actions
  const TokenType CREATE = "CREATE";
  const TokenType DROP = "DROP";
  const TokenType SELECT = "SELECT";
  const TokenType ALTER = "ALTER";
  const TokenType USE = "USE";

  // Keywords
  const TokenType FROM = "FROM";
}


struct Token {
  token_type::TokenType type;
  std::string literal;

  Token() {}

  Token(token_type::TokenType type, std::string literal) :
    type(type),
    literal(literal) {}

  Token(token_type::TokenType type, char literal) :
    type(type),
    literal(std::string(1, literal)) {}

  void operator=(const Token &rhs) {
    type = rhs.type;
    literal = rhs.literal;
  }
};


#endif /* __TOKENS_HPP__ */
