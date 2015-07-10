#ifndef DmpSimuEventAction_h
#define DmpSimuEventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include "DmpSimAlg.h"

class DmpRootEventDataManager;
class DmpSimSvcDataMgr;

class DmpSimuEventAction : public G4UserEventAction
{
public:

  DmpSimuEventAction(DmpRootEventDataManager*);
  virtual ~DmpSimuEventAction();
  
public:
  virtual void   BeginOfEventAction(const G4Event*);
  virtual void   EndOfEventAction(const G4Event*);
  
  void SetDrawFlag   (G4String val)  {drawFlag = val;};
  
private:

  G4int       stkHitCollID;                
  G4int       bgoHitCollID;                
  G4int       psdHitCollID;                
  G4String    drawFlag;

  DmpRootEventDataManager* rootEventDataManager;
  //DmpSimSvcDataMgr* fDataMgr;
  DmpSimAlg* fDmpSimAlg;
};

#endif

    





