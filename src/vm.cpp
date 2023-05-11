//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"
#include <chrono>
#include <cmath>
#include <iomanip>

using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}


void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}


void VM::run(bool DEBUG, bool per_profile, bool trace)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);
  auto start = chrono::high_resolution_clock::now();
  auto interval = 1us;
  unordered_map<string, int> function_count;
  vector<VMInstr> trace_instru;
  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];
    
    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      // TODO
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    if(per_profile){
      auto stop = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
      if(duration > interval){
        try{
          function_count.at(frame->info.function_name) += 1;
        }
        catch(exception e){
          function_count.insert({frame->info.function_name, 1});
        }
      }
    }

    if(trace){
      trace_instru.push_back(instr);
    }
    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }

    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }

    // TODO: Finish LOAD and STORE
    else if(instr.opcode() == OpCode::STORE){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      int temp = get<int>(instr.operand().value());
      if(temp >= frame->variables.size()){
      frame->variables.push_back(x);
      }
      else{
        frame->variables[temp] = x;
      }
    }

    else if(instr.opcode() == OpCode::LOAD){
      VMValue x = frame->variables.at(get<int>(instr.operand().value()));
      frame->operand_stack.push(x);
    }
    
    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    // TODO: Finish SUB, MUL, DIV, AND, OR, NOT, COMPLT, COMPLE,
    // CMPGT, CMPGE, CMPEQ, CMPNE
    else if(instr.opcode() == OpCode::SUB){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }

    else if(instr.opcode() == OpCode::MUL){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }

    else if(instr.opcode() == OpCode::DIV){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }
    
    else if(instr.opcode() == OpCode::AND){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(y) && get<bool>(x));
    }

    else if(instr.opcode() == OpCode::NOT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(x));
    }

    else if(instr.opcode() == OpCode::OR){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(y) || get<bool>(x));
    }

    else if(instr.opcode() == OpCode::CMPLE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }

    else if(instr.opcode() == OpCode::CMPGE){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }
    
    else if(instr.opcode() == OpCode::CMPLT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }

    else if(instr.opcode() == OpCode::CMPGT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }

    else if(instr.opcode() == OpCode::CMPEQ){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y, x));
    }

    else if(instr.opcode() == OpCode::CMPNE){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(eq(y, x)));
    }
    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    // TODO: Finish JMP and JMPF
    else if(instr.opcode() == OpCode::JMP){
      frame->pc = get<int>(instr.operand().value());
    }

    else if(instr.opcode() == OpCode::JMPF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if (get<bool>(x) == false)
      {
        frame->pc = get<int>(instr.operand().value());
      }    
    }
    
    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------


    // TODO: Finish CALL, RET
    else if(instr.opcode() == OpCode::CALL){
      string name = get<string>(instr.operand().value());
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      new_frame->info = frame_info[name];
      for(int i = 0; i < frame_info[name].arg_count; i++)
      {
        VMValue x = frame->operand_stack.top();
        new_frame->operand_stack.push(x);
        frame->operand_stack.pop();
      }
      call_stack.push(new_frame);
      frame = new_frame;
    }

    else if(instr.opcode() == OpCode::RET){
      VMValue v = frame->operand_stack.top();
      frame->operand_stack.pop();
      call_stack.pop();
      if(!call_stack.empty())
      {
        frame = call_stack.top();
        frame->operand_stack.push(v);
      }
    }
    
    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }

    // TODO: Finish SLEN, ALEN, GETC, TODBL, TOSTR, CONCAT
    else if (instr.opcode() == OpCode::SLEN){
      VMValue x = frame->operand_stack.top(); 
      ensure_not_null(*frame, x);
      frame->operand_stack.pop(); 
      string s = get<string>(x);
      int size = s.size();
      frame->operand_stack.push(size);  
    }
    
    else if(instr.opcode() == OpCode::ALEN){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      int i = get<int>(x);
      vector<VMValue> array = array_heap[i];
      int size = array.size();
      frame->operand_stack.push(size);
    }

    //work in progress
    else if(instr.opcode() == OpCode::GETC){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      string s = to_string(x);
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      int index = get<int>(y);
      if(index >= s.size() || index < 0){
        error("out-of-bounds string index", *frame);
      }
      else{
      string c = (s.substr(index, 1));
      frame->operand_stack.push(c);
      }
    }

    else if(instr.opcode() == OpCode::TODBL){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      if(holds_alternative<int>(x)){
        int i = get<int>(x);
        double d = (double)i;
        frame->operand_stack.push(d); 
      }
      else if(holds_alternative<string>(x)){
        try{
          string s = get<string>(x);
          double d = stod(s);
          frame->operand_stack.push(d);
        }
        catch(...){
          error("cannot convert string to double", *frame);
        }
      }
    }

    else if(instr.opcode() == OpCode::TOINT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      if(holds_alternative<string>(x)){
        try{
          string s = get<string>(x);
          int i = stoi(s);
          frame->operand_stack.push(i);
        }
        catch(...)
        {
          error("cannot convert string to int", *frame);
        }
      }
      else if(holds_alternative<double>(x)){
        double d = get<double>(x);
        int i = (int)d;
        frame->operand_stack.push(i);
      }
    }

    else if(instr.opcode() == OpCode::TOSTR){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      if(holds_alternative<int>(x)){
        int i = get<int>(x);
        string s = to_string(i);
        frame->operand_stack.push(s);
      }
      else if(holds_alternative<double>(x)){
        double d = get<double>(x);
        string s = to_string(d);
        frame->operand_stack.push(s);
      }
    }

    else if(instr.opcode() == OpCode::CONCAT){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      string string_one = get<string>(x);
      VMValue y = frame->operand_stack.top();  
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      string string_two = get<string>(y);
      string concat = string_two + string_one;
      frame->operand_stack.push(concat);
    }
    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------

    // TODO: Finish ALLOCS, ALLOCA, ADDF, SETF, GETF, SETI, GETI
    else if(instr.opcode() == OpCode::ALLOCS){
      struct_heap[next_obj_id] = {};
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }

    else if(instr.opcode() == OpCode::ALLOCA){
      VMValue val = frame->operand_stack.top();
      frame->operand_stack.pop();
      int size = get<int>(frame->operand_stack.top());
      frame->operand_stack.pop();
      array_heap[next_obj_id] = vector<VMValue>(size, val);
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }
    
    else if(instr.opcode() == OpCode::ADDF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x); 
      int obj_id = get<int>(x);
      string field_name = get<string>(instr.operand().value());
      frame->operand_stack.pop();
      struct_heap.at(obj_id).emplace(field_name, nullptr);
    }

    else if(instr.opcode() == OpCode::SETF){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      string fieldname = get<string>(instr.operand().value());;
      int obj_id = get<int>(y);
      struct_heap[obj_id][fieldname] = x;
    }

    else if(instr.opcode() == OpCode::GETF){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int obj_id = get<int>(x);
      string fieldname = get<string>(instr.operand().value());;
      frame->operand_stack.push(struct_heap[obj_id][fieldname]);
    }

    else if(instr.opcode() == OpCode::SETI){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      int index = get<int>(y);
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();
      int obj_id = get<int>(z);
      if(index >= array_heap[obj_id].size() || index < 0){
        error("out-of-bounds array index", *frame);
      }
      array_heap[obj_id].at(index) = x;
    }

    else if(instr.opcode() == OpCode::GETI){
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      int index = get<int>(x);
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      int obj_id = get<int>(y);
      if(index >= array_heap[obj_id].size() || index < 0){
        error("out-of-bounds array index", *frame);
      }
      frame->operand_stack.push(array_heap[obj_id].at(index));
    }
    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
  if(per_profile){
    double sum = 0;
    for(auto function : function_count){
      sum += function.second;
    }
    cout << "Time based perfomance profiler" << endl;
    for(auto function : function_count){
      string function_name = function.first;
      cout << function_name + ", " + to_string((function.second / sum)*100).substr(0,5) + "%" << endl;
    }
  }

  if(trace){
    cout << "------------------------------------------" << endl;
    cout << "This is the trace of the program execution" << endl;
    cout << "------------------------------------------" << endl;
    for(auto& instru : trace_instru){
      cout << to_string(instru) << endl;
    }
  }
}

void VM::sprof(){
  double total_instr = 0;
  for(auto& function_frame : frame_info){
    VMFrameInfo curr_frame = function_frame.second;
    total_instr += curr_frame.instructions.size();
  }
  cout << "Welcome to the profiler" << endl;
  cout << "This is the percent of instructions created by each funtion" << endl;
  for(auto& function_frame : frame_info){
    string function_name = function_frame.first;
    VMFrameInfo curr_frame = function_frame.second;
    cout << function_name + ", " + to_string((curr_frame.instructions.size() / total_instr)*100).substr(0,5) + "%" << endl;
  }
}

void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}


VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}

// TODO: Finish the rest of the following arithmetic operators

VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) - get<int>(y);
  else
    return get<double>(x) - get<double>(y);
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) * get<int>(y);
  else
    return get<double>(x) * get<double>(y);
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) / get<int>(y);
  else
    return get<double>(x) / get<double>(y);
}


VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) == get<string>(y);
  else
    return get<bool>(x) == get<bool>(y);
}

// TODO: Finish the rest of the comparison operators

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) < get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) < get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) < get<string>(y);
  else
    return get<bool>(x) < get<bool>(y);
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) <= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) <= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) <= get<string>(y);
  else
    return get<bool>(x) <= get<bool>(y);
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) > get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) > get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) > get<string>(y);
  else
    return get<bool>(x) > get<bool>(y);
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) >= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) >= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) >= get<string>(y);
  else
    return get<bool>(x) >= get<bool>(y);
}

