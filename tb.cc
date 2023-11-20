// Copyright 2023 Flavien Solt, ETH Zurich.
// Licensed under the General Public License, Version 3.0, see LICENSE for details.
// SPDX-License-Identifier: GPL-3.0-only

#include "Vtop.h"
#include "verilated.h"
#include "ticks.h"

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <fstream>
#include <cassert>

#if VM_TRACE
#if VM_TRACE_FST
#include <verilated_fst_c.h>
#else
#include <verilated_vcd_c.h>
#endif // VM_TRACE_FST
#endif // VM_TRACE

// This is a generated header
#include "interface_sizes.h"

typedef Vtop Module;

size_t curr_id_in_inputs_from_file = 0;
std::vector<uint32_t> inputs_from_file;

#if VM_TRACE
const int kTraceLevel = 6;
#if VM_TRACE_FST
  VerilatedFstC *trace_;
#else
  VerilatedVcdC *trace_;
#endif // VM_TRACE_FST
#endif // VM_TRACE

void read_inputs_from_file(int simlen) {
  // Make sure we call this function only once
  assert(inputs_from_file.size() == 0);

  std::ifstream in_file(PATH_TO_RANDOM_INPUTS_FILE);
  int expected_num_ints = FULL_RANDOM ? simlen * IN_DATA_WIDTH / 32 : simlen;
  uint64_t next_random_input = 0;
  while (in_file >> next_random_input && inputs_from_file.size() < expected_num_ints) {
      inputs_from_file.push_back(next_random_input);
  }
  assert(inputs_from_file.size() == expected_num_ints);
}

void randomize_inputs(Module *my_module) {
#if FULL_RANDOM == 1
  for (int i = 0; i < IN_DATA_WIDTH / 32; i++) {
    my_module->in_data[i] = inputs_from_file[curr_id_in_inputs_from_file++];
  }
#else
  int random_input = inputs_from_file[curr_id_in_inputs_from_file++];
  for (int i = 0; i < IN_DATA_WIDTH / 32; i++)
    my_module->in_data[i] = random_input + i;
#endif
}

/**
 * Runs the testbench.
 *
 * @param tb a pointer to a testbench instance
 * @param simlen the number of cycles to run
 */
std::pair<long, uint64_t> run_test(Module *my_module, int simlen, const std::string trace_filename) {
  
  srand(time(NULL)); // set random seed to current time
  uint64_t cumulated_output = 0;
  auto start = std::chrono::steady_clock::now();

#if VM_TRACE
#if VM_TRACE_FST
    trace_ = new VerilatedFstC;
#else
    trace_ = new VerilatedVcdC;
#endif // VM_TRACE_FST
    my_module->trace(trace_, kTraceLevel);
    trace_->open(trace_filename.c_str());
    size_t tick_count_ = 0;
    trace_->dump(tick_count_++);
#endif // VM_TRACE

  for (int tick_id = 0; tick_id < simlen; tick_id++) {
    randomize_inputs(my_module);
    my_module->eval();
#if VM_TRACE
      trace_->dump(tick_count_++);
#endif // VM_TRACE

    for (int word_id = 0; word_id < OUT_DATA_WIDTH / 32; word_id++) {
      cumulated_output += my_module->out_data[word_id];
      std::cout << "output [" << (32*(word_id+1)-1) << ":" << (32*word_id) << "] (tick " << tick_id << ") = 0x" << std::hex << my_module->out_data[word_id] << std::dec << std::endl;
    }
  }

#if VM_TRACE
      trace_->flush();
#endif // VM_TRACE

  auto stop = std::chrono::steady_clock::now();
  long ret = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
  return std::make_pair(ret, cumulated_output);
}

int main(int argc, char **argv, char **env) {

  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(VM_TRACE);

  ////////
  // Instantiate the module.
  ////////

  Module *my_module = new Module;

  ////////
  // Get the env vars early to avoid Verilator segfaults.
  ////////

  std::cout << "Starting getting env variables." << std::endl;
  int simlen = get_sim_length_cycles(0);
  read_inputs_from_file(simlen);
  std::string vcd_filepath = cl_get_tracefile();

  ////////
  // Run the experiment.
  ////////

  std::pair<long, uint64_t> duration_and_output = run_test(my_module, simlen, vcd_filepath);
  long duration = duration_and_output.first;
  uint64_t cumulated_output = duration_and_output.second;

  std::cout << "Testbench complete!" << std::endl;
  std::cout << "Output signature: " << std::dec << cumulated_output << "." << std::endl;
  std::cout << "Elapsed time: " << std::dec << duration << "." << std::endl;

  delete my_module;
  exit(0);
}
