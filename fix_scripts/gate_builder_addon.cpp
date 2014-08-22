// This software takes the raw, simplified data format
// from the watchboy detector data and arranges a fastTree
// which determines the charge on each PMT based on a global
// pedastal subtraction.

#define DEBUG false
#define KEEPHISTS true

// ROOT headers needed
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>

// C++ headers
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <cmath>

void process(char* filename);
unsigned long long mean(std::vector<unsigned long long> list);

int main(int argc, char** argv)
{
  std::cout << "Setup to process: " << argc-1 << " files" << std::endl;
  for(int i=1; i<argc; i++)
  {
    process(argv[i]);
    std::cout << "Finished processing " << argv[i] << std::endl;
  }
  return 0;
}

void process(char* filename)
{
  // Take time[] from pureData, and place the average in procData
  TFile file(filename, "update");
  TTree* pureData = (TTree*)file.Get("pureData");
  TTree* procData = (TTree*)file.Get("procData");

  if( !(pureData) || !(procData) )
  {
    std::cout << "Tree structure doesn't exist, exiting" << std::endl;
    return;
  }

  // We only need time from pureData so turn everything else off
  pureData->SetBranchStatus("*", 0);
  pureData->SetBranchStatus("target_time", 1);
  pureData->SetBranchStatus("veto_time", 1);
  unsigned long long target_time[16], veto_time[36];
  pureData->SetBranchAddress("target_time", &target_time);
  pureData->SetBranchAddress("veto_time", &veto_time);

  // This is a bit painful, but to add a branch to an existing tree we must
  // grab EVERYTHING from that tree as well and rewrite it
  double target_1[16], target_4Minus2[16], target_8[16], target_4Minus2Mean1[16];
  double veto_1[36], veto_4Minus2[36], veto_8[36], veto_3MinusMean1[36];
  // And the variable we want to add
  unsigned long long time;
  procData->SetBranchAddress("target_1", &target_1);
  procData->SetBranchAddress("target_4Minus2", &target_4Minus2);
  procData->SetBranchAddress("target_8", &target_8);
  procData->SetBranchAddress("target_4Minus2Mean1", &target_4Minus2Mean1);
  procData->SetBranchAddress("veto_1", &veto_1);
  procData->SetBranchAddress("veto_4Minus2", &veto_4Minus2);
  procData->SetBranchAddress("veto_8", &veto_8);
  procData->SetBranchAddress("veto_3MinusMean1", &veto_3MinusMean1);
  //procData->SetBranchAddress("time", &time);
  TBranch* timebranch = procData->Branch("time", &time);
  const int entries = procData->GetEntries();
  if(procData->GetEntries() != pureData->GetEntries())
  {
    std::cout << "procData and pureData don't match! Fix this!" << std::endl;
    return;
  }
  
  for(int i=0; i<entries; ++i)
  {
    pureData->GetEvent(i);
    procData->GetEvent(i);
    std::vector<unsigned long long> timelist;
    for(int j=0; j<16; j++)
      timelist.push_back(target_time[j]);
    // NOT USING ORPHAN PMTS!! They can cause problems
    for(int j=0; j<32; j++)
      timelist.push_back(veto_time[j]);
    time = mean(timelist);
    timebranch->Fill();
  }
  file.Write("", TObject::kOverwrite);
  return;
}

unsigned long long mean(std::vector<unsigned long long> list)
{
  long double avg=0;
  long double size=list.size();
  for(auto it=list.begin();
      it!=list.end(); ++it)
    avg += static_cast<long double>(*it)/size;

  return static_cast<unsigned long long>(avg);
}
