// Description: ntuple maker for the STK
//
// Author(s):
//  - creation by X.Wu, 15/07/2013

#include "DmpSimuStkStepEventMaker.hh"
#include "G4UnitsTable.hh"

#include "G4TrajectoryContainer.hh"
#include "G4VTrajectoryPoint.hh"
#include "G4VTrajectory.hh"
#include "G4Trajectory.hh"
#include "G4VProcess.hh"
#include "G4TransportationManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "DmpG4RunManager.hh"


#include "DmpDetectorConstruction.hh"
//#include "DmpSimuBgoHit.hh"
#include "DmpSimuStkHit.hh"
#include "DmpSimuStkDigi.hh"
#include "DmpSimuStkDigitizer.hh"

#include "G4RunManager.hh"
#include "G4DigiManager.hh"
#include "G4Run.hh"

#include "TH1D.h"
#include "TTree.h"

#include "DmpLog.h"
#include "DmpCore.h"
#include "DmpIOSvc.h"
#include "DmpSimuConfigParser.h"


DmpSimuStkStepEventMaker::DmpSimuStkStepEventMaker()
  //:tree(0)
  :debugEvent(-100), //-100 no event
   fSimuConfig(0),
   fSecondaryStepECut(0.) // MeV
{
  //set debugEvent to a particular event number for debug printout  
  //debugEvent = -1; //-1 is all events
  //debugEvent = -100; //-100 no event

  fSimuConfig      = (DmpSimuConfigParser*)(gCore->ServiceManager()->Get("DmpSimuConfigParser"));
  if(!fSimuConfig){
    DmpLogWarning<<"####################################################"<<DmpLogEndl;
    DmpLogWarning<<"#                                                  #"<<DmpLogEndl;
    DmpLogWarning<<"#  No configuration manager provided!              #"<<DmpLogEndl;
    DmpLogWarning<<"#   ==> Running with the default parameters...     #"<<DmpLogEndl;
    DmpLogWarning<<"#                                                  #"<<DmpLogEndl;
    DmpLogWarning<<"####################################################"<<DmpLogEndl;
  }
  else{
    std::vector<std::string> volumes = fSimuConfig->GetSTKMoreSteppingVolumes();
    for(int i=0; i<volumes.size();i++) fMoreSteppingVolumes.push_back(G4String(volumes.at(i)));
    //fMoreSteppingVolumes = fSimuConfig->GetSTKMoreSteppingVolumes();
    fSecondaryStepECut   = fSimuConfig->GetReal("STK", "SecondaryStepECut", 0.0);
  }

  DmpLogInfo<<"Adiditional stepping volumes: "<<DmpLogEndl;
  for(int i=0; i<fMoreSteppingVolumes.size();i++) DmpLogInfo<<"   "<<fMoreSteppingVolumes.at(i)<<DmpLogEndl;
  DmpLogInfo<<DmpLogEndl;
  DmpLogInfo<<"SecondaryStepECut (MeV)   = "<< fSecondaryStepECut <<DmpLogEndl;




}

DmpSimuStkStepEventMaker::~DmpSimuStkStepEventMaker()
{
}


void DmpSimuStkStepEventMaker::AddCollections(const G4Run* aRun)
{ 

  /*
  tree = aTree;
  dmpStkStepEvent = new DmpEvtStkStep();
  tree->Branch("DmpEvtStkStep", "DmpEvtStkStep",  &dmpStkStepEvent);
  */

  /*
  // navigator to DAMPE geometry
  DmpG4RunManager* runManager  = (DmpG4RunManager*) G4RunManager::GetRunManager();
  dmpDetector = (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());
  */

  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  /*
  fPvtrackSteps  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuPVtrackStepsCollection"),  const_cast<char *>("DmpStkStep"));
  fBackSteps     = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuBackStepsCollection"),     const_cast<char *>("DmpStkStep"));
  fElectronSteps = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuElectronStepsCollection"), const_cast<char *>("DmpStkStep"));
  fPositronSteps = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuPositronStepsCollection"), const_cast<char *>("DmpStkStep"));
  fPvtrack2Steps = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuPVtrack2StepsCollection"), const_cast<char *>("DmpStkStep"));
  */
  fPvtrackSteps  = new TClonesArray("DmpStkStep",100);
  fBackSteps     = new TClonesArray("DmpStkStep",100);
  fElectronSteps = new TClonesArray("DmpStkStep",100);
  fPositronSteps = new TClonesArray("DmpStkStep",100);
  fPvtrack2Steps = new TClonesArray("DmpStkStep",100);
  fSecondarySteps = new TClonesArray("DmpStkStep",100);

  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuPVtrackStepsCollection",  fPvtrackSteps);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuBackStepsCollection",     fBackSteps);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuElectronStepsCollection", fElectronSteps);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuPositronStepsCollection", fPositronSteps);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuPVtrack2StepsCollection", fPvtrack2Steps);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuSecondaryStepsCollection", fSecondarySteps);

}


void DmpSimuStkStepEventMaker::BeginRun(const G4Run* aRun){
  // navigator to DAMPE geometry
  DmpG4RunManager* runManager  = (DmpG4RunManager*) G4RunManager::GetRunManager();
  dmpDetector = (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

}

void DmpSimuStkStepEventMaker::BeginEvent(const G4Event* evt)
{
  //clear all ntuple data vectors here
  //set event number
  eventNumber = evt->GetEventID();

  is_pvtrack  = false;
  is_eletrack = false;
  is_postrack = false;
  is_backtrack= false;
  pvtrackID      = -1;
  eletrackID     = -1;
  postrackID     = -1;
  currentTrackID = -100;

  eDepositAcc_pvtrack = 0.;
  eDepositAcc_ele     = 0.;
  eDepositAcc_pos     = 0.;
  eDepositAcc_back    = 0.;

  //dmpStkStepEvent->Reset();
  fPvtrackSteps   ->Delete();
  fBackSteps      ->Delete();
  fElectronSteps  ->Delete();
  fPositronSteps  ->Delete();
  fPvtrack2Steps  ->Delete();
  fSecondarySteps ->Delete();


  fPvtrackSteps   ->Clear();
  fBackSteps      ->Clear();
  fElectronSteps  ->Clear();
  fPositronSteps  ->Clear();
  fPvtrack2Steps  ->Clear();
  fSecondarySteps ->Clear();

}

/*
void DmpSimuStkStepEventMaker::save()
{ 

}
*/

void DmpSimuStkStepEventMaker::FillStep(const G4Step* aStep)
{
  // *
  // * This collection is used for cosmic muons studies
  // *  introduced by Andrii Tykhonov
  // *

  /*
  //std::cout<<"\n\n\n\n aStep->GetSecondaryInCurrentStep()->size(): "<< aStep->GetSecondaryInCurrentStep()->size() <<"\n\n\n\n";
  G4Track* track1     = aStep->GetTrack();
  if(track1->GetTrackID()!=0){
  //if(aStep->GetSecondaryInCurrentStep()->size() > 1){
    std::cout<<"\n\n\n\n";
    //for(int i=0; i<aStep->GetSecondaryInCurrentStep()->size(); i++){

    std::cout<<"\naStep->GetSecondaryInCurrentStep()->at(i)->GetVertexKineticEnergy() = "<<track1->GetVertexKineticEnergy()<<DmpLogEndl;

    std::cout<<"fSecondaryVertexECut*MeV = "<< fSecondaryVertexECut*MeV<< DmpLogEndl;
    std::cout<<track1->GetVertexMomentumDirection().x()<<std::endl;
    std::cout<<track1->GetVertexMomentumDirection().y()<<std::endl;
    std::cout<<track1->GetVertexMomentumDirection().z()<<std::endl;

    G4ThreeVector pos = track1->GetVertexPosition();
    std::cout<<pos.x()<<std::endl;
    std::cout<<pos.y()<<std::endl;
    std::cout<<pos.z()<<std::endl;
    std::cout<<track1->GetTrackID()<<std::endl;

  }
  */

  //* Get secondary vertex
  /*
  if(aStep->GetSecondaryInCurrentStep()->size() > 0) if( aStep->GetSecondaryInCurrentStep()->at(0)->GetVertexKineticEnergy() > fSecondaryVertexECut*MeV ){
    DmpSimuVertex *scndVtx = (DmpSimuVertex*)fSecondaryVrtx->ConstructedAt(fSecondaryVrtx->GetLast()+1);

    G4ThreeVector p   = aStep->GetSecondaryInCurrentStep()->at(0)->GetVertexMomentumDirection();
    G4ThreeVector pos = aStep->GetSecondaryInCurrentStep()->at(0)->GetVertexPosition();
    scndVtx->p_direction_x = p.x();
    scndVtx->p_direction_y = p.y();
    scndVtx->p_direction_z = p.z();
    scndVtx->ekin = aStep->GetSecondaryInCurrentStep()->at(0)->GetVertexKineticEnergy();
    scndVtx->x = pos.x();
    scndVtx->y = pos.y();
    scndVtx->z = pos.z();
    scndVtx->child_trackID.push_back(aStep->GetSecondaryInCurrentStep()->at(0)->GetTrackID());
    scndVtx->parentID = aStep->GetSecondaryInCurrentStep()->at(0)->GetParentID();
    for(int i=1; i<aStep->GetSecondaryInCurrentStep()->size(); i++){
      scndVtx->child_trackID.push_back(aStep->GetSecondaryInCurrentStep()->at(i)->GetTrackID());
      pos = aStep->GetSecondaryInCurrentStep()->at(i)->GetVertexPosition();
      if(scndVtx->x != pos.x() || scndVtx->y != pos.y() || scndVtx->z != pos.z()){
        DmpLogError<<"Unrecognized situation: more than one secondary vertex per step ==> please consider a proper treatment of this situation! For now ==> thowing exception!" << DmpLogEndl;
        throw;
      }
    }
  }
  */

  G4Track* track     = aStep->GetTrack();
  G4int stepNumber   = track->GetCurrentStepNumber();
  G4StepPoint* point1 = aStep->GetPreStepPoint();
  G4StepPoint* point2 = aStep->GetPostStepPoint();
  G4bool enterVolume = (point1->GetStepStatus() == fGeomBoundary);
  G4bool leaveVolume = (point2->GetStepStatus() == fGeomBoundary);
  G4ThreeVector pos1  = point1->GetPosition();
  G4ThreeVector pos2  = point2->GetPosition();
  const G4VTouchable* touch1 = point1->GetTouchable();
  const G4VTouchable* touch2 = point2->GetTouchable();
  G4String thisVolume  = "";
  G4String nextVolume  = "";
  if(touch1->GetVolume())  thisVolume = touch1->GetVolume()->GetName();
  if(touch2->GetVolume())  nextVolume = touch2->GetVolume()->GetName();
  G4int vindex  = dmpDetector->getDetectorIndex(thisVolume);
  G4int vindexN = dmpDetector->getDetectorIndex(nextVolume);

  if( aStep->GetTrack()->GetParentID()==0 &&
      //abs(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode()) == 13 &&
      //nextVolume == ""
      //vindex == DmpDetectorConstruction::STKStrip
      (vindex == DmpDetectorConstruction::STKStrip || IsInTheSteppingVolumeList(thisVolume))

      )
  {
    //DmpStkStep* pvtrack2_step = new DmpStkStep();
    //fPvtrack2Steps->GetSize()
    DmpStkStep *pvtrack2_step = (DmpStkStep*)fPvtrack2Steps->ConstructedAt(fPvtrack2Steps->GetLast()+1);

    pvtrack2_step->step_parentID =  track->GetParentID();
    pvtrack2_step->step_trackID =  track->GetTrackID();
    pvtrack2_step->step_px   = point2->GetMomentum().x();
    pvtrack2_step->step_py   = point2->GetMomentum().y();
    pvtrack2_step->step_pz   = point2->GetMomentum().z();
    pvtrack2_step->step_ekin = point2->GetKineticEnergy();
    pvtrack2_step->step_x    = point2->GetPosition().x(); // + point1->GetPosition().x())*0.5;
    pvtrack2_step->step_y    = point2->GetPosition().y(); // + point1->GetPosition().y())*0.5;
    pvtrack2_step->step_z    = point2->GetPosition().z(); //+ point1->GetPosition().z())*0.5;
    pvtrack2_step->step_stepNumber    = stepNumber;
    pvtrack2_step->step_isEntering    = enterVolume;
    pvtrack2_step->step_thisVolume    = thisVolume;
    pvtrack2_step->step_nextVolume    = nextVolume;
    pvtrack2_step->step_thisIndex     = vindex;
    pvtrack2_step->step_nextIndex     = vindexN;
    pvtrack2_step->step_stepLength    = aStep->GetStepLength();
    pvtrack2_step->step_trackLength   = track->GetTrackLength();
    pvtrack2_step->step_energyDeposit = aStep->GetTotalEnergyDeposit();
    pvtrack2_step->step_pdg = aStep->GetTrack()->GetDynamicParticle()->GetPDGcode();

    /*
    std::cout<<std::endl;
    std::cout<<"thisVolume: "<< thisVolume <<std::endl;
    std::cout<<"dmpDetector->getDetectorIndex(thisVolume): "<< dmpDetector->getDetectorIndex(thisVolume) <<std::endl;
    std::cout<<"dmpDetector->getDetectorIndex(thisVolume): "<< dmpDetector->getDetectorIndex(thisVolume) <<std::endl;
    std::cout<< dmpDetector->GetBgoDetectorDescription()->isVolumeBgoBar(thisVolume)<<std::endl;
    std::cout<<"vindex:"<<vindex<<std::endl;
    std::cout<<std::endl;
    */

    //++dmpStkStepEvent->pvtrack2_nsteps;
    //dmpStkStepEvent->pvtrack2_steps.push_back(*pvtrack2_step);
  }
  //std::cout<< "aStep->GetTrack()->GetDynamicParticle()->GetPDGcode(): "<<aStep->GetTrack()->GetDynamicParticle()->GetPDGcode()<<std::endl;



  //*
  //* Collection of secondary steps and secondary vertices
  //* Author: Andrii Tykhonov
  //* Used for the beam test preparation analysis
  //*
  if(//aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() == 22 &&
     aStep->GetTrack()->GetParentID()!= 0 &&
     point2->GetKineticEnergy()> fSecondaryStepECut*MeV &&
     (vindex == DmpDetectorConstruction::STKStrip || IsInTheSteppingVolumeList(thisVolume)) )
      {
    DmpStkStep *secondary_step = (DmpStkStep*)fSecondarySteps->ConstructedAt(fSecondarySteps->GetLast()+1);

    secondary_step->step_parentID =  track->GetParentID();
    secondary_step->step_trackID  =  track->GetTrackID();
    secondary_step->step_px   = point2->GetMomentum().x();
    secondary_step->step_py   = point2->GetMomentum().y();
    secondary_step->step_pz   = point2->GetMomentum().z();
    secondary_step->step_ekin = point2->GetKineticEnergy();
    secondary_step->step_x    = point2->GetPosition().x(); // + point1->GetPosition().x())*0.5;
    secondary_step->step_y    = point2->GetPosition().y(); // + point1->GetPosition().y())*0.5;
    secondary_step->step_z    = point2->GetPosition().z(); //+ point1->GetPosition().z())*0.5;
    secondary_step->step_stepNumber    = stepNumber;
    secondary_step->step_isEntering    = enterVolume;
    secondary_step->step_thisVolume    = thisVolume;
    secondary_step->step_nextVolume    = nextVolume;
    secondary_step->step_thisIndex     = vindex;
    secondary_step->step_nextIndex     = vindexN;
    secondary_step->step_stepLength    = aStep->GetStepLength();
    secondary_step->step_trackLength   = track->GetTrackLength();
    secondary_step->step_energyDeposit = aStep->GetTotalEnergyDeposit();
    secondary_step->step_pdg = aStep->GetTrack()->GetDynamicParticle()->GetPDGcode();

  }





  //*
  //* Xin's code
  //*

  //fill stepping info here
  G4bool sel = false;
  //only stepping through primary charged paticles or e+/e- paire from primary photon conversion
  if (aStep->GetTrack()->GetParentID()==0) {    
    if (aStep->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) sel = true;

    G4int trackID = aStep->GetTrack()->GetTrackID();
    if(trackID != currentTrackID) {
      pvtrackID      = trackID;
      currentTrackID = trackID;
      if(eventNumber == debugEvent || debugEvent == -1) DmpLogDebug << "   primary track ID set to " << pvtrackID <<  DmpLogEndl;
      /***
      if(sel) {
	tt_stepping_trackID    -> push_back( trackID );
	tt_stepping_trackPDG   -> push_back( aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() );
	tt_stepping_parentID   -> push_back( aStep->GetTrack()->GetParentID() );
	++tt_stepping_ntracks;
      }
      ***/
    }
    eDepositAcc_pvtrack += aStep->GetTotalEnergyDeposit(); 
    is_pvtrack  = true;
    is_eletrack = false;
    is_postrack = false;
    is_backtrack= false;
    
  }
  else if (aStep->GetPreStepPoint()->GetMomentum().z()<0) {  // new coordinate system (".. .z()>0" in the old one...)
    sel = true;
    is_pvtrack  = false;
    is_eletrack = false;
    is_postrack = false;
    is_backtrack= true;
    G4int trackID = aStep->GetTrack()->GetTrackID();
    if(trackID != currentTrackID) {
      currentTrackID = trackID;
      /***
      tt_stepping_trackID    -> push_back( trackID );
      tt_stepping_trackPDG   -> push_back( aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() );
      tt_stepping_parentID   -> push_back( aStep->GetTrack()->GetParentID() );
      ++tt_stepping_ntracks;
      ***/
    }
    eDepositAcc_back += aStep->GetTotalEnergyDeposit(); 

  }
  else if (aStep->GetTrack()->GetParentID()==pvtrackID && aStep->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) {
    sel = true;
    is_pvtrack  = false;
    is_backtrack= false;
    G4int trackID = aStep->GetTrack()->GetTrackID();
    if(trackID != currentTrackID) {
      currentTrackID = trackID;
      /***
      tt_stepping_trackID    -> push_back( trackID );
      tt_stepping_trackPDG   -> push_back( aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() );
      tt_stepping_parentID   -> push_back( aStep->GetTrack()->GetParentID() );
      ++tt_stepping_ntracks;
      ***/
    }
    if(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() == 11) {
       is_eletrack = true;
       is_postrack = false;
       eDepositAcc_ele += aStep->GetTotalEnergyDeposit(); 
    }
    else if(aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() == -11) {
      is_eletrack = false;
      is_postrack = true;
      eDepositAcc_pos += aStep->GetTotalEnergyDeposit(); 
    }
 
  }
  else {
    is_pvtrack   = false;
    is_eletrack  = false;
    is_postrack  = false;
    is_backtrack = false;
  } 

  if(!is_pvtrack && !is_eletrack && !is_postrack && !is_backtrack) return;

  //return if particle goes out to World 
  if(aStep->GetTrack()->GetCurrentStepNumber() !=1 &&
     dmpDetector->isWorldVolume(aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName())) return;
     //aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName() == "World" ) return;

  if(is_backtrack && 
     //aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName() == "World" ) return;
     dmpDetector->isWorldVolume(aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName())) return;
  //
  if(!aStep->GetTrack()->GetNextVolume()) return; 

  //G4StepPoint* point1 = aStep->GetPreStepPoint();
  //G4StepPoint* point2 = aStep->GetPostStepPoint();
  if(eventNumber == debugEvent || debugEvent == -1) {
    DmpLogDebug << " new step:  current track ID= " << currentTrackID  << ", step number = "
	   << aStep->GetTrack()->GetCurrentStepNumber() << DmpLogEndl;
    DmpLogDebug << "       prestep  at " << aStep->GetPreStepPoint()->GetPosition()  << " in volume "
	   << aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName()  << DmpLogEndl;
    DmpLogDebug << "      poststep  at " << aStep->GetPostStepPoint()->GetPosition() << " in volume "
	     << aStep->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetName() << DmpLogEndl;
    G4String thisProc  = "U Limit";
    G4String nextProc  = "U Limit";
    if(point1->GetProcessDefinedStep() !=0) thisProc  = point1->GetProcessDefinedStep()->GetProcessName();
    if(point2->GetProcessDefinedStep() !=0) nextProc  = point2->GetProcessDefinedStep()->GetProcessName();
    DmpLogDebug << "       this process: " << thisProc << "    next process: " << nextProc << DmpLogEndl;
  }
  if(sel) {
    //G4ThreeVector pos1  = point1->GetPosition();
    //G4ThreeVector pos2  = point2->GetPosition();
    //G4TouchableHandle touch1 = point1->GetTouchableHandle();
    //G4TouchableHandle touch2 = point2->GetTouchableHandle();
    //G4String thisVolume  = touch1->GetVolume()->GetName();
    //G4String nextVolume  = touch2->GetVolume()->GetName();

    //only keep steps outside the calorimeter
    if(!(dmpDetector->GetBgoDetectorDescription()->isVolumeInsideCalorimeter(thisVolume))){
    //if(thisVolume != "BGODetectorX" && thisVolume != "BGODetectorY" &&
    //   thisVolume != "BGOLayerX"    && thisVolume != "BGOLayerY"    && thisVolume != "Payload" ) {

      //G4Track* track         = aStep->GetTrack();
      //G4int stepNumber = track->GetCurrentStepNumber();
      //G4bool enterVolume = (point1->GetStepStatus() == fGeomBoundary);
      //G4bool leaveVolume = (point2->GetStepStatus() == fGeomBoundary);

      //G4int vindex = dmpDetector->getDetectorIndex(thisVolume);
      /*
      G4int vindex = -1;
      if(dmpDetector->isWorldVolume(thisVolume)) vindex = 0;
      else if(dmpDetector->GetBgoDetectorDescription()->isVolumeBgoBar(thisVolume))         vindex = 1;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeConverter(thisVolume))      vindex = 2;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeThickConverter(thisVolume)) vindex = 3;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTile(thisVolume))  vindex = 4;
      */
      /*
      if(thisVolume == "World") vindex = 0; 
      else if(thisVolume == "BGODetectorX" || thisVolume == "BGODetectorY" )   vindex = 1; 
      else if(thisVolume == "BGOLayerX"    || thisVolume == "BGOLayerY"    )   vindex = 1; 
      else if(thisVolume == "Converter")      vindex = 2; 
      else if(thisVolume == "ThickConverter") vindex = 3; 
      else if(thisVolume == "Active Tile X" || thisVolume == "Active Tile Y")  vindex = 4; 
      else if(thisVolume == "Plane")          vindex = 5; 
      else if(thisVolume == "STKDetectorX"  || thisVolume == "STKDetectorY")   vindex  = 6; 
      else if(thisVolume == "STK")            vindex = 7; 
      else if(thisVolume == "PSD")            vindex = 8; 
      else if(thisVolume == "Payload")        vindex = 9; 
      else if(thisVolume == "PlaneThinW")     vindex = 10; 
      else if(thisVolume == "PlaneThickW")    vindex = 11;
      */


      G4int vindexN = dmpDetector->getDetectorIndex(nextVolume);
      /*
      G4int vindexN = -1;
      if(dmpDetector->isWorldVolume(nextVolume)) vindex = 0;
      else if(dmpDetector->GetBgoDetectorDescription()->isVolumeBgoBar(nextVolume))         vindex = 1;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeConverter(nextVolume))      vindex = 2;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeThickConverter(nextVolume)) vindex = 3;
      else if(dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTile(nextVolume))  vindex = 4;
      */
      /*
      if(nextVolume == "World") vindexN = 0; 
      else if(nextVolume == "BGODetectorX" || nextVolume == "BGODetectorY" )   vindexN = 1; 
      else if(nextVolume == "BGOLayerX"    || nextVolume == "BGOLayerY"    )   vindexN = 1; 
      else if(nextVolume == "Converter")      vindexN = 2; 
      else if(nextVolume == "ThickConverter") vindexN = 3; 
      else if(nextVolume == "Active Tile X" || nextVolume == "Active Tile Y")  vindexN = 4; 
      else if(nextVolume == "Plane")          vindexN = 5; 
      else if(nextVolume == "STKDetectorX"  || nextVolume == "STKDetectorY")   vindexN  = 6; 
      else if(nextVolume == "STK")            vindexN = 7; 
      else if(nextVolume == "PSD")            vindexN = 8; 
      else if(nextVolume == "Payload")        vindexN = 9; 
      else if(nextVolume == "PlaneThinW")     vindexN = 10; 
      else if(nextVolume == "PlaneThickW")    vindexN = 11;
      */

      if(eventNumber == debugEvent || debugEvent == -1) {
	//G4int thisVolumeCopy = touch1->GetCopyNumber();
	//G4int nextVolumeCopy = touch2->GetCopyNumber();
	//G4double kinEnergy     = track->GetKineticEnergy();
	//G4double globalTime    = track->GetGlobalTime();
	G4int trackID = track->GetTrackID();
     
        DmpLogDebug << "   track ID= " << trackID << "  step number= " << stepNumber
	            << ",  step length= " << aStep->GetStepLength() << ",  track length= " << track->GetTrackLength() << DmpLogEndl;
        DmpLogDebug << ",  energy depo= " << aStep->GetTotalEnergyDeposit();
	//if(thisVolume == "Converter" || thisVolume == "ThickConverter" ) {
	if(dmpDetector->GetStkDetectorDescription()->isVolumeConverter(thisVolume)||
	   dmpDetector->GetStkDetectorDescription()->isVolumeThickConverter(thisVolume)) {

	  if(is_pvtrack) {DmpLogDebug << " accumulated edep= " << eDepositAcc_pvtrack;}
	  else if(is_eletrack) {DmpLogDebug << " accumulated edep= " << eDepositAcc_ele;}
	  else if(is_postrack) {DmpLogDebug << " accumulated edep= " << eDepositAcc_pos;}
	  else if(is_backtrack) {DmpLogDebug << " accumulated edep= " << eDepositAcc_back;}
	  if( stepNumber==1 || leaveVolume) {DmpLogDebug << " SELECTED!" <<  DmpLogEndl;}
	  else  {DmpLogDebug <<  DmpLogEndl;}
	  G4String thisProc  = "Uset Limit";
	  G4String nextProc  = "Uset Limit";
	  if(point1->GetProcessDefinedStep() !=0) thisProc  = point1->GetProcessDefinedStep()->GetProcessName();
	  if(point2->GetProcessDefinedStep() !=0) nextProc  = point2->GetProcessDefinedStep()->GetProcessName();
	  DmpLogDebug << "       this process: " << thisProc << "    next process: " << nextProc << DmpLogEndl;
	  DmpLogDebug << "       point1 momentum (MeV): " <<point1->GetMomentum() << DmpLogEndl;
	  DmpLogDebug << "       point2 momentum (MeV): " <<point2->GetMomentum() << DmpLogEndl;
	  //G4cout << "       track  momentum (MeV): " << track->GetMomentum() << "   kinetic energy (MeV): " << kinEnergy << DmpLogEndl;
	  DmpLogDebug << "       prestep  at " << pos1 << " in volume " << thisVolume  << "    enter = " << enterVolume  << DmpLogEndl;
	  DmpLogDebug << "       poststep at " << pos2 << " in volume " << nextVolume  << "    leave = " << leaveVolume  << DmpLogEndl;
	}
      }
    
      //bool volSel = (thisVolume == "Active Tile X" || thisVolume == "Active Tile Y" || thisVolume == "PSD" );
      bool volSel = (dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTile(thisVolume)|| dmpDetector->GetPsdDetectorDescription()->isVolumeInPSD(thisVolume) );
      if(is_backtrack && volSel)   {
	/***	
	if(stepNumber == 1  || nextVolume=="Payload" || (aStep->GetTotalEnergyDeposit()>0 && leaveVolume) ) {
	  if(eventNumber == debugEvent || debugEvent == -1) {
	    G4int trackID = track->GetTrackID();
	    G4cout << "   track ID= " << trackID << "  step number= " << stepNumber 
		   << ",  step length= " << aStep->GetStepLength() << ",  track length= " << track->GetTrackLength() << DmpLogEndl;
	    G4cout << ",  energy depo= " << aStep->GetTotalEnergyDeposit();
	    {G4cout << " accumulated edep= " << eDepositAcc_back;}
	    if( stepNumber==1 || leaveVolume) {G4cout << " SELECTED!" <<  DmpLogEndl;}
	    else  {G4cout <<  DmpLogEndl;}
	    G4String thisProc  = "Uset Limit";
	    G4String nextProc  = "Uset Limit";
	    if(point1->GetProcessDefinedStep() !=0) thisProc  = point1->GetProcessDefinedStep()->GetProcessName();
	    if(point2->GetProcessDefinedStep() !=0) nextProc  = point2->GetProcessDefinedStep()->GetProcessName();
	    G4cout << "       this process: " << thisProc << "    next process: " << nextProc << DmpLogEndl;
	    G4cout << "       point1 momentum (MeV): " <<point1->GetMomentum().x() << " " << point1->GetMomentum().y() << " " << point1->GetMomentum().z() << DmpLogEndl;
	    G4cout << "       point2 momentum (MeV): " <<point2->GetMomentum().x() << " " << point2->GetMomentum().y() << " " << point2->GetMomentum().z() << DmpLogEndl;
	    G4cout << "       prestep  at " << pos1 << " in volume " << thisVolume  << "    enter = " << enterVolume  << DmpLogEndl;
	    G4cout << "       poststep at " << pos2 << " in volume " << nextVolume  << "    leave = " << leaveVolume  << DmpLogEndl;
	    G4cout << "       PDG code    " <<aStep->GetTrack()->GetDynamicParticle()->GetPDGcode() << DmpLogEndl;
	  }
	}
	
	if(stepNumber == 1) {
	  tt_step_back_trackID -> push_back(  aStep->GetTrack()->GetTrackID() );
	  tt_step_back_pdg     -> push_back(  aStep->GetTrack()->GetDynamicParticle()->GetPDGcode()  );
	  tt_step_back_px      -> push_back(  point1->GetMomentum().x()  );
	  tt_step_back_py      -> push_back(  point1->GetMomentum().y()  );
	  tt_step_back_pz      -> push_back(  point1->GetMomentum().z()  );
	  tt_step_back_ekin    -> push_back(  point1->GetKineticEnergy() );
	  tt_step_back_x       -> push_back(  point1->GetPosition().x()  );
	  tt_step_back_y       -> push_back(  point1->GetPosition().y()  );
	  tt_step_back_z       -> push_back(  point1->GetPosition().z()  );
	  tt_step_back_stepNumber    -> push_back( stepNumber-1 );
	  tt_step_back_isEntering    -> push_back( enterVolume  );
	  tt_step_back_thisVolume    -> push_back( thisVolume   );
	  tt_step_back_nextVolume    -> push_back( thisVolume   ); //on purpose
	  tt_step_back_thisIndex     -> push_back( vindex       );
	  tt_step_back_nextIndex     -> push_back( vindex       ); //on purpose
	  tt_step_back_stepLength    -> push_back( aStep->GetStepLength()  );
	  tt_step_back_trackLength   -> push_back( track->GetTrackLength() );
	  tt_step_back_energyDeposit -> push_back( eDepositAcc_back );	 
	  ++tt_back_nsteps;
	}
	***/

	//filling steps that leaves the volume and has deposited energy 
	if(leaveVolume) {
	  //if(aStep->GetTotalEnergyDeposit()>0 || nextVolume=="Payload") {
	  if(aStep->GetTotalEnergyDeposit()>0) {
	    //DmpStkStep* backstep = new DmpStkStep();
	    DmpStkStep *backstep = (DmpStkStep*)fBackSteps->ConstructedAt(fBackSteps->GetLast()+1);

	    backstep->step_trackID =  aStep->GetTrack()->GetTrackID();
	    backstep->step_pdg  = aStep->GetTrack()->GetDynamicParticle()->GetPDGcode();
	    backstep->step_px   = point2->GetMomentum().x();
	    backstep->step_py   = point2->GetMomentum().y();
	    backstep->step_pz   = point2->GetMomentum().z();
	    backstep->step_ekin = point2->GetKineticEnergy();
	    backstep->step_x    = point2->GetPosition().x();
	    backstep->step_y    = point2->GetPosition().y();
	    backstep->step_z    = point2->GetPosition().z();
	    backstep->step_stepNumber = stepNumber;
	    backstep->step_isEntering = enterVolume;
	    backstep->step_thisVolume = thisVolume;
	    backstep->step_nextVolume = nextVolume;
	    backstep->step_thisIndex = vindex;
	    backstep->step_nextIndex = vindexN;
	    backstep->step_stepLength  = aStep->GetStepLength();
	    backstep->step_trackLength = track->GetTrackLength();
	    backstep->step_energyDeposit = eDepositAcc_back;

	    //++dmpStkStepEvent->back_nsteps;
	    //dmpStkStepEvent->back_steps.push_back(*backstep);
	  }
	}
 
      }

      if(is_pvtrack && volSel)   {
	//if(eventNumber == debugEvent || debugEvent == -1) G4cout << "pvtrack   track ID= " << currentTrackID << "  step number= " << stepNumber << DmpLogEndl;
	//filling step 0 (point 1 of step 1)
	if(stepNumber == 1) {
	  //DmpStkStep* pvtrack_step = new DmpStkStep();
          DmpStkStep *pvtrack_step = (DmpStkStep*)fPvtrackSteps->ConstructedAt(fPvtrackSteps->GetLast()+1);

	  pvtrack_step->step_px   = point1->GetMomentum().x();
	  pvtrack_step->step_py   = point1->GetMomentum().y();
	  pvtrack_step->step_pz   = point1->GetMomentum().z();
	  pvtrack_step->step_ekin = point1->GetKineticEnergy();
	  pvtrack_step->step_x    = point1->GetPosition().x();
	  pvtrack_step->step_y    = point1->GetPosition().y();
	  pvtrack_step->step_z    =  point1->GetPosition().z();
	  pvtrack_step->step_stepNumber    = stepNumber-1;
	  pvtrack_step->step_isEntering    = enterVolume;
	  pvtrack_step->step_thisVolume    = thisVolume;
	  pvtrack_step->step_nextVolume    = thisVolume; //on purpose
	  pvtrack_step->step_thisIndex     = vindex;
	  pvtrack_step->step_nextIndex     = vindex; //on purpose
	  pvtrack_step->step_stepLength    = aStep->GetStepLength();
	  pvtrack_step->step_trackLength   = track->GetTrackLength();
	  pvtrack_step->step_energyDeposit = eDepositAcc_pvtrack;

	  //++dmpStkStepEvent->pvtrack_nsteps;
	  //dmpStkStepEvent->pvtrack_steps.push_back(*pvtrack_step);

	}
	//filling steps that leaves the volume
	if(leaveVolume) {
	  //DmpStkStep* pvtrack_step = new DmpStkStep();
          DmpStkStep *pvtrack_step = (DmpStkStep*)fPvtrackSteps->ConstructedAt(fPvtrackSteps->GetLast()+1);

	  pvtrack_step->step_px   = point2->GetMomentum().x();
	  pvtrack_step->step_py   = point2->GetMomentum().y();
	  pvtrack_step->step_pz   = point2->GetMomentum().z();
	  pvtrack_step->step_ekin = point2->GetKineticEnergy();
	  pvtrack_step->step_x    = point2->GetPosition().x();
	  pvtrack_step->step_y    = point2->GetPosition().y();
	  pvtrack_step->step_z    = point2->GetPosition().z();
	  pvtrack_step->step_stepNumber    = stepNumber;
	  pvtrack_step->step_isEntering    = enterVolume;
	  pvtrack_step->step_thisVolume    = thisVolume;
	  pvtrack_step->step_nextVolume    = nextVolume;
	  pvtrack_step->step_thisIndex     = vindex;
	  pvtrack_step->step_nextIndex     = vindexN;
	  pvtrack_step->step_stepLength    = aStep->GetStepLength();
	  pvtrack_step->step_trackLength   = track->GetTrackLength();
	  pvtrack_step->step_energyDeposit = eDepositAcc_pvtrack;

	  //++dmpStkStepEvent->pvtrack_nsteps;
	  //dmpStkStepEvent->pvtrack_steps.push_back(*pvtrack_step);
	}
      }

      if(is_eletrack && volSel)  {
	//if(eventNumber == debugEvent || debugEvent == -1) G4cout << "eletrack   track ID= " << currentTrackID << "  step number= " << stepNumber << DmpLogEndl;
 	//filling step 0 (point 1 of step 1)
	if(stepNumber == 1) {
	  //DmpStkStep* electron_step = new DmpStkStep();
          DmpStkStep *electron_step = (DmpStkStep*)fElectronSteps->ConstructedAt(fElectronSteps->GetLast()+1);

	  electron_step->step_px   = point1->GetMomentum().x();
	  electron_step->step_py   = point1->GetMomentum().y();
	  electron_step->step_pz   = point1->GetMomentum().z();
	  electron_step->step_ekin = point1->GetKineticEnergy();
	  electron_step->step_x    = point1->GetPosition().x();
	  electron_step->step_y    = point1->GetPosition().y();
	  electron_step->step_z    = point1->GetPosition().z();
	  electron_step->step_stepNumber    = stepNumber-1;
	  electron_step->step_isEntering    = enterVolume;
	  electron_step->step_thisVolume    = thisVolume;
	  electron_step->step_nextVolume    = thisVolume; //on purpose
	  electron_step->step_thisIndex     = vindex;
	  electron_step->step_nextIndex     = vindex; //on purpose
	  electron_step->step_stepLength    = aStep->GetStepLength();
	  electron_step->step_trackLength   = track->GetTrackLength();
	  electron_step->step_energyDeposit = eDepositAcc_ele;
	  electron_step->step_pdg = 11;

	  //++dmpStkStepEvent->ele_nsteps;
	  //dmpStkStepEvent->electron_steps.push_back(*electron_step);

	}
	//filling steps that leaves the volume
	if(leaveVolume) {
	  //DmpStkStep* electron_step = new DmpStkStep();
          DmpStkStep *electron_step = (DmpStkStep*)fElectronSteps->ConstructedAt(fElectronSteps->GetLast()+1);

	  electron_step->step_px   = point2->GetMomentum().x();
	  electron_step->step_py   = point2->GetMomentum().y();
	  electron_step->step_pz   = point2->GetMomentum().z();
	  electron_step->step_ekin = point2->GetKineticEnergy();
	  electron_step->step_x    = point2->GetPosition().x();
	  electron_step->step_y    = point2->GetPosition().y();
	  electron_step->step_z    = point2->GetPosition().z();
	  electron_step->step_stepNumber    = stepNumber;
	  electron_step->step_isEntering    = enterVolume;
	  electron_step->step_thisVolume    = thisVolume;
	  electron_step->step_nextVolume    = nextVolume;
	  electron_step->step_thisIndex     = vindex;
	  electron_step->step_nextIndex     = vindexN;
	  electron_step->step_stepLength    = aStep->GetStepLength();
	  electron_step->step_trackLength   = track->GetTrackLength();
	  electron_step->step_energyDeposit = eDepositAcc_ele;
	  electron_step->step_pdg = 11;

	  //++dmpStkStepEvent->ele_nsteps;
	  //dmpStkStepEvent->electron_steps.push_back(*electron_step);
	}
      }
      if(is_postrack && volSel)  {
	//if(eventNumber == debugEvent || debugEvent == -1) G4cout << "postrack   track ID= " << currentTrackID << "  step number= " << stepNumber << DmpLogEndl;
	//filling step 0 (point 1 of step 1)
	if(stepNumber == 1) {
	  //DmpStkStep* positron_step = new DmpStkStep();
          DmpStkStep *positron_step = (DmpStkStep*)fPositronSteps->ConstructedAt(fPositronSteps->GetLast()+1);

	  positron_step->step_px   = point1->GetMomentum().x();
	  positron_step->step_py   = point1->GetMomentum().y();
	  positron_step->step_pz   = point1->GetMomentum().z();
	  positron_step->step_ekin = point1->GetKineticEnergy();
	  positron_step->step_x    = point1->GetPosition().x();
	  positron_step->step_y    = point1->GetPosition().y();
	  positron_step->step_z    = point1->GetPosition().z();
	  positron_step->step_stepNumber    = stepNumber-1;
	  positron_step->step_isEntering    = enterVolume;
	  positron_step->step_thisVolume    = thisVolume;
	  positron_step->step_nextVolume    = thisVolume; //on purpose
	  positron_step->step_thisIndex     = vindex;
	  positron_step->step_nextIndex     = vindex; //on purpose
	  positron_step->step_stepLength    = aStep->GetStepLength();
	  positron_step->step_trackLength   = track->GetTrackLength();
	  positron_step->step_energyDeposit = eDepositAcc_pos;
	  positron_step->step_pdg = -11;

	  //++dmpStkStepEvent->pos_nsteps;
	  //dmpStkStepEvent->positron_steps.push_back(*positron_step);
	}
	//filling steps that leaves the volume
	if(leaveVolume) {
	  //DmpStkStep* positron_step = new DmpStkStep();
          DmpStkStep *positron_step = (DmpStkStep*)fPositronSteps->ConstructedAt(fPositronSteps->GetLast()+1);

	  positron_step->step_px   = point2->GetMomentum().x();
	  positron_step->step_py   = point2->GetMomentum().y();
	  positron_step->step_pz   = point2->GetMomentum().z();
	  positron_step->step_ekin = point2->GetKineticEnergy();
	  positron_step->step_x    = point2->GetPosition().x();
	  positron_step->step_y    = point2->GetPosition().y();
	  positron_step->step_z    = point2->GetPosition().z();
	  positron_step->step_stepNumber    = stepNumber;
	  positron_step->step_isEntering    = enterVolume;
	  positron_step->step_thisVolume    = thisVolume;
	  positron_step->step_nextVolume    = nextVolume;
	  positron_step->step_thisIndex     = vindex;
	  positron_step->step_nextIndex     = vindexN;
	  positron_step->step_stepLength    = aStep->GetStepLength();
	  positron_step->step_trackLength   = track->GetTrackLength();
	  positron_step->step_energyDeposit = eDepositAcc_pos;
	  positron_step->step_pdg = -11;

         //++dmpStkStepEvent->pos_nsteps;
         //dmpStkStepEvent->positron_steps.push_back(*positron_step);

       }
      }
    }
  }
}

bool DmpSimuStkStepEventMaker::IsInTheSteppingVolumeList(std::string &volume){

  for(int i=0; i<fMoreSteppingVolumes.size();i++){
    if(fMoreSteppingVolumes.at(i).contains(volume)) return true;
  }
  return false;
}







