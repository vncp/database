#ifndef __REPL_HPP__
#define __REPL_HPP__

#include <string_view>
#include <lexer.hpp>
#include <tokens.hpp>
#include <iostream>


const string_view REPL_PROMPT = "Vincent > ";
void repl() {
  std::string input;
  while (true) {
    std::cout << REPL_PROMPT;
    getline(cin, input);
    Lexer lexer(input);
    for (Token token = lexer.nextToken(); token.type != token_type::ENDOFFILE; token = lexer.nextToken()) {
      std::cout << std::string(token) << std::endl;
      if (token.type == token_type::COMMAND) {
        token = lexer.nextToken();
        if (token.type == token_type::EXIT_CMD) {
          std::cout << std::string(token) << std::endl;
          return ;
        }
      }
    }
  }
}


#endif /* __REPL_HPP__ */