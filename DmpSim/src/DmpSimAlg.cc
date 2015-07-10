/*
 *  $Id: DmpSimAlgBoot.cc, 2014-05-09 10:12:45 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 12/04/2014
*/

#include "DmpSimAlg.h"
#include "DmpRootEventDataManager.hh"
#include "DmpDetectorConstruction.hh"
#include "DmpSimuPrimaryGeneratorAction.hh"
#include "DmpSimuRunAction.hh"
#include "DmpSimuTrackingAction.hh"
#include "DmpSimuSteppingAction.hh"
#include "DmpSimuEventAction.hh"
#include "QGSP_BIC.hh"
//#include "QGSP_BERT.hh"

#include <boost/lexical_cast.hpp>
//#include <DmpEvtMCNudMSD.h>
#include "DmpLog.h"
#include "DmpCore.h"

#include <fstream>
#include <sstream>
#include <iostream>




//-------------------------------------------------------------------
DmpSimAlg::DmpSimAlg()
 :DmpVAlg("Sim/SimAlg"),
  fMacFile(""),
  fEventID(0),
  fBeamOnCondition(false),
  fSTKsimulationOn(true),
  fBGOsimulationOn(true),
  fNUDsimulationOn(true),
  fPSDsimulationOn(true),
  fSATsimulationOn(true),
  fVisualize(false),
  fSeed(-1),
  gdmlGeometryMainName("")
  //gdmlGeometryPath("")
{
  //fDetector = new DmpSimDetector(); // must create it at here, since will set option before Initialize(). Do NOT "delete fDetector" in destructor, fRunMgr will delete it
}

//-------------------------------------------------------------------
DmpSimAlg::~DmpSimAlg(){
}

//-------------------------------------------------------------------


/*
void DmpSimAlgBoot::Set(const std::string &type,const std::string &argv){
  if(type == "Physics"){
    fPhyListName = argv;
  }else if(type == "Gdml"){
    fDetector->SetGdml(argv);
  }else if(type == "Nud/DeltaTime"){
    DmpEvtMCNudMSD::SetDeltaTime(boost::lexical_cast<short>(argv));
  }
}
*/


//-------------------------------------------------------------------
bool DmpSimAlg::Initialize(){

  fRunMgr = new DmpG4RunManager();                                 // Construct the default run manager
  fRootEvtDataMngr = new DmpRootEventDataManager();                // set an ROOT data manager
  fDetector = new DmpDetectorConstruction;                         // Set mandatory user initialization classes
  fRunMgr->SetUserInitialization(fDetector);

  //fDetector->SetUniformMagneticFields(fMagFields);

  // DmpSimuPhysicsList to be implemented
  //runManager->SetUserInitialization(new DmpSimuPhysicsList);
  //runManager->SetUserInitialization(new QGSP_BERT);
  fRunMgr->SetUserInitialization(new QGSP_BIC);

  // Initialize G4 kernel
  fRunMgr->Initialize();

  // Set mandatory user action classes
  fPriGen = new DmpSimuPrimaryGeneratorAction;
  fRunMgr->SetUserAction(fPriGen);
  fPriGen->SetDmpRootEventDataManager(fRootEvtDataMngr);
  if(fSeed != -1) fPriGen->SetRandomSeed(fSeed);

  // Set optional user action classes
  fRunAct = new DmpSimuRunAction(fRootEvtDataMngr,fPriGen);
  fRunMgr->SetUserAction(fRunAct);

  fEvtAct = new DmpSimuEventAction(fRootEvtDataMngr);
  fRunMgr->SetUserAction(fEvtAct);

  fTrcAct = new DmpSimuTrackingAction();
  fRunMgr->SetUserAction(fTrcAct);

  fStpAct = new DmpSimuSteppingAction(fRootEvtDataMngr);
  fRunMgr->SetUserAction(fStpAct);

   // Set visualization (not tested)
 #ifdef G4VIS_USE
   fVisMgr = new G4VisExecutive;
   fVisMgr->Initialize();
 #endif

  // Get the pointer to the UI manager
  fUIMgr = G4UImanager::GetUIpointer();


  // check if macro is available
  if (fMacFile!=""){
  	ifstream infile(fMacFile.c_str());
  	if (!infile){
  		DmpLogError
  		<< "Macro file not found: "<< fMacFile<< "  ==> throwing exception!"
  		<< DmpLogEndl;
  		throw;
  	}
  }


  //
  //if (fMacFile!="")
  if (!fVisualize)
  {
    DmpLogInfo<<"[DmpSimAlg::Initialize] Running with the macro file"<<DmpLogEndl;
    if (fMacFile!="") fUIMgr->ApplyCommand(std::string("/control/execute ") + fMacFile);
    fBeamOnCondition = fRunMgr->ConfirmBeamOnCondition();
    if(!fBeamOnCondition){
      DmpLogError<<"Failed to initialize geant4 run!"<<DmpLogEndl;
      return false;
    }
    fRunMgr->SetNumberOfEventsToProcess(gCore->GetMaxEventNumber());
    fRunMgr->ConstructScoringWorlds();
    fRunMgr->RunInitialization();
    fRunMgr->InitializeEventLoop(gCore->GetMaxEventNumber(),0, -1); //gCore->GetMaxEventNumber());




  }
  else
  {  // interactive mode : define UI session
    DmpLogInfo<<"[DmpSimAlg::Initialize] Running in interactive mode"<<DmpLogEndl;
    #ifdef G4UI_USE
    char* sessionname =const_cast<char *>("MyTestSession");
    //G4UIExecutive* ui = new G4UIExecutive(1, &sessionname);
    ui = new G4UIExecutive(1, &sessionname);

    #ifdef G4VIS_USE

    //UImanager->ApplyCommand("/control/execute vis.mac");
    fUIMgr->ApplyCommand("/vis/open OGL 600x600-0+0");
    fUIMgr->ApplyCommand("/vis/viewer/set/autoRefresh false");
    fUIMgr->ApplyCommand("/vis/viewer/set/background 1.0 1.0 1.0");
    fUIMgr->ApplyCommand("/vis/verbose errors");
    fUIMgr->ApplyCommand("/vis/drawVolume");
    fUIMgr->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 90. 180.");
    fUIMgr->ApplyCommand("/vis/scene/add/trajectories smooth");
    fUIMgr->ApplyCommand("/vis/modeling/trajectories/create/drawByCharge");
    fUIMgr->ApplyCommand("/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true");
    fUIMgr->ApplyCommand("/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 1");
    fUIMgr->ApplyCommand("/vis/scene/endOfEventAction accumulate");
    fUIMgr->ApplyCommand("/vis/viewer/set/autoRefresh true");
    fUIMgr->ApplyCommand("/vis/verbose warnings");
    fUIMgr->ApplyCommand("/vis/viewer/set/style surface");
    fUIMgr->ApplyCommand("/vis/scene/add/axes 0 0 0 1500 mm");
    #endif
    if (ui->IsGUI()){
      // UImanager->ApplyCommand("/control/execute g4.ini");
      fUIMgr->ApplyCommand("/control/verbose 2");
      fUIMgr->ApplyCommand("/control/saveHistory");
      fUIMgr->ApplyCommand("/run/verbose 1");
      fUIMgr->ApplyCommand("/tracking/storeTrajectory 2");
      fUIMgr->ApplyCommand("/tracking/verbose 1");
      fUIMgr->ApplyCommand("/run/initialize");
    }
    if (fMacFile!="") fUIMgr->ApplyCommand(std::string("/control/execute ") + fMacFile);


    ui->SessionStart();
    delete ui;

  #endif
  }
  return true;
}

//-------------------------------------------------------------------
bool DmpSimAlg::Finalize(){
	if (!fVisualize) {
		if (fMacFile != "") {
			fRunMgr->TerminateEventLoop();
			fRunMgr->RunTermination();
		}
	}


  delete fRunMgr;
  delete fRootEvtDataMngr;
  //delete fPhyFactory;
#ifdef G4VIS_USE
  delete fVisMgr;
#endif

  return true;
}




//-------------------------------------------------------------------
bool DmpSimAlg::ProcessThisEvent() {
	if (fVisualize) {
		//std::cout<<" --1\n";
		//ui->SessionStart();
		//ui->IsGUI();
		//std::cout<<" --2\n";
		//delete ui;
		//std::cout<<" --3\n";
		gCore->TerminateRun();
		return true;
	} else {

		DmpLogInfo << "Processing event : "
				<< gCore->GetCurrentEventID() << DmpLogEndl;

		if ("" == fMacFile) {
			return true;
		}
		//fRunMgr->ProcessOneEvent(fEventID);
		fRunMgr->ProcessThisEvent(fEventID);
		fRunMgr->TerminateOneEvent();
		if (fRunMgr->IsRunAborted()) {
			return false;
		}
		++fEventID;

		DmpLogInfo << "... done processing event" << DmpLogEndl<< DmpLogEndl <<DmpLogEndl;
		return true;
	}
}


//-------------------------------------------------------------------
void DmpSimAlg::Set(const std::string &type,const std::string &argv){
  if(type == "MacFile"){
    fMacFile = argv;
    return;
  }
  if(std::string("GeometryMainGdmlName") == type){
    gdmlGeometryMainName = std::string(argv);
    return;
  }
  if(std::string("RandomSeed") == type){
    fSeed = atol(argv.c_str());
    return;
  }
  else if(type == "STKsimulation"){
    fSTKsimulationOn = ParseBoolValue(argv);
    return;
  }
  else if(type == "BGOsimulation"){
    fBGOsimulationOn = ParseBoolValue(argv);
    return;
  }
  else if(type == "NUDsimulation"){
    fNUDsimulationOn = ParseBoolValue(argv);
    return;
  }
  else if(type == "PSDsimulation"){
    fPSDsimulationOn = ParseBoolValue(argv);
    return;
  }
  else if(type == "SATsimulation"){
    fSATsimulationOn = ParseBoolValue(argv);
    return;
  }
  else if(type == "Visualization"){
     if      (argv == std::string("True")  || argv == std::string("true")  || argv == std::string("TRUE"))   fVisualize = true;
     else if (argv == std::string("False") || argv == std::string("false") || argv == std::string("FALSE"))  fVisualize = false;
     else {
       DmpLogError<<"[DmpSimAlg::Set] Unknown value = "<< type << " for parameter = "<< argv << " ==> throwing exception!"<< DmpLogEndl;
       throw;
    }   
    return;  
  }
  DmpLogError<<"[DmpSimAlg::Set] Unknown argument type: "<< type << " ==> throwing exception!"<<DmpLogEndl;
  throw;
}




bool DmpSimAlg::ParseBoolValue(const std::string &argv){
  if(argv=="Yes" || argv=="yes" || argv=="YES" || argv=="True" || argv=="TRUE" || argv=="true" || argv=="On" || argv=="on" || argv=="ON"){
    return true;
  }
  else if (argv=="No" || argv=="no" || argv=="NO" || argv=="False" || argv=="FALSE" || argv=="false" || argv=="Off" || argv=="off" || argv=="OFF"){
    return false;
  }
  DmpLogError<<"[DmpSimAlg::ParseBoolValue] Unknown value: "<<argv<<" ==> throwing exception!"<<DmpLogEndl;
  throw;
}
