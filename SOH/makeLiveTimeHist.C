{
    
  gROOT->SetStyle("Plain");
  TString month[24],time[24], year[24];
  ifstream dataList;

  int mtIn=0; 
  //read in the runlist
  //cerr<<"Looking at Data from "<<listName<<endl;
  dataList.open("livetimePoints.txt");
  // runList.open("marchTest");
  while(!dataList.eof()){
    dataList>>month[mtIn]>>year[mtIn]>>time[mtIn];
    mtIn++;
  }
  dataList.close();
  

  TH1F *h1= new TH1F("h1","Live Detector Time each Month of 2013",12,1,12);
  TH1F *h2= new TH1F("h2","Live Detector Time each Month of 2014",12,1,12);
  h1->SetXTitle("Month of 2013 [1-12]");
  h1->SetYTitle("Live Time [s]");
  h1->SetMarkerStyle(28);
  h1->SetMarkerColor(2);
  h2->SetMarkerStyle(28);
  h2->SetMarkerColor(2);
  h2->SetXTitle("Monthof 2014 [1-12]");
  h2->SetYTitle("Live Time [s]"); 
  for(int i=0;i<mtIn;i++){
    if(year[i].Atoi()==13){
      h1->Fill(month[i].Atof(),time[i].Atof());
    }
    else if(year[i].Atoi()==14){
      h2->Fill(month[i].Atof(),time[i].Atof());
    } 
  }
  h1->Draw("P");
}
