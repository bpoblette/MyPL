//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME:
// DESC:
//----------------------------------------------------------------------

#include "lexer.h"

using namespace std;


Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}


char Lexer::read()
{
  ++column;
  return input.get();
}


char Lexer::peek()
{
  return input.peek();
}


void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}


Token Lexer::next_token()
{
  string string_char = "";
  char ch = read();

  //comments and EOF
  while(ch == '#' || isspace(ch))
  {
    if(ch == '\n')
    {
      line++;
      column = 0;
    }
    else if(ch == '#')
    {
      while (peek() != '\n' && peek() != EOF)
      {
        ch = read();
      }
      ch = read();
      if(ch == '\n')
      {
        line++;
        column = 0;
      }
      else if(ch == EOF)
      {
        return Token(TokenType::EOS, "end-of-stream", line, column);
      }
    }
    ch = read();
  }

  if(ch == EOF)
  {
    return Token(TokenType::EOS, "end-of-stream", line, column);
  }

  //single and double character tokens
  if(ch == '?')
  {
    error("unexpected character '?'", line, column);
  }

  if(ch == '!')
  {
    if(peek() == '=')
    {
      ch = read();
      return Token(TokenType::NOT_EQUAL, "!=", line, column-1);
    }
    else
    {
      string wrong_token = "";
      ch = read();
      wrong_token += ch;
      error("expecting '!=' found '!" + wrong_token + "'", line, column-1);
    }
  }

  if(ch == '=')
  {
    if(peek() == '=')
    {
      ch = read();
      return Token(TokenType::EQUAL, "==", line, column-1);
    }
    else
    {
      return Token(TokenType::ASSIGN, "=", line, column);
    }
  }

  if(ch == '<')
  {
    if(peek() == '=')
    {
      ch = read();
      return Token(TokenType::LESS_EQ, "<=", line, column-1);
    }
    else
    {
      return Token(TokenType::LESS, "<", line, column);
    }
  }

  if(ch == '>')
  {
    if(peek() == '=')
    {
      ch = read();
      return Token(TokenType::GREATER_EQ, ">=", line, column-1);
    }
    return Token(TokenType::GREATER, ">", line, column);
  }

  if(ch == '+')
  {
    return Token(TokenType::PLUS, "+", line, column);
  }

  if(ch == '-')
  {
    return Token(TokenType::MINUS, "-", line, column);
  }

  if(ch == '*')
  {
    return Token(TokenType::TIMES, "*", line, column);
  }
  
  if(ch == '/')
  {
    return Token(TokenType::DIVIDE, "/", line, column);
  }

  if(ch == '.')
  {
    return Token(TokenType::DOT, ".", line, column);
  }

  if(ch == ',')
  {
    return Token(TokenType::COMMA, ",", line, column);
  }

  if(ch == '{')
  {
    return Token(TokenType::LBRACE, "{", line, column);
  }

  if(ch == '}')
  {
    return Token(TokenType::RBRACE, "}", line, column);
  }

  if(ch == '(')
  {
    return Token(TokenType::LPAREN, "(", line, column);
  }

  if(ch == ')')
  {
    return Token(TokenType::RPAREN, ")", line, column);
  }

  if(ch == '[')
  {
    return Token(TokenType::LBRACKET, "[", line, column);
  }

  if(ch == ']')
  {
    return Token(TokenType::RBRACKET, "]", line, column);
  }

  if(ch == ';')
  {
    return Token(TokenType::SEMICOLON, ";", line, column);
  }


  //chars
  if(ch == '\'')
  {
    int start_line = line;
    int start_column = column;

    if(peek() == '\'')
    {
      error("empty character", line, column+1);
    }
    else if(peek() == '\n')
    {
      error("found end-of-line in character", line, column + 1);
    }
    else
    {
      ch = read();
      string_char += ch;
      if(ch == '\\')
      {
        if(peek() == 't' || peek() == 'n' || peek() == '0')
        {
          ch = read();
          string_char += ch;
          ch = read();
          return Token(TokenType::CHAR_VAL, string_char, line, start_column);
        }
        else
        {
          error("error", line, column);
        }
      }
      if(peek() == '\n')
      {
        ch = read();
        string_char += ch;
        error("found end-of-line in character at", line, column);
      }
      if(isalpha(peek()))
      {
        string wrong_char;
        ch = read();
        wrong_char += ch;
        error("expecting ' found " + wrong_char, line, column);
      }
      if(peek() == '\'')
      {
        ch == read();
        return Token(TokenType::CHAR_VAL, string_char, line, start_column);
      }
      if(peek() == EOF)
      {
        ch = read();
        string_char += ch;
        error("found end-of-file in character", line, column - 1);
      }
      else
      {
        error("error", line, column);
      }
    }
  }
  //strings
  if(ch == '\"')
  {
    int start_column = column;
    
    while(peek() != '\"' && peek() != EOF)
    {
      ch = read();
      string_char += ch;
      if(peek() == '\n')
      {
        ch = read();
        string_char += ch;
        error("found end-of-line in string", line, column);
      }
    }

    if(peek() == EOF)
    {
      ch = read();
      string_char += ch;
      error("found end-of-file in string", line, column);
    }
    ch = read();
    return Token(TokenType::STRING_VAL, string_char, line, start_column);
  }

  //Ints and doubles
  if (isdigit(ch))
  {
    int start_column = column;
    string_char += ch;
    if(ch == '0')
    {
      if(isspace(peek()))
      {
        return Token(TokenType::INT_VAL, string_char, line, start_column);
      }
      else if(isdigit(peek()))
      {
        error("leading zero in number", line, column);
      }
      else if(ch == '.')
      {
        string_char += ch;
        ch = read();
        while(isdigit(ch))
        {
          string_char += ch;
          ch = read();
        }
        return Token(TokenType::DOUBLE_VAL, string_char, line, start_column);
      }
    }
    while (isdigit(peek()))
    {
      ch = read();
      string_char += ch;
    }

    if(peek() == '.')
    {
      ch = read();
      string_char += ch;
      // ch = read();
      if(!isdigit(ch) && ch != '.')
      {
        error("missing digit in '" + string_char + "\'", line, column);
      }
      while (isdigit(peek()))
      {
        ch = read();
        string_char += ch;
        if(peek() == '.')
        {
          return Token(TokenType::DOUBLE_VAL, string_char, line, start_column);
        }
      }
      return Token(TokenType::DOUBLE_VAL, string_char, line, start_column);
    }
    else
    {
      return Token(TokenType::INT_VAL, string_char, line, start_column);
    }
    
  }
  
  //Reserved words and ID
  if(isalpha(ch))
  {
    string_char += ch;
    int start_column = column;

    while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
    {
      ch = read();
      string_char += ch;
      if(string_char == "and")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::AND, string_char, line, start_column);
      }
      else if(string_char == "or")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::OR, string_char, line, start_column); 
      }
      else if(string_char == "not")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::NOT, string_char, line, start_column); 
      }
      else if(string_char == "else")
      {
        if(peek() == 'i')
        {
          ch = read();
          string_char += ch;
          if(peek() == 'f')
          {
            ch = read();
            string_char += ch;
            if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
            {
              ch = read();
              string_char += ch;
              while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
              {
                ch = read();
                string_char += ch;
              }
              return Token(TokenType::ID, string_char, line, start_column);
            }
            else
            {
              return Token(TokenType::ELSEIF, string_char, line, start_column);
            }
          }
          else if(peek() != 'f')
          {
            ch = read();
            string_char += ch;
            return Token(TokenType::ID, string_char, line, start_column);
          }
        }
        return Token(TokenType::ELSE, string_char, line, start_column);
      }
      else if(string_char == "struct")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::STRUCT, string_char, line, start_column); 
      }
      else if(string_char == "array")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::ARRAY, string_char, line, start_column); 
      }
      else if(string_char == "for")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::FOR, string_char, line, start_column); 
      }
      else if(string_char == "while")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::WHILE, string_char, line, start_column); 
      }
      else if(string_char == "return")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::RETURN, string_char, line, start_column); 
      }
      else if(string_char == "new")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::NEW, string_char, line, start_column); 
      }
      else if(string_char == "if")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::IF, string_char, line, start_column); 
      }
      else if(string_char == "int")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::INT_TYPE, string_char, line, start_column); 
      }
      else if(string_char == "double")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::DOUBLE_TYPE, string_char, line, start_column); 
      }
      else if(string_char == "bool")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::BOOL_TYPE, string_char, line, start_column); 
      }
      else if(string_char == "char")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::CHAR_TYPE, string_char, line, start_column); 
      }
      else if(string_char == "true")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::BOOL_VAL, string_char, line, start_column); 
      }
      else if(string_char == "false")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::BOOL_VAL, string_char, line, start_column); 
      }
      else if(string_char == "string")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::STRING_TYPE, string_char, line, start_column); 
      }
      else if(string_char == "null")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::NULL_VAL, string_char, line, start_column); 
      }
      else if(string_char == "void")
      {
        if(isalpha(peek()) || isdigit(peek()) || peek() == '_')
        {
          ch = read();
          string_char += ch;
          while(isalpha(peek()) || isdigit(peek()) || peek() == '_')
          {
            ch = read();
            string_char += ch;
          }
        }
        else
          return Token(TokenType::VOID_TYPE, string_char, line, start_column); 
      }
    }
    return Token(TokenType::ID, string_char, line, start_column);



  //   if(isalpha(peek()))
  //   {
  //     ch = read();
  //     while (!isspace(ch) && ch != EOF)
  //     {
  //       string_char += ch;

  //       if(string_char == "if")
  //       {
  //         if(peek() == '(')
  //         {
  //           return Token(TokenType::IF, "if", line, start_column);
  //         }
  //       }

  //       if(string_char == "elseif")
  //       {
  //         if(peek() == '(')
  //         {
  //           return Token(TokenType::ELSEIF, "elseif", line, start_column);
  //         }
  //       }
  //       ch = read();
  //     } 
  //   }
      
  //   else if(!isalpha(peek()) && !isspace(peek()))
  //   {
  //     ch = read();
  //     while (!isspace(ch))
  //     {
  //       string_char += ch;
  //       ch = read();
  //     } 
  //   }
  //   if(string_char == "struct")
  //   {
  //     return Token(TokenType::STRUCT, string_char, line, start_column);
  //   }
  //   else if(string_char  == "array")
  //   {
  //     return Token(TokenType::ARRAY, string_char, line, start_column);
  //   }
  //   else if(string_char  == "while")
  //   {
  //     return Token(TokenType::WHILE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "for")
  //   {
  //     return Token(TokenType::FOR, string_char, line, start_column);
  //   }
  //   else if(string_char  == "if")
  //   {
  //     return Token(TokenType::IF, string_char, line, start_column);
  //   }
  //   else if(string_char  == "elseif")
  //   {
  //     return Token(TokenType::ELSEIF, string_char, line, start_column);
  //   }
  //   else if(string_char  == "else")
  //   {
  //     return Token(TokenType::ELSE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "and")
  //   {
  //     return Token(TokenType::AND, string_char, line, start_column);
  //   }
  //   else if(string_char  == "or")
  //   {
  //     return Token(TokenType::OR, string_char, line, start_column);
  //   }
  //   else if(string_char  == "not")
  //   {
  //     return Token(TokenType::NOT, string_char, line, start_column);
  //   }
  //   else if(string_char  == "new")
  //   {
  //     return Token(TokenType::NEW, string_char, line, start_column);
  //   }
  //   else if(string_char  == "return")
  //   {
  //     return Token(TokenType::RETURN, string_char, line, start_column);
  //   }
  //   else if(string_char  == "int")
  //   {
  //     return Token(TokenType::INT_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "double")
  //   {
  //     return Token(TokenType::DOUBLE_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "bool")
  //   {
  //     return Token(TokenType::BOOL_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "string")
  //   {
  //     return Token(TokenType::STRING_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "char")
  //   {
  //     return Token(TokenType::CHAR_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "void")
  //   {
  //     return Token(TokenType::VOID_TYPE, string_char, line, start_column);
  //   }
  //   else if(string_char  == "null")
  //   {
  //     return Token(TokenType::NULL_VAL, string_char, line, start_column);
  //   }
  //   else if(string_char  == "true")
  //   {
  //     return Token(TokenType::BOOL_VAL, string_char, line, start_column);
  //   }
  //   else if(string_char  == "false")
  //   {
  //     return Token(TokenType::BOOL_VAL, string_char, line, start_column);
  //   }
  //   else
  //   {
  //     return Token(TokenType::ID, string_char, line, start_column);
  //   }
  }
  return Token(TokenType::EOS, "end-of-stream", line, column);
}
  

