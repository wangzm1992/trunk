#ifndef DmpRootEventDataManager_h
#define DmpRootEventDataManager_h 1

#include <vector>
#include "globals.hh"
#include "G4Run.hh"
#include "G4Event.hh"

#include "DmpRootEventDataMessenger.hh"
#include "DmpSimuStkHitEventMaker.hh"
#include "DmpSimuStkDigiEventMaker.hh"
#include "DmpSimuPrimariesEventMaker.hh"
#include "DmpSimuStkStepEventMaker.hh"
#include "DmpSimuTrajectoryEventMaker.hh"
#include "DmpSimuRunEventHeaderMaker.hh"
#include "DmpSimuPsdHitEventMaker.hh"
#include "DmpSimuNudHitEventMaker.hh"
#include "DmpSimuBgoHitEventMaker.hh"
#include "DmpSimAlg.h"
//#include "DmpSimSvcDataMgr.h"





//class DmpSimuRunEventNtupleMaker;
//class DmpSimuPrimariesNtupleMaker;
//class DmpSimuStkStepNtupleMaker;
//class DmpSimuTrajectoryNtupleMaker;
//class DmpSimuStkHitEventMaker;
//class DmpSimuStkDigiEventMaker;
//class DmpSimuPsdHitNtupleMaker;
//class DmpSimuBgoHitNtupleMaker;

//class TFile;
//class TTree;
class TH1D;

//class DmpRootNtupleMessenger;

class DmpSimuPrimaryGeneratorAction;


class DmpRootEventDataManager
{
public:
  
  DmpRootEventDataManager();
  ~DmpRootEventDataManager();
  
  void Book(const G4Run* aRun);
  void Save();
  
  void EndEvent(const G4Event* evt);
  void BeginEvent(const G4Event* evt);
  void FillEvent(const G4Event* evt);
  void FillStep(const G4Step* aStep);
  void FillRun(const G4Run* aRun, DmpSimuPrimaryGeneratorAction*);
  void BeginRun(const G4Run* aRun);

  void SetRootGenDebug(G4bool val)     { keepMCgeneratorDebugInfo = val;}
  void SetCalEcut(G4double val)        { calEcut          = val;}
  void SetSiDigicut(G4int val)         { siDigicut        = val;}
  /***
  void SetGenerationOnly(G4bool val)   { generationOnly = val;}
  void SetKeepOnlyDownward(G4bool val) { keepOnlyDownward = val;}
  ***/

  /*
  TTree* GetGenTree()                  { return treeGen; }
  DmpSimuPrimariesNtupleMaker* GetDmpSimuPrimariesNtupleMaker() 
                                       { return dmpSimuPrimariesNtupleMaker; }
  G4bool GenerationOnly()              { return generationOnly; }
  */

private:
  
  DmpSimuRunEventHeaderMaker*    dmpSimuRunEventHeaderMaker;
  DmpSimuPrimariesEventMaker*    dmpSimuPrimariesEventMaker;
  DmpSimuStkStepEventMaker*      dmpSimuStkStepEventMaker;
  DmpSimuTrajectoryEventMaker*   dmpSimuTrajectoryEventMaker;
  DmpSimuStkHitEventMaker*       dmpSimuStkHitEventMaker;
  DmpSimuStkDigiEventMaker*      dmpSimuStkDigiEventMaker;
  DmpSimuPsdHitEventMaker*       dmpSimuPsdHitEventMaker;
  DmpSimuNudHitEventMaker*       dmpSimuNudHitEventMaker;
  DmpSimuBgoHitEventMaker*       dmpSimuBgoHitEventMaker;


  DmpRootEventDataMessenger* rootMessenger;  //* TODO: rootMessenger is DEPRECATED and should be removed! (use python interfaces instead of Geant4 ones)

  G4bool   keepMCgeneratorDebugInfo;  // true for Particle Generation Debug
  G4double calEcut;
  G4int    siDigicut;

  //DmpSimSvcDataMgr* fDataMgr;
  DmpSimAlg* fDmpSimAlg;

};


#endif

