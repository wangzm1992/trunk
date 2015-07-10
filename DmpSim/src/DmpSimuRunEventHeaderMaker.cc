// Description: ntuple maker for the STK
//
// Author(s):
//  - creation by X.Wu, 15/07/2013

#include "DmpSimuRunEventHeaderMaker.hh"

#include "TTree.h"

#include "DmpLog.h"
#include "DmpCore.h"
#include "DmpIOSvc.h"

DmpSimuRunEventHeaderMaker::DmpSimuRunEventHeaderMaker()
  :
  //tree(0), treeRun(0),
  m_runNumber(-1)
{

}

DmpSimuRunEventHeaderMaker::~DmpSimuRunEventHeaderMaker()
{
}


/*
void DmpSimuRunEventHeaderMaker::book(const G4Run* aRun, TTree* aTree)
{  


  //tree = aTree;
  //dmpSimuEventHeader = new DmpEvtSimuHeader();
  //tree->Branch("DmpEvtSimuHeader",  "DmpEvtSimuHeader",     &dmpSimuEventHeader  );
  //m_runNumber = aRun->GetRunID();

}
*/



void DmpSimuRunEventHeaderMaker::BeginEvent(const G4Event* evt)
{
  //clear all ntuple data vectors here
  //set event number

  dmpSimuEventHeader->Reset();
}



void DmpSimuRunEventHeaderMaker::FillStep(const G4Step* aStep)
{
}

void DmpSimuRunEventHeaderMaker::FillEvent(const G4Event* evt)
{  
  dmpSimuEventHeader->SetRunNumber(m_runNumber);
  dmpSimuEventHeader->SetEventNumber(evt->GetEventID());
}

//void DmpSimuRunEventHeaderMaker::beginRun(const G4Run* aRun, TTree* aTree)
void DmpSimuRunEventHeaderMaker::BeginRun(const G4Run* aRun)
{
  //clear all ntuple data vectors here
  //set event number

  /*
  treeRun = aTree;
  dmpSimuRunHeader = new DmpRunSimuHeader();
  treeRun->Branch("DmpRunSimuHeader",  "DmpRunSimuHeader",     &dmpSimuRunHeader  );
  dmpSimuRunHeader->Reset();
  */

  m_runNumber = aRun->GetRunID();
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  //dmpSimuEventHeader = (DmpEvtSimuHeader*)(fDataMgr->GetOutContainer("DmpEvtSimuHeader"));
  //dmpSimuRunHeader = (DmpRunSimuHeader*)(fDataMgr->GetMetadataContainer());

  dmpSimuEventHeader = new DmpEvtSimuHeader();
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpEvtSimuHeader",dmpSimuEventHeader);

  dmpSimuRunHeader = new DmpRunSimuHeader();
  DmpIOSvc::GetInstance()->AddOutputPerJobContainer("DmpRunSimuHeader",dmpSimuRunHeader);

}


void DmpSimuRunEventHeaderMaker::FillRun(const G4Run* aRun , DmpSimuPrimaryGeneratorAction* dmpSimuPrimaryGeneratorAction)
{
  //dmpSimuRunHeader = new DmpRunSimuHeader();
  //DmpIOSvc::GetInstance()->AddOutputContainer("DampeSimulationRunMetadata/DmpRunSimuHeader",dmpSimuRunHeader);

  dmpSimuRunHeader->SetRunNumber(aRun->GetRunID());
  dmpSimuRunHeader->SetEventNumber(aRun->GetNumberOfEvent());
  //particleSource comes from Simulation/DmpSim/include/DmpSimuPrimaryGeneratorAction.hh
  dmpSimuRunHeader->SetSpectrumType(dmpSimuPrimaryGeneratorAction->GetSpectrumType());
  dmpSimuRunHeader->SetSourceType(dmpSimuPrimaryGeneratorAction->GetSourceType());
  //dmpSimuRunHeader->SetVertexRadius(dmpSimuPrimaryGeneratorAction->GetVertexRadius());
  dmpSimuRunHeader->SetVertexRadius((int)dmpSimuPrimaryGeneratorAction->GetParticleSource()->GetCurrentSource()->GetPosDist()->GetRadius());
  DmpLogDebug <<"[DmpSimuRunEventHeaderMaker::FillRun] Vertex Radius " << dmpSimuPrimaryGeneratorAction->GetParticleSource()->GetCurrentSource()->GetPosDist()->GetRadius() << DmpLogEndl;
  DmpLogDebug <<"[DmpSimuRunEventHeaderMaker::FillRun] Vertex Radius H " <<dmpSimuRunHeader->GetVertexRadius() << DmpLogEndl;
  dmpSimuRunHeader->SetSourceGen(dmpSimuPrimaryGeneratorAction->GetSourceGen());
  dmpSimuRunHeader->SetMaxEne(dmpSimuPrimaryGeneratorAction->GetParticleSource()->GetCurrentSource()->GetEneDist()->GetEmax());
  dmpSimuRunHeader->SetMinEne(dmpSimuPrimaryGeneratorAction->GetParticleSource()->GetCurrentSource()->GetEneDist()->GetEmin());

  

//G4cout <<" MaxEne " << dmpSimuPrimaryGeneratorAction->GetParticleSource()->GetCurrentSource()->GetEneDist()->GetEmax() << DmpLogEndl;


  //dmpSimuRunHeader->SetEventNumber(evt->GetEventID())
}



