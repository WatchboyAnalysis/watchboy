{
  gROOT->SetStyle("Plain");
  Int_t numMonths=12;
  Double_t mNum[numMonths];
  Double_t lT[numMonths];
  Double_t lT_tot=0;
  TH1F *h1= new TH1F("h1","LiveTime per Month",numMonths*10,(13.+7/12.),13+(7+numMonths)/12.);
  TH1F *h2= new TH1F("h2","Integrated LiveTime per Month",numMonths*10,(13.+7/12.),13+(7+numMonths)/12.);
  h1->SetXTitle("Year and Month [yy+mm/12]");
  h1->SetYTitle("Livetime [s]");
  h1->SetMarkerColor(2);
  h2->SetMarkerColor(4);
  h1->SetLineColor(2);
  h2->SetLineColor(4);
  h1->SetMarkerStyle(3);
  h2->SetMarkerStyle(3);
  lT[0]=413804;//july13
  lT[1]=1442450;//august 13
  lT[2]=1632630;//september 13
  lT[3]=1775980;// october 13
  lT[4]=272455; //november 13
  lT[5]=977572;//december 13
  lT[6]=1486860;//january 14
  lT[7]=723268;//february 14
  lT[8]=2313300;//march 14
  lT[9]=2463690;//april 14
  lT[10]=1830400;//may 14
  lT[11]=995472;//june14
  for( int i=0;i<numMonths;i++){
    mNum[i]=13.0+(i+7)/12.;
    cerr<<mNum[i]<<endl;
    //lT[i]=2600000;
    lT_tot+=lT[i];
    h1->Fill(mNum[i],lT[i]);
    h2->Fill(mNum[i],lT_tot);
  }
  h2->Draw("P");
  h1->Draw("Psame");


}
