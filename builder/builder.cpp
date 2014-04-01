// This is run as a post-processing builder, to prepare for analysis
#include "Reader.h"
#include "Writer.h"
#include "Processor.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  // Ask for output name:
  std::string output_name;
  std::cout << "Output file name: ";
  std::cin >> output_name;

  // Input: List of TFiles
  RN::Reader* reader = new RN::Reader(argc, argv);
  RN::Writer* writer = new RN::Writer(output_name);
  RN::Processor* proc = new RN::Processor(reader, writer);

  proc->Process();

  delete reader;
  delete writer;
  delete proc;
  return 1;
}
