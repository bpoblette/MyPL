//----------------------------------------------------------------------
// FILE: code_generate_tests.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: S. Bowers
// DESC: Basic code generator tests
//----------------------------------------------------------------------



#include <iostream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "mypl_exception.h"
#include "lexer.h"
#include "ast_parser.h"
#include "vm.h"
#include "code_generator.h"

using namespace std;


streambuf* stream_buffer;


void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s + "\n";
  return result;
}

//----------------------------------------------------------------------
// Simple getting started tests
//----------------------------------------------------------------------
//----------------------------------------------------------------------
TEST(BasicCodeGenTest, StringConcat) {
  stringstream in(build_string({
        "void main() {",
        "  string s1 = \"blue\"", 
        "  string s2 = \"green\"",
        "  print(concat(concat(s1, s2), \" \"))", 
        "  print(concat(concat(s1, s1), \" \"))", 
        "  print(concat(concat(s2, s1), \" \"))", 
        "  print(concat(s2, s2))", 
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("bluegreen blueblue greenblue greengreen", out.str());
  restore_cout();
}
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
