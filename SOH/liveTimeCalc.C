//
//  SoH.C
//
//
//  Created by Timothy Shokair on 7/11/14.
//  This code simply parses through runs and calculates livetimes for a given Month
//

#include "TStyle.h"
#include "TClonesArray.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRint.h"
#include "TTree.h"
#include "TBranch.h"
#include "TChain.h"
#include "TFile.h"
#include "TObject.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TList.h"
#include "TRandom.h"
#include "TLegend.h"

//cpp
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <list>
#include <cmath>

using namespace std;

void LiveTimeCalc(char*);
int main(int argc, char * argv[1]){
    
  LiveTimeCalc(argv[1]);
  return 0;
}
void LiveTimeCalc(char * month){
    
  gROOT->SetStyle("Plain");
  Int_t nEntries;
    
  //char * month="marchTest";
    
  double sum1,sum2,sum3,sum4,sum5 = 0;
  double channel_energy=0;
  double startTime,endTime=0;
  //double charge[1000];
  double summed_charge,veto_summed_charge;
  double runtime=0;
  unsigned long long time;
  Int_t numTarget=16;
  Int_t numVeto=36;
  Int_t targetEvents_1=0,targetEvents_2=0;
  Int_t vetoEvents_1=0,vetoEvents_2=0;
  Int_t eventsBoth_1=0,eventsBoth_2=0;
  Int_t eventsEither_1=0,eventsEither_2=0;
  double target_thresh=500;
  double veto_thresh=500;
  //double charge[16]={0};
  // double vetoCharge[16]={0};
    
  //Nathaniel Parameters
  Int_t singleTargetFlag, singleVetoFlag;
  double targetPMTQ[16];
  double vetoPMTQ[36];
  double targetPMTPed[16];
  double vetoPMTPed[36];
  double signalGateSigma[52];
  double pedestalGateMean[52];
  double pedestalGateSigma[52];
    
  const int maxNumPMTs =36;
  const int numPMTGroups =2;
  Char_t groupNames[numPMTGroups][50]={"Target","Veto"};
  Char_t physicsNames[numPMTGroups+1][50]={"Neutron","MuonInV","MuonInVT"};
    
    
  Int_t numPMTs[numPMTGroups] ={16,36} ; //# of channels in each group
    
  Int_t qRangeAll[numPMTGroups] ={5000,3000} ; // Max SPE Charge in each region; for histogram ranges
  Int_t qRangePhysics[numPMTGroups] ={250,250} ; // Max SPE Charge in each region, physics events; for histogram ranges
    
  double signalThresholds[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt threshold; readin from file; based on fit to pedestal of (pedestal subtracted) signal gate
  double pmtQperSPE[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt spe calibration; readin from file; based on LED data
    
  double pedestalThresholdsLower[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt lower threshold for pedestal gate; readin from file; based on fit to pedestal of gate
  double pedestalThresholdsUpper[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt upper threshold for pedestal gate; readin from file; based on fit to pedestal of gate
  double qPMT[numPMTGroups][maxNumPMTs];
  double qTotal[numPMTGroups];
    
  //set up isotropy parameters
  double sumQSquare_T,sumQSquare_V;
  double sumQ_T, sumQ_V;
  double iso_T,iso_V;
    
    
    
    
  //1D Histograms that diplay the rates given a specific type of cut
  //TH1F *h1= new TH1F("h1","Live Detector Time each month",12,1,12);
    
    
  Int_t numfiles =0;
  TString files[2000];
  ifstream runList;
  TString mName=month;
  TString listName=mName+".txt";
  TString canvas=mName+"Canvas.C";
  TString outFile1=mName+"EventRateTable.C";
  TString outFile2=mName+"PMTRateTable.C";

    
  //read in the runlist
  cerr<<"Looking at Data from "<<listName<<endl;
  runList.open(listName);
  // runList.open("marchTest");
  while(!runList.eof()){
    runList>>files[numfiles];
    numfiles++;
  }
  runList.close();
  //set parameters to extract the day and hour information from the file name
  TString str;
  TString day;
  TString hour;
  TString mth;
  TString year;
  Int_t dayNum, hourNum, monthNum, yearNum;
  Int_t currentDay,previousDay;
    
  //
  Int_t numTPerEvent,numVPerEvent;
  Double_t liveTime=0;
    
  for (Int_t l=0; l<(numfiles-1);++l){
    Int_t numEvents=0;
    targetEvents_1=0;
    vetoEvents_1=0;
    targetEvents_2=0;
    vetoEvents_2=0;
    eventsBoth_1=0;
    eventsEither_1=0;
    double numTargetCut=0;
    double numVetoCut=0;
    TFile *f1 = new TFile( files[l]);
    TTree *wb = (TTree*)f1->Get("procData");
    TChain *slowChain = new TChain("slowTree");
    nEntries= wb->GetEntries();
    cerr<<nEntries<<" events in the file "<<l<<endl;
    double charge[numTarget];
    double vetoCharge[numVeto];
    slowChain->Add(files[l]);
    str = files[l];
    day= str(61,2);
    hour=str(64,2);
    mth=str(58,2);
    year=str(55,2);
    dayNum=day.Atoi();
    hourNum=hour.Atoi();
    monthNum=mth.Atoi();
    yearNum=year.Atoi();
    Double_t dayHour;
    dayHour= dayNum/1.0+hourNum/24.0;
    cerr<<"day and hour " <<dayHour<<endl;
    currentDay=dayNum;

        
    wb->SetBranchAddress("target_4Minus2Mean1",&targetPMTQ);
    wb->SetBranchAddress("veto_3MinusMean1",&vetoPMTQ);
    wb->SetBranchAddress("target_1",&targetPMTPed);
    wb->SetBranchAddress("veto_1",&vetoPMTPed);
    wb->SetBranchAddress("time",&time);
        
    slowChain->SetBranchAddress("fit_means",&pedestalGateMean);
    slowChain->SetBranchAddress("fit_std_devs",&pedestalGateSigma);
    slowChain->SetBranchAddress("gateDev",&signalGateSigma);
        
    //wb->SetBranchAddress("veto_summed_charge",&veto_summed_charge);
        
    wb->GetEntry(0);
    runtime=0;
    startTime=time;
    wb->GetEntry(nEntries-1);
    endTime=time;
    runtime=(endTime-startTime)*4/(pow(10,9));
    //in case error in runtime, make sure we don't crash by diving by zero, but print that there was a problem
    if(runtime==0){
      cerr<<"Error with run time, it was 0."<<endl;
    }
    else if(runtime>3660){
      cerr<<"Error with run time, run was too long"<<endl;
    }
    else{
      liveTime+=runtime;
    }
        
    delete wb;
    delete f1;
    previousDay=currentDay;
    cerr<<"runtime = "<<runtime <<"s"<<endl;
  }
  ofstream liveTimes;
  liveTimes.open("livetimePoints.txt",ios_base::out|ios_base::app);
  liveTimes<<monthNum<<" "<<yearNum<<" "<<liveTime<<endl;
  cerr<<liveTime<<" live seconds in "<<month<<endl;
  liveTimes.close();
  //draw all 1D histograms on a single plot and write that plot to file
  /*
  TFile *f0 = new TFile("liveTimes.C","Update");
  TH1F* h1 = (*TH1F)f0.Get("h1");
  h1->Fill(monthNum,liveTime);      
  h1->Write();       
  f0->Close();
  */
}
