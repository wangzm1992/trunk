// Description: This class hinerits from G4UserEventAction. It is used to
// access information in the G4Event and process them, for example digitization  
// and storage in the output ntuple
//
// Author(s):
//  - creation by X.Wu, 09/07/2013

#include "DmpSimuEventAction.hh"

#include "DmpRootEventDataManager.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4UImanager.hh"
#include "G4ios.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"
#include "G4DigiManager.hh"

#include "DmpSimuStkHit.hh"
#include "DmpSimuDigitization.hh"
#include "DmpEvtSimuHeader.h"
#include "DmpCore.h"
//#include "DmpSimSvcDataMgr.h"



DmpSimuEventAction::DmpSimuEventAction(DmpRootEventDataManager* rootEventManager)
  :stkHitCollID(-1),bgoHitCollID(-1),psdHitCollID(-1), drawFlag("all")
{ 
  // Get pointer to the simulation algorithm
  fDmpSimAlg = (DmpSimAlg*)gCore->AlgorithmManager()->Get("Sim/SimAlg");

  //set the digitizer to the G4DigiManager
  G4DigiManager * fDM = G4DigiManager::GetDMpointer();
  
  DmpSimuDigitization * myDM = new DmpSimuDigitization( "DmpSimuDigitization" );
  fDM->AddNewModule(myDM);
  
  rootEventDataManager = rootEventManager;
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
}

DmpSimuEventAction::~DmpSimuEventAction()
{
}

void DmpSimuEventAction::BeginOfEventAction(const G4Event* evt)
{

  //G4int evtNb = evt->GetEventID();
  G4SDManager * SDman = G4SDManager::GetSDMpointer();  
  
  //* STK
  if(fDmpSimAlg->StkSimulationEnabled()){
    if (stkHitCollID==-1) stkHitCollID = SDman->GetCollectionID("STKHitCollection");
  }

  //* PSD
  if(fDmpSimAlg->PsdSimulationEnabled()){
    if(psdHitCollID==-1) psdHitCollID = SDman->GetCollectionID("PSDHitCollection");
  }

  //* BGO
  if(fDmpSimAlg->BgoSimulationEnabled()){
    if(bgoHitCollID==-1) bgoHitCollID = SDman->GetCollectionID("BGOHitCollection");
  }
  
  //ask ntupleManager to clear ntuple data vectors
  rootEventDataManager->BeginEvent(evt);

  //fDataMgr->ResetEvent();

}

void DmpSimuEventAction::EndOfEventAction(const G4Event* evt)
{

  //G4int event_id = evt->GetEventID();
  
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  G4DigiManager * fDM = G4DigiManager::GetDMpointer();
  
  if (HCE)
  {

    //* STK
    if(fDmpSimAlg->StkSimulationEnabled()){
      DmpSimuStkHitsCollection* THC = 0;
      THC = (DmpSimuStkHitsCollection*)(HCE->GetHC(stkHitCollID));
    }

    DmpSimuDigitization * myDM = (DmpSimuDigitization*)fDM->FindDigitizerModule( "DmpSimuDigitization" );
    myDM->Digitize();
  }
 
  //ask ntupleManager to fill ntuple data vectors 
  rootEventDataManager->FillEvent(evt);


  /*
  DmpEvtSimuHeader* eventHeader = (DmpEvtSimuHeader*) fDataMgr->GetOutObject("DmpEvtSimuHeader");
  eventHeader->SetEventNumber(evt->GetEventID());
  //eventHeader->SetRunNumber(mRunNumber);
  std::cout<< "\n\n\n\n\n\n\n test:"<<((DmpEvtSimuHeader*) (fDataMgr->GetOutObject("DmpEvtSimuHeader")))->GetEventNuber()<<"\n\n\n\n\n";
  fDataMgr->FillEvent();
  */


}













