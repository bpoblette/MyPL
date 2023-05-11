//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#include "simple_parser.h"


SimpleParser::SimpleParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}


void SimpleParser::eat(TokenType t, const std::string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void SimpleParser::error(const std::string& msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def();
    else
      fun_def();
  }
  eat(TokenType::EOS, "expecting end-of-file");
}


void SimpleParser::struct_def()
{
  // TODO: finish this
  eat(TokenType::STRUCT, "expecting STRUCT");
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::LBRACE, "expecting LBRACE");
  if(!match(TokenType::RBRACE))
    fields();
  eat(TokenType::RBRACE, "expecting RBRACE");
}

void SimpleParser::fields()
{
    data_type();
    eat(TokenType::ID, "expecting ID");
    while (match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting COMMA");
      data_type();
      eat(TokenType::ID, "expecting ID");
    }
}

void SimpleParser::fun_def()
{

  if(!match(TokenType::VOID_TYPE))
  {
    data_type();
  }
  else
    eat(TokenType::VOID_TYPE, "expecting VOID_TYPE");
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::LPAREN, "expecting LPAREN");
  params();
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  eat(TokenType::RBRACE, "expecting RBRACE");
}


void SimpleParser::params()
{
  if(!match(TokenType::RPAREN))
  {
    data_type();
    eat(TokenType::ID, "expecting ID");
    while(match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting COMMA");
      data_type();
      eat(TokenType::ID, "expecting ID");
    }
  }
}

void SimpleParser::data_type()
{
  if(match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting ID");
  }
  else if(match(TokenType::ARRAY))
  {
    eat(TokenType::ARRAY, "expecting ARRAY");
    if(match(TokenType::ID))
      eat(TokenType::ID, "expecting ID");
    else
      base_type();  
  }
  else
    base_type();
}

void SimpleParser::base_type()
{
  if(match(TokenType::INT_TYPE))
    eat(TokenType::INT_TYPE, "expecting INT_TYPE");
  else if(match(TokenType::DOUBLE_TYPE))
    eat(TokenType::DOUBLE_TYPE, "expecting DOUBLE_TYPE");
  else if(match(TokenType::BOOL_TYPE))
    eat(TokenType::BOOL_TYPE, "expecting BOOL_TYPE");
  else if(match(TokenType::CHAR_TYPE))
    eat(TokenType::CHAR_TYPE, "expecting CHAR_TYPE");
  else if(match(TokenType::STRING_TYPE))
    eat(TokenType::STRING_TYPE, "expecting STRING_TYPE");
  else
    error("expecting VAL_TYPE");
}

void SimpleParser::stmt()
{
  
  if(match(TokenType::IF))
    if_stmt();
  else if(match(TokenType::WHILE))
    while_stmt();
  else if(match(TokenType::FOR))
    for_stmt();
  else if(match(TokenType::RETURN))
    ret_stmt();
  else if(match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, TokenType::CHAR_TYPE, TokenType::STRING_TYPE, TokenType::ARRAY}))
    vdecl_stmt();
  else if(match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting ID");
    if(match(TokenType::LPAREN))
      call_expr();
    else if(match(TokenType::ID))
      vdecl_stmt();
    else
      assign_stmt();
  }
  else
    error("expecting stmt");
}

void SimpleParser::vdecl_stmt()
{
  if(!match(TokenType::ID))
    data_type();
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::ASSIGN, "expectin ASSIGN");
  expr();
}

void SimpleParser::assign_stmt()
{
  lvalue();
  eat(TokenType::ASSIGN, "expecting ASSIGN");
  expr();
}

void SimpleParser::lvalue()
{
  while(match({TokenType::DOT, TokenType::LBRACKET}))
  {
    if(match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting DOT");
      eat(TokenType::ID, "expecting ID");
    }
    else
    {
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      expr();
      eat(TokenType::RBRACKET, "expecting RBRACKET");
    }
  }
}

void SimpleParser::if_stmt()
{
  eat(TokenType::IF, "expecting IF");
  eat(TokenType::LPAREN, "expecting LPAREN");
  expr();
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE))
  {
    stmt();
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
  if_stmt_t();
}

void SimpleParser::if_stmt_t()
{
  if(match(TokenType::ELSE))
  {
    eat(TokenType::ELSE, "expecting ELSE");
    eat(TokenType::LBRACE, "expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt();
    eat(TokenType::RBRACE, "expecting RBRACE");
  }
  else if(match(TokenType::ELSEIF))
  {
    eat(TokenType::ELSEIF, "expecting ELSEIF");
    eat(TokenType::LPAREN, "expecting LPAREN");
    expr();
    eat(TokenType::RPAREN, "expecting RPAREN");
    eat(TokenType::LBRACE, "expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt();
    eat(TokenType::RBRACE, "expecting RBRACE");
    if_stmt_t();
  }

}

void SimpleParser::while_stmt()
{
  eat(TokenType::WHILE, "expecting WHILE");
  eat(TokenType::LPAREN, "expecting LPAREN");
  expr();
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  eat(TokenType::RBRACE, "expecting RBRACE");
}

void SimpleParser::for_stmt()
{
  eat(TokenType::FOR, "expecting FOR");
  eat(TokenType::LPAREN, "expecting LPAREN");
  vdecl_stmt();
  eat(TokenType::SEMICOLON, "expecting SEMICOLON");
  expr();
  eat(TokenType::SEMICOLON, "expecting SEMICOLON");
  eat(TokenType::ID, "expecting ID");
  assign_stmt();
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  eat(TokenType::RBRACE, "expecting RBRACE");
}

void SimpleParser::call_expr()
{
  //eat(TokenType::ID, "expecting ID");
  eat(TokenType::LPAREN, "expecting LPAREN");
  if(!match(TokenType::RPAREN))
  {
    expr();
    while(match(TokenType::COMMA))
    {
    eat(TokenType::COMMA, "expecting COMMA");
    expr();
    }
  }
  eat(TokenType::RPAREN, "expecting RPAREN");
}

void SimpleParser::ret_stmt()
{
  eat(TokenType::RETURN, "expecting RETURN");
  expr();
}

void SimpleParser::expr()
{
  if(match(TokenType::LPAREN))
  {
    eat(TokenType::LPAREN, "expecting LPAREN");
    expr();
    eat(TokenType::RPAREN, "expecting RPAREN");
  }
  else if(match(TokenType::NOT))
  {
    eat(TokenType::NOT, "expecting NOT");
    expr();
  }
  else
    rvalue();

  if(bin_op())
  {
    advance();
    expr();
  }
}

void SimpleParser::rvalue()
{
  if(match(TokenType::NULL_VAL))
    eat(TokenType::NULL_VAL, "expecting NULL_VAL");
  else if (match(TokenType::NEW))
    new_rvalue();
  else if(match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting ID");
    if(match(TokenType::LPAREN))
      call_expr();
    else
      var_rvalue();
  }
  else
    base_rvalue();
}

void SimpleParser::new_rvalue()
{
  eat(TokenType::NEW, "expectin NEW");
  if(match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting ID");
    if(match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      expr();
      eat(TokenType::RBRACKET, "expecting RBRACKET");
    }
  }
  else
  {
    base_type();
    eat(TokenType::LBRACKET, "expecting LBRACKET");
    expr();
    eat(TokenType::RBRACKET, "expecting RBRACKET");
  }
}

void SimpleParser::base_rvalue()
{
  if(match(TokenType::INT_VAL))
    eat(TokenType::INT_VAL, "expecting INT_VAL");
  else if(match(TokenType::DOUBLE_VAL))
    eat(TokenType::DOUBLE_VAL, "expecting DOUBLE_VAL");
  else if(match(TokenType::BOOL_VAL))
    eat(TokenType::BOOL_VAL, "expecting BOOL_VAL");
  else if(match(TokenType::CHAR_VAL))
    eat(TokenType::CHAR_VAL, "expecting CHAR_VAL");
  else if(match(TokenType::STRING_VAL))
    eat(TokenType::STRING_VAL, "expecting STRING_VAL");
  else  
    error("expecting BASE_VAL");
}

void SimpleParser::var_rvalue()
{
  //todo
  //eat(TokenType::ID, "expecting ID");
  while (match({TokenType::DOT, TokenType::LBRACKET}))
  {
    if(match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting DOT");
      eat(TokenType::ID, "expecting ID");
    }
    else
    {
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      expr();
      eat(TokenType::RBRACKET, "expecting RBRACKET");
    }
  }
  
}

// TODO: Implement the rest of your recursive descent functions
//       here. See simple_parser.h

