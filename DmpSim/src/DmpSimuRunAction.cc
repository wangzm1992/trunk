// Description: This class hinerits from G4UserRunAction. It is used to
// evoke the save() method of the ntuple manager at the end of the run 
//
// Author(s):
//  - creation by X.Wu, 09/07/2013

#include "DmpSimuRunAction.hh"
#include "DmpRootEventDataManager.hh"
#include "DmpSimuPrimaryGeneratorAction.hh"
//#include "DmpSimSvcDataMgr.h"


#include <stdlib.h>
#include "G4Run.hh"
#include "G4UImanager.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"

#include "DmpCore.h"
#include "DmpLog.h"

DmpSimuRunAction::DmpSimuRunAction(DmpRootEventDataManager* rootEvtMgr, DmpSimuPrimaryGeneratorAction* rootPrimMgr)
  :rootEventDataManager(rootEvtMgr), dmpSimuPrimaryGeneratorAction(rootPrimMgr)
{
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
}

DmpSimuRunAction::~DmpSimuRunAction()
{
}

void DmpSimuRunAction::BeginOfRunAction(const G4Run* aRun)
{  
	//std::cout<<"  --- a1\n";
  DmpLogInfo << "[DmpSimuRunAction::BeginOfRunAction] ### Run " << aRun->GetRunID() << " start." << DmpLogEndl;
  //std::cout<<"  --- a2\n";
  rootEventDataManager->Book(aRun);
  //std::cout<<"  --- a3\n";
  rootEventDataManager->BeginRun(aRun);
  //std::cout<<"  --- a4\n";
  //can also set random seed for multiple jobs of the same simulation
  /***
  if ( fReadRndm ) {
    G4cout << "\n---> rndm status restored from file: " << fRndmFileName << DmpLogEndl;
    CLHEP::HepRandom::restoreEngineStatus(fRndmFileName);
    CLHEP::HepRandom::showEngineStatus();
  }   
  ***/
  //fDataMgr->BookBranch();
}


void DmpSimuRunAction::EndOfRunAction(const G4Run* aRun)
{


  rootEventDataManager->FillRun(aRun, dmpSimuPrimaryGeneratorAction);
  DmpLogInfo << "[DmpSimuRunAction::EndOfRunAction]  End of Run, saving the ntuple " << DmpLogEndl;
  rootEventDataManager->Save();
  //fDataMgr->SaveOutput();

  //can also save random seed for multiple jobs of the same simulation
  /***
  if ( fSaveRndm ) { 
    G4int runNumber = run->GetRunID();
    std::ostringstream fileName;
    fileName << "run" << runNumber << "end.rndm";
    CLHEP::HepRandom::saveEngineStatus(fileName.str().c_str()); 
    CLHEP::HepRandom::showEngineStatus();
  }     
  ***/
}









