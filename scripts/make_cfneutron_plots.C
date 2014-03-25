#include <string>
#include <sstream>
using namespace std;

void make_cfneutron_plots(string fname)
{
// Pretty colors
  const int nrgb = 5;
  const int ncont = 256;
  double stops[nrgb] = {0.00, 0.30, 0.61, 0.84, 1.00 };
  double red[nrgb] = {0.00, 0.00, 0.57, 0.90, 0.51 };
  double green[nrgb] = {0.00, 0.65, 0.95, 0.20, 0.00 };
  double blue [nrgb] = {0.51, 0.55, 0.15, 0.00, 0.10 };
  TColor::CreateGradientColorTable(nrgb, stops, red, green, blue, ncont);
  gStyle->SetNumberContours(ncont);
  gStyle->SetOptStat(0);

  TFile* file = new TFile(fname.c_str());
  TCanvas* c1 = new TCanvas();

  TH1I* mult = file->Get("mult");
  TH1F* timing = file->Get("timing");
  TH1F* nspec = file->Get("nspec");
  TH1F* nhspec = file->Get("nhspec");
  TH1F* externspec = file->Get("externspec");
  TH1F* pnspec = file->Get("pureNeutrons");

  nspec->SetLineColor(kBlack);
  nhspec->SetLineColor(kBlue);
  externspec->SetLineColor(kGreen);
  pnspec->SetLineColor(kRed);

  c1->Divide(2,2);
  c1->cd(1);
  c1->GetPad(1)->SetLogy();
  TLegend* specLegend = new TLegend(0.5, 0.67, 0.88, 0.88);
  nspec->GetXaxis()->SetRange(0, 20000 / nspec->GetBinWidth(0));
  nspec->SetTitle("Neutron Energy Spectrum");
  nspec->GetXaxis()->SetTitle("ADC Integrated Charge");
  nspec->Draw();
  c1->Update();
  specLegend->AddEntry(nspec, "#Deltat < 50 #mus");
  nhspec->Draw("same");
  specLegend->AddEntry(nhspec, "100 #muS < #Deltat < 150 #muS");
  externspec->Draw("same");
  specLegend->AddEntry(externspec, "Uncorrelated events");
  pnspec->Draw("same");
  specLegend->AddEntry(pnspec, "Pure Neutron Spectrum");
  specLegend->Draw();

  c1->cd(2);
  c1->GetPad(2)->SetLogy();
  timing->GetXaxis()->SetRange(0, 1200);
  timing->SetTitle("#Deltat Distribution");
  timing->GetXaxis()->SetTitle("#muS");
  timing->SetLineColor(kBlack);
  TF1* gd_fit = new TF1("gd_fit", "[0]*TMath::Exp([1]*x)", 0, 2000);
  gd_fit->SetParameters(10000, -1e-4);
  timing->Fit(gd_fit, "Q+", "", 5, 60);
  timing->Draw("hist");
  TF1* f1 = timing->GetFunction("fitter_extern");
  f1->SetLineStyle(9);
  f1->SetLineColor(kGreen+3);
  TF1* f2 = timing->GetFunction("fitter_h");
  f2->SetLineStyle(2);
  f2->SetLineColor(kBlue);
  f1->SetRange(0, 2000);
  f2->SetRange(0, 2000);
  f1->Draw("same");
  f2->Draw("same");
  gd_fit->SetLineColor(kOrange);
  gd_fit->Draw("same");
  TLegend* timingLeg = new TLegend(0.5, 0.67, 0.88, 0.88);
  stringstream fitss, fitss_f1, fitss_f2;
  fitss << -1/gd_fit->GetParameter(1);
  fitss_f1 << -f1->GetParameter(1)*1000000;
  fitss_f2 << -1/f2->GetParameter(1);
  string gd_param = "Gd Capture Time: "+(fitss.str()).substr(0, 5)+" #mu S";
  string f1_param = "Uncorrelated Rate: "+(fitss_f1.str()).substr(0, 5)+" hZ";
  string f2_param = "Hydrogen Capture Time: "+(fitss_f2.str()).substr(0, 5)+" #mu S";
  timingLeg->AddEntry(gd_fit, gd_param.c_str());
  timingLeg->AddEntry(f2, f2_param.c_str());
  timingLeg->AddEntry(f1, f1_param.c_str());
  timingLeg->Draw();
  

  c1->cd(3);
  c1->GetPad(3)->SetLogy();
  
  mult->SetTitle("Event Multiplicity (50 #muS window)");
  mult->GetXaxis()->SetTitle("Multiplicity");
  mult->GetXaxis()->SetRange(0, 12);
  TF1* salmon = new TF1("salmon", "[0]*TMath::Poisson(x, [1])", 0, 10);
  salmon->SetParameters(1000, 4);
  mult->Fit(salmon, "Q");
  mult->Draw("E1");
  stringstream ss;
  ss << salmon->GetParameter(1);
  string fitparam = "Poisson Mean: "+(ss.str()).substr(0, 4);
  TLegend* multLeg = new TLegend(0.5, 0.67, 0.88, 0.88);
  multLeg->AddEntry(salmon, fitparam.c_str());
  multLeg->Draw();


  c1->cd(4);
  c1->GetPad(4)->SetLogy();
  c1->GetPad(4)->SetLogx();
  tsa->GetXaxis()->SetTitle("#Deltat_{23} #muS");
  tsa->GetYaxis()->SetTitle("#Deltat_{12} #muS");
  tsa->SetTitle("Time Series Analysis");
  tsa->Draw("colz");

}
