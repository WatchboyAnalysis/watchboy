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
void fill_slowTree(char* fname);

int main(int argc, char* argv[])
{
  for(int i=1; i<argc; i++)
  {
    process(argv[i]);
    fill_slowTree(argv[i]);
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
	      << fname << std::endl;
    return;
  }
  
  if(tree->GetLeaf("gateCharge"))
  {
    std::cout << "already processed " << fname << std::endl;
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
      std::cout << "\033[31;1m"<< std::floor(double(i)/entries*100) 
		<< "%\033[37;0m\r" << std::flush;
  }
  std::cout << "" << std::endl;

  for(int pmt=0; pmt<numPmts; pmt++)
  {
    TH1F* currentHist = (TH1F*)hists->At(pmt);
    TF1* g_fit = new TF1("g_fit", "[0]*TMath::Gaus(x, [1], [2])", -100, 100);
    g_fit->SetParameters(currentHist->GetMaximum(), 0, 20);
    currentHist->Fit(g_fit, "Q+", "", -75, 75);
  }

  gateChargeBranch->Fill();
  proc->SetBranchStatus("*", 1);
  file->Write("", TObject::kOverwrite);

  delete file;
  return;
}

void fill_slowTree(char* fname)
{
  TFile* file = new TFile(fname, "update");
  TTree* slowTree = (TTree*)file->Get("slowTree");
  TTree* histTree = (TTree*)file->Get("histTree");

  if(!histTree)
    return;

  if(slowTree->GetLeaf("gateAmp"))
  {
    std::cout << "Already modified slowTree in file: "
	      << fname << std::endl;
    return;
  }

  double target_voltages[16], target_currents[16], veto_voltages[36],
    veto_currents[36], target_gate_start[8], target_gate_end[8],
    target_gate_width[8], veto_gate_start[8], veto_gate_end[8],
    veto_gate_width[8], fit_amplitudes[52], fit_means[52], fit_std_devs[52],
    fit_chi2perndf[52];
  slowTree->SetBranchAddress("target_voltages", &target_voltages);
  slowTree->SetBranchAddress("target_currents", &target_currents);
  slowTree->SetBranchAddress("veto_voltages", &veto_voltages);
  slowTree->SetBranchAddress("veto_currents", &veto_currents);
  slowTree->SetBranchAddress("target_gate_start", &target_gate_start);
  slowTree->SetBranchAddress("target_gate_end", &target_gate_end);
  slowTree->SetBranchAddress("target_gate_width", &target_gate_width);
  slowTree->SetBranchAddress("veto_gate_start", &veto_gate_start);
  slowTree->SetBranchAddress("veto_gate_end", &veto_gate_end);
  slowTree->SetBranchAddress("veto_gate_width", &veto_gate_width);
  slowTree->SetBranchAddress("fit_amplitudes", &fit_amplitudes);
  slowTree->SetBranchAddress("fit_means", &fit_means);
  slowTree->SetBranchAddress("fit_std_devs", &fit_std_devs);
  slowTree->SetBranchAddress("fit_chi2perndf", &fit_chi2perndf);
  slowTree->GetEvent(0);

  // New branches
  double gateAmp[52], gateMean[52], gateDev[52], gateChi[52];
  TBranch* AmpBr = slowTree->Branch("gateAmp", &gateAmp, "gateAmp[52]/D");
  TBranch* MeanBr = slowTree->Branch("gateMean", &gateMean, "gateMean[52]/D");
  TBranch* DevBr = slowTree->Branch("gateDev", &gateDev, "gateDev[52]/D");
  TBranch* ChiBr = slowTree->Branch("gateChi", &gateChi, "gateChi[52]/D");

  TObjArray* gateCharge=0;
  histTree->SetBranchAddress("gateCharge", &gateCharge);
  histTree->GetEvent(0);

  int entries = slowTree->GetEntries();
  for(int i=0; i<entries; i++)
  {
    for(int pmt=0; pmt<52; pmt++)
    {
      TF1* fit=((TH1F*)gateCharge->At(pmt))->GetFunction("g_fit");
      gateAmp[pmt]=double(fit->GetParameter(0));
      gateMean[pmt]=double(fit->GetParameter(1));
      gateDev[pmt]=double(fit->GetParameter(2));
      gateChi[pmt]=double(fit->GetChisquare() / fit->GetNDF());
    }
    AmpBr->Fill();
    MeanBr->Fill();
    DevBr->Fill();
    ChiBr->Fill();
  }

  file->Write("", TObject::kOverwrite);
  delete file;
  return; 
}
