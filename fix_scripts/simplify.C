#define DEBUG false
//
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
//#include "../library/SLMarsDetector_16.h"

// TODO: add the PMT's from the orphan tree and muon paddles
typedef std::map<double, int> EventMap;
typedef std::pair<double, int> EventPair;

class SLMarsDetector_16;
class SLMarsDetector_2;
class SLSis3316EventHeader_ROOT;

streampos trash_events(string name, ifstream& file);
//void fillgates(double pmt[], SLMarsDetector_16* detector, int it);
void fillgates(double pmt[], SLSis3316EventHeader_ROOT header[], int it);
void fillgates(double pmt[], double errorcharge);
void addslowtree(TTree* slow);
void addconfigtree(TTree* config);
//void fillgates(double pmt[], SLMarsDetector_2* detector, int it);
// some typedefs for my maps
// map will be map<time, eventnum>

// Changing from adc time to ns
unsigned long long adc2ns = 1;

void simplify(string input_name)
{
  gSystem->Load("../library/libRN.so");

  // ifstream temp_file("good_blocks_of_data_fastTree.temp");
  // ifstream temp_o1_file("good_blocks_of_data_orphanTree1.temp");
  // ifstream temp_o2_file("good_blocks_of_data_orphanTree2.temp");

  ifstream temp_file( (input_name+".fastTree.temp").c_str() );
  ifstream temp_o1_file( (input_name + ".orphanTree1.temp").c_str() );
  ifstream temp_o2_file( (input_name + ".orphanTree2.temp").c_str() );

  string cc;
  string filename;
  string outname;
  string trash_can;
  temp_file >> filename;
  temp_file >> outname;
  // Remove the first two lines from temp_o1 and o2
  temp_o1_file >> trash_can;
  temp_o1_file >> trash_can;
  temp_o2_file >> trash_can;
  temp_o2_file >> trash_can;

  //int start_event = trash_events(filename, begin);
  TFile* file = new TFile(filename.c_str());
  TTree* tree = (TTree*)file->Get("fastTree");
  TTree* orphan1 = (TTree*)file->Get("orphanTree1");
  TTree* orphan2 = (TTree*)file->Get("orphanTree2");

  // Setup the output file
  TFile* output = new TFile(outname.c_str(), "RECREATE");

  // Add in the slow tree information
  addslowtree((TTree*)file->Get("slowTree"));
  addconfigtree((TTree*)file->Get("configTree"));
  //TTree* smartTree = new TTree("data", "Simplified Data Structure");
  // Declare Needed branches here:
  unsigned long long event_time;
  // double target_pmt_charge[16];
  // double veto_pmt_charge[36];
  // double target_summed_charge;
  // double veto_summed_charge;
  // smartTree->Branch("target_pmt_charge", target_pmt_charge, "target_pmt_charge[16]/D");
  // smartTree->Branch("veto_pmt_charge", veto_pmt_charge, "veto_pmt_charge[36]/D");
  // smartTree->Branch("target_summed_charge", &target_summed_charge);
  // smartTree->Branch("veto_summed_charge", &veto_summed_charge);
  // smartTree->Branch("time", &event_time);

  // Tree with pure gate information
  double target_gate_charge[16][8];
  unsigned long long target_time[16];
  double veto_gate_charge[36][8];
  unsigned long long veto_time[36];
  TTree* gateTree = new TTree("pureData", "Contains all of the data from each gate");
  gateTree->Branch("target_pmt", target_gate_charge, "target_pmt[16][8]/D");
  gateTree->Branch("target_time", target_time, "target_time[16]/l");
  gateTree->Branch("veto_pmt", veto_gate_charge, "veto_pmt[36][8]/D");
  gateTree->Branch("veto_time", veto_time, "veto_time[36]/l");

  SLMarsDetector_16* detector;
  SLMarsDetector_2* orphan_det1;
  SLMarsDetector_2* orphan_det2;

  // SLMarsDetector_16 contains an array of SLSis3316EventHeader_ROOT

  tree->SetBranchAddress("detector_event.", &detector);
  orphan1->SetBranchAddress("veto_paddle_event_1.", &orphan_det1);
  orphan2->SetBranchAddress("veto_paddle_event_2.", &orphan_det2);

  SLSis3316EventHeader_ROOT* header = detector->channels;
  SLSis3316EventHeader_ROOT* header_o1 = orphan_det1->channels;
  SLSis3316EventHeader_ROOT* header_o2 = orphan_det2->channels;


  double prev_time = 0;
  double current_time = 0;
  int events = tree->GetEntries();
  int orphan_events1 = orphan1->GetEntries();
  int orphan_events2 = orphan2->GetEntries();

  //tree->GetEvent(start_event);

  int interesting_events = 0;
  int total_events = 0;
  int last_event = 0;
  bool is_consecutive = false;

  // Check that the timestamps matchup for each event
  // It turns out that events can be out of order so here
  // are some tools to deal with that
  EventMap map1024, map1280, map1408, mapOrphan1, mapOrphan2;
  double timecheck[5]={0,0,0,0,0};
  int bad_counter=0;
  double idcheck[3]={0,0,0};

  vector<int> o1_events_fromfile, o2_events_fromfile;
  cout << "Filling Orphan Vectors\n";
  while( !temp_o1_file.eof() )
  {
    temp_o1_file >> cc;
    o1_events_fromfile.push_back(atoi(cc.c_str()));
  }
  while( !temp_o2_file.eof() )
  {
    temp_o2_file >> cc;
    o2_events_fromfile.push_back(atoi(cc.c_str()));
  }
  cout << "Producing map of orphan events map<timestamp, event_num>\n";

  // There is a performance issue occuring with std::map.erase
  // which in theory runs in constant time. However it may not
  // be playing nicely with cint and is causing this code to run
  // like ~N^(1.5). It standard root fashion, here is a workaround...
  EventMap copyOrphan1, copyOrphan2;
  for(vector<int>::iterator it=o1_events_fromfile.begin();
      it!=o1_events_fromfile.end(); ++it)
  {
    orphan1->GetEvent(*it);
    mapOrphan1.insert(EventPair(header_o1[0].Timestamp*adc2ns, *it));
    copyOrphan1.insert(EventPair(header_o1[0].Timestamp*adc2ns, *it));
  }
  for(vector<int>::iterator it=o2_events_fromfile.begin();
      it!=o2_events_fromfile.end(); ++it)
  {
    orphan2->GetEvent(*it);
    mapOrphan2.insert(EventPair(header_o2[0].Timestamp*adc2ns, *it));
    copyOrphan2.insert(EventPair(header_o2[0].Timestamp*adc2ns, *it));
  }
  o1_events_fromfile.clear();
  o2_events_fromfile.clear();


  TStopwatch watch;
  watch.Start();
  vector<double> watchtimes;

  double errorcharge=-10000;
  ULong64_t errortime=0;

  cout << "Beginning to loop over events from file" << endl;
  while( !temp_file.eof() )
  {
    if(temp_file.eof())break;
    temp_file >> cc;
    int event_nums[3];
    event_nums[0] = atoi(cc.c_str());
    if(temp_file.eof())break;
    // Whenever there is a break in the events (or beginning) need to trash
    if( event_nums[0] - last_event != 1 )
    {
      cout << endl << ":::   Trashing events " << last_event;
      streampos pos = trash_events(filename, temp_file);
      temp_file.seekg(pos);
      string cc_in_loop;
      temp_file >> cc_in_loop;
      event_nums[0] = atoi(cc_in_loop.c_str());
      tree->GetEvent(event_nums[0]);
      cout << " -> " << event_nums[0] << endl;
    }
    if(temp_file.eof())break;
    temp_file >> cc;
    event_nums[1] = atoi(cc.c_str());
    if(temp_file.eof())break;
    temp_file >> cc;
    event_nums[2] = atoi(cc.c_str());

    // Make sure the three events are consecutive in number
    if( event_nums[1] - event_nums[0] == 1 && event_nums[2] - event_nums[1] == 1 )
    {
      //target_summed_charge = 0;
      //veto_summed_charge = 0;
      int error_test = tree->GetEvent(event_nums[0]);
      event_time = header[0].Timestamp*adc2ns;
      timecheck[0]=header[0].Timestamp*adc2ns;
      idcheck[0]=header[0].ChannelID;
      for( int j = 0; j < 16; j++ )
      {
	// target_pmt_charge[j] = header[j].AccumSumGate3 -
	//   (30/12.)*header[j].AccumSumGate1;
	fillgates(target_gate_charge[j], header, j);
	target_time[j]=header[j].Timestamp*adc2ns;
	//target_summed_charge += target_pmt_charge[j];
      }
      // Grab veto information
      tree->GetEvent(event_nums[1]);
      timecheck[1]=header[0].Timestamp*adc2ns;
      idcheck[1]=header[0].ChannelID;
      for( int j = 0; j < 16; j++ )
      {
	// veto_pmt_charge[j] = header[j].AccumSumGate4 -
	//   (60.0/12.0)*header[j].AccumSumGate1;
	fillgates(veto_gate_charge[j], header, j);
	veto_time[j]=header[j].Timestamp*adc2ns;
	//veto_summed_charge += veto_pmt_charge[j];
      }
      tree->GetEvent(event_nums[2]);
      timecheck[2]=header[0].Timestamp*adc2ns;
      idcheck[2]=header[0].ChannelID;
      for( int j = 0; j < 16; j++ )
      {
	// veto_pmt_charge[j+16] = header[j].AccumSumGate4 -
	//   (60.0/12.0)*header[j].AccumSumGate1;
	fillgates(veto_gate_charge[j+16], header, j);
	veto_time[j+16]=header[j].Timestamp*adc2ns;
	//veto_summed_charge += veto_pmt_charge[j+16];
      }
      last_event = event_nums[2];

      if( event_nums[0] % 10000 == 0 | event_nums[0] % 10000 == 1 | event_nums[0] % 10000 == 2 )
      {
	double evpersecond = watch.RealTime() / 10000;
	cout << "Processed event: " << event_nums[0] << " with a rate of " << evpersecond
	     << " events per second\r" << flush;
	watch.Start();
      }

      if( idcheck[0] != 1024 || idcheck[1] != 1280 || idcheck[2] != 1408 ||
	  fabs(timecheck[0]-timecheck[1])>10 || fabs(timecheck[0]-timecheck[2])>10 )
      {
	bad_counter++;
	for(int blah=0; blah<3; blah++)
	{
	  if(idcheck[blah]==1024)
	    map1024.insert(EventPair(timecheck[blah], event_nums[blah]));
	  else if(idcheck[blah]==1280)
	    map1280.insert(EventPair(timecheck[blah], event_nums[blah]));
	  else if(idcheck[blah]==1408)
	    map1408.insert(EventPair(timecheck[blah], event_nums[blah]));
	  else
	    cout << "Found an event that doesn't belong to these adc cards ... look into this";
	}
      }
      else // event is good in fast tree, so go ahead and fill in from the orphans
      {
	// Orphan PMTs, must be within 10 of event_time
	int orphan_last=0;
	int o1_event=-1;
	double orphan_error = 20;

	for(EventMap::iterator o1_it=mapOrphan1.begin();
	    o1_it!=mapOrphan1.end(); ++o1_it)
	{
	  double o_time = fabs( double(o1_it->first) - double(event_time) );
	  if( o_time < orphan_error )
	  {
	    o1_event=o1_it->second;
	    mapOrphan1.erase(mapOrphan1.begin(), o1_it);
	    break;
	  }
	  // If the orphan time has been removed, or not written, skip
	  if( double(o1_it->first) - double(event_time) > 100 )break;
	}
	if(o1_event != -1)
	{
	  orphan1->GetEvent(o1_event);
	  for(int j=0; j<2; j++)
	  {
	    // veto_pmt_charge[j+32]=header_o1[j].AccumSumGate4 -
	    //   (60.0/12.0)*header_o1[j].AccumSumGate1;
	    fillgates(veto_gate_charge[j+32], header_o1, j);
	    veto_time[j+32]=header_o1[j].Timestamp*adc2ns;
	    //veto_summed_charge += veto_pmt_charge[j+32];
	  }
	}
	else
	  for(int j=0; j<2; j++)
	  {
	    fillgates(veto_gate_charge[j+32], errorcharge);
	    veto_time[j+32]=errortime;
	  }

	int o2_event=-1;
	for(EventMap::iterator o2_it=mapOrphan2.begin();
	    o2_it!=mapOrphan2.end(); ++o2_it)
	{
	  double o_time = fabs( double(o2_it->first) - double(event_time) );
	  if( o_time < orphan_error )
	  {
	    o2_event=o2_it->second;
	    mapOrphan2.erase(mapOrphan2.begin(), o2_it);
	    break;
	  }
	  // If the orphan time has been removed, or not written, skip
	  if( double(o2_it->first) - double(event_time) > 100 )break;
	}
	if(o2_event != -1)
	{
	  orphan2->GetEvent(o2_event);
	  for(int j=0; j<2; j++)
	  {
	    // veto_pmt_charge[j+34]=header_o2[j].AccumSumGate4 -
	    //   (60.0/12.0)*header_o2[j].AccumSumGate1;
	    fillgates(veto_gate_charge[j+34], header_o2, j);
	    veto_time[j+34]=header_o2[j].Timestamp*adc2ns;
	    //veto_summed_charge += veto_pmt_charge[j+34];
	  }
	}
	else
	  for(int j=0; j<2; j++)
	  {
	    fillgates(veto_gate_charge[j+34], errorcharge);
	    veto_time[j+34]=errortime;
	  }

  	//smartTree->Fill();
	gateTree->Fill();
      }	// finished filling orphans
    }
  }

  cout << "\nfinished the main loop\n";
  cout << "Adding back (" <<map1024.size()<< ") misordered events\n";
  // Now fill up the output file with all of those bad events fixed up


  if( map1024.size()!=map1280.size() || map1024.size()!=map1408.size() )
    cout << "DOOOOOM!!!, we didn't write to all of the cards equally!!!"
  	 << "\n 1024:" << map1024.size() << " 1280:" << map1280.size() << " 1408:" << map1408.size() << endl;

  EventMap::iterator it1280=map1280.begin(), it1408=map1408.begin();
  double allowed_time_error = 10;

  int mapcounter;

  for( EventMap::iterator it1024=map1024.begin();
       it1024!=map1024.end(); ++it1024 )
  {
    ++mapcounter;
    cout << mapcounter << " of " << map1024.size()
	 << " events added back\r" << flush;

    int event1=it1024->second;
    int event2=it1280->second;
    int event3=it1408->second;

    tree->GetEvent(event1);
    event_time = header[0].Timestamp*adc2ns;
    for( int j = 0; j < 16; j++ )
    {
      // target_pmt_charge[j] = header[j].AccumSumGate3 -
      // 	(30/12.)*header[j].AccumSumGate1;
      fillgates(target_gate_charge[j], header, j);
      target_time[j]=header[j].Timestamp*adc2ns;
      //target_summed_charge += target_pmt_charge[j];
    }
    tree->GetEvent(event2);
    for( int j = 0; j < 16; j++ )
    {
      // veto_pmt_charge[j] = header[j].AccumSumGate4 -
      // 	(60.0/12.0)*header[j].AccumSumGate1;
      fillgates(veto_gate_charge[j], header, j);
      veto_time[j]=header[j].Timestamp*adc2ns;
      //veto_summed_charge += veto_pmt_charge[j];
    }
    tree->GetEvent(event3);
    for( int j = 0; j < 16; j++ )
    {
      // veto_pmt_charge[j+16] = header[j].AccumSumGate4 -
      // 	(60.0/12.0)*header[j].AccumSumGate1;
      fillgates(veto_gate_charge[j+16], header, j);
      veto_time[j+16]=header[j].Timestamp*adc2ns;
      //veto_summed_charge += veto_pmt_charge[j+16];
    }
    // Orphan PMTs, must be within 20 of event_time
    int orphan_last=0;
    int o1_event=-1;
    for(EventMap::iterator o1_it=copyOrphan1.begin();
	o1_it!=copyOrphan1.end(); ++o1_it)
    {
      double o_time = fabs( double(o1_it->first) - double(event_time) );
      if( o_time < 20 )
      {
	o1_event=o1_it->second;
	copyOrphan1.erase(copyOrphan1.begin(), o1_it);
	break;
      }
      // If the orphan time has been removed, or not written, skip
      if( double(o1_it->first) - double(event_time) > 100 )break;
    }
    if(o1_event != -1)
    {
      orphan1->GetEvent(o1_event);
      for(int j=0; j<2; j++)
      {
	// veto_pmt_charge[j+32]=header_o1[j].AccumSumGate4 -
	//   (60.0/12.0)*header_o1[j].AccumSumGate1;
	fillgates(veto_gate_charge[j+32], header_o1, j);
	veto_time[j+32]=header_o1[j].Timestamp*adc2ns;
	//veto_summed_charge += veto_pmt_charge[j+32];
      }
    }
    else
      for(int j=0; j<2; j++)
      {
	fillgates(veto_gate_charge[j+32], errorcharge);
	veto_time[j+32]=errortime;
      }

    int o2_event=-1;
    for(EventMap::iterator o2_it=copyOrphan2.begin();
	o2_it!=copyOrphan2.end(); ++o2_it)
    {
      double o_time = fabs( double(o2_it->first) - double(event_time) );
      if( o_time < 20 )
      {
	o2_event=o2_it->second;
	copyOrphan2.erase(copyOrphan2.begin(), o2_it);
	break;
      }
      // If the orphan time has been removed, or not written, skip
      if( double(o2_it->first) - double(event_time) > 100 )break;
    }
    if(o2_event != -1)
    {
      orphan2->GetEvent(o2_event);
      for(int j=0; j<2; j++)
      {
	// veto_pmt_charge[j+34]=header_o2[j].AccumSumGate4 -
	//   (60.0/12.0)*header_o2[j].AccumSumGate1;
	fillgates(veto_gate_charge[j+34], header_o2, j);
	veto_time[j+34]=header_o2[j].Timestamp*adc2ns;
	//veto_summed_charge += veto_pmt_charge[j+34];
      }
    }
    else
      for(int j=0; j<2; j++)
      {
	fillgates(veto_gate_charge[j+34], errorcharge);
	veto_time[j+34]=errortime;
      }

    gateTree->Fill();
    //smartTree->Fill();
    ++it1280;
    ++it1408;
  }

  cout << "\nWriting to file" << endl;

  output->Write("", TObject::kOverwrite);
  output->Close();
  delete tree;
  delete file;

  return;
}

streampos trash_events(string name, ifstream& ifile)
{
  TFile* file = new TFile(name.c_str());
  TTree* tree = (TTree*)file->Get("fastTree");
  string line;

  streampos pos;

  SLMarsDetector_16* detector;
  tree->SetBranchAddress("detector_event.", &detector);
  SLSis3316EventHeader_ROOT* header=detector->channels;
  bool is_trash = true;
  streampos pos_start = ifile.tellg();
  ifile >> line;
  int event = atoi( line.c_str() );
  ifile.clear();
  ifile.seekg(pos_start);

  tree->GetEvent(event);
  while( is_trash )
  {
    streampos temp_pos, last_pos;
    pos = ifile.tellg();
    if( header[0].ChannelID == 1024 )
    {
      temp_pos = ifile.tellg();
      ifile >> line;
      event = atoi(line.c_str());
      tree->GetEvent(event);
      if( header[0].ChannelID == 1280 )
      {
	temp_pos = ifile.tellg();
	ifile >> line;
	event = atoi(line.c_str());
	tree->GetEvent(event);
	if( header[0].ChannelID == 1408 )
	{
	  is_trash = false;
	  ifile.seekg(last_pos);
	}
      }
    }
    else{
      temp_pos = ifile.tellg();
      ifile >> line;
      event = atoi( line.c_str() );
      tree->GetEvent(atoi(line.c_str()));
    }
    last_pos = temp_pos;

  }

  return ifile.tellg();
}


void fillgates(double pmt[], SLSis3316EventHeader_ROOT header[], int it)
{
  pmt[0] = header[it].AccumSumGate1;
  pmt[1] = header[it].AccumSumGate2;
  pmt[2] = header[it].AccumSumGate3;
  pmt[3] = header[it].AccumSumGate4;
  pmt[4] = header[it].AccumSumGate5;
  pmt[5] = header[it].AccumSumGate6;
  pmt[6] = header[it].AccumSumGate7;
  pmt[7] = header[it].AccumSumGate8;

  return;
}

void fillgates(double pmt[], double errorcharge)
{
  for(int i=0; i<8; i++)
    pmt[i]=errorcharge;

  return;
}

void addslowtree(TTree* slow)
{
  // Slow tree has two events, one at the beginning
  // of a run, and another at the end
  TTree* tree = new TTree("slowTree", "Detector information at run start/end");

  double target_voltages[16],
    target_currents[16],
    veto_voltages[36],
    veto_currents[36];

  // Gate widths in ns
  double target_gate_start[8]={ adc2ns * 0, adc2ns * 12, adc2ns * 12, adc2ns * 12,
				adc2ns * 12, adc2ns * 72, adc2ns * 72, adc2ns * 72 };
  double target_gate_end[8]={ adc2ns * 12, adc2ns * 32, adc2ns * 42, adc2ns * 52,
			      adc2ns * 72, adc2ns * 102, adc2ns * 112, adc2ns * 122 };
  double target_gate_width[8];

  double veto_gate_start[8]={ adc2ns * 0, adc2ns * 12, adc2ns * 12, adc2ns * 12,
				adc2ns * 12, adc2ns * 72, adc2ns * 72, adc2ns * 72 };
  double veto_gate_end[8]={ adc2ns * 12, adc2ns * 42, adc2ns * 52, adc2ns * 72,
			      adc2ns * 102, adc2ns * 102, adc2ns * 112, adc2ns * 122 };
  double veto_gate_width[8];

  for(int i=0; i<8; i++)
  {
    target_gate_width[i] = target_gate_end[i] - target_gate_start[i];
    veto_gate_width[i] = veto_gate_end[i] - veto_gate_start[i];
  }

  tree->Branch("target_voltages", &target_voltages, "target_voltages[16]/D");
  tree->Branch("target_currents", &target_currents, "target_currents[16]/D");
  tree->Branch("veto_voltages", &veto_voltages, "veto_voltages[36]/D");
  tree->Branch("veto_currents", &veto_currents, "veto_currents[36]/D");
  tree->Branch("target_gate_start", &target_gate_start, "target_gate_start[8]/D");
  tree->Branch("target_gate_end", &target_gate_end, "target_gate_end[8]/D");
  tree->Branch("target_gate_width", &target_gate_width, "target_gate_width[8]/D");
  tree->Branch("veto_gate_start", &veto_gate_start, "veto_gate_start[8]/D");
  tree->Branch("veto_gate_end", &veto_gate_end, "veto_gate_end[8]/D");
  tree->Branch("veto_gate_width", &veto_gate_width, "veto_gate_width[8]/D");

  // There are two formats that the tree may be in depending on the high
  // voltage power supply configuration
  // For earlier data it's this:
  if( slow->GetLeaf("ISEG0.channel.measuredVoltage") )
  {
    // Voltages in this leaf setup are as follows:
    // ISEG0 (V16-V31, excluding V20 & V27)
    // ISEG1 (V0 - V15)
    // ISEG2 (T0-T15)
    // ISEG3 {32,33,34,35,20,27}

    TLeaf* VoltLeaf0 = slow->GetLeaf("ISEG0.channel.measuredVoltage");
    TLeaf* VoltLeaf1 = slow->GetLeaf("ISEG1.channel.measuredVoltage");
    TLeaf* VoltLeaf2 = slow->GetLeaf("ISEG2.channel.measuredVoltage");
    TLeaf* VoltLeaf3 = slow->GetLeaf("ISEG3.channel.measuredVoltage");
    TLeaf* CurrLeaf0 = slow->GetLeaf("ISEG0.channel.measuredCurrent");
    TLeaf* CurrLeaf1 = slow->GetLeaf("ISEG1.channel.measuredCurrent");
    TLeaf* CurrLeaf2 = slow->GetLeaf("ISEG2.channel.measuredCurrent");
    TLeaf* CurrLeaf3 = slow->GetLeaf("ISEG3.channel.measuredCurrent");

    for(int ent=0; ent<slow->GetEntries(); ent++)
    {
      slow->GetEvent(ent);
      for(int slot=0; slot<16; slot++)
      {
	target_voltages[slot] = VoltLeaf2->GetValue(slot);
	target_currents[slot] = CurrLeaf2->GetValue(slot);
	veto_voltages[slot] = VoltLeaf1->GetValue(slot);
	veto_currents[slot] = CurrLeaf1->GetValue(slot);
	if( slot!=4 && slot!=11 )
	{
	  veto_voltages[slot+16] = VoltLeaf0->GetValue(slot);
	  veto_currents[slot+16] = CurrLeaf0->GetValue(slot);
	}
      }
      // Now the less straigt forward pmts
      veto_voltages[32] = VoltLeaf3->GetValue(0);
      veto_voltages[33] = VoltLeaf3->GetValue(1);
      veto_voltages[34] = VoltLeaf3->GetValue(2);
      veto_voltages[35] = VoltLeaf3->GetValue(3);
      veto_voltages[20] = VoltLeaf3->GetValue(4);
      veto_voltages[27] = VoltLeaf3->GetValue(5);
      veto_currents[32] = CurrLeaf3->GetValue(0);
      veto_currents[33] = CurrLeaf3->GetValue(1);
      veto_currents[34] = CurrLeaf3->GetValue(2);
      veto_currents[35] = CurrLeaf3->GetValue(3);
      veto_currents[20] = CurrLeaf3->GetValue(4);
      veto_currents[27] = CurrLeaf3->GetValue(5);
      tree->Fill();
    }
  } // End ISEG style

  if(slow->GetLeaf("VHS_4_1.VM.0"))
  {
    const std::string target_card[16]={"12_3", "12_3", "12_3", "12_3", "12_3", "12_3",
				       "12_3", "12_3", "12_3", "12_3", "12_3", "12_3",
				       "4_1", "4_1", "4_1", "4_1"};
    const std::string target_channel[16]={"0", "1", "0", "3", "2", "5", "6", "7", "8",
					  "9", "10", "11", "0", "1", "2", "3"};
    const std::string veto_card[36]={"12_1", "12_1", "4_3", "12_1", "12_1", "12_1", "12_1",
				     "12_1", "12_1", "12_1", "12_1", "4_3", "12_1", "4_2",
				     "4_2", "12_1", "12_2", "12_2", "12_2", "12_2", "4_2",
				     "12_2", "4_2", "12_2", "12_2", "12_2", "12_2", "12_2",
				     "12_2", "12_2", "12_2", "12_2", "12_2", "12_2", "12_3", "12_3"};
    const std::string veto_channel[36]={"0", "6", "3", "2", "3", "4", "5",
					"1", "8", "7", "9", "2", "10", "3",
					"2", "11", "0", "4", "4", "1", "0",
					"2", "1", "6", "5", "3", "8", "9",
					"6", "7", "10", "11", "0", "0", "4", "4"};
    for(int evt=0; evt<slow->GetEntries(); evt++)
    {
      slow->GetEvent(evt);
      for(int slot=0; slot<16; slot++)
      {
	TLeaf* vleaf = slow->GetLeaf(("VHS_"+target_card[slot]+".VM."+target_channel[slot]).c_str());
	target_voltages[slot]=vleaf->GetValue(0);
	TLeaf* ileaf = slow->GetLeaf(("VHS_"+target_card[slot]+".IM."+target_channel[slot]).c_str());
	target_currents[slot]=ileaf->GetValue(0);
      }	// Filled Target
      for(int slot=0; slot<36; slot++)
      {
	if(veto_card[slot]=="4_3")
	{
	  veto_voltages[slot]=-1;
	  veto_currents[slot]=-1;
	} // Card 4_3 is a problem card in terms of reporting information
	else
	{
	  TLeaf* vleaf = slow->GetLeaf(("VHS_"+veto_card[slot]+".VM."+veto_channel[slot]).c_str());
	  veto_voltages[slot]=vleaf->GetValue(0);
	  TLeaf* ileaf =slow->GetLeaf(("VHS_"+veto_card[slot]+".IM."+veto_channel[slot]).c_str());
	  veto_currents[slot]=ileaf->GetValue(0);
	}
      }	// Filled Veto
      tree->Fill();
    } // Beginning and End Event Filled
  } // End VHS style

  return;
}

void addconfigtree(TTree* config)
{
  TTree* tree = config->CloneTree(config->GetEntries());
  return;
}
