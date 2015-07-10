// Description: manager for output data to a ROOT file
//
// Author(s):
//  - creation by Andrii Tykhonov, 27/01/2014

#include "DmpRootEventDataManager.hh"
#include "DmpCore.h"

#include "G4SystemOfUnits.hh"

#include "TFile.h"
#include "TTree.h"

#include "DmpLog.h"
#include "DmpCore.h"

DmpRootEventDataManager::DmpRootEventDataManager()

  :keepMCgeneratorDebugInfo (true     ),
   calEcut                  (-1       ),
   siDigicut                (-1       )
{

  fDmpSimAlg = (DmpSimAlg*)gCore->AlgorithmManager()->Get("Sim/SimAlg");


  // * Header
  dmpSimuRunEventHeaderMaker    = new DmpSimuRunEventHeaderMaker();

  // * Auxiliary simulation info
  dmpSimuPrimariesEventMaker    = new DmpSimuPrimariesEventMaker();
  dmpSimuTrajectoryEventMaker   = new DmpSimuTrajectoryEventMaker();

  // * BGO
  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpSimuBgoHitEventMaker       = new DmpSimuBgoHitEventMaker();
  }

  // * STK
  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkStepEventMaker      = new DmpSimuStkStepEventMaker();
    dmpSimuStkHitEventMaker       = new DmpSimuStkHitEventMaker();
    dmpSimuStkDigiEventMaker      = new DmpSimuStkDigiEventMaker();
  }

  // * PSD
  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpSimuPsdHitEventMaker       = new DmpSimuPsdHitEventMaker();
  }

  // * NUD
  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpSimuNudHitEventMaker       = new DmpSimuNudHitEventMaker();
  }

  //create a messenger for this class
  rootMessenger = new DmpRootEventDataMessenger(this);  //* TODO: rootMessenger is DEPRECATED and should be removed! (use python interfaces instead of Geant4 ones)


  //G4cout << "ROOT FILE NAME: " << rootFileName << G4endl;
  //rootMessenger = new DmpRootNtupleMessenger();



}

DmpRootEventDataManager::~DmpRootEventDataManager()
{
  //if ( rootFile ) delete rootFile;
  delete dmpSimuRunEventHeaderMaker;
  // * General
  delete dmpSimuPrimariesEventMaker;
  delete dmpSimuTrajectoryEventMaker;

  // * STK
  if(fDmpSimAlg->StkSimulationEnabled()){
    delete dmpSimuStkStepEventMaker;
    delete dmpSimuStkHitEventMaker;
    delete dmpSimuStkDigiEventMaker;
  }

  // * PSD
  if(fDmpSimAlg->PsdSimulationEnabled()){
    delete dmpSimuPsdHitEventMaker;
  }

  // * NUD
  if(fDmpSimAlg->NudSimulationEnabled()){
    delete dmpSimuNudHitEventMaker;
  }

  // * BGO
  if(fDmpSimAlg->BgoSimulationEnabled()){
    delete dmpSimuBgoHitEventMaker;
  }

  delete rootMessenger;   //* TODO: rootMessenger is DEPRECATED and should be removed! (use python interfaces instead of Geant4 ones)
}

void DmpRootEventDataManager::Book(const G4Run* aRun)
{ 

  //dmpSimuTrajectoryEventMaker  ->book(aRun, tree);
  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkHitEventMaker      ->AddCollections(aRun);
    dmpSimuStkDigiEventMaker     ->AddCollections(aRun);
    dmpSimuStkStepEventMaker     ->AddCollections(aRun);
  }

  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpSimuPsdHitEventMaker      ->AddCollections(aRun);
  }

  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpSimuNudHitEventMaker      ->AddCollections(aRun);
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpSimuBgoHitEventMaker      ->AddCollections(aRun);
  }

 /*
 fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
 fDataMgr->BookAllBranches();
 */


}

void DmpRootEventDataManager::BeginEvent(const G4Event* evt)
{
  //fDataMgr->ResetEvent();

  dmpSimuRunEventHeaderMaker  ->BeginEvent(evt);
  dmpSimuPrimariesEventMaker  ->BeginEvent(evt);
  dmpSimuTrajectoryEventMaker ->BeginEvent(evt);

  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkHitEventMaker     ->BeginEvent(evt);
    dmpSimuStkDigiEventMaker    ->BeginEvent(evt);
    dmpSimuStkStepEventMaker    ->BeginEvent(evt);
  }

  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpSimuPsdHitEventMaker     ->BeginEvent(evt);
  }

  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpSimuNudHitEventMaker     ->BeginEvent(evt);
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpSimuBgoHitEventMaker     ->BeginEvent(evt);
  }

}

void DmpRootEventDataManager::BeginRun(const G4Run* aRun)
{
  //fDataMgr->ResetRun();
  dmpSimuRunEventHeaderMaker  ->BeginRun(aRun);
  dmpSimuPrimariesEventMaker  ->BeginRun(aRun);
  dmpSimuTrajectoryEventMaker ->BeginRun(aRun);

  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkStepEventMaker    ->BeginRun(aRun);
    dmpSimuStkDigiEventMaker    ->BeginRun(aRun);
    dmpSimuStkHitEventMaker     ->BeginRun(aRun);
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpSimuBgoHitEventMaker     ->BeginRun(aRun);
  }

  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpSimuPsdHitEventMaker     ->BeginRun(aRun);
  }

  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpSimuNudHitEventMaker     ->BeginRun(aRun);
  }
}


void DmpRootEventDataManager::Save()
{
  //fDataMgr->SaveOutput();
}


void DmpRootEventDataManager::FillStep(const G4Step* aStep)
{
  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkStepEventMaker ->FillStep(aStep);
  }
}


void DmpRootEventDataManager::FillEvent(const G4Event* evt)
{  
  // *
  // * Fill event
  // *
  if(keepMCgeneratorDebugInfo){
    dmpSimuRunEventHeaderMaker   ->FillEvent(evt);
    dmpSimuPrimariesEventMaker   ->FillEvent(evt);
  }
  dmpSimuTrajectoryEventMaker  ->FillEvent(evt);

  if(fDmpSimAlg->StkSimulationEnabled()){
    dmpSimuStkHitEventMaker      ->FillEvent(evt);
    dmpSimuStkDigiEventMaker     ->FillEvent(evt);
  }
  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpSimuPsdHitEventMaker      ->FillEvent(evt);
  }

  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpSimuNudHitEventMaker      ->FillEvent(evt);
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpSimuBgoHitEventMaker      ->FillEvent(evt);
  }

  // *
  // * Put event to the tree
  // *
  bool toKeep = true;
  if(fDmpSimAlg->StkSimulationEnabled()){
    if(!dmpSimuStkDigiEventMaker->passTotalDigiCut(siDigicut))   toKeep = false;
  }

  // *   TODO: replace BGO ntuple thing with Event Class
  if(fDmpSimAlg->BgoSimulationEnabled()){
    if(!dmpSimuBgoHitEventMaker ->passTotalEnergyCut(calEcut))  toKeep = false;
  }

  if(toKeep){
    //fDataMgr->FillEvent();
  }
}


void DmpRootEventDataManager::FillRun(const G4Run* aRun, DmpSimuPrimaryGeneratorAction* dmpSimuPrimaryGeneratorAction)
{
  dmpSimuRunEventHeaderMaker -> FillRun(aRun, dmpSimuPrimaryGeneratorAction);
  //fDataMgr->FillRun();
}
