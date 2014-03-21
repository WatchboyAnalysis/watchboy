#define DEBUG false

#include <TTree.h>
#include <TFile.h>
#include <iostream>
#include <cstdlib>
using namespace std;

int main( int argc, char** argv )
{
  TFile* infile = new TFile(argv[2]);
  TTree* tree = (TTree*)infile->Get(argv[1]);
  int entries = tree->GetEntries();
  if(DEBUG) entries=100;

  int begin_at = atoi( argv[3] );
  for(int i=begin_at; i < entries; i++)
  {
    
    if( tree->GetEvent(i) == -1 )
    {
      cout << i << endl << 1 << endl;
      delete tree;
      exit(0);
    }
  }
  cout << entries - 1 << endl << -1 << endl;
  delete tree;
  return 0;
}
