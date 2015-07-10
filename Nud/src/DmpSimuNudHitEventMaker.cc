// Description: event maker for Nud
//
// Author(s):
//  - added by Tiekuang Dong 2012/12/23 (revised by A.Tykhonov)

#include "DmpSimuNudHitEventMaker.hh"
#include "DmpNudDetectorDescription.hh"
#include "DmpDetectorConstruction.hh"
#include "DmpSimuNudHit.hh"

#include "G4RunManager.hh"
#include "G4DigiManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "TTree.h"

#include "DmpIOSvc.h"
#include "DmpCore.h"

DmpSimuNudHitEventMaker::DmpSimuNudHitEventMaker()
  :debugEvent(-100)
{
  //set debugEvent to a particular event number for debug printout  
  //debugEvent = -1; //-1 is all events
  //debugEvent = -100; //-100 no event
}

DmpSimuNudHitEventMaker::~DmpSimuNudHitEventMaker()
{
}


void DmpSimuNudHitEventMaker::AddCollections(const G4Run* aRun)
{
  //fDataMgr   = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  /*
  fNudHits0  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuNudHits0Collection"),  const_cast<char *>("DmpNudHit"));
  fNudHits1  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuNudHits1Collection"),  const_cast<char *>("DmpNudHit"));
  fNudHits2  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuNudHits2Collection"),  const_cast<char *>("DmpNudHit"));
  fNudHits3  = fDataMgr->NewOutCollection(const_cast<char *>("DmpSimuNudHits3Collection"),  const_cast<char *>("DmpNudHit"));
  */
  fNudHits0 = new TClonesArray("DmpNudHit",10000);
  fNudHits1 = new TClonesArray("DmpNudHit",10000);
  fNudHits2 = new TClonesArray("DmpNudHit",10000);
  fNudHits3 = new TClonesArray("DmpNudHit",10000);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuNudHits0Collection",fNudHits0);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuNudHits1Collection",fNudHits1);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuNudHits2Collection",fNudHits2);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpSimuNudHits3Collection",fNudHits3);


}


void DmpSimuNudHitEventMaker::BeginEvent(const G4Event* evt)
{
  eventNumber = evt->GetEventID();

  fNudHits0 ->Delete();
  fNudHits1 ->Delete();
  fNudHits2 ->Delete();
  fNudHits3 ->Delete();

  fNudHits0 ->Clear();
  fNudHits1 ->Clear();
  fNudHits2 ->Clear();
  fNudHits3 ->Clear();

}

void DmpSimuNudHitEventMaker::BeginRun(const G4Run* aRun)
{
}

void DmpSimuNudHitEventMaker::FillStep(const G4Step* aStep)
{
}

void DmpSimuNudHitEventMaker::FillEvent(const G4Event* evt)
{  
  //fill even info here, also filter and save ntuple
  //bool toKeep = true;

  //NUD hits
  DmpSimuNudHitsCollection* CHC0 = 0;
  DmpSimuNudHitsCollection* CHC1 = 0;
  DmpSimuNudHitsCollection* CHC2 = 0;
  DmpSimuNudHitsCollection* CHC3 = 0;

  G4SDManager * SDman = G4SDManager::GetSDMpointer();  
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();

  if (HCE) {
  
    CHC1 = (DmpSimuNudHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("NUDHitCollection1")));
     
    CHC2 = (DmpSimuNudHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("NUDHitCollection2")));
 
    CHC3 = (DmpSimuNudHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("NUDHitCollection3")));
    
    CHC0 = (DmpSimuNudHitsCollection*) (HCE->GetHC(SDman->GetCollectionID("NUDHitCollection0")));
 
  }


  if (CHC0) {
    G4int n_hit = CHC0->entries();
    if(eventNumber == debugEvent || debugEvent == -1) {
      DmpLogDebug << "Number of NUD hits0 in this event =  " << n_hit << DmpLogEndl;
    }   
    
    G4RunManager* runManager = G4RunManager::GetRunManager();
    DmpDetectorConstruction* dmpDetector =
      (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

    G4int NbOfNudBlocks = 4;
    
    if(NbOfNudBlocks != dmpDetector->GetNudDetectorDescription()->GetNbOfNUDStrips()) DmpLogWarning<< "WARNING NbOfNudBlocks is not "<< NbOfNudBlocks<< "!!!"<< DmpLogEndl;
    G4double Edep=0.;
    G4double time=0.; 
    G4ThreeVector pos;
  //  G4int  NudBlockNumber;
   
    nud_totE0 = 0.;
    for (int i=0;i<n_hit;i++) {
      Edep           = (*CHC0)[i]->GetEdep();
      pos            = (*CHC0)[i]->GetPos();
   
      time           = (*CHC0)[i]->GetTime();
      if(eventNumber == debugEvent || debugEvent == -1) {
        DmpLogDebug << Edep/MeV <<  "   "  << time << "     " << pos.x()/mm << " " <<  pos.y()/mm << " " <<  pos.z()/mm << DmpLogEndl;
      }

      //DmpNudHit *hit = (DmpNudHit*)fNudHits0->ConstructedAt(fNudHits0->GetLast()+1);
      new((*fNudHits0)[fNudHits0->GetLast()+1]) DmpNudHit(Edep, time);
      //DmpNudHit* dmpNudHit = new DmpNudHit(Edep, time);
      nud_totE0 += Edep;
      //dmpEvtNudHit->nudHits0.push_back(*dmpNudHit);
}
}



 if (CHC1) {
 
    G4int n_hit = CHC1->entries();
    if(eventNumber == debugEvent || debugEvent == -1) {
      DmpLogDebug << "Number of NUD hits1 in this event =  " << n_hit << DmpLogEndl;
    }   
    
    G4RunManager* runManager = G4RunManager::GetRunManager();
    DmpDetectorConstruction* dmpDetector =
      (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

    G4int NbOfNudBlocks = 4;
    
    if(NbOfNudBlocks != dmpDetector->GetNudDetectorDescription()->GetNbOfNUDStrips()) DmpLogWarning<< "WARNING NbOfNudBlocks is not "<< NbOfNudBlocks<< "!!!"<< DmpLogEndl;
   G4double Edep=0.;
    G4double time=0.; 
    G4ThreeVector pos;
  //  G4int  NudBlockNumber;
    nud_totE1 = 0;
    for (int i=0;i<n_hit;i++) {
      Edep           = (*CHC1)[i]->GetEdep();
      pos            = (*CHC1)[i]->GetPos();
  
      time           = (*CHC1)[i]->GetTime();
      if(eventNumber == debugEvent || debugEvent == -1) {
        DmpLogDebug << Edep/MeV <<  "   "  << time << "     " << pos.x()/mm << " " <<  pos.y()/mm << " " <<  pos.z()/mm << DmpLogEndl;
      }        
    
      new((*fNudHits1)[fNudHits1->GetLast()+1]) DmpNudHit(Edep, time);
      //DmpNudHit* dmpNudHit = new DmpNudHit(Edep, time);
      nud_totE1 += Edep;
      //dmpEvtNudHit->nudHits1.push_back(*dmpNudHit);
}
}


  if (CHC2) {
    G4int n_hit = CHC2->entries();
    if(eventNumber == debugEvent || debugEvent == -1) {
      DmpLogDebug << "Number of NUD hits2 in this event =  " << n_hit << DmpLogEndl;
    }   
    
    G4RunManager* runManager = G4RunManager::GetRunManager();
    DmpDetectorConstruction* dmpDetector =
      (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

    G4int NbOfNudBlocks = 4;
    
    if(NbOfNudBlocks != dmpDetector->GetNudDetectorDescription()->GetNbOfNUDStrips()) DmpLogWarning<< "WARNING NbOfNudBlocks is not "<< NbOfNudBlocks<< "!!!"<< DmpLogEndl;
    G4double Edep=0.;
    G4double time=0.; 
    G4ThreeVector pos;
  //  G4int  NudBlockNumber;
    nud_totE2 = 0;
    for (int i=0;i<n_hit;i++) {
      Edep           = (*CHC2)[i]->GetEdep();
      pos            = (*CHC2)[i]->GetPos();
 
      time           = (*CHC2)[i]->GetTime();
      if(eventNumber == debugEvent || debugEvent == -1) {
        DmpLogDebug << Edep/MeV <<  "   "  << time << "     " << pos.x()/mm << " " <<  pos.y()/mm << " " <<  pos.z()/mm << DmpLogEndl;
      }        
    
      new((*fNudHits2)[fNudHits2->GetLast()+1]) DmpNudHit(Edep, time);
      //DmpNudHit* dmpNudHit = new DmpNudHit(Edep, time);
      nud_totE2 += Edep;
      //dmpEvtNudHit->nudHits2.push_back(*dmpNudHit);
}
}

 //NUD hits
 // DmpSimuNudHitsCollection* CHC3 = 0;
 // G4SDManager * SDman = G4SDManager::GetSDMpointer();  
 // G4HCofThisEvent* HCE = evt->GetHCofThisEvent();

 
  if (CHC3) {
    G4int n_hit = CHC3->entries();
    if(eventNumber == debugEvent || debugEvent == -1) {
      DmpLogDebug << "Number of NUD hits3 in this event =  " << n_hit << DmpLogEndl;
    }   
    
    G4RunManager* runManager = G4RunManager::GetRunManager();
    DmpDetectorConstruction* dmpDetector =
      (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

    G4int NbOfNudBlocks = 4;
    
    if(NbOfNudBlocks != dmpDetector->GetNudDetectorDescription()->GetNbOfNUDStrips()) DmpLogWarning<< "WARNING NbOfNudBlocks is not "<< NbOfNudBlocks<< "!!!"<< DmpLogEndl;
    G4double Edep=0.;
    G4double time=0.; 
    G4ThreeVector pos;
  //  G4int  NudBlockNumber;
   
    nud_totE3 = 0;
    for (int i=0;i<n_hit;i++) {
      Edep           = (*CHC3)[i]->GetEdep();
      pos            = (*CHC3)[i]->GetPos();
 
      time           = (*CHC3)[i]->GetTime();
      if(eventNumber == debugEvent || debugEvent == -1) {
        DmpLogDebug << Edep/MeV <<  "   "  << time << "     " << pos.x()/mm << " " <<  pos.y()/mm << " " <<  pos.z()/mm << DmpLogEndl;
      }        
    
      new((*fNudHits3)[fNudHits3->GetLast()+1]) DmpNudHit(Edep, time);
      //DmpNudHit* dmpNudHit = new DmpNudHit(Edep, time);
      nud_totE3 += Edep;
      //dmpEvtNudHit->nudHits3.push_back(*dmpNudHit);

    }
 }
    if(eventNumber == debugEvent || debugEvent == -1) {
      DmpLogDebug << "NUD: total energy deposited: " << nud_totE0 <<DmpLogEndl;
      DmpLogDebug << "NUD: total energy deposited: " << nud_totE1 <<DmpLogEndl;
      DmpLogDebug << "NUD: total energy deposited: " << nud_totE2 <<DmpLogEndl;
      DmpLogDebug << "NUD: total energy deposited: " << nud_totE3 <<DmpLogEndl;
    }

}



G4bool DmpSimuNudHitEventMaker::passTotalEnergyCut(G4double cut)
{  
 // return (tt_nud_totE > cut);
return true;
}




