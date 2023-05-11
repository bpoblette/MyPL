//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>             // for debugging
#include "code_generator.h"

using namespace std;


// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}

//complete
void CodeGenerator::visit(FunDef& f)
{
  string name = f.fun_name.lexeme();
  int param_size = f.params.size();
  VMFrameInfo new_frame{name, param_size};
  curr_frame = new_frame;
  var_table.push_environment();
  int i = 0;
  for(auto& params : f.params){
    curr_frame.instructions.push_back(VMInstr::STORE(i));
    var_table.add(params.var_name.lexeme());
    i++;
  }
  for(auto& stmt : f.stmts){
    stmt->accept(*this);
  }
  if(curr_frame.instructions.empty() || curr_frame.instructions.back().opcode() != OpCode::RET){
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());
  }
  vm.add(curr_frame);
  var_table.pop_environment();
}

//complete
void CodeGenerator::visit(StructDef& s)
{
  struct_defs[s.struct_name.lexeme()] = s;
}

//complete
void CodeGenerator::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::RET());
}

//complete
void CodeGenerator::visit(WhileStmt& s)
{
  int starting_index = curr_frame.instructions.size();
  s.condition.accept(*this);
  int jmpf_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto& stmt : s.stmts){
    stmt->accept(*this);
  }
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(starting_index));
  int nop_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions.at(jmpf_index).set_operand(nop_index);
  }

//complete
void CodeGenerator::visit(ForStmt& s)
{
  var_table.push_environment();
  s.var_decl.accept(*this);
  int starting_index = curr_frame.instructions.size();
  s.condition.accept(*this);
  int jmpf_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto& stmt : s.stmts){
    stmt->accept(*this);
  }
  var_table.pop_environment();
  s.assign_stmt.accept(*this);
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(starting_index));
  curr_frame.instructions.push_back(VMInstr::NOP());
  int nop_index = curr_frame.instructions.size() - 1;
  curr_frame.instructions.at(jmpf_index).set_operand(nop_index);  
}

//complete
void CodeGenerator::visit(IfStmt& s)
{
  vector<int> jmp_instr;
  s.if_part.condition.accept(*this);
  int jmpf_instr_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto& stmt : s.if_part.stmts){
    stmt->accept(*this);
  }
  var_table.pop_environment();
  jmp_instr.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::JMP(-1));
  int jmp_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions.at(jmpf_instr_index).set_operand(jmp_index);

  if(s.else_ifs.size() > 0){
    for(auto& st : s.else_ifs){
      st.condition.accept(*this);
      jmpf_instr_index = curr_frame.instructions.size();
      curr_frame.instructions.push_back(VMInstr::JMPF(-1));
      var_table.push_environment();
      for(auto& stmt : st.stmts){
        stmt->accept(*this);
      }
      var_table.pop_environment();
      jmp_instr.push_back(curr_frame.instructions.size());
      curr_frame.instructions.push_back(VMInstr::JMP(-1));
      int new_index = curr_frame.instructions.size();
      curr_frame.instructions.at(jmpf_instr_index).set_operand(new_index);
      curr_frame.instructions.push_back(VMInstr::NOP());
    }
  }
  if(s.else_stmts.size() > 0){
      for (auto& stmt: s.else_stmts) {
        stmt->accept(*this);
      }
      jmp_instr.push_back(curr_frame.instructions.size());
      curr_frame.instructions.push_back(VMInstr::JMP(-1));
  }

  int end = curr_frame.instructions.size();
  for (auto& index : jmp_instr) {
    curr_frame.instructions.at(index).set_operand(end);
  }
  curr_frame.instructions.push_back(VMInstr::NOP());
}

//complete
void CodeGenerator::visit(VarDeclStmt& s)
{
  s.expr.accept(*this);
  var_table.add(s.var_def.var_name.lexeme());
  curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.var_def.var_name.lexeme())));
}

//complete
void CodeGenerator::visit(AssignStmt& s)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue[0].var_name.lexeme())));
  for(int i = 0; i < s.lvalue.size(); i++){
    VarRef v = s.lvalue[i];
    string v_vame = v.var_name.lexeme();
    if(i != 0){
      curr_frame.instructions.push_back(VMInstr::GETF(v_vame));
    }
    if(v.array_expr.has_value()){
      v.array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
  curr_frame.instructions.pop_back();
  s.expr.accept(*this);
  if(s.lvalue.size() > 1 && s.lvalue.back().array_expr == nullopt){
    curr_frame.instructions.push_back(VMInstr::SETF(s.lvalue.back().var_name.lexeme()));
  }
  else if(s.lvalue.back().array_expr != nullopt){
    curr_frame.instructions.push_back(VMInstr::SETI());
  }
  else{
    curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.lvalue.back().var_name.lexeme())));
  }

}

//work in progress
void CodeGenerator::visit(CallExpr& e)
{
  for(auto& args : e.args){
    args.accept(*this);
  }

  if(e.fun_name.lexeme() == "print"){
    curr_frame.instructions.push_back(VMInstr::WRITE());
  }
  else if(e.fun_name.lexeme() == "length"){
    curr_frame.instructions.push_back(VMInstr::SLEN());
  }
  else if(e.fun_name.lexeme() == "get"){
    curr_frame.instructions.push_back(VMInstr::GETC());
  }
  else if(e.fun_name.lexeme() == "input"){
    curr_frame.instructions.push_back(VMInstr::READ());
  }
  else if(e.fun_name.lexeme() == "concat"){
    curr_frame.instructions.push_back(VMInstr::CONCAT());
  }
  else if(e.fun_name.lexeme() == "to_string"){
    curr_frame.instructions.push_back(VMInstr::TOSTR());
  }
  else if(e.fun_name.lexeme() == "to_int"){
    curr_frame.instructions.push_back(VMInstr::TOINT());
  }
  else if(e.fun_name.lexeme() == "to_double"){
    curr_frame.instructions.push_back(VMInstr::TODBL());
  }

  else{
    curr_frame.instructions.push_back(VMInstr::CALL(e.fun_name.lexeme()));
  }
}

//complete
void CodeGenerator::visit(Expr& e)
{
  e.first->accept(*this);
  if(e.op.has_value()){
    e.rest->accept(*this);
    if(e.op.value().type() == (TokenType::PLUS)){
      curr_frame.instructions.push_back(VMInstr::ADD());
    }
    else if(e.op.value().type() == TokenType::MINUS){
      curr_frame.instructions.push_back(VMInstr::SUB());
    }
    else if(e.op.value().type() == TokenType::DIVIDE){
      curr_frame.instructions.push_back(VMInstr::DIV());  
    }
    else if(e.op.value().type() == TokenType::TIMES){
      curr_frame.instructions.push_back(VMInstr::MUL());
    }
    else if(e.op.value().type() == TokenType::AND){
      curr_frame.instructions.push_back(VMInstr::AND());
    }
    else if(e.op.value().type() == TokenType::OR){
      curr_frame.instructions.push_back(VMInstr::OR());
    }
    else if(e.op.value().type() == TokenType::GREATER){
      curr_frame.instructions.push_back(VMInstr::CMPGT());
    }
    else if(e.op.value().type() == TokenType::GREATER_EQ){
      curr_frame.instructions.push_back(VMInstr::CMPGE());
    }
    else if(e.op.value().type() == TokenType::LESS){
      curr_frame.instructions.push_back(VMInstr::CMPLT());
    }
    else if(e.op.value().type() == TokenType::LESS_EQ){
      curr_frame.instructions.push_back(VMInstr::CMPLE());
    }
    else if(e.op.value().type() == TokenType::EQUAL){
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
    }
    else if(e.op.value().type() == TokenType::NOT_EQUAL){
      curr_frame.instructions.push_back(VMInstr::CMPNE());
    }
  }
  if(e.negated){
    curr_frame.instructions.push_back(VMInstr::NOT());
  }
}

//complete
void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}
 
//complete
void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}

//complete
void CodeGenerator::visit(SimpleRValue& v)
{
  if(v.value.type() == TokenType::INT_VAL) {
    int val = stoi(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if(v.value.type() == TokenType::DOUBLE_VAL){
    double val = stod(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if (v.value.type() == TokenType::NULL_VAL){
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
  }
  else if (v.value.type() == TokenType::BOOL_VAL){
    if(v.value.lexeme() == "true"){
      curr_frame.instructions.push_back(VMInstr::PUSH(true));
    }
    else{
      curr_frame.instructions.push_back(VMInstr::PUSH(false));
    }
  }
  else if(v.value.type() == TokenType::STRING_VAL){
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t", "\t");
    replace_all(s, "\\r", "\r");
    replace_all(s, "\\\\", "\\");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));
  }
  else if(v.value.type () == TokenType::CHAR_VAL){
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t", "\t");
    replace_all(s, "\\r", "\r");
    replace_all(s, "\\\\", "\\");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));
  }
}

//work in progress
void CodeGenerator::visit(NewRValue& v)
{
  if(v.array_expr.has_value()){
    v.array_expr->accept(*this);
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::ALLOCA());
  }
  else{
    curr_frame.instructions.push_back(VMInstr::ALLOCS());
    for(auto& field : struct_defs[v.type.lexeme()].fields){
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::ADDF(field.var_name.lexeme()));
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETF(field.var_name.lexeme()));
    }
  }
}

//completed
void CodeGenerator::visit(VarRValue& v)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(v.path[0].var_name.lexeme())));
  for(int i = 0; i < v.path.size(); i++){
    VarRef r = v.path[i];
    string var_name = r.var_name.lexeme();
    if(i != 0){
      curr_frame.instructions.push_back(VMInstr::GETF(var_name));
    } 
    if(r.array_expr.has_value()){
      r.array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
}
    

