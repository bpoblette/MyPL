//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------
#include "ast_parser.h"
#include "iostream"

using namespace std;


ASTParser::ASTParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void ASTParser::advance()
{
  curr_token = lexer.next_token();
}


void ASTParser::eat(TokenType t, const string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void ASTParser::error(const string& msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool ASTParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else
      fun_def(p);
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}


void ASTParser::struct_def(Program& p)
{
  StructDef s;
  eat(TokenType::STRUCT, "expecting STRUCT");
  s.struct_name = curr_token;
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::LBRACE, "expecting LBRACE");
  if(!match(TokenType::RBRACE)){
    fields(s);
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
  p.struct_defs.push_back(s);
}

void ASTParser::fields(StructDef& s)
{
  VarDef v;
  data_type(v.data_type);
  v.var_name = curr_token;
  eat(TokenType::ID, "expecting ID");
  s.fields.push_back(v);
  while(match(TokenType::COMMA)){
    VarDef v2;
    eat(TokenType::COMMA, "expecting COMMA");
    data_type(v2.data_type);
    v2.var_name = curr_token;
    eat(TokenType::ID, "expecting ID");
    s.fields.push_back(v2);
  }
}


void ASTParser::fun_def(Program& p)
{
  FunDef f;
  //Check for DATA_TYPE or VOID_TYPE, otherwise ERROR
  if(!match(TokenType::VOID_TYPE)){
    data_type(f.return_type);
  }
  else{
    //This should be VOID TYPE
    f.return_type.type_name = curr_token.lexeme();
    eat(TokenType::VOID_TYPE, "expecting VOID_TYPE");
  }
  f.fun_name = curr_token;
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::LPAREN, "expecting LPAREN");
  params(f);
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  //While the curr_token isn't RBRACE, we should be expecting a stmt
  while(!match(TokenType::RBRACE)){
    stmt(f.stmts);
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
  p.fun_defs.push_back(f);
}

void ASTParser::params(FunDef& f)
{
  VarDef v;
  if(!match(TokenType::RPAREN)){
    data_type(v.data_type);
    v.var_name = curr_token;
    eat(TokenType::ID, "expecting ID");
    f.params.push_back(v);
    while(match(TokenType::COMMA)){
      VarDef v2;
      eat(TokenType::COMMA, "expecting COMMA");
      data_type(v2.data_type);
      v2.var_name = curr_token;
      eat(TokenType::ID, "expecting ID");
      f.params.push_back(v2);
    }
  }
}

void ASTParser::data_type(DataType& d)
{
  if(match(TokenType::ID)){
    // d.is_array = false;
    d.type_name = curr_token.lexeme();
    eat(TokenType::ID, "expecting ID");
  }
  else if(match(TokenType::ARRAY)){
    d.is_array = true;
    eat(TokenType::ARRAY, "expecting ARRAY");
    if(match(TokenType::ID)){
      d.type_name = curr_token.lexeme();
      eat(TokenType::ID, "expecting ID");
    }
    else{
      d.type_name = curr_token.lexeme();
      base_type();
    }
  }
  else{
    // d.is_array = false;
    d.type_name = curr_token.lexeme();
    base_type();
  }
}

void ASTParser::base_type()
{
  //Nothing is modified here since we will be calling setting a DataType's type_name just before calling this
  if(match(TokenType::INT_TYPE)){
    eat(TokenType::INT_TYPE, "expecting INT_TYPE");
  }
  else if(match(TokenType::DOUBLE_TYPE)){
    eat(TokenType::DOUBLE_TYPE, "expecting DOUBLE_TYPE");
  }
  else if(match(TokenType::BOOL_TYPE)){
    eat(TokenType::BOOL_TYPE, "expecting BOOL_TYPE");
  }
  else if(match(TokenType::CHAR_TYPE)){
    eat(TokenType::CHAR_TYPE, "exptecing CHAR_TYPE");
  }
  else if(match(TokenType::STRING_TYPE)){
    eat(TokenType::STRING_TYPE, "expecting STRING_TYPE");
  }
  else{
    error("expecting VAL_TYPE");
  }
}

void ASTParser::stmt(std::vector<std::shared_ptr<Stmt>>& s)
{
  if(match(TokenType::IF)){
    IfStmt i;
    if_stmt(i);
    s.push_back(std::make_shared<IfStmt> (i));
  }
  else if(match(TokenType::WHILE)){
    WhileStmt w;
    while_stmt(w);
    s.push_back(std::make_shared<WhileStmt> (w));
  }
  else if(match(TokenType::FOR)){
    ForStmt f;
    for_stmt(f);
    s.push_back(std::make_shared<ForStmt> (f));
  }
  else if(match(TokenType::RETURN)){
    ReturnStmt r;
    ret_stmt(r);
    s.push_back(std::make_shared<ReturnStmt> (r));
  }
  else if(match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, TokenType::CHAR_TYPE, TokenType::STRING_TYPE, TokenType::ARRAY})){
    VarDeclStmt v;
    vdecl_stmt(v);
    s.push_back(std::make_shared<VarDeclStmt> (v));
  }
  else if(match(TokenType::ID)){
    //If ID, either vdecl_stmt or assign_stmt
    Token name = curr_token; //This is for the odd case of a CallExpr's name attribute
    eat(TokenType::ID, "expecting ID in stmt");
    if(match(TokenType::LPAREN)){
      CallExpr c;
      c.fun_name = name;
      call_expr(c);
      // std::cout << curr_token.lexeme() << "\n";
      s.push_back(std::make_shared<CallExpr> (c));
    }
    else if(match(TokenType::ID)){
      VarDeclStmt v2;
      v2.var_def.data_type.type_name = name.lexeme();
      vdecl_stmt(v2);
      s.push_back(std::make_shared<VarDeclStmt> (v2));
    }
    else{
      AssignStmt a;
      VarRef v;
      v.var_name = name;
      a.lvalue.push_back(v);
      assign_stmt(a);
      s.push_back(std::make_shared<AssignStmt> (a));
    }
  }
  else{
    error("expecting stmt");
  }
}

void ASTParser::vdecl_stmt(VarDeclStmt& v)
{
  if(!match(TokenType::ID)){
    data_type(v.var_def.data_type);
  }
  v.var_def.var_name = curr_token;
  eat(TokenType::ID, "expecting ID");
  eat(TokenType::ASSIGN, "expecting ASSIGN");
  expr(v.expr);
}

void ASTParser::assign_stmt(AssignStmt& a)
{
  lvalue(a.lvalue);
  eat(TokenType::ASSIGN, "expecting ASSIGN");
  expr(a.expr);
}

void ASTParser::lvalue(std::vector<VarRef>& l)
{
  while(match({TokenType::DOT, TokenType::LBRACKET})){
    VarRef v;
    if(match(TokenType::DOT)){
      eat(TokenType::DOT, "expecting DOT");
      v.var_name = curr_token;
      eat(TokenType::ID, "expecting ID");
      l.push_back(v);
    }
    else{
      v = l.back();
      l.pop_back();
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      Expr e;
      expr(e);
      v.array_expr = e;
      l.push_back(v);
      eat(TokenType::RBRACKET, "expecting RBRACKER");
    }
  }
}

void ASTParser::if_stmt(IfStmt& i)
{
  eat(TokenType::IF, "expecting IF");
  eat(TokenType::LPAREN, "expecting LPAREN");
  expr(i.if_part.condition);
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE)){
    stmt(i.if_part.stmts);
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
  if_stmt_t(i);
}

void ASTParser::if_stmt_t(IfStmt& i)
{
  if(match(TokenType::ELSEIF)){
    eat(TokenType::ELSEIF, "expecting ELSEIF");
    eat(TokenType::LPAREN, "expecting LPAREN");
    BasicIf b;
    expr(b.condition);
    eat(TokenType::RPAREN, "expecting RPAREN");
    eat(TokenType::LBRACE, "expecting LBRACE");
    while(!match(TokenType::RBRACE)){
      stmt(b.stmts);
    }
    eat(TokenType::RBRACE, "expecting RBRACE");
    i.else_ifs.push_back(b);
    if_stmt_t(i);
  }
  else if(match(TokenType::ELSE))
  {
    eat(TokenType::ELSE, "expecting ELSE");
    eat(TokenType::LBRACE, "expecting LBRACE");
    while(!match(TokenType::RBRACE)){
      stmt(i.else_stmts);
    }
    eat(TokenType::RBRACE, "expecting RBRACE");
  }
}

void ASTParser::while_stmt(WhileStmt& w)
{
  eat(TokenType::WHILE, "expecting WHILE");
  eat(TokenType::LPAREN, "expecting LPAREN");
  expr(w.condition);
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE)){
    stmt(w.stmts);
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
}

void ASTParser::for_stmt(ForStmt& f)
{
  eat(TokenType::FOR, "expecting FOR");
  eat(TokenType::LPAREN, "expecting LPAREN");
  vdecl_stmt(f.var_decl);
  eat(TokenType::SEMICOLON, "expecting SEMICOLON");
  expr(f.condition);
  eat(TokenType::SEMICOLON, "expecting SEMICOLON");
  VarRef v;
  v.var_name = curr_token;
  f.assign_stmt.lvalue.push_back(v);
  eat(TokenType::ID, "expecting ID");
  assign_stmt(f.assign_stmt);
  eat(TokenType::RPAREN, "expecting RPAREN");
  eat(TokenType::LBRACE, "expecting LBRACE");
  while(!match(TokenType::RBRACE)){
    stmt(f.stmts);
  }
  eat(TokenType::RBRACE, "expecting RBRACE");
}

void ASTParser::call_expr(CallExpr& c)
{
  //c.fun_name is already set stmt->call_expr
  eat(TokenType::LPAREN, "expecting LPAREN");
  if(!match(TokenType::RPAREN)){
    Expr e;
    expr(e);
    c.args.push_back(e);
    while(match(TokenType::COMMA)){
      eat(TokenType::COMMA, "expecting COMMA");
      Expr e2;
      expr(e2);
      c.args.push_back(e2);
    }
  }
  eat(TokenType::RPAREN, "expecting RPAREN");
}

void ASTParser::ret_stmt(ReturnStmt& r)
{
  eat(TokenType::RETURN, "expecting RETURN");
  expr(r.expr);
}

void ASTParser::expr(Expr& e)
{
  //First half of expr Syntax Rules
  if(match(TokenType::LPAREN)){
    //Complex Value
    ComplexTerm c;
    eat(TokenType::LPAREN, "expecting LPAREN");
    expr(c.expr);
    eat(TokenType::RPAREN, "expecting RPAREN");
    e.first = std::make_shared<ComplexTerm> (c);
  }
  else if(match(TokenType::NOT)){
    e.negated = true;
    eat(TokenType::NOT, "expecting NOT");
    expr(e);
  }
  else{
    //Simple Value
    SimpleTerm s;
    rvalue(s.rvalue);
    e.first = std::make_shared<SimpleTerm> (s);
  }

  // //Second half of expr Syntax Rules
  if(bin_op()){
    //If there is an operator, set e.op = curr_token
    e.op = curr_token;
    advance();
    Expr e2;
    expr(e2);
    e.rest = std::make_shared<Expr> (e2);
  }
  //Else, do nothing (return empty)
}

void ASTParser::rvalue(std::shared_ptr<RValue>& r)
{
 
  if(match(TokenType::NULL_VAL)){
    SimpleRValue s;
    s.value = curr_token;
    r = std::make_shared<SimpleRValue>(s);
    eat(TokenType::NULL_VAL, "expecting NULL_VAL");
   
  }
  else if(match(TokenType::NEW)){
    NewRValue n;
    new_rvalue(n);
    r = std::make_shared<NewRValue>(n);
  }
  else if(match(TokenType::ID)){
    Token name = curr_token; //For the case of CallExpr or VarRvalue
    eat(TokenType::ID, "expecting ID");
    if(match(TokenType::LPAREN)){
      CallExpr c;
      c.fun_name = name;
      call_expr(c);
      r = std::make_shared<CallExpr>(c);
    }
    else{
      VarRValue v;
      VarRef v2;
      v2.var_name = name;
      v.path.push_back(v2);
      var_rvalue(v.path);
      r = std::make_shared<VarRValue>(v);
    }
  }
  else{
    SimpleRValue s;
    base_rvalue(s);
    r = std::make_shared<SimpleRValue>(s);
    //Else, do nothing
  }
}

void ASTParser::new_rvalue(NewRValue& n)
{
  eat(TokenType::NEW, "expecting NEW");
  if(match(TokenType::ID)){
    n.type = curr_token;
    eat(TokenType::ID, "expecting ID");
    if(match(TokenType::LBRACKET)){
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      Expr e;
      expr(e);
      n.array_expr = e;
      eat(TokenType::RBRACKET, "expecting RBRACKET");
    }
  }
  else{
    n.type = curr_token;
    base_type();
    eat(TokenType::LBRACKET, "expecting LBRACKET");
    Expr e2;
    expr(e2);
    n.array_expr = e2;
    eat(TokenType::RBRACKET, "expecting RBRACKET");
  }
}

void ASTParser::base_rvalue(SimpleRValue& s)
{
  if(match(TokenType::INT_VAL)){
    s.value = curr_token;
    eat(TokenType::INT_VAL, "expecting INV_VAL");
  }
  else if(match(TokenType::DOUBLE_VAL)){
    s.value = curr_token;
    eat(TokenType::DOUBLE_VAL, "expecting DOUBLE_VAL");
  }
  else if(match(TokenType::BOOL_VAL)){
    s.value = curr_token;
    eat(TokenType::BOOL_VAL, "expecting BOOL_VAL");
  }
  else if(match(TokenType::CHAR_VAL)){
    s.value = curr_token;
    eat(TokenType::CHAR_VAL, "expecting CHAR_VAL");
  }
  else if(match(TokenType::STRING_VAL)){
    s.value = curr_token;
    eat(TokenType::STRING_VAL, "expecting STRING_VAL");
  }
  else{
    error("expecting BASE_VAL");
  }
}

void ASTParser::var_rvalue(std::vector<VarRef>& v)
{
  while(match({TokenType::DOT, TokenType::LBRACKET})){
    VarRef v2;
    if(match(TokenType::DOT)){
      eat(TokenType::DOT, "expecting DOT");
      v2.var_name = curr_token;
      eat(TokenType::ID, "expecting ID");
      v.push_back(v2);
    }
    else{
      v2 = v.back();
      v.pop_back();
      eat(TokenType::LBRACKET, "expecting LBRACKET");
      Expr e;
      expr(e);
      v2.array_expr = e;
      v.push_back(v2);
      eat(TokenType::RBRACKET, "expecting RBRACKET");
    }
  }
}

// TODO: Finish rest of parser based on your simple parser
// implementation

