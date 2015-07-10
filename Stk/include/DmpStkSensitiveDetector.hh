#ifndef DmpStkSensitiveDetector_h
#define DmpStkSensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include "globals.hh"

class DmpDetectorConstruction;
class DmpSimuPrimaryGeneratorAction;
class DmpStkROGeometry;
class G4HCofThisEvent;
class G4Step;

#include "DmpSimuStkHit.hh"

class DmpStkSensitiveDetector : public G4VSensitiveDetector
{
public:
  
  DmpStkSensitiveDetector(G4String);
  ~DmpStkSensitiveDetector();
  
  void Initialize(G4HCofThisEvent*);
  G4bool ProcessHits(G4Step* astep,G4TouchableHistory* ROHist);
  void EndOfEvent(G4HCofThisEvent*);
  void clear();
  void DrawAll();
  void PrintAll();
  
private:
  void __addHit(G4int* THitID, G4int NChannel, double edep, G4Step* aStep, G4int isX, G4int PlaneNumber, G4int BlockNumber, G4int  StripNumber, bool is_backtrack, G4int trackID, bool isPrim);

private:
  
  DmpSimuStkHitsCollection*  TrackerCollection;      
  DmpDetectorConstruction* dmpDetector;
  DmpSimuPrimaryGeneratorAction* primaryGeneratorAction;

  G4bool isPhotonRun;

  G4int (*ThitXID);
  G4int (*ThitYID);
  
  G4int NbOfTKRLayers;
  G4int NbOfTKRStrips;
  G4int NbOfTKRChannels;


};

#endif






