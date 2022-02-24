#ifndef __REPL_HPP__
#define __REPL_HPP__

#include <lexer.hpp>
#include <parser.hpp>
#include <tokens.hpp>
#include <ast.hpp>
#include <iostream>
#include <string>
#include <data_objs.hpp>
#include <proto_generator.hpp>
#include <objects.hpp>
#include <evaluator.hpp>

const std::string repl_prompt = "> ";

void repl()
{
  std::string input;
  while (true)
  {
    std::cout << repl_prompt;
    getline(cin, input);
    Lexer lexer(input);
    SQLParser parser(&lexer);
    try
    {
      ast::Program *program = parser.parseSql();
      cout << std::string(*program) << endl;
    }
    catch (const expected_token_error &e)
    {
      cerr << e.what() << endl;
    }
    catch (const unknown_type_error &e)
    {
      cerr << e.what() << endl;
    }
    catch (const unassigned_parse_function_error &e)
    {
      cerr << e.what() << endl;
    }
    catch (const unknown_command_error &e)
    {
      cerr << e.what() << endl;
    }
    catch (const runtime_error &e)
    {
      cerr << e.what() << endl;
    }
  }
}

#endif /* __REPL_HPP__ */