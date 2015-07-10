// Description: This class describes the detector geometry of the DAMPE PSD detector
//
// Author(s):
//  - creation by      X.Wu, 13/07/2013
//  - implemented by Y.Zhou, 10/10/2013
//  - modified by A.Tykhonov 05/02/2014 --> helper function added:
//                isVolumeInPSD(G4String& volumename)


#include "DmpPsdDetectorDescription.hh"
//#include "DmpPsdDetectorMessenger.hh"
#include "DmpPsdSensitiveDetector.hh"
//#include "DmpPsdROGeometry.hh"

#include "G4Material.hh"
#include "G4ProductionCuts.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
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
#include "G4GDMLParser.hh"
#include "G4PhysicalVolumeStore.hh"
#include <sstream>

#include "DmpLog.h"

DmpPsdDetectorDescription::DmpPsdDetectorDescription(G4GDMLParser *gdmlParser)
    :logicPSD(0),logicStrip_A(0),logicStrip_B(0),
      psdSensitiveDetector(0),aPsdRegion(0),fParser(gdmlParser)
{
  // create commands for interactive definition of the payload
  //psdDetectorMessenger = new DmpPsdDetectorMessenger(this);

    logicPSD = fParser->GetVolume("PSDlog_envelope");
    logicStrip_A = fParser->GetVolume("PSDlog_strip_A");
    logicStrip_B = fParser->GetVolume("PSDlog_strip_B");
}

DmpPsdDetectorDescription::~DmpPsdDetectorDescription()
{ 
  //delete psdDetectorMessenger;
}
/*
void DmpPsdDetectorDescription::Construct(G4VPhysicalVolume* physi, G4LogicalVolume* logi )
{
  physiPSD = physi;
  logicPSD = logi;
  
  DefineMaterials();
  DefineStripSolid();

  // PSD detector
  // Although strip_A and strip_B are different LogicalVolume ,I use the same set of copyNums(0-81) to
  // identify their positioned PhyiscalVolume.It's not a common way,but I think it's not a big problem
  // for PSD,because PSD has only 82 strips,no ambiguity will occur.Anyway,end-user should not see the
  // difference.
  G4LogicalVolume* logicStrip_A=new G4LogicalVolume(solidStrip_A,PSDMaterial,"Strip A");
  G4LogicalVolume* logicStrip_B=new G4LogicalVolume(solidStrip_B,PSDMaterial,"Strip B");
  
  G4double detectorPosition=-PSDSizeZ/2.0+DistanceToBottom+stripThickness/2.0;
  // Layer-Y,which is at the bottom of PSD,oriented in the Y direction; copyNum:0-40,from -X to +X
  // TypeB strip is at the edge of the Layer,copyNum:0,40; other strips are TypeA,copyNum:1-39
  G4RotationMatrix rotStripY;
  G4ThreeVector trasStripY;
  G4Transform3D transformStripY;
  rotStripY.rotateY(90*deg);
  rotStripY.rotateZ(90*deg);
  trasStripY=G4ThreeVector(-40*stripGap/2.0,0,detectorPosition);
  transformStripY=G4Transform3D(rotStripY,trasStripY);
  new G4PVPlacement(transformStripY,logicStrip_B,"Y-layer Phyiscal Strip B",logicPSD,false,0,fCheckOverlaps);

  G4cout << "    Adding PSD strips ...... " << DmpLogEndl;
  G4cout << "       Placing negative side y-strip type B " << 0 << " at x = " << trasStripY.x()  << " y = " << trasStripY.y()  
	 << " z = " << trasStripY.z()  <<  DmpLogEndl;

  for(int copyNum=1;copyNum<40;copyNum++){
      if(copyNum%2){
          trasStripY=G4ThreeVector(-40*stripGap/2.0+copyNum*stripGap,0,detectorPosition+sublayerGap+stripThickness);
      }
      else{
           trasStripY=G4ThreeVector(-40*stripGap/2.0+copyNum*stripGap,0,detectorPosition);
      }
      transformStripY=G4Transform3D(rotStripY,trasStripY);
      new G4PVPlacement(transformStripY,logicStrip_A,"Y-layer Phyiscal Strip A",logicPSD,false,copyNum,fCheckOverlaps);
      G4cout << "       Placing y-strip type A " << copyNum << " at x = " << trasStripY.x()  << " y = " << trasStripY.y()  
	     << " z = " << trasStripY.z()  <<  DmpLogEndl;
  }
  rotStripY.rotateY(180*deg);
  trasStripY=G4ThreeVector(40*stripGap/2.0,0,detectorPosition);
  transformStripY=G4Transform3D(rotStripY,trasStripY);
  new G4PVPlacement(transformStripY,logicStrip_B,"Y-layer Phyiscal Strip B",logicPSD,false,40,fCheckOverlaps);
  G4cout << "       Placing positive side y-strip type B " << 40 << " at x = " << trasStripY.x()  << " y = " << trasStripY.y()  
	 << " z = " << trasStripY.z()  <<  DmpLogEndl;

  // Layer-X,oriented in the X-direction; copyNum:41-81,from +Y to -Y
  // TypeB strip is at the edge of the Layer,copyNum:41,81;other strips are TypeA,copyNum:42-80
  G4RotationMatrix rotStripX;
  G4ThreeVector trasStripX;
  G4Transform3D transformStripX;
  rotStripX.rotateY(90*deg);
  rotStripX.rotateX(180*deg);
  trasStripX=G4ThreeVector(0,-40*stripGap/2.0,detectorPosition+2*stripThickness+sublayerGap+layerGap);
  transformStripX=G4Transform3D(rotStripX,trasStripX);
  new G4PVPlacement(transformStripX,logicStrip_B,"X-layer Phyiscal Strip B",logicPSD,false,41,fCheckOverlaps);
  G4cout << "       Placing negative side x-strip type B " << 41 << " at x = " << trasStripX.x()  << " y = " << trasStripX.y()
	 << " z = " << trasStripX.z()  <<  DmpLogEndl;
  rotStripX.rotateX(-180*deg);//rotate back
  for(int copyNum=42;copyNum<81;copyNum++){
      if(copyNum%2==0){
          trasStripX=G4ThreeVector(0,-40*stripGap/2.0+(copyNum-41)*stripGap,detectorPosition+3*stripThickness+2*sublayerGap+layerGap);
      }
      else{
           trasStripX=G4ThreeVector(0,-40*stripGap/2.0+(copyNum-41)*stripGap,detectorPosition+2*stripThickness+sublayerGap+layerGap);
      }
      transformStripX=G4Transform3D(rotStripX,trasStripX);
      new G4PVPlacement(transformStripX,logicStrip_A,"X-layer Phyiscal Strip A",logicPSD,false,copyNum,fCheckOverlaps);
      G4cout << "       Placing x-strip type A " << copyNum << " at x = " << trasStripX.x()  << " y = " << trasStripX.y()  
	     << " z = " << trasStripX.z()  <<  DmpLogEndl;
  }
  //rotStripX.rotateX(180*deg);
  trasStripX=G4ThreeVector(0,40*stripGap/2.0,detectorPosition+2*stripThickness+sublayerGap+layerGap);
  transformStripX=G4Transform3D(rotStripX,trasStripX);
  new G4PVPlacement(transformStripX,logicStrip_B,"X-layer Phyiscal Strip B",logicPSD,false,81,fCheckOverlaps);
  G4cout << "       Placing positive side x-strip type B " << 81 << " at x = " << trasStripX.x()  << " y = " << trasStripX.y()
	 << " z = " << trasStripX.z()  <<  DmpLogEndl;


  // Cuts by Regions, use defult for PSD 
  G4String regName[] = {"PSD"};
  if (aPsdRegion) delete aPsdRegion;

  aPsdRegion = new G4Region(regName[0]);
  logicPSD->SetRegion(aPsdRegion);
  aPsdRegion->AddRootLogicalVolume(logicPSD);
  
  //Sensitive Detector Manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  if(!psdSensitiveDetector) {
    psdSensitiveDetector = new DmpPsdSensitiveDetector("PsdSD");
    SDman->AddNewDetector( psdSensitiveDetector );		
   }
  if(logicStrip_A)
    logicStrip_A->SetSensitiveDetector(psdSensitiveDetector);
  if(logicStrip_B)
    logicStrip_B->SetSensitiveDetector(psdSensitiveDetector);

  // Visualization attributes
  //

  logicPSD->SetVisAttributes(G4VisAttributes::Invisible);
  // Some visualization styles
  G4VisAttributes* VisAtt3= new G4VisAttributes(G4Colour(0.8,0.2,0.3));
  VisAtt3->SetVisibility(true);
  VisAtt3->SetForceWireframe(TRUE);
  
  // Visible Volumes
  logicStrip_A->SetVisAttributes(VisAtt3);
  logicStrip_B->SetVisAttributes(VisAtt3);

  G4cout << "DmpPsdDetectorDescription construction completed !!! " << DmpLogEndl;
  return;
}
*/
void DmpPsdDetectorDescription::ComputeParameters()
{
    ParsePsdParametersFromGDML();

    G4Box* envelope_box=(G4Box*)logicPSD->GetSolid();
    PSDSizeXY = 2*envelope_box->GetXHalfLength();
    PSDSizeZ = 2*envelope_box->GetZHalfLength();
    PSDCenter = PSDMainBodyZ/2;
    G4int effective_length=(G4int)((stripWidthB-stripWidthA/2)*2+(NbOfLayerStrips-1)*stripGap);

    DmpLogInfo << "DmpPsdDetectorDescription::ComputeParameters: calculated PSD parameters " << DmpLogEndl;
    DmpLogInfo << "       PSD size XY/Z: " << PSDSizeXY << "/" << PSDSizeZ << DmpLogEndl;
    DmpLogInfo << "       Number of layers: " << NbOfLayers << DmpLogEndl;
    DmpLogInfo << "       Number of strips in each layer: " << NbOfLayerStrips << DmpLogEndl;
    DmpLogInfo << "       Layer gap: " << layerGap/mm <<"mm" << DmpLogEndl;
    DmpLogInfo << "       Strip gap: " << stripGap/mm << "mm" << DmpLogEndl;
    DmpLogInfo << "   Each X/Y layer consists 2 sub-layers: " << DmpLogEndl;
    DmpLogInfo << "       Number of strips in bottom sub-layer: " << NbOfBottomSublayerStrips << DmpLogEndl;
    DmpLogInfo << "       Number of strips in up sub-layer: " << NbOfUpSublayerStrips << DmpLogEndl;
    DmpLogInfo << "       Sub-layer gap: " << sublayerGap/mm << "mm" << DmpLogEndl;
    DmpLogInfo << "   Strip dimenstion(Thickness/Width/Body-Length): " << DmpLogEndl;
    DmpLogInfo << "       Type A: " << stripThickness/mm <<"mm/" << stripWidthA/mm << "mm/" << stripBodyLength/mm << "mm" << DmpLogEndl;
    DmpLogInfo << "       Type B: " << stripThickness/mm <<"mm/" << stripWidthB/mm << "mm/" << stripBodyLength/mm << "mm" << DmpLogEndl;
    DmpLogInfo << "   Effective detection area: " << DmpLogEndl;
    DmpLogInfo << "       "<<effective_length<<"mm*"<<effective_length<<"mm"<<DmpLogEndl;

}

void DmpPsdDetectorDescription::PrintDimensions()
{
    std::cout << "DmpPsdDetectorDescription::PrintDimensions: PSD strip parameters " << DmpLogEndl;
    //G4cout << "PSD detector consists of " << NbOfLayers << " Layers: X and Y" << DmpLogEndl;
}

void DmpPsdDetectorDescription::ParsePsdParametersFromGDML()
{
    DmpLogInfo<< "[DmpStkDetectorDescription::ParseStkParametersFromGDML] parsing STK parameters from GDML... \n";

    G4GDMLAuxListType auxinfo = fParser->GetVolumeAuxiliaryInformation(logicPSD);
    G4String auxtype;
    for(G4GDMLAuxListType::iterator iter=auxinfo.begin();iter != auxinfo.end();iter++){
        auxtype=iter->type;
        if     (auxtype == "PSDaux_NbOfLayers")                 NbOfLayers = atoi((iter->value).c_str());
        else if(auxtype == "PSDaux_NbOfLayerStrips")            NbOfLayerStrips = atoi((iter->value).c_str());
        else if(auxtype == "PSDaux_NbOfUpSublayerStrips")       NbOfUpSublayerStrips = atoi((iter->value).c_str());
        else if(auxtype == "PSDaux_NbOfBottomSublayerStrips")   NbOfBottomSublayerStrips = atoi((iter->value).c_str());
        else if(auxtype == "PSDaux_NbOfPSDStrips")              NbOfPSDStrips = atoi((iter->value).c_str());
        else if(auxtype == "PSDaux_stripGap")                   stripGap = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_layerGap")                   layerGap = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_sublayerGap")                sublayerGap = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_stripBodyLength")            stripBodyLength = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_stripHeadLength")            stripHeadLength = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_stripThickness")             stripThickness = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_stripWidthA")                stripWidthA = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_stripWidthB")                stripWidthB = atof((iter->value).c_str())*mm;
        else if(auxtype == "PSDaux_mainBodyHeight")             PSDMainBodyZ = atof((iter->value).c_str())*mm;
    }
}

void DmpPsdDetectorDescription::SetCopyNumber()
{
    DmpLogDebug<< "PSD: Assign copyNum to sensitive strips"<<DmpLogEndl;

    G4PhysicalVolumeStore* PV_Store=G4PhysicalVolumeStore::GetInstance();
    G4VPhysicalVolume* physvolPtr=0;
    std::ostringstream volname;

    //First,Set Layer_Y(1-->41)
    for(G4int copy_id=1;copy_id<42;copy_id++){
        volname.str("");
        volname.clear();
        volname << "PSDphy_strip_Y" << copy_id;
        physvolPtr = PV_Store->GetVolume(volname.str(),false);
        physvolPtr->SetCopyNo(copy_id);
        DmpLogDebug<< "\t"<<volname.str()<<" : "<<physvolPtr->GetCopyNo()<<DmpLogEndl;
    }
    //Then, Set Layer_X(42-->82)
    for(G4int copy_id=1;copy_id<42;copy_id++){
        volname.str("");
        volname.clear();
        volname << "PSDphy_strip_X" << copy_id;
        physvolPtr = PV_Store->GetVolume(volname.str(),false);
        physvolPtr->SetCopyNo(copy_id+41);
        DmpLogDebug<< "\t"<<volname.str()<<" : "<<physvolPtr->GetCopyNo()<<DmpLogEndl;
    }
}

void DmpPsdDetectorDescription::defineSensitiveDetector()
{
    SetCopyNumber();

    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    if(!psdSensitiveDetector) {
      psdSensitiveDetector = new DmpPsdSensitiveDetector("PsdSD");
      SDman->AddNewDetector( psdSensitiveDetector );
     }
    if(logicStrip_A)
      logicStrip_A->SetSensitiveDetector(psdSensitiveDetector);
    if(logicStrip_B)
      logicStrip_B->SetSensitiveDetector(psdSensitiveDetector);

}

G4bool DmpPsdDetectorDescription::isVolumeInPSD(G4String& volumename){
    return volumename.contains("PSD");
}











