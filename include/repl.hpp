/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: REPL loop to handle errors and take stdin as input.
 */
#ifndef __REPL_HPP__
#define __REPL_HPP__

#include <lexer.hpp>
#include <parser.hpp>
#include <tokens.hpp>
#include <ast.hpp>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <data_objs.hpp>
#include <proto_generator.hpp>
#include <objects.hpp>
#include <evaluator.hpp>

// Empty prompt if stdin is not from tty
const std::string repl_prompt = isatty(fileno(stdin)) ? ">> " : "";

void repl()
{
  DatabaseObject nil_database("nil");
  DatabaseObject *current_database = &nil_database;
  std::string input;
  cout << "Vincent Pham - CS457 Database Management Systems\n";
  cout << "PA2 - SQL Lexer, Parser, and Evaluator\n";
  while (true)
  {
    std::cout << repl_prompt;
    getline(cin, input);
    Lexer lexer(input);
    SQLParser parser(&lexer);
    try
    {
      ast::Program *program = parser.parseSql();
      eval(program, current_database);
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