// Description: This class hinerits from G4VSensitiveDetector. It is used to
// build the STK sensitive area geometry and produce DmpSimuStkHit collection
//
// Author(s):
//  - creation by X.Wu, 12/07/2013

#include "G4RunManager.hh"
#include "DmpStkSensitiveDetector.hh"
//#include "DmpStkROGeometry.hh"
//#include <ctime>
//#include <sys/time.h>


#include "DmpSimuStkHit.hh"
#include "DmpDetectorConstruction.hh"
#include "DmpStkDetectorDescription.hh"
#include "DmpSimuPrimaryGeneratorAction.hh"

#include "G4VPhysicalVolume.hh"

#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include "DmpLog.h"



//#define STK_NTUPLER_DEBUG_MODE

DmpStkSensitiveDetector::DmpStkSensitiveDetector(G4String name):G4VSensitiveDetector(name)
{
  G4RunManager* runManager = G4RunManager::GetRunManager();
  dmpDetector = (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

  G4int NbOfTKRTiles  =  dmpDetector->GetStkDetectorDescription()->GetNbOfTKRTiles();
  G4int NbOfTKRBlocks =  dmpDetector->GetStkDetectorDescription()->GetNbOfTKRBlocks();
  NbOfTKRStrips  = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips();
  NbOfTKRLayers  = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRLayers();  
  DmpLogInfo << "[DmpStkSensitiveDetector::DmpStkSensitiveDetector] In constructor... " << DmpLogEndl;

  DmpLogInfo <<  "  " << NbOfTKRTiles  << " tracker ladders " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRBlocks << " tracker blocks " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRLayers << " tracker layers " << DmpLogEndl;
  //G4cout <<  "  " << NbOfTKRStrips << " tracker strips per tile (ladder) " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRStrips << " tracker strips per ladder " << DmpLogEndl;

  NbOfTKRStrips = NbOfTKRStrips*NbOfTKRTiles*NbOfTKRBlocks;  
 
  DmpLogInfo <<  "  NbOfTKRStrips in DmpStkSensitiveDetector is the number of strips per layer : " << NbOfTKRStrips << DmpLogEndl;
  
  //NbOfTKRChannels = NbOfTKRStrips* NbOfTKRTiles * NbOfTKRLayers;
  //G4cout <<  "  total number of readout channels (why times NbOfTKRTiles again?): " << NbOfTKRChannels << DmpLogEndl;
  NbOfTKRChannels = NbOfTKRStrips * NbOfTKRLayers;
  DmpLogInfo <<  "  total number of readout channels per view : " << NbOfTKRChannels << DmpLogEndl;

  ThitXID = new G4int[NbOfTKRChannels];
  ThitYID = new G4int[NbOfTKRChannels];
  //set the collection name (in G4VSensitiveDetector)
  collectionName.insert("STKHitCollection");

  DmpLogInfo << "[DmpStkSensitiveDetector::DmpStkSensitiveDetector] ... finished constructing " << DmpLogEndl;

}

DmpStkSensitiveDetector::~DmpStkSensitiveDetector()
{
  delete [] ThitXID;
  delete [] ThitYID;
}

void DmpStkSensitiveDetector::Initialize(G4HCofThisEvent*)
{
  TrackerCollection = new DmpSimuStkHitsCollection
    (SensitiveDetectorName,collectionName[0]);

 for (G4int i=0;i<NbOfTKRChannels;i++)
   {
     ThitXID[i] = -1;
     ThitYID[i] = -1;
   };

 G4RunManager* runManager = G4RunManager::GetRunManager();
 primaryGeneratorAction =
   (DmpSimuPrimaryGeneratorAction*)(runManager->GetUserPrimaryGeneratorAction());

 isPhotonRun = false;
 if(primaryGeneratorAction->GetParticleDefinition()->GetParticleName() == "gamma") isPhotonRun = true;

}


G4bool DmpStkSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* )
//G4bool DmpStkSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* ROhist)
{ 
   
  G4double edep = aStep->GetTotalEnergyDeposit();
  if ((edep/keV == 0.)) return false;      

  bool is_backtrack= false;
  //if(aStep->GetPreStepPoint()->GetMomentum().z()>0) is_backtrack= true;  //<-- old coordinate system
  if(aStep->GetPreStepPoint()->GetMomentum().z()<0) is_backtrack= true;  //<-- new coordinate system

  G4int StripTotal = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips();
  G4int TileTotal  = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRTiles();  //number of tiles per layer

  // This TouchableHistory is used to obtain the physical volume
  // of the hit
  //G4TouchableHistory* theTouchable
  //  = (G4TouchableHistory*)(aStep->GetPreStepPoint()->GetTouchable());
  
  
  //G4cout << "       point1 momentum (MeV): " <<aStep->GetPreStepPoint()->GetMomentum().x() << DmpLogEndl;
  //G4VPhysicalVolume* phys_tile = theTouchable->GetVolume();  
  
  //G4VPhysicalVolume* plane = theTouchable->GetVolume(1);  //this is a Tile
  
  //G4int PlaneNumber = 0;
  //PlaneNumber=plane->GetCopyNo();

  //  ROhist->MoveUpHistory();
  //G4VPhysicalVolume* tile = ROhist->GetVolume(1);
  G4VPhysicalVolume* tile = aStep->GetPreStepPoint()->GetTouchable()->GetVolume(1); // ROhist->GetVolume(1);
  //G4String TileName = dmpDetector->GetStkDetectorDescription()->GetTileName(tile->GetLogicalVolume()); //tile->GetName();

  if(!(dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTile(tile->GetName())))
  {
    std::string error = "[DmpStkDetectorDescription::defineSensitiveDetector] ERROR: tile is neither X nor Y!";
    DmpLogError<< error<< DmpLogEndl;
    throw error;
  }

  //G4int    PlaneNumber  = dmpDetector->GetStkDetectorDescription()->GetPlaneNumber(tile->GetLogicalVolume());
  //G4String PlaneName    = dmpDetector->GetStkDetectorDescription()->GetPlaneName(tile->GetLogicalVolume());
  G4int    NBlocksLayer = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRBlocks();  // 4

  /*
  G4int LayerNumber = PlaneNumber/NBlocksLayer;  //0-5
  G4int BlockNumber = (PlaneNumber%NBlocksLayer);//0-3
  */
  //G4int TileNumber;
  G4int LadderNumber;
  G4int PlaneNumber;
  G4int BlockNumber;
  G4int IsX;
  G4int IsY;
  dmpDetector->GetStkDetectorDescription()->GetSensitiveDetectorParamters(tile->GetLogicalVolume(), PlaneNumber, BlockNumber, LadderNumber, IsX, IsY);
  //G4int TileNumber  = dmpDetector->GetStkDetectorDescription()->GetTileNumber(tile->GetLogicalVolume()); //tile->GetCopyNo();


  // The RO History is used to obtain the real strip
  // of the hit


  G4VPhysicalVolume* strip = 0;
  //strip = ROhist->GetVolume();
  strip = aStep->GetPreStepPoint()->GetTouchable()->GetVolume();
  G4int StripNumber = strip->GetCopyNo();


  //  if (strip){
  //G4String StripName   = dmpDetector->GetStkDetectorDescription()->GetStripName(strip->GetLogicalVolume()); //strip->GetName();
  //G4int    StripNumber = dmpDetector->GetStkDetectorDescription()->GetStripNumber(strip->GetLogicalVolume());



  /*
  if    ( dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTileX(tile->GetName()) ){

  }
  else if( dmpDetector->GetStkDetectorDescription()->isVolumeSTKActiveTileY(tile->GetName())){
  }
  else{
  */


  //Number of RO strips on a tile: 0-767
  //  }




  DmpLogDebug<<DmpLogEndl;
  DmpLogDebug<<"[DmpStkSensitiveDetector::ProcessHits] tile->GetName()" <<tile->GetName()<<DmpLogEndl;
  //G4cout<<"[DmpStkSensitiveDetector::ProcessHits]   TileNumber: "<<TileNumber<<DmpLogEndl;
  //G4cout<<"[DmpStkSensitiveDetector::ProcessHits]   TileName: "<<TileName<<DmpLogEndl;
  DmpLogDebug<<"[DmpStkSensitiveDetector::ProcessHits]   PlaneNumber: "<<PlaneNumber<<DmpLogEndl;
  //G4cout<<"[DmpStkSensitiveDetector::ProcessHits]   PlaneName: "<<PlaneName<<DmpLogEndl;
  //G4cout<<"[DmpStkSensitiveDetector::ProcessHits]   LayerNumber: "<<LayerNumber<<DmpLogEndl;
  DmpLogDebug<<"[DmpStkSensitiveDetector::ProcessHits]   BlockNumber: "<<BlockNumber<<DmpLogEndl;
  DmpLogDebug<<"[DmpStkSensitiveDetector::ProcessHits] strip->GetName(): "<<strip->GetName()<<DmpLogEndl;
  //G4cout<<"[DmpStkSensitiveDetector::ProcessHits]   StripName: "<<StripName<<DmpLogEndl;
  DmpLogDebug<<"[DmpStkSensitiveDetector::ProcessHits]   StripNumber: "<<StripNumber<<DmpLogEndl;


  /***
  G4cout << " Strip Number = " << StripNumber << ", name = " << StripName  << DmpLogEndl;
  G4cout << " Tile Number = " << TileNumber << ", name = " << TileName  << DmpLogEndl;
  G4cout << " Block Index = " << PlaneNumber << ", name = " << PlaneName  << DmpLogEndl;
  G4cout << " Layer Number = " << LayerNumber  << ", Block Number = " << BlockNumber <<DmpLogEndl;

  //StripTotal=768, TileTotal=4 (numnber of tiles per ladders)
  //G4cout << " StripTotal = " << StripTotal  << " TileTotal = " << TileTotal << DmpLogEndl;
  ***/
  //Ladder number


  //G4int NTile = (TileNumber%TileTotal);



  /*
  G4int j=0;
  

  //4 tiles wire-bonded to a ladder 
  for (j=0;j<TileTotal;j++)
    {
      //strip number within a block (0-3071)
      if(NTile==j) StripNumber += StripTotal*NTile;
    }
    */
  StripNumber += StripTotal*LadderNumber;
  
  G4int NChannel = 0;
  //NChannel = PlaneNumber*TileTotal*StripTotal + StripNumber;
  NChannel = (PlaneNumber*NBlocksLayer + BlockNumber)*TileTotal*StripTotal + StripNumber;

  //strip number witin a layer (0-12287)
  StripNumber += BlockNumber*TileTotal*StripTotal;

 
  G4int trackID = aStep->GetTrack()->GetTrackID();
  bool isPrim = false;
  if((trackID == 2 || trackID == 3) && isPhotonRun) isPrim = true;
  else if (trackID == 1) isPrim = true;

  //G4cout << " Channel Number = " << NChannel  << " Strip number on plane = " << StripNumber << " plane name " << PlaneName << DmpLogEndl;


  /*
  std::cout<<DmpLogEndl;
  std::cout<<DmpLogEndl;
  sruct timeval tv;
  gettimeofday(&tv,NULL);
  std::cout<< "time: "<< tv.tv_usec <<DmpLogEndl;
  std::cout<<DmpLogEndl;
  std::cout<<DmpLogEndl;
  */


  if (IsX){
    __addHit(ThitXID, NChannel, edep, aStep, 1, PlaneNumber, BlockNumber, StripNumber, is_backtrack, trackID, isPrim);
  }

  if(IsY){
    __addHit(ThitYID, NChannel, edep, aStep, 0, PlaneNumber, BlockNumber, StripNumber, is_backtrack, trackID, isPrim);
  }
  
  return true;
}

void DmpStkSensitiveDetector::EndOfEvent(G4HCofThisEvent* HCE)
{
  //G4cout << "DmpStkSensitiveDetector::EndOfEvent called: register the DmpStkHitsCollection " << DmpLogEndl;
  static G4int HCID = -1;
  if(HCID<0)
    { 
      HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
  HCE->AddHitsCollection(HCID,TrackerCollection);


  for (G4int i=0;i<NbOfTKRChannels;i++) 
    {
      ThitXID[i] = -1;
      ThitYID[i] = -1;
    };
}


void DmpStkSensitiveDetector::__addHit(G4int* ThitID, G4int NChannel, double edep, G4Step* aStep, G4int isX, G4int PlaneNumber, G4int BlockNumber, G4int  StripNumber, bool is_backtrack, G4int trackID, bool isPrim){


  DmpLogDebug<<"[DmpStkSensitiveDetector::__addHit] The hit is on an X silicon plane"<<DmpLogEndl;



  if (ThitID[NChannel]==-1) {
    DmpSimuStkHit* TrackerHit = new DmpSimuStkHit;
    TrackerHit->SetPlaneType(isX);
    TrackerHit->AddSil(edep);
    TrackerHit->SetPos(aStep->GetPreStepPoint()->GetPosition());
    TrackerHit->SetNSilPlane(PlaneNumber);
    TrackerHit->SetNSilBlock(BlockNumber);
    TrackerHit->SetNStrip(StripNumber);
    TrackerHit->SetNtracks(1);

    //count back scattering tracks
    if(is_backtrack) TrackerHit->SetNtracksBack(1);

    //current track ID
    TrackerHit->SetTrackID(trackID);
    if(isPrim) {
      TrackerHit->AddSilPrim(edep);
      TrackerHit->SetPrimTrackID(trackID);
      TrackerHit->SetNtracksPrim(1);
    }

    // * charged tracks
    if(aStep->GetTrack()->GetDefinition()->GetPDGCharge()){
      TrackerHit->AddChrgTrkEnergy(aStep->GetTrack()->GetTotalEnergy());
    }
    ThitID[NChannel] = TrackerCollection->insert(TrackerHit) -1;
    //G4cout << "X new strip: track ID " << aStep->GetTrack()->GetTrackID() <<  " edp " << edep<< " prim edp " << TrackerHit->GetEdepSilPrim() << " NChannel " << NChannel << " isPrim " << isPrim << DmpLogEndl;
  }
  else { // This is not new strip
    (*TrackerCollection)[ThitID[NChannel]]->AddSil(edep);
    if(isPrim) {
      (*TrackerCollection)[ThitID[NChannel]]->AddSilPrim(edep);
    }
    if(aStep->GetTrack()->GetDefinition()->GetPDGCharge()){
      (*TrackerCollection)[ThitID[NChannel]]->AddChrgTrkEnergy(aStep->GetTrack()->GetTotalEnergy());
    }
    G4int oldID = (*TrackerCollection)[ThitID[NChannel]]->GetTrackID();
    //G4cout << "  X more hit old track " << oldID << " new track " << trackID << " isPrim " << isPrim << DmpLogEndl;
    if(oldID != trackID) {
      if(isPrim) {
        G4int oldPrimID = (*TrackerCollection)[ThitID[NChannel]]->GetPrimTrackID();
        (*TrackerCollection)[ThitID[NChannel]]->SetPrimTrackID(oldPrimID*100 + trackID);
        G4int oldP = (*TrackerCollection)[ThitID[NChannel]]->GetNtracksPrim();
        (*TrackerCollection)[ThitID[NChannel]]->SetNtracksPrim(oldP + 1);
      }
      //save old ID if not primary track
      bool oldPrim = false;
      if((oldID == 2 || oldID == 3) && isPhotonRun) oldPrim = true;
      else if (oldID == 1) oldPrim = true;
      if(!oldPrim) (*TrackerCollection)[ThitID[NChannel]]->SetTrackID1(oldID);
      //update current track ID
      (*TrackerCollection)[ThitID[NChannel]]->SetTrackID(trackID);
      //update number of tracks
      G4int oldN = (*TrackerCollection)[ThitID[NChannel]]->GetNtracks();
      (*TrackerCollection)[ThitID[NChannel]]->SetNtracks(oldN + 1);
      //G4cout << "    number of tracks before" << oldN << DmpLogEndl;
      //update number of back scattering tracks
      if(is_backtrack) {
        G4int oldNback = (*TrackerCollection)[ThitID[NChannel]]->GetNtracksBack();
        (*TrackerCollection)[ThitID[NChannel]]->SetNtracksBack(oldNback + 1);
        //G4cout << "    number of back tracks before" << oldNback << DmpLogEndl;
      }
    }
  }
}


void DmpStkSensitiveDetector::clear()
{} 

void DmpStkSensitiveDetector::DrawAll()
{} 

void DmpStkSensitiveDetector::PrintAll()
{} 













