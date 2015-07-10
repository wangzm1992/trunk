#ifndef DMPSIMUPSDHITEVENTMAKER_HH
#define DMPSIMUPSDHITEVENTMAKER_HH

#include <vector>
#include "globals.hh"
#include "TClonesArray.h"


#include "DmpPsdHit.hh"
//#include "DmpSimSvcDataMgr.h"


class TTree;
class G4Run;
class G4Event;

class DmpSimuPsdHitEventMaker
{
public:
    DmpSimuPsdHitEventMaker();
    ~DmpSimuPsdHitEventMaker();

    void AddCollections(const G4Run* aRun);
    //void save();


    void BeginRun(const G4Run* aRun);
    void FillEvent(const G4Event* evt);
    void BeginEvent(const G4Event* evt);

private:
    //TTree* tree;
    G4int debugEvent;
    //G4int eventNumber;
    //DmpEvtPsdHit* PsdHitEvt;
    //DmpSimSvcDataMgr* fDataMgr;
    TClonesArray* fPsdHits;
};

#endif
