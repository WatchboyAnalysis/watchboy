#include "Reader.h"
#include "Writer.h"
#include "Processor.h"
#include <numeric>
#include <vector>
#include <cmath>
#include <iostream>

RN::Processor::Processor(RN::Reader* reader, RN::Writer* writer)
  : mReader(reader), mWriter(writer)
{
  // charge cut is actually number of hit pmts above threshold
  target_charge_cut = 0;
  veto_charge_cut = 36;
  target_cb_cut = 1.0;
  veto_cb_cut = 1.0;
}

RN::Processor::~Processor(){}

void RN::Processor::Process()
{
  while(mReader->isAlive())
  {
    mReader->NextEvent();
    GetVariables();
    SumCharge();
    if( target_hits < target_charge_cut || veto_hits > veto_charge_cut )
      continue;
    ChargeBalance();
    if( target_cb > target_cb_cut || veto_cb > veto_cb_cut )
      continue;
    FillWriter();
    mWriter->Fill();
  }
  mWriter->Write();
  return;
}

void RN::Processor::GetVariables()
{
  target_charge.clear();
  veto_charge.clear();
  for(int i=0; i<16; i++)
    if(mReader->target_4Minus2Mean1[i] >= mReader->gateMean[i] + 4*std::abs(mReader->gateDev[i]))
      target_charge.push_back(mReader->target_4Minus2Mean1[i]);
  for(int i=0; i<36; i++)
    if(mReader->veto_4Minus2Mean1[i] >= mReader->gateMean[i+16] + 4*std::abs(mReader->gateDev[i+16]))
      veto_charge.push_back(mReader->veto_4Minus2Mean1[i]);
  time = mReader->time + mReader->time_offset;

  return;
}

void RN::Processor::ChargeBalance()
{
  // target
  double N = target_charge.size();
  N=16;
  double sumsq = std::inner_product(target_charge.begin(), target_charge.end(), 
				    target_charge.begin(), 0.0);
  double sum = std::accumulate(target_charge.begin(), target_charge.end(), 0.0);
  target_cb = 0;
  if(N>0)
    target_cb = std::sqrt( sumsq/(sum*sum) - 1/N );



  N = veto_charge.size();
  N=36;
  sumsq = std::inner_product(veto_charge.begin(), veto_charge.end(), 
			     veto_charge.begin(), 0.0);
  sum = std::accumulate(veto_charge.begin(), veto_charge.end(), 0.0);
  veto_cb = 0;
  if(N>0)
    veto_cb = std::sqrt( sumsq/(sum*sum) - 1/N );

  return;
}

void RN::Processor::SumCharge()
{
  target_total = std::accumulate(target_charge.begin(), target_charge.end(), 0.0);
  veto_total = std::accumulate(veto_charge.begin(), veto_charge.end(), 0.0);
  return;
}

void RN::Processor::FillWriter()
{
  mWriter->target_total = target_total;
  mWriter->veto_total = veto_total;
  mWriter->target_cb = target_cb;
  mWriter->veto_cb = veto_cb;
  mWriter->time = time;
  return;
}
