//Root

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

//cpp
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <list>
#include <cmath>

using namespace std;

void analysisTemplate(char*, char*);

int main(int argc, char *argv[]){
  if(argc==3) analysisTemplate(argv[1], argv[2]);
  else return 1;
  return 0; 
}

void analysisTemplate(char* inName, char* outName){

  //arguments:
  //char are in and out filenames
  // inName is a pattern - all files beginning with that pattern are added to chain
  // file with histograms <<outName>>_Histograms.root is created

  //Tree variables - think twice before changing
  //------------------------------------------------------------------//

  double targetPMTQ[16];
  double vetoPMTQ[36];
  double targetPMTPed[16];
  double vetoPMTPed[36];
  unsigned long long eventTime;





  TChain *chain = new TChain("procData");  
  //TChain *histChain = new TChain("histTree"); 

  Char_t files[200];
  sprintf(files,"%s*",inName);

  cout<<"Looking for files matching: "<<files<<endl;
  chain->Add(files);
  //  histChain->Add(files);

  chain->SetBranchAddress("target_4Minus2Mean1",&targetPMTQ);
  chain->SetBranchAddress("veto_4Minus2Mean1",&vetoPMTQ);
  chain->SetBranchAddress("target_1",&targetPMTPed);
  chain->SetBranchAddress("veto_1",&vetoPMTPed);
  chain->SetBranchAddress("time",&eventTime);




  //Analysis variables - change at will
  //------------------------------------------------------------------//
  Char_t hName[100];
  Char_t hTitle[100];
  Long64_t nEvent = chain->GetEntries();


  const int maxNumPMTs =36;
  const int numPMTGroups =2;

  Char_t groupNames[numPMTGroups][50]={"Target","Veto"};
  Char_t physicsNames[numPMTGroups+1][50]={"Neutron","MuonInV","MuonInVT"};


  int numPMTs[numPMTGroups] ={16,36} ; //# of channels in each group

  int qRangeAll[numPMTGroups] ={5000,3000} ; // Max SPE Charge in each region; for histogram ranges
  int qRangePhysics[numPMTGroups] ={250,250} ; // Max SPE Charge in each region, physics events; for histogram ranges

  double signalThresholds[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt threshold; readin from file; based on fit to pedestal of (pedestal subtracted) signal gate
  double pmtQperSPE[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt spe calibration; readin from file; based on LED data

  double pedestalThresholdsLower[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt lower threshold for pedestal gate; readin from file; based on fit to pedestal of gate
  double pedestalThresholdsUpper[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt upper threshold for pedestal gate; readin from file; based on fit to pedestal of gate



  double qTotal[numPMTGroups];
  double qBal[numPMTGroups];

  Double_t qSqSum,qSum;
  double qPMT[numPMTGroups][maxNumPMTs]; 


  double qMuon =50; //charge above which a veto signal is considered a "muon"
  double qNeutronLower =20; //charge above which a veto signal is considered a "muon"
  double qNeutronUpper =200; //charge above which a veto signal is considered a "muon"

  int physicsEvent[numPMTGroups+1]; // 0 = neutron in target; 1= muon in veto; 2 = muon in veto and target
  int physicsEventCount[numPMTGroups+1];

  int tRangeAll[numPMTGroups] ={1000000,1000000} ; // Max interevent time in us for each region; for histogram ranges
  int tRangePhysics[numPMTGroups] ={200,200} ; // Max interevent time in us for each region; for histogram ranges
 
  const int timeArrayDepth =4;

  double physicsDeltaT[numPMTGroups+1][timeArrayDepth]; //time difference between last 5 phys events in each volume (us)

  unsigned long long physicsTimes[numPMTGroups+1][timeArrayDepth]; //time stamps of last 5 phsics events in each volume

  unsigned long long lastMuonV_Times[timeArrayDepth]; //time stamps of last muon in veto prior to neutron event in physicsTimes array
  double lastMuonV_DeltaT[timeArrayDepth]; //Dt of last muon in veto prior to neutron event in physicsTimes array

  unsigned long long lastMuonVT_Times[timeArrayDepth]; //time stamps of last muon in veto and target prior to neutron event in physicsTimes array
  double lastMuonVT_DeltaT[timeArrayDepth]; //Dt of last muon in veto and target prior to neutron event in physicsTimes array


  int baselineCut;

  //Analysis Histograms - change at will
  //------------------------------------------------------------------//

  TH2D* h_qTotal_qBal[numPMTGroups];
  TH2D* h_qTotal_qBal_Phy[numPMTGroups];

  TH1D* h_qTotal[numPMTGroups];
  TH1D* h_qTotal_Phy[numPMTGroups];

  TH1D* h_tInterEvent[numPMTGroups+1];
  TH1D* h_tInterEvent_Phy[numPMTGroups+1];

  TH1D* h_tLastMuonV;
  TH1D* h_tLastMuonV_Phy;

  TH1D* h_tLastMuonVT;
  TH1D* h_tLastMuonVT_Phy;

  TH1D* h_tLastMuonVT_DoubleN;
  TH1D* h_tLastMuonVT_DoubleN_Phy;



  //list for writing out histograms
  TList* hList = new TList();


  //Define Analysis Histograms - change at will
  //------------------------------------------------------------------//
  for (int j=0;j<numPMTGroups;j++) {

    sprintf(hName,"h_qTotal_qBal_%s",groupNames[j]);
    sprintf(hTitle,"qBal v  qTotal; %s qTotal (spe); Charge Balance",groupNames[j]);
    h_qTotal_qBal[j] = new TH2D(hName,hTitle,1000,-1*qRangeAll[j]/10,qRangeAll[j],150,-0.1,1.4);
    hList->Add(h_qTotal_qBal[j]);

    sprintf(hName,"h_qTotal_qBal_Physics_%s",groupNames[j]);
    sprintf(hTitle,"qBal v  qTotal; %s qTotal (spe); Charge Balance",groupNames[j]);
    h_qTotal_qBal_Phy[j] = new TH2D(hName,hTitle,1000,-1*qRangePhysics[j]/10,qRangePhysics[j],150,-0.1,1.4);
    hList->Add(h_qTotal_qBal_Phy[j]);

  
    sprintf(hName,"h_qTotal_%s",groupNames[j]);
    sprintf(hTitle,"qTotal; %s qTotal (spe); ",groupNames[j]);
    h_qTotal[j] = new TH1D(hName,hTitle,1000,-1*qRangeAll[j]/10,qRangeAll[j]);
    hList->Add(h_qTotal[j]); 

    sprintf(hName,"h_qTotal_Physics_%s",groupNames[j]);
    sprintf(hTitle,"qTotal; %s qTotal (spe); ",groupNames[j]);
    h_qTotal_Phy[j] = new TH1D(hName,hTitle,1000,-1*qRangePhysics[j]/10,qRangePhysics[j]);
    hList->Add(h_qTotal_Phy[j]); 
  }

  for (int j=0;j<numPMTGroups+1;j++) {
    sprintf(hName,"h_tInterEvent_%s",physicsNames[j]);
    sprintf(hTitle,"Inter-event Time (%s); Inter-event Time (%s) (#mus); ",physicsNames[j],physicsNames[j]);
    h_tInterEvent[j] = new TH1D(hName,hTitle,550,-1*tRangeAll[j]/10,tRangeAll[j]);
    hList->Add(h_tInterEvent[j]); 

    sprintf(hName,"h_tInterEvent_Physics_%s",physicsNames[j]);
    sprintf(hTitle,"Inter-event time (%s); Inter-event (%s) (#mus); ",physicsNames[j],physicsNames[j]);
    h_tInterEvent_Phy[j] = new TH1D(hName,hTitle,550,-1*tRangePhysics[j]/10,tRangePhysics[j]);
    hList->Add(h_tInterEvent_Phy[j]); 


  }

  sprintf(hName,"h_tLastMuonV_Physics");
  sprintf(hTitle,"Time since Last Muon in Veto; Time since last muon (#mus); ");
  h_tLastMuonV_Phy = new TH1D(hName,hTitle,550,-1*tRangePhysics[0]/10,tRangePhysics[0]);
  hList->Add(h_tLastMuonV_Phy); 

  sprintf(hName,"h_tLastMuonV");
  sprintf(hTitle,"Time since Last Muon in Veto; Time since last muon (#mus); ");
  h_tLastMuonV = new TH1D(hName,hTitle,220,-1*tRangeAll[0]/10,tRangeAll[0]);
  hList->Add(h_tLastMuonV); 

  sprintf(hName,"h_tLastMuonVT_Physics");
  sprintf(hTitle,"Time since Last Muon in Veto and Target; Time since last muon (#mus); ");
  h_tLastMuonVT_Phy = new TH1D(hName,hTitle,550,-1*tRangePhysics[0]/10,tRangePhysics[0]);
  hList->Add(h_tLastMuonVT_Phy); 

  sprintf(hName,"h_tLastMuonVT");
  sprintf(hTitle,"Time since Last Muon in Veto and Target; Time since last muon (#mus); ");
  h_tLastMuonVT = new TH1D(hName,hTitle,220,-1*tRangeAll[0]/10,tRangeAll[0]);
  hList->Add(h_tLastMuonVT); 

  sprintf(hName,"h_tLastMuonVT_DoubleN_Physics");
  sprintf(hTitle,"Time since Last Muon in Veto and Target to double neutron; Time since last muon (#mus); ");
  h_tLastMuonVT_DoubleN_Phy = new TH1D(hName,hTitle,550,-1*tRangePhysics[0]/10,tRangePhysics[0]);
  hList->Add(h_tLastMuonVT_DoubleN_Phy); 

  sprintf(hName,"h_tLastMuonVT_DoubleN");
  sprintf(hTitle,"Time since Last Muon in Veto and Target to double n; Time since last muon (#mus); ");
  h_tLastMuonVT_DoubleN = new TH1D(hName,hTitle,220,-1*tRangeAll[0]/10,tRangeAll[0]);
  hList->Add(h_tLastMuonVT_DoubleN); 



  // readin pedestal  mean, sigma, signal pedestal sigma.
  //being done here for 1st file in chain - should be made into function, so can be updated for each file in main event loop

  cout<<"open first file in chain"<<endl;

  TFile* firstFile = chain->GetFile();

  TTree *histTree = (TTree *)firstFile->Get("histTree");


  cout<<"Setup obj arrays"<<endl;

  TObjArray* signalHistArray;
  TObjArray* pedestalHistArray;

  cout<<"Setup branches"<<endl;
  histTree->SetBranchAddress("gateCharge", &signalHistArray);
  histTree->SetBranchAddress("pedestalCharge", &pedestalHistArray);

  Long64_t nHistEvent = histTree->GetEntries();
  cout<<nHistEvent<<" entries in HistChain"<<endl;
  cout<<"get entry"<<endl;
  //histTree->GetEntry(0);
 
  // cout<<"define hists"<<endl;
  // TH1F* signalHists[52];
  // TH1F* pedestalHists[52];
  // TF1* signalFits[52];
  // TF1* pedestalFits[52];
  
  // cout<<"Get sig and ped hists"<<endl;
  // for (int k=0;k<(numPMTs[0] + numPMTs[1]);k++) {
  //   cout<<"PMT "<<k<<endl;
  //   signalHists[k] = (TH1F*)signalHistArray->At(k);
  //   signalFits[k] = signalHists[k]->GetFunction("g_fit");

  //   pedestalHists[k] = (TH1F*)pedestalHistArray->At(k);
  //   pedestalFits[k] = pedestalHists[k]->GetFunction("g_fit");
  // }

  // firstFile->Close();

  // cout<<"Get sig and ped fits"<<endl;
  // int totalPMTCounter;
  // for (int j=0;j<numPMTGroups;j++) {
  //   for (int k=0;k<numPMTs[j];k++) {
  //     totalPMTCounter = j*numPMTs[0]+k; //need to index total pmt # as well; only works becuse there are two groups - beware

  //     cout<<"PMT "<<k<<"\t"<<totalPMTCounter<<endl; 

  //    signalThresholds[j][k] = 4 *  signalFits[totalPMTCounter]->GetParameter(2); //4 sigma above mean; par(2) of gaus fit is sigma; mean is already subtracted from tree value
  //     pedestalThresholdsLower[j][k] = pedestalFits[totalPMTCounter]->GetParameter(1) - 4 *  pedestalFits[totalPMTCounter]->GetParameter(2); //4 sigma below mean; par(2) of gaus fit is sigma, par(1) is mean
  //     pedestalThresholdsUpper[j][k] = pedestalFits[totalPMTCounter]->GetParameter(1) + 4 *  pedestalFits[totalPMTCounter]->GetParameter(2); //4 sigma below mean; par(2) of gaus fit is sigma, par(1) is mean

  //   }
  // }

  //temp init of spe cal values
  for (int j=0;j<numPMTGroups;j++) {
    physicsEventCount[j]=0;

    for (int k=0;k<maxNumPMTs;k++) {
      pmtQperSPE[j][k] = 200; //approx 
      signalThresholds[j][k] =0.25; //approx 1/4 spe
    }
  }

  cerr<<"There are "<<nEvent<<" events"<<endl;
  
  for(Long64_t i=0;i<nEvent;i++){    // scan through events
    if(i%100000==0) cerr<<i<<" / "<<nEvent<<endl;
    chain->GetEntry(i);

    //check if current event is transition to another file in chain
    //if so update data from pedestal fits
    //TODO - temp filled above


    //check if all pedestals are in range - i.e. that baseline is stable for each PMT
    //skip this event if _any_ PMT has unstable baseline
    // baselineCut=1;
    // for (int k=0;k<numPMTs[0];k++) {
    //   if ( (targetPMTPed[k] < pedestalThresholdsLower[0][k]) || (targetPMTPed[k] >pedestalThresholdsUpper[0][k]) ) baselineCut=0;
    // }
    // for (int k=0;k<numPMTs[1];k++) {
    //   if ( (vetoPMTPed[k] < pedestalThresholdsLower[1][k]) || (vetoPMTPed[k] >pedestalThresholdsUpper[1][k]) ) baselineCut=0;
    // }

    // if (baselineCut ==1) {
      //init and fill analysis variables
      for (int k=0;k<numPMTs[0];k++) {
	qPMT[0][k] = targetPMTQ[k]/pmtQperSPE[0][k];
      }
      for (int k=0;k<numPMTs[1];k++) {
	qPMT[1][k] = vetoPMTQ[k]/pmtQperSPE[1][k];
      }


      for (int j=0;j<numPMTGroups;j++) {
	qTotal[j]=0;
	qBal[j]=0;
	for (int k=0;k<numPMTs[j];k++) {
	  //cout<<"i,j,k: "<<i<<"\t"<<j<<"\t"<<k<<"\t"<<endl;
	  qTotal[j]+=  qPMT[j][k];
	}
      }



      //calc qBal
      for (int j=0;j<numPMTGroups;j++) {
	qSqSum = 0;
	qSum = 0;
	for (int k=0;k<numPMTs[j];k++){
	  if ( qPMT[j][k] > signalThresholds[j][k]) {
	    qSum += qPMT[j][k];
	    qSqSum += qPMT[j][k] * qPMT[j][k];
	  }
	}
	if (qSum>0) qBal[j] = sqrt( qSqSum/(qSum * qSum) -1/numPMTs[j]);
	else qBal[j] =-0.05;
      }

      //determine if the event contains a "neutron" or "muon" in various volumes
      //neutron in target
      if ((qBal[0] >0) && (qBal[0]<0.95) && (qTotal[0]> qNeutronLower) && (qTotal[0]< qNeutronUpper)) physicsEvent[0]=1; 
      else physicsEvent[0]=0;
      //muon in veto
      if ((qBal[1] >0) && (qBal[1]<0.95) && (qTotal[1]> qMuon)) physicsEvent[1]=1;
      else physicsEvent[1]=0;
      //muon in veto and target
      if ((qBal[1] >0) && (qBal[1]<0.95) && (qTotal[1]> qMuon) && (qBal[0] >0) && (qBal[0]<0.95) && (qTotal[0]> qNeutronUpper)) physicsEvent[2]=1; 
      else physicsEvent[2]=0;

      for (int j=0;j<numPMTGroups+1;j++) {
	physicsEventCount[j]+=physicsEvent[j];
	if (physicsEvent[j] ==1 ) {

	  //shuffle down exisiting timing entries
	  for (int k=0;k<timeArrayDepth-1;k++){
	    physicsDeltaT[j][k] = physicsDeltaT[j][k+1];
	    physicsTimes[j][k] = physicsTimes[j][k+1];
	  }
	  physicsTimes[j][timeArrayDepth-1] = eventTime;
	  physicsDeltaT[j][timeArrayDepth-1] = (eventTime - physicsTimes[j][timeArrayDepth-2])*0.004; //4ns per sample
	}
      }

      //determine times for last muon in veto before neutron
      if (physicsEvent[0] ==1 ) { // "neutron" in current event
	for (int k=0;k<timeArrayDepth-1;k++){
	  lastMuonV_DeltaT[k] = lastMuonV_DeltaT[k+1];
	  lastMuonV_Times[k] = lastMuonV_Times[k+1];
	}
	lastMuonV_Times[timeArrayDepth-1] = physicsTimes[1][timeArrayDepth-1]; //physicsTimes[1] holds muon (inV ) timestamps
	lastMuonV_DeltaT[timeArrayDepth-1] = (physicsTimes[0][timeArrayDepth-1] - physicsTimes[1][timeArrayDepth-1])*0.004; //4ns per sample; taking difference between last neutron and last muon (in V) time stamps to get to time to last muon before current neutron
      }

      //determine times for last muon in veto and target before neutron
      if (physicsEvent[0] ==1 ) { // "neutron" in current event
	for (int k=0;k<timeArrayDepth-1;k++){
	  lastMuonVT_DeltaT[k] = lastMuonVT_DeltaT[k+1];
	  lastMuonVT_Times[k] = lastMuonVT_Times[k+1];
	}
	lastMuonVT_Times[timeArrayDepth-1] = physicsTimes[2][timeArrayDepth-1]; //physicsTimes[2] holds muon (in V&T) timestamps
	lastMuonVT_DeltaT[timeArrayDepth-1] = (physicsTimes[0][timeArrayDepth-1] - physicsTimes[2][timeArrayDepth-1])*0.004; //4ns per sample; taking difference between last neutron and last muon (in V&T) time stamps to get to time to last muon before current neutron
      }

      //fill histograms
      for (int j=0;j<numPMTGroups;j++) {
	//charge
	h_qTotal[j]->Fill(qTotal[j]);
	h_qTotal_qBal[j]->Fill(qTotal[j],qBal[j]);
	h_qTotal_Phy[j]->Fill(qTotal[j]);
	h_qTotal_qBal_Phy[j]->Fill(qTotal[j],qBal[j]);
      }
      for (int j=0;j<numPMTGroups+1;j++) {
	//timing
	if (physicsEventCount[j]>timeArrayDepth) { //make sure timing array were fully populated
	  if (physicsEvent[j] ==1 ) {
	    h_tInterEvent[j]->Fill(physicsDeltaT[j][1]);
	    h_tInterEvent_Phy[j]->Fill(physicsDeltaT[j][1]);
	  }
	}

      }


      //time to last muon:
      if (physicsEventCount[0]>timeArrayDepth) { //make sure timing array were fully populated
	if (physicsEvent[0] ==1 ) {
	  h_tLastMuonV->Fill(lastMuonV_DeltaT[1]);
	  h_tLastMuonV_Phy->Fill(lastMuonV_DeltaT[1]);
	  h_tLastMuonVT->Fill(lastMuonVT_DeltaT[1]);
	  h_tLastMuonVT_Phy->Fill(lastMuonVT_DeltaT[1]);
	
	  //look for isolated double neutrons and apply muon veto
	
	  if (lastMuonV_DeltaT[1]>100.0) { //no muon in veto within 100us of neutron[1]
	    if ( (physicsDeltaT[0][1]) < 50.0) { //two neutrons within 50us (neutron[1],neutron[2])
	      if ((physicsDeltaT[0][0]) > 50.0 && (physicsDeltaT[0][2]) > 50.0 ) { //two neutrons are isolated: > 50us to (neutron[0],neutron[3]) from (neutron[1],neutron[2])
		h_tLastMuonVT_DoubleN->Fill(lastMuonVT_DeltaT[1]);
		h_tLastMuonVT_DoubleN_Phy->Fill(lastMuonVT_DeltaT[1]);
	      }
	    }
	  }

	}
      }


      if(i%10000000==0) { //writeout hist at intervals of 10M events

	//write out histograms
	char oname[200];

	sprintf(oname,"%s_Histograms.root",outName);
	TFile outfile(oname, "recreate");
  
	hList->Write();
  
	outfile.Close();

      
      }
    
      //  }

  }

  


  //write out histograms
  char oname[200];

  sprintf(oname,"%s_Histograms.root",outName);
  TFile outfile(oname, "recreate");
  
  hList->Write();
  
  outfile.Close();


}
