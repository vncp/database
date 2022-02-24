#ifndef __REPL_HPP__
#define __REPL_HPP__

#include <string_view>
#include <lexer.hpp>
#include <parser.hpp>
#include <tokens.hpp>
#include <ast.hpp>
#include <iostream>


const string_view REPL_PROMPT = "> ";
void repl() {
  std::string input;
  while (true) {
    std::cout << REPL_PROMPT;
    getline(cin, input);
    Lexer lexer(input);
    SQLParser parser(&lexer);
    try {
      ast::Program *program = parser.parseSql();
      cout << std::string(*program) << endl;
    } catch (const expected_token_error &e) {
      cerr << e.what() << endl;
    } catch (const unknown_type_error &e) {
      cerr << e.what() << endl;
    } catch (const unassigned_parse_function_error &e) {
      cerr << e.what() << endl;
    } catch (const unknown_command_error &e) {
      cerr << e.what() << endl;
    } catch (const runtime_error &e) {
      cerr << e.what() << endl;
    }

  }
}


#endif /* __REPL_HPP__ */