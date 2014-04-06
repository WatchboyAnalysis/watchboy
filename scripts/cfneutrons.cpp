// Plot the pure neutron spectrum for the cf runs
// The algorithm:
// :Use Timing information (deltat12) for ALL events
// :Fit an exponential to the tale (this will be the uncorrelated events)
// :Get the energy spectrum for all events where t_12 < ~50us
// -- integrate the area under the fit in this region, then find an equal number
// of events in the t_12>300us window and subtract this energy spectrum away
// from the first

// Root
#include <TTree.h>
#include <TFile.h>
#include <TRint.h>
#include <TChain.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TF1.h>

// cpp
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

TChain* createChain(int argc, char* argv[]);

int main(int argc, char* argv[])
{
  TFile* file = new TFile("neutrons.root", "recreate");

  TChain* chain = createChain(argc, argv);
  // Assign addresses
  double target_charge, veto_charge, target_cb, veto_cb;
  unsigned long long time;
  chain->SetBranchAddress("target_total", &target_charge);
  chain->SetBranchAddress("veto_total", &veto_charge);
  chain->SetBranchAddress("target_cb", &target_cb);
  chain->SetBranchAddress("veto_cb", &veto_cb);
  chain->SetBranchAddress("time", &time);

  const int chainEntries = chain->GetEntries();
  const double adc2us = 4/1000.0;
  const int maxCharge = 30000;
  TH1F* histogram = new TH1F("timing", "timing", 2000, 0, 2000);
  TH1F* neutrons = new TH1F("nspec", "neutron spectrum", 200, 0, maxCharge);
  TH1F* hydrogen = new TH1F("nhspec", "capture on hydrogen", 200, 0, maxCharge);
  TH1F* externals = new TH1F("externspec", "externals", 200, 0, maxCharge);
  TH1F* tsa_neutrons = new TH1F("tsa_spec", "neutron spectrum from tsa plot", 200, 0, maxCharge);
  TH1I* multiplicity = new TH1I("mult", "Event multiplicity", 20, 0, 20);

  unsigned long long t1=0;
  unsigned long long t2=0;
  unsigned long long t3=0;

  // cuts
  const double max_target_cb = 0.5;
  const double max_veto_charge = 500;
  const double min_target_charge = 0;
  const double tale_min = 800;
  const double tale_max = 2000;
  const double neutrons_min = 0;
  const double neutrons_time = 50;
  const double hydro_min = 100;
  const double hydro_max = 150;

  double previous_charge = 0;

  // TSA Plots
  const int nbins = 100;
  double logmin = -0.3;
  double logmax = 4.5;
  double binwidth = (logmax-logmin)/double(nbins);
  double bin_array[nbins+1];
  bin_array[0]=std::pow(10, logmin);
  for(int i=1; i<=nbins; i++)
    bin_array[i]=bin_array[0]+std::pow(10,logmin + i*binwidth);
  TH2F* tsa = new TH2F("tsa", "time series analysis", nbins, bin_array, nbins, bin_array);

  // Keep track of timing in order to get multiplicity
  std::vector<unsigned long long> time_vec;
  time_vec.resize(1, 0);

  for(int evt=0; evt<chainEntries; ++evt)
  {
    // Print out progress
    if(!(evt%1000))
      std::cout << std::floor(double(evt)/chainEntries*100) << "%\r";
    previous_charge = target_charge;
    chain->GetEvent(evt);    

    if(veto_charge<max_veto_charge &&
       target_charge>min_target_charge &&
       target_cb < max_target_cb )
    {
      
      t3=t2;
      t2=t1;
      t1=time;
      double d12 = (t1-t2)*adc2us;
      double d23 = (t2-t3)*adc2us;
      tsa->Fill(d23, d12);
      if((time-time_vec[0])*adc2us < 50)
	time_vec.push_back(time);
      else
      {
	multiplicity->Fill(time_vec.size());
	time_vec.clear();
	time_vec.push_back(time);
      }

      histogram->Fill(d12);
      if(d12 < neutrons_time && d12 > neutrons_min)
	neutrons->Fill(target_charge);
      if(d12 < tale_max && d12 > tale_min)
	externals->Fill(target_charge);
      if(d12 < hydro_max && d12 > hydro_min)
	hydrogen->Fill(target_charge);
      if(d12 < neutrons_time && d23 < neutrons_time)
	tsa_neutrons->Fill(previous_charge);
    }
  }
  std::cout << std::endl;
  // Events in tale:
  TF1* fitter_extern = new TF1("fitter_extern", "[0]*TMath::Exp([1]*x)",
			       0, 2000);
  fitter_extern->SetParameters(1000, -4e-4);
  histogram->Fit(fitter_extern, "QO", "", tale_min, tale_max);
  TF1* fitter_h = new TF1("fitter_h", "[0]*TMath::Exp([1]*x)",
			  0, 2000);
  fitter_h->SetParameters(1000, -4e-3);
  histogram->Fit(fitter_h, "QO+", "", hydro_min, hydro_max);

  

  int tale_events = fitter_extern->Integral(tale_min, tale_max);
  int bkg_events = fitter_extern->Integral(neutrons_min, neutrons_time);

  int h_long_events = fitter_h->Integral(hydro_min, hydro_max);
  int h_short_events = fitter_h->Integral(neutrons_min, neutrons_time);

  double talebkgratio = bkg_events / double(tale_events);
  externals->Scale(talebkgratio);
  double h_ratio = h_short_events / h_long_events;
  hydrogen->Scale(h_ratio);

  TH1F* pureNeutrons = new TH1F("pureNeutrons", "pureNeutrons", 200, 0, maxCharge);
  for(int i=0; i<neutrons->GetNbinsX(); i++)
    pureNeutrons->SetBinContent(i, neutrons->GetBinContent(i) - externals->GetBinContent(i) 
				- hydrogen->GetBinContent(i));

  file->Write();
  std::cout << "Wrote histograms to neutrons.root" << std::endl;

  // TRint* app = new TRint("EVIL", &argc, argv);
  // TCanvas* c1 = new TCanvas();
  //c1->Divide(1,2);
  //c1->cd(1);
  //histogram->Draw();
  //fitter_h->Draw("same");
  //fitter_extern->Draw("same");
  //c1->cd(2);
  // externals->SetLineColor(kBlack);
  // neutrons->SetLineColor(kGreen);
  // pureNeutrons->SetLineColor(kRed);
  // hydrogen->SetLineColor(kBlue);
  // neutrons->Draw();
  // externals->Draw("same");
  // hydrogen->Draw("same");
  // pureNeutrons->Draw("same");
  //app->Run();

  delete histogram;
  delete externals;
  delete neutrons;
  delete pureNeutrons;
  //delete c1;
  delete chain;
  delete file;
  return 0;
}

TChain* createChain(int argc, char* argv[])
{
  // Load the files into the TChain
  std::string treename = "data";
  std::string endswith = ".root";
  TChain* chain = new TChain(treename.c_str());
  for(int addfile=0; addfile<argc; addfile++)
  {
    std::string fname = argv[addfile];
    // check for root file
    if( !fname.compare(fname.size()-endswith.size(), endswith.size(), endswith) )
      chain->Add(fname.c_str());
  }

  return chain;
}
