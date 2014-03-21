// watchboy health monitor HV + IV
#include <TTree.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TLeaf.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>		// distance
#include <numeric>		// accumulate, inner_product
#include <cmath>
using namespace std;

const string target_card[16]={"12_3", "12_3", "12_3", "12_3", "12_3", "12_3",
			      "12_3", "12_3", "12_3", "12_3", "12_3", "12_3",
			      "4_1", "4_1", "4_1", "4_1"};
const string target_channel[16]={"0", "1", "0", "3", "2", "5", "6", "7", "8",
				 "9", "10", "11", "0", "1", "2", "3"};
const string veto_card[36]={"12_1", "12_1", "4_3", "12_1", "12_1", "12_1", "12_1",
			    "12_1", "12_1", "12_1", "12_1", "4_3", "12_1", "4_2",
			    "4_2", "12_1", "12_2", "12_2", "12_2", "12_2", "4_2",
			    "12_2", "4_2", "12_2", "12_2", "12_2", "12_2", "12_2",
			    "12_2", "12_2", "12_2", "12_2", "12_2", "12_2", "12_3", "12_3"};
const string veto_channel[36]={"0", "6", "3", "2", "3", "4", "5",
			       "1", "8", "7", "9", "2", "10", "3",
			       "2", "11", "0", "4", "4", "1", "0",
			       "2", "1", "6", "5", "3", "8", "9",
			       "6", "7", "10", "11", "0", "0", "4", "4"};

TChain* loadChain(int argc, char* argv[]);
double mean(vector<double> v);
double std_dev(vector<double> v);
void writeReport(vector<vector<double> >* voltages, vector<bool> &pmt_flags_v,
		 vector<vector<double> >* currents, vector<bool> &pmt_flags_i);

int main(int argc, char* argv[])
{
  vector<vector<double> >* voltages = new vector<vector<double> >;
  vector<vector<double> >* currents = new vector<vector<double> >;
  voltages->resize(52);
  currents->resize(52);
  TChain* chain = loadChain(argc, argv);
  int entries = chain->GetEntries();

  cout << entries/2 << " files\n";

  for(int i=0; i<entries; i++)
  {
    chain->GetEvent(i);
    for(int slot=0; slot<16; slot++)
    {
      TLeaf* vleaf = chain->GetLeaf(("VHS_"+target_card[slot]+".VM."+target_channel[slot]).c_str());
      voltages->at(slot).push_back(vleaf->GetValue(0));
      TLeaf* ileaf = chain->GetLeaf(("VHS_"+target_card[slot]+".IM."+target_channel[slot]).c_str());
      currents->at(slot).push_back(ileaf->GetValue(0));      
    }
    for(int slot=0; slot<36; slot++)
    {
      if(veto_card[slot]=="4_3")
      {
	voltages->at(slot+16).push_back(-1);
	currents->at(slot+16).push_back(-1);
      }
      else
      {
	TLeaf* vleaf = chain->GetLeaf(("VHS_"+veto_card[slot]+".VM."+veto_channel[slot]).c_str());
	voltages->at(slot+16).push_back(vleaf->GetValue(0));
	TLeaf* ileaf = chain->GetLeaf(("VHS_"+veto_card[slot]+".IM."+veto_channel[slot]).c_str());
	currents->at(slot+16).push_back(ileaf->GetValue(0));
      }
    }
  }
  
  const double allowed_dv = 100;
  const double allowed_di = 50e-06;

  vector<bool> pmt_flags_v;
  vector<bool> pmt_flags_i;
  pmt_flags_v.resize(52,0);
  pmt_flags_i.resize(52,0);


  for( auto pmt = voltages->begin();
       pmt!=voltages->end(); ++pmt )
  {
    double meanvalue = mean(*pmt);
    double deviation = std_dev(*pmt);
    //cout << "Mean: " << meanvalue << " Dev: " << deviation << endl;
    for(auto it2 = (*pmt).begin();
	it2!=(*pmt).end(); ++it2)
    {
      //cout << distance((*it).begin(), it2) << " ";
      if( abs(*it2-meanvalue) > allowed_dv )
	pmt_flags_v.at( distance(voltages->begin(), pmt) ) = true;
    }
  }

  for( auto pmt = currents->begin();
       pmt!=currents->end(); ++pmt )
  {
    double meanvalue = mean(*pmt);
    double deviation = std_dev(*pmt);
    for(auto it2 = (*pmt).begin();
	it2!=(*pmt).end(); ++it2)
      if( abs(*it2-meanvalue) > allowed_di )
	pmt_flags_i.at( distance(currents->begin(), pmt) ) = true;
  }

  bool error = false;
  // Now if we managed to pick up some flags lets say so
  for(auto it = pmt_flags_v.begin();
      it!=pmt_flags_v.end(); ++it)
    if(*it==true)
    {
      cout << "PMT: " << distance(pmt_flags_v.begin(), it) << " varied VOLTAGE more than "
	   << allowed_dv << " from its mean value" << endl;
      error = true;
    }

  for(auto it = pmt_flags_i.begin();
      it!=pmt_flags_i.end(); ++it)
    if(*it==true)
    {
      cout << "PMT: " << distance(pmt_flags_i.begin(), it) << " varied CURRENT more than "
	   << allowed_di << " from its mean value" << endl;
      error = true;
    }

  if(error == true)
  {
    cout << ">>> See error.txt for full report <<<" << endl;
    writeReport(voltages, pmt_flags_v, currents, pmt_flags_i);
  }

  delete currents;
  delete voltages;
  return 0;
}

void writeReport(vector<vector<double> >* voltages, vector<bool> &pmt_flags_v,
		 vector<vector<double> >* currents, vector<bool> &pmt_flags_i)
{
  fstream file("error.txt", fstream::out);

  for(auto it=pmt_flags_v.begin();
      it!=pmt_flags_v.end(); ++it)
    if(*it==true)		// write report
    {
      int pmt = distance(pmt_flags_v.begin(), it);
      file << "PMT " << pmt << " Mean V: " << mean(voltages->at(pmt))
	   << " Deviation: " << std_dev(voltages->at(pmt)) << endl;
      file << "PMT " << pmt << " Voltages:: ";
      for(auto volts=(voltages->at(pmt)).begin();
	  volts!=(voltages->at(pmt)).end(); ++volts)
	file << "<" << *volts << "> ";
      file << endl;
    }

  file << endl << endl;

  for(auto it=pmt_flags_i.begin();
      it!=pmt_flags_i.end(); ++it)
    if(*it==true)		// write report
    {
      int pmt = distance(pmt_flags_i.begin(), it);
      file << "PMT " << pmt << " Mean I: " << mean(currents->at(pmt))
	   << " Deviation: " << std_dev(currents->at(pmt)) << endl;
      file << "PMT " << pmt << " Currents:: ";
      for(auto amps=(currents->at(pmt)).begin();
	  amps!=(currents->at(pmt)).end(); ++amps)
	file << "<" << *amps << "> ";
      file << endl;
    }

  return;
}

double mean(vector<double> v)
{
  return accumulate(v.begin(), v.end(), 0.0)/double(v.size());
}

double std_dev(vector<double> v)
{
  double av = mean(v);
  double sq_sum = inner_product(v.begin(), v.end(), v.begin(), 0.0);
  return sqrt(sq_sum/v.size()-av*av);
}

TChain* loadChain(int argc, char* argv[])
{
  string treename = "slowTree";
  string endswith = ".root";
  TChain* chain = new TChain(treename.c_str());
  for(int i=0; i<argc; i++)
  {
    string fname = argv[i];
    if(!(fname.compare(fname.size()-endswith.size(), endswith.size(), endswith)))
      chain->Add(fname.c_str());
  }

  return chain;
}
