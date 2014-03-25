#include <TH1F.h>
#include <TFile.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TKey.h>
#include <string>
#include <iostream>

void clean(char* fname);
void dispose(char* fname);

int main(int argc, char* argv[])
{
  for(int i=1; i<argc; i++)
  {
    clean(argv[i]);
    dispose(argv[i]);
    std::cout << "Processed: " << argv[i] << std::endl;
  }

  return 1;
}

void clean(char* fname)
{
  TFile* file = new TFile(fname, "update");
  if(file->Get("histTree"))
    return;

  TTree* tree = new TTree("histTree", "contains charge histograms");
  TObjArray* hists = new TObjArray();
  for(long long int i=0; i<52; i++)
  {
    std::string name="histogram"+std::to_string(i);
    hists->Add(file->Get(name.c_str()));
  }
  tree->Branch("pedestalCharge", &hists);
  tree->Fill();

  file->Write("", TObject::kOverwrite);
  file->Close();
  delete file;
  return;
}

void dispose(char* fname)
{
  TFile* file = new TFile(fname, "update");
  for(long long int i=0; i<52; i++)
  {
    std::string name="histogram"+std::to_string(i)+";1";
    file->Delete(name.c_str());
  }

  file->Purge();
  //file->Write("", TObject::kOverwrite);
  file->Write();
  file->Close();
  delete file;
  return;
}
