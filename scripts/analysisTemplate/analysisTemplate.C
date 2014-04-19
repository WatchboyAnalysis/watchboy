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
  unsigned long long eventTime;





  TChain *chain = new TChain("procData");  
  //TChain *histChain = new TChain("histTree"); 

  Char_t files[200];
  sprintf(files,"%s*",inName);

  cout<<"Looking for files matching: "<<files<<endl;
  chain->Add(files);

  chain->SetBranchAddress("target_4Minus2Mean1",&targetPMTQ);
  chain->SetBranchAddress("veto_4Minus2Mean1",&vetoPMTQ);
  chain->SetBranchAddress("time",&eventTime);


  //Analysis variables - change at will
  //------------------------------------------------------------------//
  Char_t hName[100];
  Char_t hTitle[100];
  Long64_t nEvent = chain->GetEntries();

  const int maxNumPMTs =36;
  const int numPMTGroups =2;

  Char_t groupNames[nPMTGroups][50]={"Target","Veto"};


  int numPMTs[2] ={16,32} ; //# of channels in each group

  double pmtThresholds[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt threshold; readin from file; based on fit to pedestal
  double pmtQperSPE[numPMTGroups][maxNumPMTs]; //contains pmt-by-pmt spe calibration; readin from file; based on LED data



  double qTotal[numPMTGroups];
  double qBal[numPMTGroups];

  double eventTimeSeconds;

  Double_t qSqSum,qSum;
  double qPMT[numPMTGroups][maxNumPMTs]; 

  //Analysis Histograms - change at will
  //------------------------------------------------------------------//

  TH2D* h_qTotal_qBal[numPMTGroups];

  TH1D* h_qTotal[numPMTGroups];

  //list for writing out histograms
  TList* hList = new TList();


  //Define Analysis Histograms - change at will
  //------------------------------------------------------------------//
  for (int j=0;j<numPMTGroups;j++) {

    sprintf(hName,"h_qTotal_qBal_%s",groupNames[j]);
    sprintf(hTitle,"qBal v  qTotal; %s qTotal (spe); Charge Balance",groupNames[j]);
    h_qTotal_qBal[i] = new TH2D(hName,hTitle,420,-10,200,150,-0.1,1.4);
    hList->Add(h_qTotal_qBal[j]);

    sprintf(hName,"h_qTotal_%s",groupNames[j]);
    sprintf(hTitle,"qTotal; %s qTotal (spe); ",groupNames[j]);
    h_qTotal[i] = new TH1D(hName,hTitle,420,-10,200);
    hList->Add(h_qTotal[j]); 
  }



  //temp init of threshold and spe values
  for (int j=0;j<numPMTGroups;j++) {
    for (int k=0;k<maxNumPMTs;k++) {
      pmtThresholds[j][k] = 100/200; //approx 4 sigma above threshold
      pmtQperSPE[j][k] = 200; //approx 
    }
  }

  cerr<<"There are "<<nEvent<<" events"<<endl;
  
  for(Long64_t i=0;i<nEvent;i++){    // scan through events
    if(i%100000==0) {
      cerr<<i<<" / "<<nEvent<<endl;
  
      //check if current event is transition to another file in chain
      //if so update data from pedestal fits
      //TODO - temp filled above

      //init and fill analysis variables

      for (int k=0;k<numPMTs[0];k++) {
	qPMT[0][k] = targetPMTQ[k]/pmtQperSPE[0][k];
      }
      for (int k=0;k<numPMTs[1];k++) {
	qPMT[1][k] = targetPMTQ[k]/pmtQperSPE[1][k];
      }


      for (int j=0;j<numPMTGroups;j++) {
	qTotal[j]=0;
	qBal[j]=0;
	for (int k=0;k<numPMTs[j];k++) {
	  qTotal[j]+=  qPMT[j][k];
	}
      }

      //calc qBal
      for (int j=0;j<numPMTGroups;j++) {
	qSqSum = 0;
	qSum = 0;
	for (int k=0;j<numPMTs[j];k++){
	  if ( qPMT[j][k] > pmtThresholds[j][k]) {
	    qSum += qPMT[j][k];
	    qSqSum += qPMT[j][k] * qPMT[j][k];
	  }
	}
	if (qSum>0) qBal[j] = sqrt( qSqSum/(qSum * qSum) -1/numPMTs[j]);
	else qBal[j] =-0.05;
      }


      //fill histograms
      for (int j=0;j<numPMTGroups;j++) {
	h_qTotal[j]->Fill(qTotal[j]);
	h_qTotal_qBal[j]->Fill(qTotal[j],qBal[j]);
      }






    }
  }

  


  //write out histograms
  char oname[200];

  sprintf(oname,"%s_Histograms.root",outName);
  TFile outfile(oname, "recreate");
  
  hList->Write();
  
  outfile.Close();


}
