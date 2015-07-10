/*
 *  $Id: DmpSimAlgBoot.h, 2014-05-15 00:21:39 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 12/04/2014
*/

#ifndef DmpSimAlg_H
#define DmpSimAlg_H

#include "DmpVAlg.h"
#include "DmpG4RunManager.hh"
#include "G4UImanager.hh"
#include "G4RunManager.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#include "G4UIterminal.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif


class DmpDetectorConstruction;
class DmpSimuPrimaryGeneratorAction;
class DmpSimuRunAction;
class DmpSimuEventAction;
class DmpSimuTrackingAction;
class DmpRootEventDataManager;
class DmpSimuSteppingAction;

class DmpSimAlg : public DmpVAlg { //, public G4RunManager{
/*
 *  Geant4 service
 *
 *
 */
public:
  DmpSimAlg();
  ~DmpSimAlg();

  bool Initialize();
  bool ProcessThisEvent();
  bool Finalize();
  void Set(const std::string &type,const std::string &argv);

  //bool Get(const std::string &type);
  bool StkSimulationEnabled(){   return fSTKsimulationOn;  }
  bool BgoSimulationEnabled(){   return fBGOsimulationOn;  }
  bool NudSimulationEnabled(){   return fNUDsimulationOn;  }
  bool PsdSimulationEnabled(){   return fPSDsimulationOn;  }
  bool SatSimulationEnabled(){   return fSATsimulationOn;  }

public:
  std::string GetMainGdmlName() {return gdmlGeometryMainName;}
  //std::string GetGdmlPath()     {return gdmlGeometryPath; }


private:
  bool ParseBoolValue(const std::string &argv);

private:
  DmpG4RunManager               *fRunMgr;
  DmpDetectorConstruction       *fDetector;
  DmpSimuPrimaryGeneratorAction *fPriGen;
  DmpSimuRunAction              *fRunAct;
  DmpSimuEventAction            *fEvtAct;
  DmpSimuTrackingAction         *fTrcAct;
  DmpSimuSteppingAction         *fStpAct;
  G4UImanager                   *fUIMgr;
  DmpRootEventDataManager       *fRootEvtDataMngr;

#ifdef G4VIS_USE
  G4VisManager                  *fVisMgr;
#endif

  bool          fBeamOnCondition;
  std::string   fMacFile;
  long          fEventID;

  bool fSTKsimulationOn;
  bool fBGOsimulationOn;
  bool fNUDsimulationOn;
  bool fPSDsimulationOn;
  bool fSATsimulationOn;

  bool fVisualize;
  long fSeed;
  //std::vector<std::string> fMagFields;


  G4UIExecutive* ui;

  //
  //G4PhysListFactory             *fPhyFactory;
  //std::string                    fPhyListName;       // default is QGSP_BIC

private:
  std::string gdmlGeometryMainName;
  //std::string gdmlGeometryPath;

};

#endif

