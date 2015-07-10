// Description: This class hinerits from G4VUserDetectorConstruction. It is used to
// manage the detector geometry description of the DAMPE detector
//
// Author(s):
//  - creation by X.Wu, 11/07/2013

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#define PATHNAME_BUFFER_SIZE 100
#endif


#include "DmpDetectorConstruction.hh"
#include "DmpDetectorMessenger.hh"
//#include "DmpGeometryManager.h"
/*
#include "DmpPsdDetectorDescription.hh"
#include "DmpStkDetectorDescription.hh"
#include "DmpBgoDetectorDescription.hh"
#include "DmpNudDetectorDescription.hh"
*/
#include "G4Material.hh"
#include "G4ProductionCuts.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4TransportationManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"


#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4ios.hh"
#include "G4RegionStore.hh"

#include <stdlib.h>
#include <sstream>

#include "TString.h"

#include "DmpLog.h"
#include "DmpCore.h"

#include "G4ChordFinder.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4ClassicalRK4.hh"

#define PSD_POSITION_GDML_NAME  "psd_z_offset"
#define STK_POSITION_GDML_NAME  "stk_z_offset"
#define BGO_POSITION_GDML_NAME  "bgo_z_offset"
#define NUD_POSITION_GDML_NAME  "nud_z_offset"
#define WORLD_SIZE_Z_GDML_NAME  "world_size"
#define WORLD_SIZE_XY_GDML_NAME "world_size"
#define WORLD_NAME_GDML         "World"

#define GEOMETRY_PATH_ENVVARIABLE  "DMPSWGEOMETRY"

DmpDetectorConstruction::DmpDetectorConstruction()
  :solidWorld(0),logicWorld(0),physiWorld(0),
   //solidHalfPosWorld(0),logicHalfPosWorld(0),physiHalfPosWorld(0),
   //solidHalfNegWorld(0),logicHalfNegWorld(0),physiHalfNegWorld(0),
   solidPayload(0),logicPayload(0),physiPayload(0),
   solidPSD(0),logicPSD(0),physiPSD(0),
   solidSTK(0),logicSTK(0),physiSTK(0),
   solidBGO(0),logicBGO(0),physiBGO(0),
   solidNUD(0),logicNUD(0),physiNUD(0)
{

  // create commands for interactive definition of the payload
  detectorMessenger = new DmpDetectorMessenger(this);

  // Config parser
  fSimuConfig     = (DmpSimuConfigParser*)(gCore->ServiceManager()->Get("DmpSimuConfigParser"));
  if(!fSimuConfig){
    DmpLogWarning<<"####################################################"<<DmpLogEndl;
    DmpLogWarning<<"#                                                  #"<<DmpLogEndl;
    DmpLogWarning<<"#  No configuration manager provided!              #"<<DmpLogEndl;
    DmpLogWarning<<"#   ==> Running with the default parameters...     #"<<DmpLogEndl;
    DmpLogWarning<<"#                                                  #"<<DmpLogEndl;
    DmpLogWarning<<"####################################################"<<DmpLogEndl;
  }
  else{
    fMagFields    = fSimuConfig->GetStringArray("DAMPE", "UniformMagneticFields");
    fGdmlFileName = fSimuConfig->Get("DAMPE", "GeoemetryMainGDML", "");
    fGdmlSubPath  = fSimuConfig->Get("DAMPE", "GeoemetryMainGDMLPath", "");
    //fGdmlFileName = "Geometry/" + fGdmlFileName;
  }

  fGdmlSubPath = std::string("Geometry/") +  fGdmlSubPath;

  DmpLogInfo<<"Main geometry GDML file: "<< fGdmlFileName<< DmpLogEndl;
  DmpLogInfo<<"Path to main  GDML file: "<< fGdmlSubPath<< DmpLogEndl;
  for(int i=0; i<fMagFields.size(); i++){
    DmpLogInfo<<"Uniform magnetic fields["<< i <<"] = "<< fMagFields[i] <<"  ==> volumes and B values in Tesla are show;"<< DmpLogEndl;
  }



}

DmpDetectorConstruction::~DmpDetectorConstruction()
{ 
  delete detectorMessenger;
}

G4VPhysicalVolume* DmpDetectorConstruction::Construct()
{
  /*
   * Construct detector from GDML description
   */  
  G4VPhysicalVolume* world = ConstructGDML();
  if( not ConstructMagneticFields()) return 0;
  return world;
}


bool DmpDetectorConstruction::ConstructMagneticFields(){
  for(int i=0; i<fMagFields.size(); i++){

    /*
    DmpLogError<<"\n\n\n\n\n";
    DmpLogError<<"fMagFields[i] = "<<fMagFields[i]<<DmpLogEndl;
    */

    //* Parse magnetic field
    std::string mfield = fMagFields[i];
    int pos1 = mfield.find("(");
    int pos2 = mfield.find(")");
    if(pos1<0 || pos2 <0){
      DmpLogError<<"Config parser error! ==> throwing exception!"<< DmpLogEndl;
      throw;
    }

    std::string volumename = mfield.substr(0,pos1);
    std::string bvector    = mfield.substr(pos1+1,pos2-pos1-1);
    std::vector<std::string> bvalues;
    StringExplode(bvector,";",&bvalues,true);
    if(bvalues.size()!=3){
      DmpLogError<<"Config parser error! ==> throwing exception!"<< DmpLogEndl;
      throw;
    }



    //* Assign magnetic field to the volume
    DmpLogInfo<<"\n\n\n"<<DmpLogEndl;
    DmpLogInfo<<"+-----------------------------------------------------------------------------\\"<<DmpLogEndl;
    DmpLogInfo<<"|   Assigning magnetic field to the volume: "<< volumename<<DmpLogEndl;
    DmpLogInfo<<"|   B = ("<<bvalues[0]<<" , "<<bvalues[1]<<" , "<<bvalues[2]<<") Tesla"<<DmpLogEndl;


    //* Find the volume
    G4LogicalVolume* bfieldvolume;
    std::vector<std::string> subvolumes;
    StringExplode(volumename,"::",&subvolumes, true);
    G4LogicalVolume* mother =  fParser->GetWorldVolume()->GetLogicalVolume();
    std::vector<std::string>::iterator subvolume;
    for(subvolume = subvolumes.begin(); subvolume!=subvolumes.end(); ++subvolume){
      int max = mother->GetNoDaughters();
      G4VPhysicalVolume* daugter;
      int sub_i;
      //DmpLogInfo<<"\n\n"<<DmpLogEndl;
      for(sub_i=0;sub_i<max; sub_i++){
        daugter = mother->GetDaughter(sub_i);
        //DmpLogInfo<<"   daugter->GetName() = "<<daugter->GetName()<<DmpLogEndl;
        if(daugter->GetName()== G4String(*subvolume)) break;
        //DmpLogInfo<<"   done"<<DmpLogEndl;
      }
      if(sub_i == max){
        DmpLogError<<"Volume: "<< *subvolume<<"   not found in the mother volume: "<<mother->GetName()<< " ==>throwing Exception!"<<DmpLogEndl;
        throw;
      }

      bfieldvolume = daugter->GetLogicalVolume();
      mother       = daugter->GetLogicalVolume();
    }

    //*
    //* Once the volume is found, assign magnetic field
    //*  for details see: http://geant4.lngs.infn.it/courseBelfast2013/exercises/task1/task1c/index.html
    //*
    G4UniformMagField* magneticField = new G4UniformMagField(G4ThreeVector(atof(bvalues[0].c_str()) * tesla, atof(bvalues[1].c_str()) * tesla, atof(bvalues[2].c_str())*tesla  ));
    G4Mag_UsualEqRhs* fEquationMagneticField = new G4Mag_UsualEqRhs(magneticField);
    G4double minStepMagneticField     = 0.0025 *mm ;
    G4FieldManager* fieldManagerMagneticField = new G4FieldManager(magneticField);
    G4ClassicalRK4* stepperMagneticField = new G4ClassicalRK4( fEquationMagneticField );
    fieldManagerMagneticField -> SetDetectorField(magneticField);
    G4ChordFinder* fChordFinder = new G4ChordFinder( magneticField, minStepMagneticField, stepperMagneticField);

    //*
    bfieldvolume->SetFieldManager( fieldManagerMagneticField, true ) ;


    DmpLogInfo<<"|   done"<<DmpLogEndl;
    DmpLogInfo<<"+-----------------------------------------------------------------------------/\n\n"<<DmpLogEndl;







  }
  return true;
}



G4VPhysicalVolume* DmpDetectorConstruction::ConstructGDML(){
  //* get pointer to the simulation algorithm
  fDmpSimAlg = (DmpSimAlg*)gCore->AlgorithmManager()->Get("Sim/SimAlg"); 

  /* Get current working directory */
  char * curpath = NULL;
  //path = getcwd(NULL, 0);


// ----------------------------------------------------------------------------
// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__   /* 
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)curpath, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL); */
   char path[PATHNAME_BUFFER_SIZE];
   getwd(path);
   curpath = path;   
// ----------------------------------------------------------------------------
#else
  curpath = get_current_dir_name();
#endif  

  if(curpath==NULL){
    std::string error("[DmpDetectorConstruction::ConstructGDML] ERROR: problem obtaing current working directory");
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }


  /*   Switch to top working directory, where Geometry folder resides 	 */
  char* geometryPath = NULL;
  geometryPath = getenv (GEOMETRY_PATH_ENVVARIABLE);
  if (geometryPath==NULL){
    std::ostringstream errorstr;
    errorstr<<"[DmpDetectorConstruction::ConstructGDML] ERROR: problem geometry path, "<<GEOMETRY_PATH_ENVVARIABLE<<" is not defined!";
    std::string error(errorstr.str());
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }
  chdir(geometryPath);
  //chdir("../share");//from ./Simulation/DmpSim/test TO ./


  /*   Construct geometry 	 */
  fParser = new G4GDMLParser();
  //fParser->Read("./Geometry/DAMPE.gdml");
  //fParser->Read("./Geometry/DAMPE_BT_OCTOBER.gdml");
  //std::string gdmlName = DmpGeometryManager::GetGeometryMainGdmlName();
  //fParser->Read(gdmlName.c_str());
  std::string gdmlfilename = fDmpSimAlg->GetMainGdmlName();
  if(gdmlfilename==std::string("")) gdmlfilename = fGdmlFileName;
  fParser->Read((fGdmlSubPath + gdmlfilename).c_str());




  // * Reading xml geometry parameter files and construct subdetectors
  xmlEngine = new TXMLEngine;
  XMLDocPointer_t xmldocDAMPEparameter  = xmlEngine->ParseFile(TString(geometryPath).Append("/Geometry/Parameters/DAMPE.xml"));
  nodeDAMPEparameter  = xmlEngine->DocGetRootElement(xmldocDAMPEparameter);
  if(fDmpSimAlg->StkSimulationEnabled()){
    XMLDocPointer_t xmldocSTKparameter  = xmlEngine->ParseFile(TString(geometryPath).Append("/Geometry/Parameters/STK.xml"));
    XMLNodePointer_t nodeSTKparameter  = xmlEngine->DocGetRootElement(xmldocSTKparameter);
    dmpStkDetectorDescription = new DmpStkDetectorDescription(fParser,xmlEngine,nodeSTKparameter);
  }
  if(fDmpSimAlg->BgoSimulationEnabled()){
    XMLDocPointer_t xmldocBGOparameter  = xmlEngine->ParseFile(TString(geometryPath).Append("/Geometry/Parameters/BGO.xml"));
    XMLNodePointer_t nodeBGOparameter  = xmlEngine->DocGetRootElement(xmldocBGOparameter);
    dmpBgoDetectorDescription = new DmpBgoDetectorDescription(fParser,xmlEngine,nodeBGOparameter);
  }
  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpPsdDetectorDescription = new DmpPsdDetectorDescription(fParser);
  }
  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpNudDetectorDescription = new DmpNudDetectorDescription(fParser);
  }

  /*   Compute payload parameters */
  ComputePayloadParameters();


  /*   Define sensitive volumes  */
  if(fDmpSimAlg->StkSimulationEnabled()) dmpStkDetectorDescription->defineSensitiveDetector();
  if(fDmpSimAlg->BgoSimulationEnabled()) dmpBgoDetectorDescription->defineSensitiveDetector();
  if(fDmpSimAlg->PsdSimulationEnabled()) dmpPsdDetectorDescription->defineSensitiveDetector();
  if(fDmpSimAlg->NudSimulationEnabled()) dmpNudDetectorDescription->defineSensitiveDetector();


  /*   Switch back to current working directory	 */
 // chdir("../run");
 chdir(curpath);

  /*   Return world volume     */
  physiWorld = fParser->GetWorldVolume();
  return physiWorld;
}


void DmpDetectorConstruction::UpdateGeometry()
{
  //  delete payloadSD;
  G4RunManager::GetRunManager()->DefineWorldVolume(ConstructGDML());
  G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  G4RegionStore::GetInstance()->UpdateMaterialList(physiWorld);

}

void DmpDetectorConstruction::ComputePayloadParameters()
{
  // get the parameters of PSD, STK, BGO, NUD
  if(fDmpSimAlg->PsdSimulationEnabled()){
    dmpPsdDetectorDescription->ComputeParameters();
    PSDSizeZ  = dmpPsdDetectorDescription->GetSizeZ();
    PSDSizeXY = dmpPsdDetectorDescription->GetSizeXY();
    PSDCenter = __GetXMLParameter(PSD_POSITION_GDML_NAME,nodeDAMPEparameter,xmlEngine)*mm;  //fParser->GetConstant(PSD_POSITION_GDML_NAME)*mm;
  }

  //dmpStkDetectorDescription->ComputeParameters();
  if(fDmpSimAlg->BgoSimulationEnabled()){
    dmpBgoDetectorDescription->ComputeParameters();
    BGOSizeZ  = dmpBgoDetectorDescription->GetSizeZ();
    BGOSizeXY = dmpBgoDetectorDescription->GetSizeXY();
    BGOCenter = __GetXMLParameter(BGO_POSITION_GDML_NAME,nodeDAMPEparameter,xmlEngine)*mm;  //fParser->GetConstant(BGO_POSITION_GDML_NAME)*mm;
  }

  if(fDmpSimAlg->NudSimulationEnabled()){
    dmpNudDetectorDescription->ComputeParameters();
    NUDSizeZ  = dmpNudDetectorDescription->GetSizeZ();
    NUDSizeXY = dmpNudDetectorDescription->GetSizeXY();
    NUDCenter = __GetXMLParameter(NUD_POSITION_GDML_NAME,nodeDAMPEparameter,xmlEngine)*mm;  //fParser->GetConstant(NUD_POSITION_GDML_NAME)*mm;
  }

  if(fDmpSimAlg->StkSimulationEnabled()){
    STKSizeZ  = dmpStkDetectorDescription->GetSizeZ();
    STKSizeXY = dmpStkDetectorDescription->GetSizeXY();
    STKCenter = __GetXMLParameter(STK_POSITION_GDML_NAME,nodeDAMPEparameter,xmlEngine)*mm;  //fParser->GetConstant(STK_POSITION_GDML_NAME)*mm;
  }


  if(fDmpSimAlg->SatSimulationEnabled()){
    // compute the global definitions: World, Payload, PSD, STK, BGO, NUD
    //PayloadSizeZ = 1.1*(STKSizeZ + BGOSizeZ + PSDSTKDistance + BGOSTKDistance + PSDSizeZ);
    //PayloadSizeZ = 1.1 * ((fParser->GetConstant(BGO_POSITION_GDML_NAME)*mm + BGOSizeZ/2) - (fParser->GetConstant(PSD_POSITION_GDML_NAME)*mm -PSDSizeZ/2));
    PayloadSizeZ = 1.1 * ((NUDCenter+NUDSizeZ/2)-(PSDCenter-PSDSizeZ/2));
    PayloadSizeXY = (PSDSizeXY);
    // World
    WorldSizeZ  = __GetXMLParameter(WORLD_SIZE_Z_GDML_NAME,  nodeDAMPEparameter,xmlEngine)*mm; //fParser->GetConstant(WORLD_SIZE_Z_GDML_NAME) *mm;
    WorldSizeXY = __GetXMLParameter(WORLD_SIZE_XY_GDML_NAME, nodeDAMPEparameter,xmlEngine)*mm; //fParser->GetConstant(WORLD_SIZE_XY_GDML_NAME)*mm;
  }

  DmpLogDebug << "DmpDetectorConstruction: calculated Payload size " << G4endl;
  DmpLogDebug << "       Payload size XY = PSDSizeXY: "  << PayloadSizeXY << G4endl;
  DmpLogDebug << "       PayloadSizeZ = "<<PayloadSizeZ<<"\n";
  DmpLogDebug << "       BGOSizeZ = "<<BGOSizeZ<<"\n";
  DmpLogDebug << "       BGO size XY/Z: " << BGOSizeXY << "/" << BGOSizeZ << " placed at z= "<< BGOCenter << G4endl;
  DmpLogDebug << "       BGOCenter = "<<BGOCenter<<"\n";
  DmpLogDebug << "       PSDSizeZ = "<<PSDSizeZ<<"\n";
  DmpLogDebug << "       PSD size XY/Z: " << PSDSizeXY << "/" << PSDSizeZ << " placed at z= "<< PSDCenter << G4endl;
  DmpLogDebug << "       PSDCenter = "<<PSDCenter<<"\n";
  DmpLogDebug << "       STKSizeZ = "<<STKSizeZ<<"\n";
  DmpLogDebug << "       STK size XY: " << STKSizeXY<< G4endl;
  DmpLogDebug << "       STKCenter = "<<STKCenter<<"\n";
  DmpLogDebug << "       NUDSizeZ = "<<NUDSizeZ<<"\n";
  DmpLogDebug << "       NUD size XY: " << NUDSizeXY<< G4endl;
  DmpLogDebug << "       NUDCenter = "<<NUDCenter<<"\n";
  DmpLogDebug << "       world size XY/Z: " << WorldSizeXY << "/" << WorldSizeZ << G4endl;

}

G4bool DmpDetectorConstruction::isWorldVolume(const G4String& volumename){
    return volumename.contains(WORLD_NAME_GDML);
}

G4int DmpDetectorConstruction::getDetectorIndex(G4String& volumename){
    if(isWorldVolume(volumename)) return World;
    if(dmpBgoDetectorDescription->isVolumeBgoBar(volumename))         return BgoBar;
    if(dmpStkDetectorDescription->isVolumeConverter(volumename))      return STKConverter;
    if(dmpStkDetectorDescription->isVolumeThickConverter(volumename)) return SKThickConverter;
    if(dmpStkDetectorDescription->isVolumeSTKActiveTile(volumename))  return STKActiveTile;
    if(dmpStkDetectorDescription->isVolumeSTKStrip(volumename))       return STKStrip;
    return Unknown;
}

double DmpDetectorConstruction::__GetXMLParameter(const char* parameter, XMLNodePointer_t node, TXMLEngine* xml)
{
  // *
  // * Coded by Yifeng
  // *
  XMLAttrPointer_t attr;
  XMLAttrPointer_t NextAttr;
  double result = -1;

  XMLNodePointer_t child = xml->GetChild(node);
  while(child!=0){
    attr = xml->GetFirstAttr(child);
    while(attr!=0){
      //char* value = xml->GetAttrValue(attr);
      if(strcmp(xml->GetAttrValue(attr),parameter)==0){
        NextAttr = xml->GetNextAttr(attr);
        result = atof(xml->GetAttrValue(NextAttr));
        return result;
      }
      attr = xml->GetNextAttr(attr);
    }
    child = xml->GetNext(child);
  }
  if (result==-1){
    DmpLogError << "[DmpDetectorConstruction::__GetXMLParameter] ERROR: parameter "<< parameter << " is not found" <<DmpLogEndl;
    throw;
  }
}

















