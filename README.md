# database CS457 
Simple SQL Implementation
Vincent Pham

## Building & Testing
```
cmake -S . -B build && cmake --build build && ./build/sql_test
```
## Running PA2 Test Script
```
cat ./PA2_test.sql | ./build/main
```
## Running Interpreter
```
./build/main
```
An install script of for protobuffers is included, but not yet needed for testing.

## Overview
This project features a lexer, parser, evaluator for SQL. It also features a protocol buffer generator which is generated when SQL commands are read. The data models are stored as database models which also have commented metadata in order to conver it back to a "SQL-like" type.
Input is first read through a lexer to generate tokens, the tokens are then ran through a parser to generate an abstract syntax tree. The tree features expressions and statements. The expressions can be further evaluated and statements are expected to complete some type of action. In the evaluator, a mapping of lambdas and AST node keys allow for dynamic execution based on type. 

## PA1 Design
PA1's Design did not involve any data storage but rather the schemas and organization of data. Since the original plan of this project was to make everything run-time through Google's protobuf, the files were formatted and stored as code-generated code. I wrote an interpreter featuring a **lexer**(lexer.hpp) which took text from stdin and generated tokens. The tokens were used by the **parser** (parser.hpp) to then create an **abstract syntax tree** (ast.hpp). The **evaluator** (evaluator.hpp) is ran by the **repl** (repl.hpp) in order to generate C++ compliant objects in data_obj.hpp. This was then used by proto_generator to convert between itself and a text file. A database object was used to represent directories in the filesystem and table objects were used to represent files. Each of the table files end in .proto for reasons mentioned earlier and contains metadata, the message (schema), and followed by the actual data separated by commas.  
#### Parser
The parser is implemented as a recurisve descent parser. It works by taking in a tokens and determining which statement parser function to dispatch to by its initial token. The statement nodes the in the abstract syntax tree contain all the expressions and literals needed to evaluate the parser. These expressions and literals are all implemeneted as pointers in order to contain falsy values. Expressions usually evaluate into multiple non-terminals or have a linked list implementation to get the next expression. This populates the AST which has a root program node containing a list of all the statements found.
#### Evaluator
The evaluator takes the program and goes through the statements one-by-one and dispatches functors to execute based on the type of each statement node. It also keeps track of the current database and handles that context in order properly provide errors and warnings to the user.
#### ProtoGenerator (File)
The 'ProtoGenerator' provides the interface for the evaluator to execute upon to write to files, read from files, and return an updated database. All the methods are static and the only state that is held is in the constructor itself, which is called upon and returned once any mutating function is called.  

## PA2 Design
PA2 consisted of being able to store, modify, and delete data through the use of filters. This part required designing the record in the TableObject of data_objs.hpp. Fields were implemented so that additional fields can be added at runtime. This was done simply by creating a linked list of records with type safe unions using std::variant. Whenever a field was added, the linked list could add falsey values every `col_count` which could be updated later. With variadic arguments and formatting strings, format branching could dispatch different methods for every operator type, though this could've possibly have been done with C++ parameter packing. Most of the operational algorithms for the SQL commands (`SELECT`, `UPDATE`, `DELETE`) were implented in proto_generator.hpp all using a similar process of keeping track of rows and cols all in linear runtime. 
Where expressions were handled by a separate function in proto_generator (`ProtoGenerator::getWhereRows`) which simply takes in table and a where query as input and returns the rows that matched the query. The query was difficult to implement because of the static typing. However, the type, operator, name, and values were all provided so a cross-branching system was used in order to determine whether a row was to be considered or not. This was required due to the static typing constraints of the language but could be a lot cleaner if a lot more time were had using double dynamic dispatching.

## Bugs
The program is pretty complex and needs to shift to smart pointers. Currently there is no destructors for handling deallocation
There are also some issues with hashing functions in unordered_map, with our complex types so a custom implementation may be needed.

