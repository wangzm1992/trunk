#ifndef DmpSimuTrajectoryEventMaker_h
#define DmpSimuTrajectoryEventMaker_h 1

#include "DmpEvtSimuTrajectory.h"
#include "DmpDetectorConstruction.hh"
#include "DmpSimuConfigParser.h"
//#include "DmpSimSvcDataMgr.h"

#include "TClonesArray.h"
#include <vector>
#include "globals.hh"
#include "G4Event.hh"
#include "G4Run.hh"

class TTree;

class DmpSimuTrajectoryEventMaker
{
public:
  
  DmpSimuTrajectoryEventMaker();
  ~DmpSimuTrajectoryEventMaker();
   
  //void book(const G4Run* aRun, TTree* aTree);
  //void save();

  //void beginEvent(const G4Event* evt);
  void BeginEvent(const G4Event* aEvent);
  void BeginRun(const G4Run* aRun);
  void FillStep(const G4Step* aStep);
  void FillEvent(const G4Event* evt);        

 private:
  


  //TTree*   tree;
  G4int  debugEvent;
  //G4int  eventNumber;
  DmpDetectorConstruction* dmpDetector;
  DmpEvtSimuTrajectory* dmpSimuTrajectoryEvent;
  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray* fTruthTrajectories;

  double fTrajectoryEcut;
  double fSecondaryVertexECut;
  DmpSimuConfigParser* fSimuConfig;

  TClonesArray* fSecondaryVrtx;


};


#endif

