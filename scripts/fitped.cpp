#include <TTree.h>
#include <TFile.h>

#include <iostream>

int main(int argc, char** argv)
{
  TFile* file = new TFile(argv[1]);
  TTree* pureData = (TTree*)file->Get("pureData");
  int entries = pureData->GetEntries();
  double target_pmt[16][8], veto_pmt[36][8];
  pureData->SetBranchAddress("target_pmt", &target_pmt);
  pureData->SetBranchAddress("veto_pmt", &veto_pmt);

}
