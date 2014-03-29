#ifndef RNPROCESSOR_h
#define RNPROCESSOR_h

#include <vector>
#include "Reader.h"
#include "Writer.h"

namespace RN
{
  class Processor
  {
    RN::Reader* mReader;
    RN::Writer* mWriter;
    // Input
    //double target_charge[16];
    //double veto_charge[36];
    std::vector<double> target_charge;
    std::vector<double> veto_charge;
    unsigned long long time;	// shared I/O
    // Output
    double target_total;
    double veto_total;
    double target_cb;		// charge balance
    double veto_cb;
    // Cut Values
    double target_cb_cut;
    double veto_cb_cut;
    int target_charge_cut;
    int veto_charge_cut;
    // current event
    int target_hits;
    int veto_hits;
  public:
    Processor(RN::Reader* reader, RN::Writer* writer);
    ~Processor();
    
    void Process();
    void GetVariables();
    void ChargeBalance();
    void SumCharge();
    void FillWriter();
  };
}

#endif
