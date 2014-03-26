#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <string>
#include <vector>
#include <cmath>

void process(char* fname);

int main(int argc, char* argv[])
{
  for(int i=1; i<argc; i++)
  {
    process(argv[i]);
    std::cout << "Processed: " << argv[i] << std::endl;
  }
  return 0;
}

void process(char* fname)
{
  TFile* file = new TFile(fname, "update");
  TTree* proc = (TTree*)file->Get("procData");
  TTree* tree = (TTree*)file->Get("histTree");
  
  if(!tree)
  {
    std::cout << "histTree doesn't exist in "
	      << file << std::endl;
    return;
  }
  
  
  proc->SetBranchStatus("*", 0);
  proc->SetBranchStatus("target_4Minus2Mean1", 1);
  proc->SetBranchStatus("veto_4Minus2Mean1", 1);

  double target[16], veto[36];
  proc->SetBranchAddress("target_4Minus2Mean1", &target);
  proc->SetBranchAddress("veto_4Minus2Mean1", &veto);

  int entries = proc->GetEntries();
  const int numPmts = 52;
  TObjArray* hists = new TObjArray();
  TObjArray* pedestal = 0;
  tree->SetBranchAddress("pedestalCharge", &pedestal);
  TBranch* gateChargeBranch = tree->Branch("gateCharge", &hists);
  tree->GetEvent(0);

  for(long long int i=0; i<numPmts; i++)
  {
    std::string histname = "4Minus2Mean1["+std::to_string(i)+"]";
    hists->Add(new TH1F(histname.c_str(), "charge balance", 1000, -500, 500));
    ((TH1F*)hists->At(i))->SetDirectory(0);
  }

  for(int i=0; i<entries; i++)
  {
    proc->GetEvent(i);
    for(int pmt=0; pmt<numPmts; pmt++)
    {
      TH1F* currentHist = (TH1F*)hists->At(pmt);
      if(pmt<16)
	currentHist->Fill(target[pmt]);
      else
	currentHist->Fill(veto[pmt-16]);
    }
    if(!(i%10000))
      std::cout << std::floor(double(i)/entries*100) << "%\r" << std::flush;
  }
  std::cout << std::endl;

  for(int pmt=0; pmt<numPmts; pmt++)
  {
    TF1* g_fit = new TF1("g_fit", "[0]*TMath::Gaus(x, [1], [2])", -100, 100);
    g_fit->SetParameters(1000, 0, 20);
    TH1F* currentHist = (TH1F*)hists->At(pmt);
    currentHist->Fit(g_fit, "Q+", "", -75, 75);
  }

  gateChargeBranch->Fill();
  proc->SetBranchStatus("*", 1);
  file->Write("", TObject::kOverwrite);

  return;
}
