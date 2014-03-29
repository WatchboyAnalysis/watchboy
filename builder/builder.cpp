// This is run as a post-processing builder, to prepare for analysis
#include "Reader.h"
#include "Writer.h"
#include "Processor.h"

int main(int argc, char* argv[])
{
  // Input: List of TFiles
  RN::Reader* reader = new RN::Reader(argc, argv);
  RN::Writer* writer = new RN::Writer("output.root");
  RN::Processor* proc = new RN::Processor(reader, writer);

  proc->Process();

  delete reader;
  delete writer;
  delete proc;
  return 1;
}
