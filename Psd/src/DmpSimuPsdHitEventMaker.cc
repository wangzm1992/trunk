// Description: ntuple maker for the PSD
//
// Author(s):
//  - created by Zhou Yong. 10/10/2013
//  - modified by A.Tykhonov 05/02/2014 --> converted from NTuple maker to Event class maker

#include "DmpSimuPsdHitEventMaker.hh"
#include "DmpCore.h"
#include "DmpIOSvc.h"

#include "G4SDManager.hh"
#include "DmpSimuPsdHit.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "TTree.h"



DmpSimuPsdHitEventMaker::DmpSimuPsdHitEventMaker()
    //:tree(0)
  :debugEvent(-100) //-100 no event
{
    //set debugEvent to a particular event number for debug printout
    //debugEvent = -1; //-1 is all events

}

DmpSimuPsdHitEventMaker::~DmpSimuPsdHitEventMaker()
{

}

/*
void DmpSimuPsdHitEventMaker::book(const G4Run* aRun,TTree* aTree)
{
    tree=aTree;
    PsdHitEvt = new DmpEvtPsdHit();
    tree->Branch("DmpEvtPsdHit", "DmpEvtPsdHit",  &PsdHitEvt);

}
*/

void DmpSimuPsdHitEventMaker::AddCollections(const G4Run* aRun)
{
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  //fPsdHits  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuPsdHitsCollection"),  const_cast<char *>("DmpPsdHit"));
  fPsdHits = new TClonesArray("DmpPsdHit",10000);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuPsdHitsCollection",fPsdHits);
}

void DmpSimuPsdHitEventMaker::BeginRun(const G4Run* aRun)
{

}

void DmpSimuPsdHitEventMaker::BeginEvent(const G4Event* evt)
{
  fPsdHits->Delete();
  fPsdHits->Clear();
}

void DmpSimuPsdHitEventMaker::FillEvent(const G4Event* evt)
{
    G4int eventNumber = evt->GetEventID();

    DmpSimuPsdHitsCollection* THC=0;
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    G4HCofThisEvent* HCE = evt->GetHCofThisEvent();

    if(HCE){
        THC = (DmpSimuPsdHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("PSDHitCollection")));
    }

    if(THC){
      int n_hit = THC->entries();
      if(eventNumber==debugEvent || debugEvent==-1){
	
      }

      G4int StripNum;
      G4double StripEdep;
      for(int i=0;i<n_hit;i++){
	StripNum=(*THC)[i]->GetStripNumber();
	StripEdep=(*THC)[i]->GetEdep();
	  
	//DmpPsdHit* hit = new  DmpPsdHit();
        DmpPsdHit *hit = (DmpPsdHit*)fPsdHits->ConstructedAt(fPsdHits->GetLast()+1);

	hit->strip = StripNum;
	hit->edep  = StripEdep;
	
	hit->pos_x =(*THC)[i]->GetPos().x();
	hit->pos_y =(*THC)[i]->GetPos().y();
	hit->pos_z =(*THC)[i]->GetPos().z();
	
	hit->ntracks     = (*THC)[i]->GetNtracks();
	hit->ntracksBack = (*THC)[i]->GetNtracksBack();
	
	hit->sumposition = (*THC)[i]->GetSumPosition();
	
	//PsdHitEvt->PsdHits.push_back(*hit);
	//++(PsdHitEvt->nPsdHit);
      }
      
      if(eventNumber==debugEvent || debugEvent==-1){
	
      }
    }
}

