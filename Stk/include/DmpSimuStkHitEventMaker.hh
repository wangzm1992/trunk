#ifndef DmpSimuStkHitEventMaker_h
#define DmpSimuStkHitEventMaker_h 1

#include <vector>
#include "globals.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "TTree.h"
#include "TClonesArray.h"
//#include "DmpEvtStkHit.hh"

#include "DmpStkHit.hh"
//#include "DmpSimSvcDataMgr.h"





class DmpSimuStkHitEventMaker
{
public:
  
  DmpSimuStkHitEventMaker();
  ~DmpSimuStkHitEventMaker();
   
  void AddCollections(const G4Run* aRun);
  void BeginRun(const G4Run* run);
  void BeginEvent(const G4Event* evt);
  void FillEvent(const G4Event* evt);

  //void FillStep(const G4Step* aStep);

 private:
  int debugEvent;
  //int eventNumber;
  //TTree*             tree;
  //const G4Run*        run;
  //DmpEvtStkHit* StkHitEvt;

  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray*     fStkHits;


};


#endif

