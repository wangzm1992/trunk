#ifndef DMPSIMUBgoHITEVENTMAKER_HH
#define DMPSIMUBgoHITEVENTMAKER_HH

#include "globals.hh"
#include "TClonesArray.h"
//#include "DmpSimSvcDataMgr.h"

class TTree;
class G4Run;
class G4Event;
class DmpEvtBgoHit;

class DmpSimuBgoHitEventMaker
{
public:
  DmpSimuBgoHitEventMaker();
  ~DmpSimuBgoHitEventMaker();
   
  void AddCollections(const G4Run* aRun);
  //void save();

  void BeginEvent(const G4Event* evt);
  void BeginRun(const G4Run* run);
  void FillEvent(const G4Event* evt);        

  G4bool passTotalEnergyCut(G4double cut);

 private:
  //TTree*   tree;
  G4int  debugEvent;
  //G4int  eventNumber;
  //DmpEvtBgoHit *BgoHitEvt;

  G4double totE;
  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray* fBgoHits;

};

#endif

