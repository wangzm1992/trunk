// Description: This class hinerits from G4VDigitizerModule. It is used to
// manage the digitization of all the sub-detectors  
//
// Author(s):
//  - creation by X.Wu, 09/07/2013

#include "DmpSimuDigitization.hh"
#include "DmpSimuStkDigi.hh"

#include "DmpSimuPsdDigitizer.hh"
#include "DmpSimuPsdHit.hh"

#include "DmpSimuBgoDigitizer.hh"
#include "DmpSimuBgoHit.hh"

#include "DmpSimuStkDigitizer.hh"
#include "DmpSimuStkHit.hh"

#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4DigiManager.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"

#include "DmpCore.h"

#include <vector>

DmpSimuDigitization::DmpSimuDigitization(G4String name)
  :G4VDigitizerModule(name)
{

  fDmpSimAlg = (DmpSimAlg*)gCore->AlgorithmManager()->Get("Sim/SimAlg");

  //set the digitizer to the G4DigiManager
  G4DigiManager * fDM = G4DigiManager::GetDMpointer();

  if(fDmpSimAlg->PsdSimulationEnabled()){
    DmpSimuPsdDigitizer* dmpSimuPsdDigitizer = new DmpSimuPsdDigitizer( "DmpSimuPsdDigitizer" );
    fDM->AddNewModule(dmpSimuPsdDigitizer);
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    DmpSimuBgoDigitizer* dmpSimuBgoDigitizer = new DmpSimuBgoDigitizer( "DmpSimuBgoDigitizer" );
    fDM->AddNewModule(dmpSimuBgoDigitizer);
  }

  if(fDmpSimAlg->StkSimulationEnabled()){
    DmpSimuStkDigitizer* dmpSimuStkDigitizer = new DmpSimuStkDigitizer( "DmpSimuStkDigitizer" );
    fDM->AddNewModule(dmpSimuStkDigitizer);
  }
  
}

DmpSimuDigitization::~DmpSimuDigitization()
{
}

void DmpSimuDigitization::Digitize()
{

  G4DigiManager * fDM = G4DigiManager::GetDMpointer();

  if(fDmpSimAlg->PsdSimulationEnabled()){
    DmpSimuPsdDigitizer * psdDM = (DmpSimuPsdDigitizer*)fDM->FindDigitizerModule( "DmpSimuPsdDigitizer" );
    psdDM->Digitize();
  }

  if(fDmpSimAlg->BgoSimulationEnabled()){
    DmpSimuBgoDigitizer * bgoDM = (DmpSimuBgoDigitizer*)fDM->FindDigitizerModule( "DmpSimuBgoDigitizer" );
    bgoDM->Digitize();
  }
 
  if(fDmpSimAlg->StkSimulationEnabled()){
    DmpSimuStkDigitizer * stkDM = (DmpSimuStkDigitizer*)fDM->FindDigitizerModule( "DmpSimuStkDigitizer" );
    stkDM->Digitize();
  }
}









