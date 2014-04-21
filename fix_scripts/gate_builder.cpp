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
#include <TRandom3.h>

// C++ headers
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <cmath>

void fit_pedestal(TFile* file, std::vector<double> &mean_values, std::vector<double> &std_dev_values,
		  std::vector<double> &amplitudes, std::vector<double> &chi2values);
double std_dev(std::vector<double> v);
double mean(std::vector<double> v);
unsigned long long mean(std::vector<unsigned long long> list);
void fill_slowTree(TFile* file, double* target_gate_width, double* veto_gate_width,
		   std::vector<double>& means, std::vector<double>& stdDevs, 
		   std::vector<double>& amplitudes, std::vector<double>& chi2values);

void process(char* filename);

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
  // Opening the current root file to be rewritten
  TFile* file = new TFile(filename, "UPDATE");
  
  // First check to make sure that this file hasn't been
  // completed already
  std::string tree_name = "procData";
  if( file->Get(tree_name.c_str()) )
  {
    std::cout << "This file has already been processed\n Exiting\n";
    return;
  }
  // make sure the file also has the original data (is a valid file)
  if( !file->Get("pureData") )
  {
    std::cout << filename << " is not a valid data file, skipping" << std::endl;
    return;
  }

#if DEBUG
  std::cout << "Currently running in DEBUG mode\n"
	    << "Only using 5 pmts and 20,000 events\n";
#endif

  // Figure out the average pedestal value from gate 2, the values of the fit
  // parameters are stored for later.
  std::vector<double> means, stdDevs, amplitudes, chi2values;
  fit_pedestal(file, means, stdDevs, amplitudes, chi2values);
  std::cout << "::::::::::::: Average pedestal values obtained ::::::::::::::\n";
  
  // Gate widths are contained in the slow tree, we will also pass the fit parameters
  // so that they can be written to the slow tree as well
  double target_gate_width[8], veto_gate_width[8];
  fill_slowTree(file, target_gate_width, veto_gate_width, means,
		stdDevs, amplitudes, chi2values);
  std::cout << "Filled the slow tree with the fit parameters" << std::endl;

  // Gate charges are in the pureData tree, separated into
  // target[16][8] and veto[36][8]
  TTree* pureData = (TTree*)file->Get("pureData");
  int entries = pureData->GetEntries();
  double target_pmt[16][8], veto_pmt[36][8];
  unsigned long long target_time[16], veto_time[36];
  pureData->SetBranchAddress("target_pmt", &target_pmt);
  pureData->SetBranchAddress("veto_pmt", &veto_pmt);
  pureData->SetBranchAddress("target_time", &target_time);
  pureData->SetBranchAddress("veto_time", &veto_time);

#if DEBUG
  entries = 20000;
#endif

  // The new tree will have: gate 1, gate 4-2, gate 8, gate 4 - gate 2 - mean(1)
  TTree* procTree = new TTree(tree_name.c_str(), "Contains processed gate charges");
  double target_1[16], target_4Minus2[16], target_8[16], target_4Minus2Mean1[16];
  double veto_1[36], veto_3[36], veto_8[36], veto_3MinusMean1[36];
  unsigned long long time;
  procTree->Branch("target_1", &target_1, "target_1[16]/D");
  procTree->Branch("target_4Minus2", &target_4Minus2, "target_4Minus2[16]/D");
  procTree->Branch("target_8", &target_8, "target_8[16]/D");
  procTree->Branch("target_4Minus2Mean1", &target_4Minus2Mean1, "target_4Minus2Mean1[16]/D");
  procTree->Branch("veto_1", &veto_1, "veto_1[36]/D");
  procTree->Branch("veto_3", &veto_3, "veto_3[36]/D");
  procTree->Branch("veto_8", &veto_8, "veto_8[36]/D");
  procTree->Branch("veto_3MinusMean1", &veto_3MinusMean1, "veto_3MinusMean1[36]/D");
  procTree->Branch("time", &time);

  int num_pmts=16+36;

#if DEBUG
  num_pmts = 5;
#endif

  std::cout << "Processing " << entries << " events\n";

  for(int i=0; i<entries; ++i)
  {
    if(!(i%10000))
      std::cout << "\033[31;1m" << "Finished processing " << i << " entries\r" << std::flush;
    pureData->GetEvent(i);
    // Adding time
    std::vector<unsigned long long> timelist;
    for(int j=0; j<16; j++)
      timelist.push_back(target_time[j]);
    // NOT USING ORPHAN PMTS!! They can cause problems
    for(int j=0; j<32; j++)
      timelist.push_back(veto_time[j]);
    time = mean(timelist);

    // Looping over all 52 pmts
    for(int j=0; j<num_pmts; j++)
      if(j<16)
      {// Target
	target_1[j]=target_pmt[j][0];
	target_4Minus2[j]=target_pmt[j][3]-target_pmt[j][1];
	target_8[j]=target_pmt[j][7];
	target_4Minus2Mean1[j]=(target_pmt[j][3]-target_pmt[j][1]) - 
	  ((target_gate_width[3] - target_gate_width[1])/target_gate_width[0])*means[j];
      }
      else
      {// Veto
	veto_1[j-16]=veto_pmt[j-16][0];
	veto_3[j-16]=veto_pmt[j-16][2];
	veto_8[j-16]=veto_pmt[j-16][7];
	veto_3MinusMean1[j-16]=(veto_pmt[j-16][2]) - 
	  ((veto_gate_width[2])/veto_gate_width[0])*means[j];
      }

    procTree->Fill();
  }
  std::cout << "\nFinished filling " << tree_name << std::endl;

  file->Write("", TObject::kOverwrite);
  return;
}

void fit_pedestal(TFile* file, std::vector<double> &mean_values, std::vector<double> &std_dev_values,
		  std::vector<double> &amplitudes, std::vector<double> &chi2values)
{
  int num_pmts=16+36;
#if DEBUG
  num_pmts=5;
#endif

  // The passed vectors should be empty ... in case not:
  mean_values.resize(0);
  mean_values.resize(num_pmts, 0);
  std_dev_values.resize(0);
  std_dev_values.resize(num_pmts, 0);
  amplitudes.resize(0);
  amplitudes.resize(num_pmts, 0);
  chi2values.resize(0);
  chi2values.resize(num_pmts, 0);

  // Grab the data from the given root file
  TTree* pureData = (TTree*)file->Get("pureData");
  int entries = pureData->GetEntries();
  double target_pmt[16][8], veto_pmt[36][8];
  pureData->SetBranchAddress("target_pmt", &target_pmt);
  pureData->SetBranchAddress("veto_pmt", &veto_pmt);

#if DEBUG
  entries=20000;
#endif

  // Using a semi-smart way of creating the histograms, i will assume
  // that the first 20000 points well represent the data and can be used
  // to guess the average value and deviation, then the histogram will
  // center there and only go out x sigma
  const int sigma = 3;
  const int rep = entries;
  // These should also be randomly distributed in the file

  std::cout << "Filling the vectors from the TTree\n";
  std::vector<double> value[num_pmts];
  for(int i=0; i<rep; ++i)
  {
    pureData->GetEvent(i);
    for(int j=0; j<num_pmts; j++)
      if(j<16)
	value[j].push_back(target_pmt[j][0]);
      else
	value[j].push_back(veto_pmt[j-16][0]);
  }

  // Based upon this information we'll initialize the histograms
  // Tree to holds the histograms
  TH1F* hists[num_pmts];
  int bin_width = 2;
  //int bins=100;
  std::cout << "Creating the histograms" << std::endl;
  for(int i=0; i<num_pmts; i++)
  {
    std::stringstream ss;
    ss << i;
    std::string hname = "histogram" + ss.str();
    double thismean = mean(value[i]);
    //double thisdev = std_dev(value[i]);
    double thisdev = 30;
    int max_value = static_cast<int>(thismean+sigma*thisdev);
    int min_value = static_cast<int>(thismean-sigma*thisdev);
    //int bin_width = int((max_value-min_value)/bins);
    int bins = (max_value-min_value)/bin_width;
    // now fix the max_value so that bins are even
    max_value = min_value+bins*bin_width;
    hists[i] = new TH1F(hname.c_str(), hname.c_str(), bins, 
			min_value, max_value);
  }

  // Now we can go ahead and fill the histograms
  std::cout << "Looping through data to fit the pedestal" << std::endl;
  // for(int i=0; i<entries; ++i)
  // {
  //   pureData->GetEvent(i);
  //   for(int j=0; j<num_pmts; j++)
  //     if(j<16)
  // 	hists[j]->Fill(target_pmt[j][0]);
  //     else
  // 	hists[j]->Fill(veto_pmt[j-16][0]);
  // }
  // Lets fit this with gaussians
  for(int i=0; i<num_pmts; i++)
    for(std::vector<double>::iterator it=value[i].begin();
	it!=value[i].end(); ++it)
      if(*it > 0)
	hists[i]->Fill(*it);

  std::cout << "Hist size: " << hists[0]->GetEntries() << std::endl;
  TF1* fits[num_pmts];
  for(int i=0; i<num_pmts; i++)
  {
    std::stringstream ss;
    ss << i;
    std::string fname = "fitter " + ss.str();
    double thismean = hists[i]->GetMean();
    double thisdev = hists[i]->GetRMS();
    fits[i] = new TF1(fname.c_str(), "[0]*TMath::Gaus(x, [1], [2])", 
		      thismean-sigma*thisdev, thismean+sigma*thisdev);
    fits[i]->SetParameters( hists[i]->GetMaximum(), thismean, thisdev );
#if KEEPHISTS
    hists[i]->Fit(fits[i], "Q");
#else
    hists[i]->Fit(fits[i], "QN");
#endif
    std::cout << "Fit " << i+1 << " of " << num_pmts << " pmts\r" << std::flush;
  }

  std::cout << "\nFinished fitting" << std::endl;
  // Finally lets fill the return vectors
  for(int i=0; i<num_pmts; i++)
  {
    amplitudes[i]=fits[i]->GetParameter(0);
    mean_values[i]=fits[i]->GetParameter(1);
    std_dev_values[i]=fits[i]->GetParameter(2);
    chi2values[i]=fits[i]->GetChisquare() / fits[i]->GetNDF();
  }
  
#if !KEEPHISTS
  for(int i=0; i<num_pmts; i++)
  {
    delete hists[i];
    delete fits[i];
  }
#endif

  delete pureData;
  return;
}

double mean(std::vector<double> v)
{
  double avg=0;
  //double vsize = v.size();
  double vsize = 0;
  for(std::vector<double>::iterator it=v.begin();
      it!=v.end(); ++it)
    if(*it > 0)
      vsize++;

  for(std::vector<double>::iterator it=v.begin();
      it!=v.end(); ++it)
    if(*it > 0)
      avg+=*it/vsize;
  return avg;
  //return std::accumulate(v.begin(), v.end(), 0)/double(v.size());
}

unsigned long long mean(std::vector<unsigned long long> list)
{
  long double avg=0;
  long double size=list.size();
  for(auto it=list.begin();
      it!=list.end(); ++it)
    avg+=static_cast<long double>(*it)/size;
  return static_cast<unsigned long long>(avg);
}

double std_dev(std::vector<double> v)
{
  double av = mean(v);
  double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
  return std::sqrt(sq_sum/v.size()-av*av);
}

void fill_slowTree(TFile* file, double* target_gate_width, double* veto_gate_width,
		   std::vector<double>& means, std::vector<double>& stdDevs, 
		   std::vector<double>& amplitudes, std::vector<double>& chi2values)
{
  TTree* slowTree = (TTree*)file->Get("slowTree");
  // This is a bit painful, but to add a branch to an existing tree we must
  // grab EVERYTHING from that tree as well and rewrite it
  double target_voltages[16], target_currents[16], veto_voltages[36], veto_currents[36],
    target_gate_start[8], target_gate_end[8], veto_gate_start[8], veto_gate_end[8];
  slowTree->SetBranchAddress("target_voltages", &target_voltages);
  slowTree->SetBranchAddress("target_currents", &target_currents);
  slowTree->SetBranchAddress("target_gate_start", &target_gate_start);
  slowTree->SetBranchAddress("target_gate_end", &target_gate_end);
  slowTree->SetBranchAddress("veto_voltages", &veto_voltages);
  slowTree->SetBranchAddress("veto_currents", &veto_currents);
  slowTree->SetBranchAddress("veto_gate_start", &veto_gate_start);
  slowTree->SetBranchAddress("veto_gate_end", &veto_gate_end);
  slowTree->SetBranchAddress("target_gate_width", target_gate_width);
  slowTree->SetBranchAddress("veto_gate_width", veto_gate_width);
  slowTree->GetEvent(0);
  // We can also add our fit parameters to the slowTree
  // ROOT is involved so no stl objects kept pass this point
  double amp_array[amplitudes.size()], 
    mean_array[means.size()], 
    dev_array[stdDevs.size()], 
    chi_array[chi2values.size()];
  auto element=0;
  
  for( auto it=amplitudes.begin();
       it!=amplitudes.end(); ++it)
    amp_array[element++]=*it;
  element=0;
  for( auto it=means.begin();
       it!=means.end(); ++it)
    mean_array[element++]=*it;
  element=0;
  for( auto it=stdDevs.begin();
       it!=stdDevs.end(); ++it)
    dev_array[element++]=*it;
  element=0;
  for( auto it=chi2values.begin();
       it!=chi2values.end(); ++it)
    chi_array[element++]=*it;

  std::string sm_amp_name = "fit_amplitudes";
  std::string lg_amp_name = sm_amp_name + "[" +
    std::to_string( static_cast<long long int>(amplitudes.size()) ) + "]/D";
  std::string sm_mean_name = "fit_means";
  std::string lg_mean_name = sm_mean_name + "[" +
    std::to_string( static_cast<long long int>(means.size()) ) + "]/D";
  std::string sm_dev_name = "fit_std_devs";
  std::string lg_dev_name = sm_dev_name + "[" +
    std::to_string( static_cast<long long int>(stdDevs.size()) ) + "]/D";
  std::string sm_chi_name = "fit_chi2perndf";
  std::string lg_chi_name = sm_chi_name + "[" +
    std::to_string( static_cast<long long int>(chi2values.size()) ) + "]/D";

  TBranch* bramp = slowTree->Branch(sm_amp_name.c_str(), &amp_array, lg_amp_name.c_str());
  TBranch* brmean = slowTree->Branch(sm_mean_name.c_str(), &mean_array, lg_mean_name.c_str());
  TBranch* brdev = slowTree->Branch(sm_dev_name.c_str(), &dev_array, lg_dev_name.c_str());
  TBranch* brchi = slowTree->Branch(sm_chi_name.c_str(), &chi_array, lg_chi_name.c_str());
  int entries = slowTree->GetEntries();
  for(int i=0; i<entries; i++)
  {
    bramp->Fill();
    brmean->Fill();
    brdev->Fill();
    brchi->Fill();
  }
}
