#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <string>
#include <vector>
#include <cmath>

#include <map>
typedef std::map<unsigned long long, int> EventMap;
typedef std::pair<unsigned long long, int> EventPair;

void organize(char* fname);

int main(int argc, char* argv[])
{
  for(int i=1; i<argc; i++)
  {
    organize(argv[i]);
    std::cout << "Organized: " << argv[i] << std::endl;
  }

  return 0;
}

void organize(char* fname)
{
  TFile* file = new TFile(fname);
  TTree* proc = (TTree*)file->Get("procData");
  TTree* pure = (TTree*)file->Get("pureData"); // organized the same?
  TTree* configTree = (TTree*)file->Get("configTree");
  TTree* slowTree = (TTree*)file->Get("slowTree");
  TTree* histTree = (TTree*)file->Get("histTree");
  
  unsigned long long time;

  proc->SetBranchStatus("*", 0);
  proc->SetBranchStatus("time", 1);
  proc->SetBranchAddress("time", &time);
  const int entries = proc->GetEntries();

  // Fill the map (this sorts the events by time)
  EventMap procMap;

  for(int i=0; i<entries; i++)
  {
    if(!(i%1000))
      std::cout << "Sorting the procTree: \033[33;1m"
		<< std::floor(double(i)/entries*100) << "%\r\033[37;0m" << std::flush;
    proc->GetEvent(i);
    procMap.insert(EventPair(time, i));
  }
  std::cout << std::endl;

  proc->SetBranchStatus("*", 1);

  

  TFile* outfile = new TFile(fname, "recreate");
  TTree* newproc = proc->CloneTree(0);
  TTree* newpure = pure->CloneTree(0);

  int counter;
  for(EventMap::iterator it=procMap.begin();
      it!=procMap.end(); ++it)
  {
    proc->GetEvent(it->second);
    pure->GetEvent(it->second);
    newproc->Fill();
    newpure->Fill();
    counter++;
    if(!(counter%1000))
      std::cout << "Filling the new tree: \033[33;1m"
  		<< std::floor(double(counter)/entries*100) << "%\r\033[37;0m" << std::flush;
  }
  std::cout << std::endl << "copying over config, slow, and hist trees" << std::endl;
  TTree* newconfig = configTree->CloneTree();
  TTree* newslow = slowTree->CloneTree();
  TTree* newhist = histTree->CloneTree();

  outfile->Write("", TObject::kOverwrite);
  return;
}
