{
    gROOT->SetStyle("Plain");
 
    TH1F *h1= new TH1F("h1","Sigma",12*750,0,12*31);
    TH1F *h2= new TH1F("h2","Average at Point",12*750,0,12*31);
    TH1F *h3= new TH1F("h3","difference between point and average",500,0,1000);
    TH1F *h4= new TH1F("h4","Rate at Point",12*750,0,12*31);
    TH1F *h5= new TH1F("h5","Outlier Rate",12*750,0,12*31);
    
    Int_t numfiles =0;
    
    
    
    TString files[2000];
    ifstream runList;
    //mName is a string that has the year and month of runs to be convereted in the formatyYYmMM
    TString mName="y14m06";
    TString listName=mName+".txt";
    TString dName="ratesAfterCuts/"+mName+"_cutRate.txt";
    TString oLName="outlierRuns/"+mName+"_outliers.txt";
    TString gRName="goodRuns/"+mName+"_goodRuns.txt";
    TString graphTitle=mName+": Rate vs Run for >1 PMT triggering in either Veto or Target";
    //read in the runlist
    runList.open(listName);
    // runList.open("marchTest");
    while(!runList.eof()){
        runList>>files[numfiles];
        numfiles++;
    }
    runList.close();
    
    //read in the rates for a given month
    TString runNumSt, rateSt;
    Int_t runNum[20000];
    Double_t rate[20000];
    Int_t numLines=0;
    ifstream readData;
    readData.open(dName);
    Int_t fileIndex[20000];
    while(!readData.eof()){
        readData>>runNumSt>>rateSt;
        runNum[numLines]=runNumSt.Atof();
        rate[numLines]=rateSt.Atof();
        fileIndex[numLines]=numLines+1;
        //cerr<<rate[numfiles]<<endl;
        numLines++;
    }
    readData.close();
    Int_t numSamples=25;
    Double_t avSum,sigSum;
    Double_t av,sig;
    int oL=0;
    Int_t n,n_b;
    
    Double_t x[numfiles],x_b[numfiles],y[numfiles],y_b[numfiles];
    ofstream outlierRuns;
    outlierRuns.open(oLName);
    ofstream goodRuns;
    goodRuns.open(gRName);
    sigSum=0;
    avSum=0;
    for(int i=0;i<numfiles;i++){
        x[i]=runNum[i];
        y[i]=rate[i];
        h4->Fill(i,rate[i]);
        avSum+=rate[i];
    }
    av=avSum/numfiles;
    cerr<<"average:"<<av<<endl;
    for(int j=0;j<numfiles;j++){
        sigSum+=(rate[j]-av)*(rate[j]-av);
    }
    sig=sigSum/(numfiles);
    sig=TMath::Sqrt(sig);
    for(int i=0;i<numfiles;i++){
        if(TMath::Abs(rate[i]-av)>.4*sig){
            // if(rate[i]>av+.4*sig || rate[i]<av-.4*sig){
            cerr<<"outlier for run "<<i<<": "<< sig<<" "<<rate[i]-av<<endl;
            //h5->Fill(i,rate[i]);
            x_b[oL]=runNum[i];
            y_b[oL]=rate[i];
            outlierRuns<<files[i+1]<<endl;
            oL++;
        }
        else goodRuns<<files[i+1]<<endl;
    }
    
    n=numfiles-1;
    n_b=oL-1;
    cerr<<oL<<" outliers"<<endl;
    gr1 = new TGraph(n,x,y);
    gr2 = new TGraph(n_b,x_b,y_b);
    //  gr->Draw("AC*");
    //gr2->SetLineColor(2);
    // gr2->Draw("CP");
    gr1->SetTitle(graphTitle);
    gr1->GetXaxis()->SetTitle("Run Number");
    gr1->GetYaxis()->SetTitle("Rate [HZ]");
    gr1->SetMarkerColor(4);
    gr1->Draw("AC*");
    // superimpose the second graph by leaving out the axis option "A" gr2->SetLineWidth(3);
    gr2->SetMarkerStyle(21);
    gr2->SetMarkerColor(2);
    gr2->Draw("P");
    outlierRuns.close();
    //h4->Draw("P0");
    //h5->Draw("P0same");
    //h1->Draw();
}
