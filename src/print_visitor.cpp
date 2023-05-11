//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include "print_visitor.h"

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}

// TODO: Finish the visitor functions

  void PrintVisitor::visit(FunDef& f)
  {
    print_indent();
    out << f.return_type.type_name + " " + f.fun_name.lexeme() + "(";
    for(int i = 0; i < f.params.size(); i++)
    {
      if(i > 0)
      {
        out << ", ";
      }
      out << f.params[i].var_name.lexeme(); 
    }
    out << ") {";
    inc_indent();
    for(int i = 0; i < f.stmts.size(); i++)
    {
      print_indent();
      f.stmts[i];
      out << "\n";
    }
    dec_indent();
    print_indent();
    out << "}";
    out << "\n";
  }

  void PrintVisitor::visit(StructDef& s)
  {
    print_indent();
    out << "struct " + s.struct_name.lexeme() + " " + "{";
    for(int i = 0; i < s.fields.size(); i++)
    {
      
    }
  }

  void PrintVisitor::visit(ReturnStmt& s)
  {

  }
  void PrintVisitor::visit(WhileStmt& s)
  {

  }
  void PrintVisitor::visit(ForStmt& s)
  {

  }
  void PrintVisitor::visit(IfStmt& s)
  {

  }
  void PrintVisitor::visit(VarDeclStmt& s)
  {

  }
  void PrintVisitor::visit(AssignStmt& s)
  {

  }
  void PrintVisitor::visit(CallExpr& e)
  {
    
  }
  void PrintVisitor::visit(Expr& e)
  {

  }
  void PrintVisitor::visit(SimpleTerm& t)
  {
    
  } 
  void PrintVisitor::visit(ComplexTerm& t)
  {

  }
  void PrintVisitor::visit(SimpleRValue& v)
  {

  }
  void PrintVisitor::visit(NewRValue& v)
  {

  }
  void PrintVisitor::visit(VarRValue& v)
  {
    
  }   