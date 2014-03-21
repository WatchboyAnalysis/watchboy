#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TLeaf.h>

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

int main(int argc, char** argv)
{
  // Build histogram of chi^2 values over many files
  // Write out to a new rootfile chiout.root
  TFile file("chiout.root", "recreate");
  TH2F* histogram = new TH2F("chisquared", "chisquared values of pedestal fit", 
			     52, 0, 52, 1000, 0, 2000);

  for(int i=1; i<argc; i++)
  {
    TFile fin(argv[i]);
    TTree* tin=(TTree*)fin.Get("slowTree");
    double chi2vals[52];
    tin->SetBranchAddress("fit_chi2perndf", &chi2vals);
    tin->GetEvent(0);

    for(int j=0; j<52; j++)
    {
      histogram->Fill(j, chi2vals[j]);
      if( chi2vals[j] > 500 )
      {
	std::cout << "Chi^2 high on pmt: "
		  << j << " file:["
		  << argv[i] << "]\n";
      }
    }
  }

  file.Write();
  return 0;
}
