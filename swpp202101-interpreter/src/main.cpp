#include <iostream>
#include <fstream>
#include <iomanip>

#include "parser.h"
#include "state.h"
#include "error.h"

using namespace std;


int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "USAGE: sf-interpreter <input assembly file>" << endl;
    return 1;
  }

  string filename = argv[1];
  error_filename = filename;

  Program* program = parse(filename);
  if (program == nullptr) {
    cout << "Error: cannot find " << filename << endl;
    return 1;
  }

  State state;
  state.set_program(program);
  uint64_t ret = state.exec_program();

  ofstream log("sf-interpreter.log");
  log << fixed << setprecision(4);
  log << "Returned: " << ret << endl;
  log << "Cost: " << state.get_cost_value() << endl;
  log << "Max heap usage (bytes): " << state.get_max_alloced_size() << endl;
  log.close();

  ofstream cost_log("sf-interpreter-cost.log");
  cost_log << state.get_cost()->to_string("");
  cost_log.close();

  return 0;
}
