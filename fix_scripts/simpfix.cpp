// Fix a mistake in the current version of simplify already run
// Future simplify won't need this

#include <TTree.h>
#include <TFile.h>

#include <iostream>
#include <cmath>
using namespace std;

void fix(char* filename);

int main(int argc, char** argv)
{
  cout << "Processing " << argc-1 << " files" << endl;
  for(int i=1; i<argc; ++i)
  {
    fix(argv[i]);
    cout << "Finished fixing " << argv[i] << endl;
  }

  return 0;
}

void fix(char* filename)
{
  double errorbin=-10000;
  ULong64_t errortime=0;
  ULong64_t allowedTime=30;

  // Grab the tfile, copy the tree, then write over it
  TFile file(filename);
  TTree* tree = (TTree*)file.Get("pureData");

  TFile out(filename, "recreate");
  TTree* output = new TTree("pureData", "Contains all of the data from each gate");

  // keep other trees as is
  TTree* slowTree = ((TTree*)file.Get("slowTree"))->CloneTree();
  TTree* configTree = ((TTree*)file.Get("configTree"))->CloneTree();

  double target_pmt[16][8], veto_pmt[36][8];
  ULong64_t target_time[16], veto_time[36];

  tree->SetBranchAddress("target_pmt", &target_pmt);
  tree->SetBranchAddress("veto_pmt", &veto_pmt);
  tree->SetBranchAddress("target_time", &target_time);
  tree->SetBranchAddress("veto_time", &veto_time);

  output->Branch("target_pmt", &target_pmt, "target_pmt[16][8]/D");
  output->Branch("veto_pmt", &veto_pmt, "veto_pmt[36][8]/D");
  output->Branch("target_time", &target_time, "target_time[16]/l");
  output->Branch("veto_time", &veto_time, "veto_time[36]/l");

  int entries = tree->GetEntries();
  for(int i=0; i<entries; i++)
  {
    tree->GetEvent(i);
    for(int j=32; j<36; j++)
      if( abs(double(target_time[0] - veto_time[j])) > allowedTime ) // then event is not associated
	for(int k=0; k<8; k++)
	{
	  veto_pmt[j][k]=errorbin;
	  veto_time[j]=errortime;
	}
    output->Fill();
  }

  out.Write();
  return;
}
