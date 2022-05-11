/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: Tokens and some helper functions for looking up tokens. 
 * They're implemented as strings rather than an enum since it's useful for debugging.
 */

#ifndef __TOKENS_HPP__
#define __TOKENS_HPP__

#include <string>
#include <sstream>
#include <unordered_map>

namespace token_type
{
  // set TokenType so we can change the type later
  using TokenType = std::string;

  // Define tokens with strings for debugging
  const TokenType ILLEGAL = "ILLEGAL"; // Unknown Char
  const TokenType ENDOFFILE = "EOF";

  // Values/ID
  const TokenType IDENTIFIER = "IDENTIFIER";
  const TokenType TYPE = "TYPE";
  const TokenType INT = "INT";
  const TokenType FLOAT = "FLOAT";

  // Types
  const TokenType INT_TYPE = "INT_TYPE";
  const TokenType CHAR_TYPE = "CHAR_TYPE";
  const TokenType VARCHAR_TYPE = "VARCHAR_TYPE";
  const TokenType FLOAT_TYPE = "FLOAT_TYPE";
  const TokenType BOOL_TYPE = "BOOL_TYPE";

  // Delimiters
  const TokenType COMMA = ",";
  const TokenType SEMICOLON = ";";

  // Scope Symbols
  const TokenType LPAREN = "(";
  const TokenType RPAREN = ")";
  const TokenType QUOTE = "'";

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
  const TokenType INSERT = "INSERT";
  const TokenType INTO = "INTO";
  const TokenType VALUES = "VALUES";
  const TokenType DELETE = "DELETE";
  const TokenType WHERE = "WHERE";
  const TokenType UPDATE = "UPDATE";
  const TokenType SET = "SET";
  const TokenType ON = "ON";
  const TokenType BEGIN = "BEGIN";
  const TokenType TRANSACTION = "TRANSACTION";
  const TokenType COMMIT = "COMMIT";

  // Arithmetic
  const TokenType BANG = "!";
  const TokenType EQ = "=";
  const TokenType NE = "!=";
  const TokenType LT = "<";
  const TokenType GT = ">";
  const TokenType PLUS = "+";
  const TokenType MINUS = "-";
  const TokenType SLASH = "/";
  const TokenType ASTERISK = "*";
  
  // JOINS
  const TokenType LEFT = "LEFT";
  const TokenType RIGHT = "RIGHT";
  const TokenType INNER = "INNER";
  const TokenType OUTER = "OUTER";
  const TokenType JOIN = "JOIN";

  const TokenType COMMAND = ".";
  const TokenType EXIT_CMD = "EXIT";

  std::unordered_map<std::string, TokenType> keyword = {
      {"JOIN", JOIN},
      {"INNER", INNER},
      {"OUTER", OUTER},
      {"LEFT", LEFT},
      {"RIGHT", RIGHT},
      {"TABLE", TABLE},
      {"DATABASE", DATABASE},
      {"CREATE", CREATE},
      {"DROP", DROP},
      {"SELECT", SELECT},
      {"ALTER", ALTER},
      {"USE", USE},
      {"FROM", FROM},
      {"ADD", ADD},
      {"INSERT", INSERT},
      {"INTO", INTO},
      {"VALUES", VALUES},
      {"DELETE", DELETE},
      {"WHERE", WHERE},
      {"UPDATE", UPDATE},
      {"SET", SET},
      {"ON", ON},
      {"BEGIN", BEGIN},
      {"TRANSACTION", TRANSACTION},
      {"COMMIT", COMMIT}
  };

  std::unordered_map<std::string, TokenType> types = {
      // Types
      {"INT", INT_TYPE},
      {"FLOAT", FLOAT_TYPE},
      {"VARCHAR", VARCHAR_TYPE},
      {"CHAR", CHAR_TYPE},
      {"BOOL", BOOL_TYPE},
  };

  std::unordered_map<std::string, TokenType> commands = {
      // Commands
      {"EXIT", EXIT_CMD}};

  TokenType lookUpIdentifier(std::string ident)
  {
    for (auto &ch : ident)
    {
      ch = toupper(ch);
    }
    if (commands.find(ident) != commands.end())
    {
      return commands[ident];
    }
    if (types.find(ident) != types.end())
    {
      return types[ident];
    }
    if (keyword.find(ident) != keyword.end())
    {
      return keyword[ident];
    }
    return IDENTIFIER;
  }

  TokenType lookUpType(std::string type)
  {
    for (auto &ch : type)
    {
      ch = toupper(ch);
    }
    if (types.find(type) != types.end())
    {
      return types[type];
    }
    return TYPE;
  }

  TokenType lookUpCommand(std::string cmd)
  {
    for (auto &ch : cmd)
    {
      ch = toupper(ch);
    }
    if (commands.find(cmd) != commands.end())
    {
      return types[cmd];
    }
    return COMMAND;
  }
}

struct Token
{
  token_type::TokenType type;
  std::string literal;

  Token() {}

  Token(const Token &right) {
    type = right.type;
    literal = right.literal;
  }

  Token(token_type::TokenType type, char literal) : type(type),
                                                    literal(std::string(1, literal)) {}

  Token(token_type::TokenType type, std::string literal) : type(type),
                                                           literal(literal) {}

  void operator=(const Token &rhs)
  {
    type = rhs.type;
    literal = rhs.literal;
  }

  operator std::string() const
  {
    std::ostringstream ss;
    ss << "{ Token: '" << type << "', Literal: '" << literal << "' }";
    return ss.str();
  }
};

#endif /* __TOKENS_HPP__ */
