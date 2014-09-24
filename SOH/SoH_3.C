//
//  SoH.C
//
//
//  Created by Timothy Shokair on 7/11/14.
//
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

void SoH(char*);
int main(int argc, char * argv[1]){
    
  SoH(argv[1]);
  return 0;
}
void SoH(char * month){
    
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
  double chiSquare[52];
    
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
  TH1F *h1= new TH1F("h1","Event Rate vs. Run in Target (>1 PMT)",750,0,31);
  TH1F *h2= new TH1F("h2","Event Rate vs. Run in Target (>2 PMT)",750,0,31);
  TH1F *h3= new TH1F("h3","Event Rate vs. Run in Veto (>1 PMT)",750,0,31);
  TH1F *h4= new TH1F("h4","Event Rate vs. Run in Veto (>2 PMT)",750,0,31);
  TH1F *h5= new TH1F("h5","Total Event Rate vs. Run (>1 PMT either)",750,0,31);
  TH1F *h6= new TH1F("h6","Total Event Rate vs. Run (>1 PMT both)",750,0,31);
  TH1F *h7= new TH1F("h7","Total Event Rate vs. Run (uncut)",750,0,31);
  TH1F *h8= new TH1F("h8","Percentage of events cut by Single Tube cut vs. Run ",1000,0,500);
  //2D Histogram to display Rates for all of the cuts in 1 plot
  TH2F *rTable= new TH2F("rTable","Event Rate Given Various Cuts",750,0,31,8,0,7);
  //2D Histograms that track individual PMT rates, and pedestal parameters.
  TH2F *PMTRates= new TH2F("pmtRates","Rate in Each Tube as a function of Run",750,0,31,48,0,48);
    
  TH2F *PMTSigmas= new TH2F("PMTSigmas","Pedastal Sigma for each Channel",750,0,31,48,0,48);
  TH2F *PMTMeans= new TH2F("PMTMeans","Pedastal Sigma for each Channel",750,0,31,48,0,48);
  int numTHits[numTarget];
  int numVHits[numVeto];
    
    
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
  Int_t dayNum, hourNum;
  Int_t currentDay,previousDay;
    
  //
  Int_t numTPerEvent,numVPerEvent;
  Double_t liveTime=0;


  TString histName[52];
  Int_t Index;
  TString InName;
  TH1F *ConverisonHist[56];
  cerr<<files[0]<<endl;
  TFile *speFile = new TFile(files[0]); 

  for (int j=0;j<numPMTGroups;j++) {
    for (int i=0;i<numPMTs[j];i++) {
      Index=i+j*numPMTs[0];
      InName=Form("%d", i);      
      if(j==0){
	histName[Index]="hTargetCharge_"+InName;
      }
      else if (j==1){
	histName[Index]="hVetoCharge_"+InName;
      }
      ConverisonHist[Index]= (TH1F*)(speFile->Get(histName[Index]));
      pmtQperSPE[j][i]=ConverisonHist[Index]->GetBinContent(210);
      //cerr<<pmtQperSPE[j][i]<<endl;
    }
    
  }
  
  delete speFile;


  for (Int_t l=1; l<(numfiles-1);++l){
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
    dayNum=day.Atoi();
    hourNum=hour.Atoi();
    Double_t dayHour;
    dayHour= dayNum/1.0+hourNum/24.0;
    cerr<<"day and hour " <<dayHour<<endl;
    currentDay=dayNum;
    Int_t excludeList[52];
    Int_t numExcluded=0;

        
    wb->SetBranchAddress("target_4Minus2Mean1",&targetPMTQ);
    wb->SetBranchAddress("veto_3MinusMean1",&vetoPMTQ);
    wb->SetBranchAddress("target_1",&targetPMTPed);
    wb->SetBranchAddress("veto_1",&vetoPMTPed);
    wb->SetBranchAddress("time",&time);
        
    slowChain->SetBranchAddress("fit_means",&pedestalGateMean);
    slowChain->SetBranchAddress("fit_std_devs",&pedestalGateSigma);
    slowChain->SetBranchAddress("gateDev",&signalGateSigma);
    slowChain->SetBranchAddress("fit_chi2perndf",&chiSquare);

        
    //wb->SetBranchAddress("veto_summed_charge",&veto_summed_charge);
        
    wb->GetEntry(0);
    runtime=0;
    startTime=time;
        
    // readin pedestal  mean, sigma, signal pedestal sigma.
    //being done here for 1st file in chain - should be made Int_to function, so can be updated for each file in main event loop
        
    Long64_t nSlowEvent = slowChain->GetEntries();
    cout<<nSlowEvent<<" entries in slowChain"<<endl;
    cout<<"get entry"<<endl;
    slowChain->GetEntry(0);
        
    Int_t totalPMTCounter;
          //make a new PMT list based on exluded runs
    Int_t numPMTsub[numPMTGroups];
    Int_t targetSubList[16];
    Int_t vetoSubList[36];
    numPMTsub[0]=0;
    numPMTsub[1]=0;
    for (Int_t j=0;j<numPMTGroups;j++) {
      for (Int_t k=0;k<numPMTs[j];k++) {
	totalPMTCounter = j*numPMTs[0]+k; //need to index total pmt # as well; only works becuse there are two groups - beware
                
	//cout<<"PMT "<<k<<"\t"<<totalPMTCounter<<endl;
	if(chiSquare[totalPMTCounter]>1000){
	  excludeList[numExcluded]=totalPMTCounter;
	  numExcluded++;
	}
	else{ 
	  if(j==0){
	    targetSubList[numPMTsub[0]]=k;
	    numPMTsub[0]++;
	  }
	  if(j==1){
	    vetoSubList[numPMTsub[1]]=k;
	    numPMTsub[1]++;
	  }
	}
	signalThresholds[j][k] = 4 *  signalGateSigma[totalPMTCounter]; //4 sigma above mean
	pedestalThresholdsLower[j][k] = pedestalGateMean[totalPMTCounter] - 4 *  pedestalGateSigma[totalPMTCounter]; //4 sigma below mean
	pedestalThresholdsUpper[j][k] = pedestalGateMean[totalPMTCounter] + 4 *  pedestalGateSigma[totalPMTCounter]; //4 sigma above mean
	if(j==0){
	  PMTMeans->Fill(dayHour,k,pedestalGateMean[k]);
	  PMTSigmas->Fill(dayHour,k,pedestalGateSigma[k]);
	}
	else if(j==1){
	  PMTMeans->Fill(dayHour,k+numTarget,pedestalGateMean[k]);
	  PMTSigmas->Fill(dayHour,k+numTarget,pedestalGateSigma[k]);
	}
      }
    }
      //temp init of spe cal values
    for (int j=0;j<numPMTGroups;j++) {
      for (int k=0;k<maxNumPMTs;k++) {
	if(pmtQperSPE[j][k]!=-1000000){
	  signalThresholds[j][k] =signalThresholds[j][k]/pmtQperSPE[j][k];
	}
	else signalThresholds[j][k]=1000000;
	//      pedesThresholdsLower[j][k] =signalThresholds[j][k]/pmtQperSPE[j][k];
      }
    }
    cerr<<numExcluded<<" PMTs excluded from run for bad fits"<<endl;

    Int_t singleTCount=0;
    Int_t singleVCount=0;
    //zero number of PMT hits each run
    for(int m=0;m<numTarget;m++){
      numTHits[m]=0;
    }
    for(int m=0;m<numVeto;m++){
      numVHits[m]=0;
    }
    for(Int_t i=0;i<nEntries;++i){
      wb->GetEntry(i);
      endTime=time;
      numEvents++;
      sumQ_T=0;
      sumQSquare_T=0;
      numTPerEvent=0;
      numVPerEvent=0;
            
      Int_t  baselineCut=1;
      for (Int_t k=0;k<numPMTsub[0];k++) {
	if ( (targetPMTPed[targetSubList[k]] < pedestalThresholdsLower[0][targetSubList[k]]) || (targetPMTPed[targetSubList[k]] >pedestalThresholdsUpper[0][targetSubList[k]]) ) baselineCut=0;
      }
      for (Int_t k=0;k<numPMTsub[1];k++) {
	if ( (vetoPMTPed[vetoSubList[k]] < pedestalThresholdsLower[1][vetoSubList[k]]) || (vetoPMTPed[vetoSubList[k]] >pedestalThresholdsUpper[1][vetoSubList[k]]) ) baselineCut=0;
      }
            
      if (baselineCut ==1) {
	//init and fill analysis variables
	for (Int_t k=0;k<numPMTsub[0];k++) {
	  if(pmtQperSPE[0][targetSubList[k]]!=-1000000){
	    qPMT[0][targetSubList[k]] = targetPMTQ[targetSubList[k]]/pmtQperSPE[0][targetSubList[k]];
	  }
	  else qPMT[0][targetSubList[k]]=0;
	  // cerr<<qPMT[0][targetSubList[k]]<<endl;

	}
	for (Int_t k=0;k<numPMTsub[1];k++) {
	   if(pmtQperSPE[1][vetoSubList[k]]!=-1000000){
	     qPMT[1][vetoSubList[k]] = vetoPMTQ[vetoSubList[k]]/pmtQperSPE[1][vetoSubList[k]];
	   }
	   else qPMT[1][vetoSubList[k]]=0;
	}
                
                
	for (Int_t j=0;j<numPMTGroups;j++) {
	  qTotal[j]=0;
	  for (Int_t k=0;k<numPMTs[j];k++) {
	    qTotal[j]+=  qPMT[j][k];
	  }
	}
                
                
                
	//calc qBal target
	for (Int_t k=0;k<numPMTsub[0];k++){

	  if ( qPMT[0][targetSubList[k]] > signalThresholds[0][targetSubList[k]]) {
	    numTPerEvent++;
	    numTHits[targetSubList[k]]++;
	  }
	}
	//calc qBal veto
	for (Int_t k=0;k<numPMTsub[1];k++){
	  if ( qPMT[1][vetoSubList[k]] > signalThresholds[1][vetoSubList[k]]) {
	    numVPerEvent++;
	    numVHits[vetoSubList[k]]++;
	  }
	}
                
                
      }
      if(numTPerEvent>1){
	targetEvents_1++;
	if(numTPerEvent>2){
	  targetEvents_2++;
	}
      }
      else if(numTPerEvent==1) {
	singleTCount++;
	//cerr<<"Single Target Event "<<singleTCount<<endl;
      }
      if(numVPerEvent>1){
	vetoEvents_1++;
	if(numVPerEvent>2){
	  vetoEvents_2++;
	}
      }
      else if (numVPerEvent==1){
	singleVCount++;
      }
      if(numTPerEvent>1 || numVPerEvent>1){
	eventsEither_1++;
	if(numTPerEvent>1 && numVPerEvent>1){
	  eventsBoth_1++;
	}
      }
    }
        
    cerr<<singleTCount<<" single Target Events"<<endl;
    cerr<<singleVCount<<" single Veto Events"<<endl;
    runtime=(endTime-startTime)*4/(pow(10,9));
    //in case error in runtime, make sure we don't crash by diving by zero, but print that there was a problem
    if(runtime==0){
      runtime=3600;
      cerr<<"runtime set to default of 1 hour"<<endl;
    }
    liveTime+=runtime;
    rTable->Fill(dayHour,Double_t(0),targetEvents_1/runtime);
    rTable->Fill(dayHour,Double_t(1),targetEvents_2/runtime);
    rTable->Fill(dayHour,Double_t(2),vetoEvents_1/runtime);
    rTable->Fill(dayHour,Double_t(3),vetoEvents_2/runtime);
    rTable->Fill(dayHour,Double_t(4),eventsEither_1/runtime);
    rTable->Fill(dayHour,Double_t(5),eventsBoth_1/runtime);
    rTable->Fill(dayHour,Double_t(6),numEvents/runtime);

    h1->Fill(dayHour,targetEvents_1/runtime);
    h2->Fill(dayHour,targetEvents_2/runtime);
    h3->Fill(dayHour,vetoEvents_1/runtime);
    h4->Fill(dayHour,vetoEvents_2/runtime);
    h5->Fill(dayHour,eventsEither_1/runtime);
    h6->Fill(dayHour,eventsBoth_1/runtime);
    h7->Fill(dayHour,numEvents/runtime);
    h8->Fill((numEvents-eventsEither_1)/runtime);
    numTargetCut=numEvents-targetEvents_1;
    numVetoCut=numEvents-vetoEvents_1;
    cerr<<"Target: "<<targetEvents_1/runtime<<" Hz, "<<double(numTargetCut/numEvents)<<"% cut"<<endl;
    cerr<<"Veto: "<<vetoEvents_1/runtime<<" Hz, "<<numVetoCut/numEvents<<" % cut"<<endl;
    cerr<<"All: "<<(numEvents-eventsEither_1)/runtime<<" Hz, "<<double(numEvents-eventsEither_1)/numEvents<<" % cut"<<endl;
    cerr<<targetEvents_1<<" events in "<<runtime<<" s of data taking. Event rate is "<<numEvents/runtime<<" Hz"<<endl;
        
    for(int m=0;m<numTarget;m++){
      PMTRates->Fill(dayHour,m,numTHits[m]/runtime);
            
    }
    for(int m=0;m<numVeto;m++){
      PMTRates->Fill(dayHour,m+numTarget,numVHits[m]/runtime);
    }
        
    delete wb;
    delete f1;
    previousDay=currentDay;
    cerr<<"next event"<<endl;
  }
  ofstream liveTimes;
  liveTimes.open("livetimes.txt",ios_base::out|ios_base::app);
  liveTimes<<liveTime<<" live seconds in "<<month<<endl;
  cerr<<liveTime<<" live seconds in "<<month<<endl;
  liveTimes.close();
    
  //draw all 1D histograms on a single plot and write that plot to file
  TFile *f0 = new TFile(canvas,"RECREATE");
  TCanvas *c1 = new TCanvas("c1","Event Rate for Various Cuts",200,10,700,500);
    
  h1->SetMarkerColor(2);
  h2->SetMarkerColor(3);
  h3->SetMarkerColor(4);
  h4->SetMarkerColor(5);
  h5->SetMarkerColor(6);
  h6->SetMarkerColor(7);
  h7->SetMarkerColor(9);
  h8->SetMarkerColor(28);
  h1->SetMarkerStyle(2);
  h2->SetMarkerStyle(3);
  h3->SetMarkerStyle(4);
  h4->SetMarkerStyle(25);
  h5->SetMarkerStyle(26);
  h6->SetMarkerStyle(32);
  h7->SetMarkerStyle(5);
  h8->SetMarkerStyle(5);
  h7->Draw("P");
  h1->Draw("Psame");
  h2->Draw("Psame");
  h3->Draw("Psame");
  h4->Draw("Psame");
  h5->Draw("Psame");
  h6->Draw("Psame");
  TLegend *leg= new TLegend(.05,.75,.35,.875);
  leg->AddEntry(h1,"Target: >1 PMT");
  leg->AddEntry(h2,"Target: >2 PMT");
  leg->AddEntry(h3,"Veto: >1 PMT");
  leg->AddEntry(h4,"Veto: >2 PMT");
  leg->AddEntry(h5,"Either: >1 PMT");
  leg->AddEntry(h6,"Both: >1 PMT");
  leg->AddEntry(h7,"No cut");
  leg->Draw();
  c1->Write();
  f0->Close();
  //Print the 2D histogram to a new canvas and write to new file.
  TFile *f1= new TFile(outFile1,"RECREATE");
  TCanvas *c2 = new TCanvas("c2","Event Rate for Various Cuts",200,10,700,500);
  rTable->Draw("colz");
  rTable->Write();
  c2->Write();
  f1->Close();
  //write out PMT rate table to new file
  TFile *f2= new TFile(outFile2,"RECREATE");
  PMTRates->Write();
  PMTSigmas->Write();
  PMTMeans->Write();
  f2->Close();
    
    

    
}
