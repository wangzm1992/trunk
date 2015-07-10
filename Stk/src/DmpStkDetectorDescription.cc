// Description: This class describes the detector geometry of the DAMPE STK detector
//
// Author(s):
//  - creation by X.Wu, 11/07/2013

#include "DmpStkDetectorDescription.hh"
#include "DmpStkDetectorMessenger.hh"
#include "DmpStkSensitiveDetector.hh"
//#include "DmpStkROGeometry.hh"

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

#include "DmpLog.h"

DmpStkDetectorDescription::DmpStkDetectorDescription(G4GDMLParser* gdmlParser, TXMLEngine* engine, XMLNodePointer_t pointer)
:fParser(gdmlParser), xmlEngine(engine), xmlSTKnodepointer(pointer)
{
	/*
	 *  Set default STK parameters
	 */
	//SensitiveDetectorCenterZ = -99999.9*m;

	TKRSizeZ    = -99999.9*m;
	TKRSizeXY   = -99999.9*m;
	TKRBlockXY  = -99999.9*m;

	TKRSiliconThickness    = -99999.9*m;
	TKRSiliconTileXY       = -99999.9*m;
	TKRSiliconPitch        = -99999.9*m;

	NbOfTKRLayers  = -9999;
	NbOfTKRTiles   = -9999;
	NbOfTKRBlocks  = -9999;
	NbOfTKRStrips  = -9999;

	TKRLayerDistance      = -99999.9*m;
	TKRViewsDistance      = -99999.9*m;
	//TKRTungstenDistance   = -99999.9*m;
	TKRActiveTileXY       = -99999.9*m;
	TKRActiveTileZ        = -99999.9*m;
	SiliconGuardRing      = -99999.9*m;
	TilesSeparation       = -99999.9*m;

	//ConverterThickness      = -99999.9*m;
	//ThickConverterThickness = -99999.9*m;

	TKRCenterGap  = -99999.9*m;

	/* Get STK model parameters from GDML description */
        getStkParametersFromGDMLandXML();
	ComputeOtherParameters();

	/* Create commands for interactive definition of the payload */
	stkDetectorMessenger = new DmpStkDetectorMessenger(this);
}


DmpStkDetectorDescription::~DmpStkDetectorDescription()
{ 
  //delete stkDetectorMessenger;
}


//void DmpStkDetectorDescription::ParseStkParametersFromGDML(){
void DmpStkDetectorDescription::getStkParametersFromGDMLandXML(){

  //ConverterThickness      = __GetXMLParameter("ConverterThickness",xmlSTKnodepointer,xmlEngine)*micrometer;
  //ThickConverterThickness = __GetXMLParameter("ThickConverterThickness",xmlSTKnodepointer,xmlEngine)*micrometer;
  TKRSiliconThickness     = __GetXMLParameter("TKRSiliconThickness",xmlSTKnodepointer,xmlEngine)*micrometer;
  TKRSiliconTileXY        = __GetXMLParameter("TKRSiliconTileXY",xmlSTKnodepointer,xmlEngine)*cm;//size
  TKRSiliconPitch         = __GetXMLParameter("TKRSiliconPitch",xmlSTKnodepointer,xmlEngine)*micrometer;  //physical pitch
  SiliconGuardRing        = __GetXMLParameter("SiliconGuardRing",xmlSTKnodepointer,xmlEngine)*mm; //guard ring thickness
  NbOfTKRStrips           = __GetXMLParameter("NbOfTKRStrips",xmlSTKnodepointer,xmlEngine);
  //ladder geometry
  NbOfTKRTiles            = __GetXMLParameter("NbOfTKRTiles",xmlSTKnodepointer,xmlEngine);
  NbOfWTKRTiles           = __GetXMLParameter("NbOfWTKRTiles",xmlSTKnodepointer,xmlEngine);
  TilesSeparation         = __GetXMLParameter("TilesSeparation",xmlSTKnodepointer,xmlEngine)*micrometer;
  TKRWRecess              = __GetXMLParameter("TKRWRecess",xmlSTKnodepointer,xmlEngine)*mm;
  //tracker geometry
  TKRLayerDistance        = __GetXMLParameter("TKRLayerDistance",xmlSTKnodepointer,xmlEngine)*cm;
  TKRViewsDistance        = __GetXMLParameter("TKRViewsDistance",xmlSTKnodepointer,xmlEngine)*mm;
  TKRCenterGap            = __GetXMLParameter("TKRCenterGap",xmlSTKnodepointer,xmlEngine)*micrometer;
  NbOfTKRLayers           = __GetXMLParameter("NbOfTKRLayers",xmlSTKnodepointer,xmlEngine);
  NbOfTKRBlocks           = __GetXMLParameter("NbOfTKRBlocks",xmlSTKnodepointer,xmlEngine);
  STKnStripsPerLadder     = __GetXMLParameter("NbOfTKRStrips",xmlSTKnodepointer,xmlEngine);
  STKnLaddersPerBlock     = __GetXMLParameter("STKnLadddersBlock",xmlSTKnodepointer,xmlEngine);

  DmpLogInfo<< "[DmpStkDetectorDescription::getStkParametersFromGDMLandXML] ... finished parsing parameters" << "\n";


  // Get materials
  DmpLogInfo<< "[DmpStkDetectorDescription::getStkParametersFromGDMLandXML] parsing STK parameters from GDML... \n";
  ConverterMaterial = fParser->GetVolume("stk_tungsten")->GetDaughter(0)->GetLogicalVolume()->GetMaterial();
  DmpLogInfo<< "[DmpStkDetectorDescription::getStkParametersFromGDMLandXML] ... finished parsing materials" << "\n";

  //* Get numbers from detector geometry directly
  TKRSizeZ = ((G4Box*)(fParser->GetVolume("STK_detector_vol")->GetSolid()))->GetZHalfLength() *2;



}





void DmpStkDetectorDescription::ComputeOtherParameters()
{

  // Compute derived parameters of the payload

  //TKRSupportThickness =TKRLayerDistance -2.*TKRSiliconThickness 
  //  - TKRViewsDistance - ConverterThickness;
  TKRSupportThickness =TKRLayerDistance -2.*TKRSiliconThickness - TKRViewsDistance;
  //TKRSizeXY = NbOfTKRTiles*TKRSiliconTileXY + (NbOfTKRTiles+1)*TilesSeparation;
  //TKRBlockXY = NbOfTKRTiles*TKRSiliconTileXY + (NbOfTKRTiles+1)*TilesSeparation;
  TKRBlockXY = NbOfTKRTiles*TKRSiliconTileXY + (NbOfTKRTiles-1)*TilesSeparation;
  TKRWBlockXY = NbOfWTKRTiles*(TKRSiliconTileXY - TKRWRecess) + (NbOfWTKRTiles-1)*TilesSeparation;
  TKRSizeXY = (NbOfTKRBlocks/2)*TKRBlockXY + TKRCenterGap;


  //TKRSizeZ = NbOfTKRLayers*TKRLayerDistance;



  DmpLogInfo << "DmpStkDetectorDescription::ComputeParameters: calculated STK size " << DmpLogEndl;
  DmpLogInfo << "       Sensor size: " << TKRSiliconTileXY << DmpLogEndl;
  DmpLogInfo << "       Number of sensors: " << NbOfTKRTiles << DmpLogEndl;
  DmpLogInfo << "       Number of block: " << NbOfTKRBlocks << DmpLogEndl;
  DmpLogInfo << "       Tile separation: " << TilesSeparation << DmpLogEndl;
  DmpLogInfo << "       Center gap: " <<  TKRCenterGap << DmpLogEndl;
  //G4cout << "       TKR block size =nsen x size + (nsen+1) x sep  = " << TKRBlockXY << DmpLogEndl;
  DmpLogInfo << "       TKR block size =nsen x size + (nsen-1) x sep  = " << TKRBlockXY << DmpLogEndl;
  DmpLogInfo << "       TKR size = 2 x block + gap = " << TKRSizeXY << DmpLogEndl;
  DmpLogInfo << "       TKR W plate size =nsen x (size-recess) + (nsen-1) x sep  = " << TKRWBlockXY << DmpLogEndl;

    
  TKRActiveTileXY = TKRSiliconTileXY - 2*SiliconGuardRing;
  TKRActiveTileZ = TKRSiliconThickness;
  if(NbOfTKRStrips != TKRActiveTileXY/TKRSiliconPitch) {
    DmpLogWarning << "Warning!!! number of strips per ladder mismatch! " << NbOfTKRStrips << " vs " << TKRActiveTileXY/TKRSiliconPitch << DmpLogEndl;
  }

  NbOfTKRStrips = TKRActiveTileXY/TKRSiliconPitch;
  DmpLogInfo << "DmpStkDetectorDescription::ComputeParameters: calculated Si sensor parameters " << DmpLogEndl;
  DmpLogInfo << "       Input GuardRing width: " << SiliconGuardRing << DmpLogEndl;
  DmpLogInfo << "       Input Pitch width: " << TKRSiliconPitch << DmpLogEndl;
  DmpLogInfo << "       Active area = sensor_size - 2xGuradRing = " << TKRActiveTileXY << DmpLogEndl;
  DmpLogInfo << "       Nstrip = Active area/Pitch = " << NbOfTKRStrips << DmpLogEndl;

  //SiliconGuardRing = TKRActiveTileXY - NbOfTKRStrips*TKRSiliconPitch;
  SiliconGuardRing = 0.5*(TKRSiliconTileXY - NbOfTKRStrips*TKRSiliconPitch);
  TKRActiveTileXY = TKRSiliconTileXY - 2*SiliconGuardRing;
  DmpLogInfo << "       Recaculated GuradRing in case of integer roundup: " << SiliconGuardRing << DmpLogEndl;
  DmpLogInfo << "       Recaculated TKRActiveTileXY in case of integer roundup: " << TKRActiveTileXY << DmpLogEndl;

  TKRXStripX = TKRYStripY = TKRSiliconPitch;
  TKRYStripX = TKRXStripY = TKRActiveTileXY;
  TKRZStrip = TKRSiliconThickness;


  /* Some tests....
  G4cout<<"\n\n\n\n test \n";
  G4cout<<"GetPlaneNumber(fParser->GetVolume(STKDetectorX-23_ActiveTileX-15))"<<GetPlaneNumber(fParser->GetVolume("STKDetectorX-23_ActiveTileX-15"))<<DmpLogEndl;
  G4cout<<"GetTileNumber(fParser->GetVolume(STKDetectorX-23_ActiveTileX-15))"<<GetTileNumber(fParser->GetVolume("STKDetectorX-23_ActiveTileX-15"))<<DmpLogEndl;
  */


}


void DmpStkDetectorDescription::defineSensitiveDetector(){
    DmpLogInfo<< "[DmpStkDetectorDescription::defineSensitiveDetector] INFO: building STK sensitive strips..." <<DmpLogEndl;

    //*
    //* Sensitive Detector Manager
    //*
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    stkSensitiveDetector = new DmpStkSensitiveDetector("StkSD");
    SDman->AddNewDetector( stkSensitiveDetector );



    //*
    //* Get STK mother physical volume
    //*
    G4VPhysicalVolume* physSTK = 0;
    for(G4int i=0; i<fParser->GetWorldVolume()->GetLogicalVolume()->GetNoDaughters();i++){
      G4VPhysicalVolume* physVol = fParser->GetWorldVolume()->GetLogicalVolume()->GetDaughter(i);
      if(physVol->GetName()!= GetPhysSTKVolumeNameGDML()) continue;
      physSTK = physVol;
    }
    if(!physSTK){
      std::string error = "[DmpStkDetectorDescription::defineSensitiveDetector] ERROR: unable to find STK physical volume";
      DmpLogError<< error <<DmpLogEndl;
      throw       error;
    }
    G4VPhysicalVolume* physSiSTK = 0;
    for(G4int i=0; i<physSTK->GetLogicalVolume()->GetNoDaughters();i++){
      G4VPhysicalVolume* physVol = physSTK->GetLogicalVolume()->GetDaughter(i);
      if(physVol->GetName()!= GetPhysSTKSiMotherVolumeNameGDML()) continue;
      physSiSTK = physVol;
    }
    if(!physSiSTK){
      std::string  error = "[DmpStkDetectorDescription::defineSensitiveDetector] ERROR: unable to find Silicon Mother physical volume";
      DmpLogError << error << DmpLogEndl;
      throw        error;
    }


    /*
    G4VPhysicalVolume* frames = 0;
    for(G4int i=0; i<physSTK->GetLogicalVolume()->GetNoDaughters();i++){
      G4VPhysicalVolume* physVol = physSTK->GetLogicalVolume()->GetDaughter(i);
      if(physVol->GetName()!= "stk_corners_PV") continue;
      frames = physVol;
    }
    if(!frames){
          std::string  error = "[DmpStkDetectorDescription::defineSensitiveDetector] ERROR: unable to find frames";
          std::cout << error << DmpLogEndl;
          throw        error;
        }
    G4double Xmax_frame = ((G4TessellatedSolid*)(frames->GetLogicalVolume()->GetSolid()))->GetMaxXExtent();
          G4double Xmin_frame = ((G4TessellatedSolid*)(frames->GetLogicalVolume()->GetSolid()))->GetMinXExtent();
          G4double Ymax_frame = ((G4TessellatedSolid*)(frames->GetLogicalVolume()->GetSolid()))->GetMaxYExtent();
          G4double Ymin_frame = ((G4TessellatedSolid*)(frames->GetLogicalVolume()->GetSolid()))->GetMinYExtent();

          std::cout<< "[DmpStkDetectorDescription::defineSensitiveDetector] Xmax_frame: "<< Xmax_frame <<DmpLogEndl;
          std::cout<< "[DmpStkDetectorDescription::defineSensitiveDetector] Xmin_frame: "<< Xmin_frame <<DmpLogEndl;
          std::cout<< "[DmpStkDetectorDescription::defineSensitiveDetector] Ymax_frame: "<< Ymax_frame <<DmpLogEndl;
          std::cout<< "[DmpStkDetectorDescription::defineSensitiveDetector] Ymin_frame: "<< Ymin_frame <<DmpLogEndl;
    */



    //*
    //* Build STK strips
    //*
    G4double SiliconGuardRing = GetSiliconGuardRing();
    G4double TKRActiveTileXY  = GetTKRActiveTileXY();
    G4double TKRActiveTileZ   = GetTKRActiveTileZ();
    G4double TKRXStripX       = GetTKRSiliconPitch();
    G4double TKRYStripX       = GetTKRActiveTileXY();
    G4double TKRYStripY       = TKRXStripX;
    G4double TKRXStripY       = TKRYStripX;
    G4double TKRZStrip        = GetTKRSiliconThickness();
    G4int    NbOfTKRStrips    = GetNbOfTKRStrips();
    G4double TKRSiliconPitch  = GetTKRSiliconPitch();

    //bool siliconTileFound = false;
    for(G4int i=0; i<physSiSTK->GetLogicalVolume()->GetNoDaughters();i++){
      G4VPhysicalVolume* tile = physSiSTK->GetLogicalVolume()->GetDaughter(i);
      if( !(isVolumeSTKActiveTileX(tile->GetName()))&&
          !(isVolumeSTKActiveTileY(tile->GetName())))
      {
        std::string error = "[DmpStkDetectorDescription::defineSensitiveDetector] ERROR: tile is neither X nor Y!";
        DmpLogError<< error<< DmpLogEndl;
        throw error;
      }

      //tile->GetLogicalVolume()->SetSensitiveDetector(stkSensitiveDetector);
      G4Material* material = tile->GetLogicalVolume()->GetMaterial();

      G4double Xmax = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMaxXExtent();
      G4double Xmin = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMinXExtent();
      G4double Ymax = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMaxYExtent();
      G4double Ymin = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMinYExtent();
      G4double Zmax = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMaxZExtent();
      G4double Zmin = ((G4TessellatedSolid*)(tile->GetLogicalVolume()->GetSolid()))->GetMinZExtent();

      Xmax-=SiliconGuardRing;
      Xmin+=SiliconGuardRing;
      Ymax-=SiliconGuardRing;
      Ymin+=SiliconGuardRing;

      G4double TileXCenter =  (Xmax + Xmin ) * 0.5;
      G4double TileYCenter =  (Ymax + Ymin ) * 0.5;
      G4double TileZCenter =  (Zmax + Zmin ) * 0.5;

      if (isVolumeSTKActiveTileX(tile->GetName())){

        G4VSolid* solidTKRStripX = new G4Box("StkStripX", TKRXStripX/2,TKRYStripX/2, TKRZStrip/2);
        G4LogicalVolume* logicTKRStripX = new G4LogicalVolume(solidTKRStripX, material ,"StkStripX",0,0,0);
        for (G4int i=0;i< NbOfTKRStrips; i++){
          new G4PVPlacement(
              0,
              G4ThreeVector(TileXCenter-TKRActiveTileXY/2 +TKRSiliconPitch/2 +(i)*TKRSiliconPitch, TileYCenter, TileZCenter),
              "StkStripX",
              logicTKRStripX,
              tile,
              false,
              i);
        }
        logicTKRStripX->SetSensitiveDetector(stkSensitiveDetector);
        logicTKRStripX->SetVisAttributes(G4VisAttributes::Invisible);  // * Graphics driver is incapable of visualizing the huge amount of strips!// * Graphics driver is incapable of visualizing the huge amount of strips!

      }
      if (isVolumeSTKActiveTileY(tile->GetName())){

        G4VSolid* solidTKRStripY = new G4Box("StkStripY", TKRXStripY/2,TKRYStripY/2, TKRZStrip/2);
        G4LogicalVolume* logicTKRStripY = new G4LogicalVolume(solidTKRStripY,material,"StkStripY",0,0,0);
        for (G4int i=0;i< NbOfTKRStrips; i++){
          new G4PVPlacement(
              0,
              G4ThreeVector(TileXCenter, TileYCenter - TKRActiveTileXY/2 +TKRSiliconPitch/2 + (i)*TKRSiliconPitch, TileZCenter),  //<-- as in the old-style geometry
              "StkStripY",
              logicTKRStripY,
              tile,
              false,
              i);
        }
        logicTKRStripY->SetSensitiveDetector(stkSensitiveDetector);
        logicTKRStripY->SetVisAttributes(G4VisAttributes::Invisible);  // * Graphics driver is incapable of visualizing the huge amount of strips!// * Graphics driver is incapable of visualizing the huge amount of strips!

      }
    }

    DmpLogInfo<< "[DmpStkDetectorDescription::defineSensitiveDetector] INFO: ... done" <<DmpLogEndl;
}



// *
// *   Interface methods to read STK geometry
// *


G4bool DmpStkDetectorDescription::isVolumeSTKActiveTile(const G4String& volumename){
    return isVolumeSTKActiveTileX(volumename)||isVolumeSTKActiveTileY(volumename);
}


G4bool DmpStkDetectorDescription::isVolumeConverter(const G4String& volumename){
    if(!(volumename.contains("Converter"))) return false;
    if(volumename.contains("ThickConverter")) return false;
    return true;
}

G4bool DmpStkDetectorDescription::isVolumeThickConverter(const G4String& volumename){
    return volumename.contains("ThickConverter");
}




void DmpStkDetectorDescription::GetSensitiveDetectorParamters(G4LogicalVolume* tile, G4int& PlaneNumber, G4int& BlockNumber, G4int& LadderNumber, G4int& IsX, G4int& IsY) {
  // *
  // *  Plane = [0; 5]
  // *
  G4int plane  = __GetPlaneNumber(tile);
  G4int tilenm = __GetTileNumber(tile);
  IsX          = isVolumeSTKActiveTileX(tile->GetName());
  IsY          = isVolumeSTKActiveTileY(tile->GetName());
  PlaneNumber  = NbOfTKRLayers -1 -  plane/NbOfTKRBlocks;  //0-5
  BlockNumber  = plane%NbOfTKRBlocks;                      //0-3
  if(IsX){
    LadderNumber = tilenm / NbOfTKRTiles;
  }
  else if (IsY){
    LadderNumber = tilenm % NbOfTKRTiles;
  }
  else{
    std::string error("[DmpStkDetectorDescription::GetSensitiveDetectorParamters] ERROR: consisitency check failed - the volume is neither X nor Y detector!");
    DmpLogError<< error << DmpLogEndl;
    throw error;
  }
}

G4bool DmpStkDetectorDescription::isVolumeSTKActiveTileX(const G4String& volumename){
    if(!(volumename.contains("ActiveTileX"))) return false;
    if(!(volumename.contains("STKDetectorX"))) return false;
    return true;
}

G4bool DmpStkDetectorDescription::isVolumeSTKActiveTileY(const G4String& volumename){
    if(!(volumename.contains("ActiveTileY"))) return false;
    if(!(volumename.contains("STKDetectorY"))) return false;
    return true;
}

G4bool DmpStkDetectorDescription::isVolumeSTKStrip(const G4String& volumename){
  if (volumename.contains("StkStrip")) return true;
  //if (volumename.contains("strip")) return true;
  return false;
}





// *
// *  Private methods
// *

double DmpStkDetectorDescription::__GetXMLParameter(const char* parameter, XMLNodePointer_t node, TXMLEngine* xml)
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
    DmpLogError << "[DmpStkDetectorDescription::__GetXMLParameter] ERROR: parameter "<< parameter << " is not found" <<DmpLogEndl;
    throw;
  }
}

G4double DmpStkDetectorDescription::__StringToDouble(G4String value){
	return atof(((std::string)(value)).c_str());
}

G4int DmpStkDetectorDescription::__StringToInt(G4String value){
	return atoi(((std::string)(value)).c_str());
}

G4String DmpStkDetectorDescription::__getAuxGDMLParamter(G4GDMLAuxListType auxinfo, std::string parameterName){
    for(G4int i=0; i<auxinfo.size(); i++){
        std::string type = auxinfo.at(i).type;
        if (type == parameterName){
            return (std::string)(auxinfo.at(i).value);
        }
    }
    DmpLogError << "[DmpStkDetectorDescription::__getAuxGDMLParamter] ERROR: parameter "<< parameterName << " mot found in GDML file --> throwing Exception"<<DmpLogEndl;
    throw;
}

G4int DmpStkDetectorDescription::__GetPlaneNumber(G4LogicalVolume* tile) {
    // *
    // * This "plane number" has nothing to do with "official" STK plane definition
    // *  for internal use only!
    // *
    G4String name = tile->GetName();
    int start = name.index("STKDetector");
    start = name.index("-",start)+1;
    int stop = name.index("_",start+1);
    if(stop<0) stop = name.index("-",start+1);
    int len = -1;
    if(stop>=0) len=stop-start+1;
    return __StringToInt(G4String(name.substr(start, len)).strip());

};


G4int  DmpStkDetectorDescription::__GetTileNumber(G4LogicalVolume* tile){
    // *
    // * Tile number within the block (0 - 15)
    // *
    G4String name = tile->GetName();
    int start = name.index("ActiveTile");
    start = name.index("-",start)+1;
    int stop = name.index("_",start+1);
    if(stop<0) stop = name.index("-",start+1);
    int len = -1;
    if(stop>=0) len=stop-start+1;
    return __StringToInt(G4String(name.substr(start, len)).strip());
};


G4int  DmpStkDetectorDescription::IsFloatingStrip(G4int isX, G4int NStrip){
  // *
  // * Check if the strip is the floating strip
  // *      see: http://dpnc.unige.ch/dampe/dokuwiki/lib/exe/fetch.php?media=stk-wiki:strip-channel-numbering_modi7.pdf
  // *           http://dpnc.unige.ch/SVNDAMPE/DAMPE/Documents/SimulationSTKLaytout.pdf
  // *
  // *           isX:     1 if the strip corresponds to the layer that measures x-coordinate (ladders parallel to y), 0 otherwise
  // *           NStrip:  number of the strip within the layer [0 ; 4*4*384]
  // *



  int block = NStrip / (STKnStripsPerLadder * STKnLaddersPerBlock);
  int isY   = !isX;
  if( (isX && block ==0) ||
      (isX && block ==2) ||
      (isY && block ==0) ||
      (isY && block ==1)){
    return NStrip % 2;
  }
  else{
    return !(NStrip % 2);
  }
}












