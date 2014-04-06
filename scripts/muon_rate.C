#include <string>

void muon_rate(std::string fname)
{
  TFile* file=new TFile(fname.c_str());
  TTree* tree =(TTree*)file->Get("data");
  unsigned long long time=0;
  double target_total, veto_total, target_cb, veto_cb;
  tree->SetBranchAddress("time", &time);
  tree->SetBranchAddress("target_total", &target_total);
  tree->SetBranchAddress("veto_total", &veto_total);
  tree->SetBranchAddress("target_cb", &target_cb);
  tree->SetBranchAddress("veto_cb", &veto_cb);

  unsigned long long tlast=0;
  unsigned int entries = tree->GetEntries();
  double adc2us = 4/1000.0;

  TH1F* timing=new TH1F("timing", "timing", 100, 0, 5000);

  for(int i=0; i<entries; i++)
  {
    tree->GetEvent(i);
    if( target_cb < 0.8 && target_total > 1000 )
    {
      timing->Fill(double(time-tlast)*adc2us);
      tlast = time;
    }
  }

  timing->Draw();
}
