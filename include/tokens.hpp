/*
 * Define Strings as tokens for simplicity. 
 * - To be improved by setting tokens as bytes later
 */

#ifndef __TOKENS_HPP__
#define __TOKENS_HPP__

#include <string>
#include <sstream>
#include <unordered_map>

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

  // Keywords
  const TokenType TABLE = "TABLE";
  const TokenType DATABASE = "DATABASE";
  const TokenType CREATE = "CREATE";
  const TokenType DROP = "DROP";
  const TokenType SELECT = "SELECT";
  const TokenType ALTER = "ALTER";
  const TokenType USE = "USE";
  const TokenType FROM = "FROM";
  const TokenType ADD = "ADD";

  const TokenType COMMAND = ".";
  const TokenType EXIT_CMD = "EXIT";

  std::unordered_map<std::string, TokenType> keyword = {
    {"TABLE", TABLE},
    {"DATABASE", DATABASE},
    {"CREATE", CREATE},
    {"DROP", DROP},
    {"SELECT", SELECT},
    {"ALTER", ALTER},
    {"USE", USE},
    {"FROM", FROM},
    {"ADD", ADD},
    // Types
    {"INT", INT_TYPE},
    {"FLOAT", FLOAT_TYPE},
    {"VARCHAR", VARCHAR_TYPE},
    {"CHAR", CHAR_TYPE},
    // Commands
    {"EXIT", EXIT_CMD}
  };

  TokenType lookUpIdentifier(std::string ident) {
    for (auto & ch : ident) {
      ch = toupper(ch);
    }
    if (keyword.find(ident) != keyword.end()) {
      return keyword[ident];
    }
    return IDENTIFIER;
  }
}

struct Token {
  token_type::TokenType type;
  std::string literal;

  Token() {}

  Token(token_type::TokenType type, char literal) :
    type(type),
    literal(std::string(1, literal)) {}

  Token(token_type::TokenType type, std::string literal) :
    type(type),
    literal(literal) {}

  void operator=(const Token &rhs) {
    type = rhs.type;
    literal = rhs.literal;
  }

  operator std::string() const {
    std::ostringstream ss;
    ss << "{ Token: '" << type << "', Literal: '" << literal << "' }";
    return ss.str();
  }
};


#endif /* __TOKENS_HPP__ */
