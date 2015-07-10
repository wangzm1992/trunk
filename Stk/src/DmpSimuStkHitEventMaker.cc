// Description: ntuple maker for the STK
//
// Author(s):
//  - creation by X.Wu, 15/07/2013

#include "DmpSimuStkHitEventMaker.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "DmpSimuStkHit.hh"
#include "DmpEvtStkHit.hh"

#include "DmpLog.h"
#include "DmpCore.h"
#include "DmpIOSvc.h"

DmpSimuStkHitEventMaker::DmpSimuStkHitEventMaker()
 :debugEvent(-100) //-100 no event
{
  //set debugEvent to a particular event number for debug printout  
  //debugEvent = -1; //-1 is all events
  //debugEvent = -100; //-100 no event
}

DmpSimuStkHitEventMaker::~DmpSimuStkHitEventMaker()
{
}

void DmpSimuStkHitEventMaker::AddCollections(const G4Run* aRun)
{
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  //fStkHits  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuStkHitsCollection"),  const_cast<char *>("DmpStkHit"));
  fStkHits = new TClonesArray("DmpStkHit",10000);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuStkHitsCollection",fStkHits);
}




void DmpSimuStkHitEventMaker::BeginEvent(const G4Event* evt)
{
    //StkHitEvt->Reset();
    //StkHitEvt->eventNumber =  evt->GetEventID();
    //StkHitEvt->eventNumber =  evt->GetEventID();
    //StkHitEvt->runNumber   =  run->GetRunID();
  fStkHits->Delete();
  fStkHits->Clear();

}

void DmpSimuStkHitEventMaker::BeginRun(const G4Run* run)
{
}


void DmpSimuStkHitEventMaker::FillEvent(const G4Event* evt)
{  
  G4int eventNumber =  evt->GetEventID();
  
  //Tracker hits
  DmpSimuStkHitsCollection* THC = 0;
  G4SDManager * SDman = G4SDManager::GetSDMpointer();  
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();

  if (HCE) {
      THC = (DmpSimuStkHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("STKHitCollection")));
  }

  if (THC) {
      int n_hit = THC->entries();
      if(eventNumber == debugEvent || debugEvent == -1) {
          DmpLogDebug << "Number of track hits in this event =  " << n_hit << DmpLogEndl;
          DmpLogDebug << "  edp  " << "  strip " << " plane " << " type "
                  << "  x    " << "   y    " << "     z    " <<  DmpLogEndl;
      }
      G4double ESil=0;
      G4double ESilPrim=0;
      G4ThreeVector pos;
      G4int NStrip, NPlane, IsX;
      //StkHitEvt->nSiHit  =  0;
      for (int i=0;i<n_hit;i++) {

          //std::cout<<"adding hit"<<DmpLogEndl;

          ESil     = (*THC)[i]->GetEdepSil();
          ESilPrim = (*THC)[i]->GetEdepSilPrim();
          NStrip = (*THC)[i]->GetNStrip();
          NPlane = (*THC)[i]->GetNSilPlane();
          IsX    = (*THC)[i]->GetPlaneType();
          pos    = (*THC)[i]->GetPos();

          //DmpStkHit* hit=new DmpStkHit();
          DmpStkHit *hit = (DmpStkHit*)fStkHits->ConstructedAt(fStkHits->GetLast()+1);

          hit->SiHit_e  =  ESil;
          hit->SiHit_e_prim = ESilPrim;
          hit->SiHit_x = pos.x();
          hit->SiHit_y = pos.y();
          hit->SiHit_z = pos.z();
          hit->SiHit_strip  = IsX*1000000 + NPlane*100000 + NStrip;
          hit->SiHit_id = i;
          hit->SiHit_trackID  = (*THC)[i]->GetPrimTrackID();
          hit->SiHit_trackID1 = (*THC)[i]->GetTrackID1();
          hit->SiHit_trackID2 = (*THC)[i]->GetTrackID();
          hit->SiHit_ntracks = (*THC)[i]->GetNtracks();
          hit->SiHit_ntracksBack = (*THC)[i]->GetNtracksBack();
          hit->SiHit_ntracksPrim = (*THC)[i]->GetNtracksPrim();
          hit->SiHit_chrgTrks_e = (*THC)[i]->GetChrgTrkEnergy();

          //++ (StkHitEvt->nSiHit);
          //StkHitEvt->SiHits.push_back(*hit);

          if(eventNumber == debugEvent || debugEvent == -1) {
            DmpLogDebug << ESil/MeV <<  "   "  << NStrip << "    " << NPlane <<  "    " << IsX << "    "
                      << (*THC)[i]->GetTrackID()
                      << "     " << pos.x()/mm << " " <<  pos.y()/mm << " " <<  pos.z()/mm << DmpLogEndl;
          }

      }

  }
}






