#ifndef DmpSimuStkStepEventMaker_h
#define DmpSimuStkStepEventMaker_h 1

#include "DmpDetectorConstruction.hh"

#include <vector>
#include "globals.hh"
#include "G4Event.hh"
#include "G4Run.hh"

#include "DmpEvtStkStep.hh"
//#include "DmpSimSvcDataMgr.h"

#include "TClonesArray.h"

class TTree;
class DmpSimuConfigParser;

class DmpSimuStkStepEventMaker
{
public:
  
  DmpSimuStkStepEventMaker();
  ~DmpSimuStkStepEventMaker();
   
  void AddCollections(const G4Run* aRun);
  //void save();

  void BeginEvent(const G4Event* evt);
  void BeginRun(const G4Run* aRun);
  void FillStep(const G4Step* aStep);
  void FillEvent(const G4Event* evt);        


private:
  bool IsInTheSteppingVolumeList(std::string &volume);

private:
  
  DmpDetectorConstruction* dmpDetector;

  //TTree*   tree;

  G4int  debugEvent;

  G4int  eventNumber;

  G4bool is_pvtrack;
  G4bool is_eletrack;
  G4bool is_postrack;
  G4bool is_backtrack;
  G4int  pvtrackID;
  G4int  eletrackID;
  G4int  postrackID;
  G4int  currentTrackID;

  G4double eDepositAcc_pvtrack;
  G4double eDepositAcc_ele;
  G4double eDepositAcc_pos;
  G4double eDepositAcc_back;

  //DmpEvtStkStep* dmpStkStepEvent;
  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray* fPvtrackSteps;
  TClonesArray* fBackSteps;
  TClonesArray* fElectronSteps;
  TClonesArray* fPositronSteps;
  TClonesArray* fPvtrack2Steps;
  TClonesArray* fSecondarySteps;
  TClonesArray* fSecondaryVrtx;

  DmpSimuConfigParser* fSimuConfig;
  std::vector<G4String> fMoreSteppingVolumes;

  double fSecondaryStepECut;  // MeV
  double fSecondaryVertexECut; // MeV
};


#endif

