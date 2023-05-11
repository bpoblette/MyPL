//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: Spring 2023
// AUTH: Brandon Poblette
// DESC: The purpose of this program is to 
//implement the starting commands for our MYPL. 
//----------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <lexer.h>
#include <token.h>
#include <mypl_exception.h>
#include <simple_parser.h>
#include <ast_parser.h>
#include <print_visitor.h>
#include <ast.h>
#include <semantic_checker.h>
#include <vm.h>
#include <code_generator.h>

using namespace std;

void display_help_message(void);

int main(int argc, char* argv[])
{
  string args[argc]; //an array to hold strings from the commandline
  ifstream file; //variable to open file
  istream* input = &cin; //variable to read from file
  string user_input = ""; //takes the user input from the terminal
  string user_input_2 = ""; //same as the last variable
  int array_size = 0; //amount of items in the array
  int linecounter = 0; //keeps track of lines that have been traversed
  char ch = ' '; //gets input from the file
  char ch_2 = ' '; //gets char from file
  


  for (int i = 0; i < argc; ++i) { //turns each element from command line as a string
    args[i] = string(argv[i]);
    array_size++;
  }

  if (array_size > 3) //if the user inputs too many commandline arguements
  {
    display_help_message(); //call help message
  }
  else
  {
    if(args[1] != "--help" && args[1] != "--parse" && args[1]!= "--lex" && args[1] !=  "--print" && args[1] !=  "--check" && args[1] !=  "--ir" && args[1] != "--sprof" && args[1] != "--per_prof")
    { //if command is not a flag
    input = new ifstream(args[1]);
    try {
      Lexer lexer(*input);
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker t;
      p.accept(t);
      VM vm;
      CodeGenerator g(vm);
      p.accept(g);
      vm.run();
      } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
      }
    }

    if(array_size == 3) //if user gave a flag and a file
    {
      file.open(args[2]); //opens file
      if(file.fail()) //checks to see if the file is open
      {
        cout << "ERROR: Unable to open file \'" << args[2] <<"\'" << endl;
      }
      else 
        input = new ifstream(args[2]); 
    }
    
    if(args[1] == "--help") 
    {
      display_help_message();
    }


    if(args[1] == "--lex")
    {
      cout << "[Lex Mode]" << endl;
      if(array_size == 2) //if args[2] doesn't have a file name
      {
        cin >> user_input;
        cout << user_input[0] << endl; //prints first letter from string input
      }
      else
      {
      try{
        Lexer lexer(*input);
        Token t = lexer.next_token();
        cout << to_string(t) << endl;
        while (t.type() != TokenType::EOS)
        {
          t = lexer.next_token();
          cout << to_string(t) << endl;
        }      
      }catch(MyPLException& ex)
        {
          cerr << ex.what() << endl;
        }
      }
    

      
    }

    if(args[1] == "--parse")
    {
      cout << "[Parse Mode]" << endl;
      if(array_size == 2) //if args[2] doesn't have a file name
      {
        cin >> user_input;
        cout << user_input[0] << user_input[1] << endl; //prints first 2 letters from user input
      }
      else
      {   
        try{
          Lexer lexer(*input);
          SimpleParser parser(lexer);
          parser.parse();
        }
        catch (MyPLException& ex)
        {
          cerr << ex.what() << endl;
        }
      }
    }

    if(args[1] == "--print")
    {
      cout << "[Print Mode]" << endl;
      if(array_size == 2) //if args[2] doesn't have a file name
      {
        int transverser = 0;
        cin >> user_input;
        while(user_input[transverser] != '\0') //while the next char isnt the end of hte line
        {
          cout << user_input[transverser];
          transverser++;
        }
        cout << endl;
      }
      else
      { 
        try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          PrintVisitor v(cout);
          p.accept(v);
          }catch (MyPLException& ex) {
          cerr << ex.what() << endl;
          }
      }
    }

    if(args[1] == "--check")
    {
      cout << "[Check Mode]" << endl;
      if(array_size == 2) //if args[2] doesn't have a file name
      {
        cin >> user_input;
        cout << user_input << endl; //prints the first line of input from user
      }    
      else
      {
        while(file.get(ch)) //print chars until the end of line character
        {
          ch = input->get();
          cout << ch;
          if(ch == '\n')
          {
            break;
          }
        }
        cout << endl;
      }
    }

    if(args[1] == "--ir")
    {
      if(array_size == 2)
      {
        getline(cin, user_input, '\n'); //gets two inputs from the users and then prints them
        getline(cin, user_input_2, '\n');
        cout << user_input << endl;
        cout << user_input_2 << endl;
      }  
      else
      {
        try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker t;
        p.accept(t);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        cout << to_string(vm) << endl;
        } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
        }

      }
      
    }
    if(args[1] == "--sprof")
    {
      if(array_size == 2)
      {
        getline(cin, user_input, '\n');
        cout << user_input << endl;
      }
      else{
          try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          VM vm;
          CodeGenerator g(vm);
          p.accept(g);
          vm.sprof();
          } catch (MyPLException& ex) {
          cerr << ex.what() << endl;
          }
        }
      }
      if(args[1] == "--per_prof")
      {
        if(array_size == 2)
        {
          getline(cin, user_input, '\n');
          cout << user_input << endl;
        }
        else{
          try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          VM vm;
          CodeGenerator g(vm);
          p.accept(g);
          vm.run(false, true);
          } 
          catch (MyPLException& ex) {
            cerr << ex.what() << endl;
          }
        }
      }
      if(args[1] == "--trace")
        if(array_size == 2)
        {
          getline(cin, user_input, '\n');
          cout << user_input << endl;
        }
        else{
          try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          VM vm;
          CodeGenerator g(vm);
          p.accept(g);
          vm.run(false, false, true);
          } 
          catch (MyPLException& ex) {
            cerr << ex.what() << endl;
          }
        }
    }

  // TODO: implement hw-1 as per the instructions in hw-1.pdf
  //   In addition:
  //   -- add your information to the file header
  //   -- comment your source code
  //   -- ensure your code is "clean" (see instructions)
  //   -- test your solution, including development of new test cases
  //   -- create hw1-writeup.pdf
  //   -- add, commit, and push your files
  //   -- double check your code has been uploaded to GitHub
  //   -- remove this comment block when you finish :-)  
  if(file.is_open())
  {
    file.close();
    delete input;
  }

  // if(!file->is_open())
  // {
  //   cout << "the file is closed now" << endl;
  // }
}

//prints to the user the options
void display_help_message(void)
{
  cout <<"Usage: ./mypl [option] [script-file]" << endl;
  cout <<"Options:" << endl;
  cout <<"  --help prints this message" << endl;
  cout <<"  --lex displays token information" << endl;
  cout <<"  --parse checks for syntax errors" << endl;
  cout <<"  --print pretty prints program" << endl;
  cout <<"  --check statically checks program" << endl;
  cout <<"  --ir print intermediate (code) representation." << endl;
  cout <<" --sprof prints out the percent of instructions per function" << endl;
  cout <<" --per_prof prints out the percent of instructions using a timer count" << endl;
  cout <<" --trace prints out the trace of the actual instructions being executed" << endl;
}