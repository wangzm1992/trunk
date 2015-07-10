#ifndef DmpSimuNudHitEventMaker_h
#define DmpSimuNudHitEventMaker_h 1

#include <vector>
#include "globals.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "TClonesArray.h"

#include "DmpNudHit.hh"
//#include "DmpSimSvcDataMgr.h"
#include "TClonesArray.h"


class TTree;

class DmpSimuNudHitEventMaker
{
public:
  
  DmpSimuNudHitEventMaker();
  ~DmpSimuNudHitEventMaker();
   
  void AddCollections(const G4Run* aRun);
  void BeginRun(const G4Run* aRun);

  void BeginEvent(const G4Event* evt);
  void FillStep(const G4Step* aStep);
  void FillEvent(const G4Event* evt);

  G4bool passTotalEnergyCut(G4double cut);

 private:


  G4int  debugEvent;
  G4int  eventNumber;

  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray* fNudHits0;
  TClonesArray* fNudHits1;
  TClonesArray* fNudHits2;
  TClonesArray* fNudHits3;

  G4double nud_totE0;
  G4double nud_totE1;
  G4double nud_totE2;
  G4double nud_totE3;


};


#endif

