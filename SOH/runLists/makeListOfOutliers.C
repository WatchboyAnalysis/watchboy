{
  
  gROOT->SetStyle("Plain");
  TString runNumSt;
  Int_t runNum;

  Int_t numfiles =0;
  TString files[20000];
  ifstream runList;
  ofstream excludeList;
  excludeList.open("excludedRuns.txt");
    
  //read in the runlist
  // cerr<<"Looking at Data from "<<listName<<endl;
  runList.open("allRuns.txt");
  while(!runList.eof()){
    runList>>files[numfiles];
    numfiles++;
  }
  runList.close();
  
  Int_t numfiles=0;
  ifstream readData;
  readData.open("outliers.txt");
  while(!readData.eof()){
    readData>>runNumSt;
    runNum=runNumSt.Atoi();
    excludeList<<files[runNum]<<endl;
    //rate[numfiles]=rateSt.Atof();
    //cerr<<rate[numfiles]<<endl;
    numfiles++;
  }
  readData.close();
  excludeList.close();


}
