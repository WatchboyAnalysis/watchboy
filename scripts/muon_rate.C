#include <string>
#include <sstream>

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
  double adc2us = 4/1000.0;	// converts adc to microseconds
  double axis_rescale = 1e-6;	// converts microseconds to seconds
  double max_time = 5;		// seconds

  TH1D* timing=new TH1D("timing", "timing", 100, 0, max_time);

  for(int i=0; i<entries; i++)
  {
    tree->GetEvent(i);
    if( target_cb < 0.15 && veto_cb < 0.45 && veto_total > 5e4 && target_total > 1e5 )
    {
      timing->Fill(double(time-tlast)*adc2us*axis_rescale);
      tlast = time;
    }
  }

  // Fit
  TF1* fit = new TF1("fit", "[0]*TMath::Exp(-[1]*x)", 0, max_time);
  fit->SetParameters(1000, 1);
  timing->Fit(fit, "N");

  // Legend
  TLegend* l1 = new TLegend(0.55, 0.80, 0.88, 0.88);
  stringstream ss;
  ss << fit->GetParameter(1);
  string leg = "Muon rate: "+(ss.str()).substr(0, 4)+" Hz";
  l1->AddEntry(fit, leg.c_str());
  l1->SetTextSize(0.04);

  // Set up the timing parameters
  gStyle->SetOptStat(0);
  TCanvas* c1=new TCanvas();
  c1->SetLogy();
  timing->SetTitle("Time between muon events");
  timing->GetXaxis()->SetTitle("Seconds");
  fit->SetLineColor(kRed);
  timing->SetLineColor(kBlack);
  timing->Draw("E");
  fit->Draw("same");
  l1->Draw();
}
