#ifndef DmpSimuStkDigitizer_h
#define DmpSimuStkDigitizer_h 1

#include "G4VDigitizerModule.hh"
#include "DmpSimuStkDigi.hh"
#include "globals.hh"
#include "DmpSimuConfigParser.h"
#include "DmpCore.h"

//class DmpSimuStkDigitizerMessenger;
class DmpDetectorConstruction;

class DmpSimuStkDigitizer : public G4VDigitizerModule
{
public:
  
  DmpSimuStkDigitizer(G4String name);
  ~DmpSimuStkDigitizer();
  
  void Digitize();
  void SetThreshold(G4double val) { STKThreshold = val;}
  inline G4int* GetThitID() {return ThitID;}

private:
  
  DmpSimuStkDigitsCollection*  DigitsCollection;

  G4double STKThreshold; // energy threshold to produce a digit
  //DmpSimuStkDigitizerMessenger* digiMessenger;

  DmpDetectorConstruction* dmpDetector;

  G4int (*ThitID);
  
  G4int NbOfTKRLayers;
  G4int NbOfTKRStrips;
  G4int NbOfTKRChannels;
  G4int NbOfTKRChannelsL;


  DmpSimuConfigParser* fSimuConfig;

  double FLOATSTRIP_FRACTION_1;
  double FLOATSTRIP_FRACTION_2;
  double FLOATSTRIP_FRACTION_3;
  double FLOATSTRIP_FRACTION_4;

  double READOUTSTRIP_FRACTION_0;
  double READOUTSTRIP_FRACTION_1;
  double READOUTSTRIP_FRACTION_2;

};

#endif








