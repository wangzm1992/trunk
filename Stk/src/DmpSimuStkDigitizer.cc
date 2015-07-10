// Description: This class hinerits from G4VDigitizerModule. It is used to
// perform the digitization of the STK  
//
// Author(s):
//  - creation by X.Wu, 14/07/2013

#include "DmpSimuStkDigitizer.hh"

//#include "DmpSimuStkDigitizerMessenger.hh"
#include "DmpSimuStkDigi.hh"
#include "DmpSimuStkHit.hh"

#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4DigiManager.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

#include "G4RunManager.hh"
#include "DmpDetectorConstruction.hh"
#include "DmpStkDetectorDescription.hh"

#include <vector>

#include "DmpLog.h"
#include "DmpDebug.h"

/*
#define FLOATSTRIP_FRACTION_1   0.305
#define FLOATSTRIP_FRACTION_2   0.062
#define FLOATSTRIP_FRACTION_3   0.012
#define FLOATSTRIP_FRACTION_4   0.0024

#define READOUTSTRIP_FRACTION_0  0.935
#define READOUTSTRIP_FRACTION_1  0.023
#define READOUTSTRIP_FRACTION_2  0.0021
*/

/*
#define PAIR_CHARGE           3.2*e-4  // e-hole charge in fC
#define ADC_PER_CHARGE        14.2857  // ADC count per fC
#define SI_IONOZATION_ENERGY  3.6      // eV
#define ADC_COUNT_PER_EV      0.0012698400000000004  //  1 / SI_IONOZATION_ENERGY * PAIR_CHARGE * ADC_PER_CHARGE
*/

DmpSimuStkDigitizer::DmpSimuStkDigitizer(G4String name)
  :G4VDigitizerModule(name)

{

  G4String colName = "STKDigitsCollection";
  collectionName.push_back(colName);
  STKThreshold = 0.0*keV;

  //create a messenger for this class
  //digiMessenger = new DmpSimuStkDigitizerMessenger(this);

  G4RunManager* runManager = G4RunManager::GetRunManager();
  dmpDetector =
    (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

  G4int NbOfTKRTiles  = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRTiles();
  G4int NbOfTKRBlocks = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRBlocks();
  NbOfTKRStrips       = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips();
  NbOfTKRLayers       = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRLayers();  
  NbOfTKRChannelsL    = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips()/2;
  DmpLogInfo << "DmpSimuDigitization::constructor called " << DmpLogEndl;

  DmpLogInfo <<  "  " << NbOfTKRTiles     << " tracker ladders per block " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRBlocks    << " tracker blocks per layer" << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRLayers    << " tracker layers " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRStrips    << " tracker strips per ladder " << DmpLogEndl;
  DmpLogInfo <<  "  " << NbOfTKRChannelsL << " tracker readout channels per ladder " << DmpLogEndl;

  NbOfTKRStrips = NbOfTKRStrips*NbOfTKRTiles*NbOfTKRBlocks;  
  DmpLogInfo <<  "  Number of strips per layer : " << NbOfTKRStrips << DmpLogEndl;
  NbOfTKRChannels = NbOfTKRStrips * NbOfTKRLayers;//divided by 2 for readout, but mutiply by 2 for double layer
  DmpLogInfo <<  "  Total number of readout channels : " << NbOfTKRChannels << DmpLogEndl;

  ThitID = new G4int[NbOfTKRChannels];


  // Config parser
  fSimuConfig     = (DmpSimuConfigParser*)(gCore->ServiceManager()->Get("DmpSimuConfigParser"));
  if(!fSimuConfig){
    DmpLogError<<"No configuration manager provided! ==> throwing exception!"<<DmpLogEndl;
    throw;
  }
  else{
	  FLOATSTRIP_FRACTION_1  = fSimuConfig->GetReal("STK", "FLOATSTRIP_FRACTION_1", 0.0);
	  FLOATSTRIP_FRACTION_2  = fSimuConfig->GetReal("STK", "FLOATSTRIP_FRACTION_2", 0.0);
	  FLOATSTRIP_FRACTION_3  = fSimuConfig->GetReal("STK", "FLOATSTRIP_FRACTION_3", 0.0);
	  FLOATSTRIP_FRACTION_4  = fSimuConfig->GetReal("STK", "FLOATSTRIP_FRACTION_4", 0.0);

	  READOUTSTRIP_FRACTION_0  = fSimuConfig->GetReal("STK", "READOUTSTRIP_FRACTION_0", 0.0);
	  READOUTSTRIP_FRACTION_1  = fSimuConfig->GetReal("STK", "READOUTSTRIP_FRACTION_1", 0.0);
	  READOUTSTRIP_FRACTION_2  = fSimuConfig->GetReal("STK", "READOUTSTRIP_FRACTION_2", 0.0);
  }

}

DmpSimuStkDigitizer::~DmpSimuStkDigitizer()
{
  delete [] ThitID;
  //delete digiMessenger;
}

void DmpSimuStkDigitizer::Digitize()
{

  DigitsCollection = new DmpSimuStkDigitsCollection
  	("DmpSimupStkDigitizer","STKDigitsCollection"); // to create the PSD Digi Collection

  for (G4int i=0;i<NbOfTKRChannels;i++)
    {
      ThitID[i] = -1;
    };

  G4DigiManager* DigiMan = G4DigiManager::GetDMpointer();
  
  G4int THCID = DigiMan->GetHitsCollectionID("STKHitCollection");  
  DmpSimuStkHitsCollection* THC = 0; 
  THC = (DmpSimuStkHitsCollection*)
    (DigiMan->GetHitsCollection(THCID));
  
  if (THC) {
    G4int n_hit = THC->entries();
    
    //G4cout << " X|Lay|B|L|S|C0|C|Ein|Eo|En/Id " <<DmpLogEndl;
    for (G4int i=0;i<n_hit;i++)
      {
	G4double ESil     = (*THC)[i]->GetEdepSil();
	G4double ESilPrim = (*THC)[i]->GetEdepSilPrim();
	
	//primary trackID
	G4int trackID = (*THC)[i] ->GetPrimTrackID();
	G4int new1 = trackID/100;
	G4int new2 = trackID - new1*100;
	  
	//number of tracks
	//G4int nTtrack = (*THC)[i] ->GetNtracks();
	
	//strip number within a layer
	G4int NStrip  = (*THC)[i]->GetNStrip();
	G4int NPlane  = (*THC)[i]->GetNSilPlane();
	G4int IsX     = (*THC)[i]->GetPlaneType();
	//G4int BlockNumber = (*THC)[i]->GetNSilBlock();
	//ladder number within a block
	//G4int ladderNumber = NStrip%(dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips()*4); 
	//strip number within a ladder
	G4int stripNumber = NStrip%(dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips()); 
	
	//strips with odd numbers are floating strip
	//G4int IsFloat = stripNumber%2;

	//reaadout channel number on a ladder
	G4int channelNumber = stripNumber/2; 
	//reaadout channel number on a layer
	G4int channelNumberLayer = NStrip/2;
        G4int IsFloat = dmpDetector->GetStkDetectorDescription()->IsFloatingStrip(IsX,NStrip);


	
	//layer number : y, x, y, x, y, x, y, x, y, x, y, x
	//               0  1  2  3  4  5  6  7  8  9  10 11
	//global readout channel number: 
	  //G4int NChannel = IsX*(NbOfTKRChannels/2) + NPlane*(NbOfTKRStrips/2) + channelNumberLayer;
	G4int NChannel = (NPlane*2+IsX)*(NbOfTKRStrips/2) + channelNumberLayer;
	
	//charge sharing
        G4int ifirst = -2;
	G4int ilast  =  3;

	if(IsFloat) ifirst = -3;
        if(IsFloat) ilast  =  5;
	for (G4int j=ifirst; j<ilast; j++) {
	  //going out of bound 
	  if( (channelNumber+j)<0 || (channelNumber+j)>=NbOfTKRChannelsL) continue;
	  
	  G4double Edep      = 0.;
	  G4double EdepPrim  = 0.;
	  G4double ADCcounts = 0.;
	  if(IsFloat) {
	    if(j==0 || j==1) {
	      Edep     = FLOATSTRIP_FRACTION_1 *ESil;          //Edep =.38*ESil;
	      EdepPrim = FLOATSTRIP_FRACTION_1 *ESilPrim;      //EdepPrim =.38*ESilPrim;
	    }
	    else if(j==-1 || j==2) {
	      Edep     = FLOATSTRIP_FRACTION_2 * ESil;         //Edep =.095*ESil;
	      EdepPrim = FLOATSTRIP_FRACTION_2 * ESilPrim;     //EdepPrim =.095*ESilPrim;
	    }
	    else if(j==-2 || j==3) {
	      Edep     = FLOATSTRIP_FRACTION_3 * ESil;            //Edep =.035*ESil;
	      EdepPrim = FLOATSTRIP_FRACTION_3 * ESilPrim;        //EdepPrim =.035*ESilPrim;
	    }
	    else if(j==-3 || j==4) {
	      Edep     = FLOATSTRIP_FRACTION_4 * ESil;
	      EdepPrim = FLOATSTRIP_FRACTION_4 * ESilPrim;
	    }
	  }
	  else {
	    if(j==0) {
	      Edep     = READOUTSTRIP_FRACTION_0 * ESil;     //Edep = ESil;
	      EdepPrim = READOUTSTRIP_FRACTION_0 * ESilPrim; // EdepPrim = ESilPrim;
	    }
	    else if(j==-1 || j==1) {
	      Edep     = READOUTSTRIP_FRACTION_1 * ESil;       //Edep =.115*ESil;
	      EdepPrim = READOUTSTRIP_FRACTION_1 * ESilPrim;   //EdepPrim =.115*ESilPrim;
	    }
	    else if(j==-2 || j==2) {
	      Edep     = READOUTSTRIP_FRACTION_2 * ESil;       //Edep =.045*ESil;
	      EdepPrim = READOUTSTRIP_FRACTION_2 * ESilPrim;   //EdepPrim =.045*ESilPrim;
	    }
	  }
	  
	  //* Get ADC counts from energy deposit (asuming all energy istranslated into charge)
	  //ADCcounts = ADC_COUNT_PER_EV * Edep / eV;


	  // This is a new digi
	  if (ThitID[NChannel+j]==-1 ) {
	    DmpSimuStkDigi* Digi = new DmpSimuStkDigi();
	    Digi->SetPlaneNumber(NPlane);
	    Digi->SetPlaneType(IsX);
	    Digi->SetStripNumber(channelNumberLayer+j);
	    Digi->SetDigiType(0);
	    Digi->SetEnergy(0);
	    Digi->SetEnergyPrim(0);
	    if(new1 == 2 || new1 == 3 || new2 == 2 || new2 == 3) {
	      Digi->SetPrimTrackID(trackID);
	    }
	    //save hit id instead
	    Digi->SetHitID(i);
	    Digi->SetNtracks(    (*THC)[i]->GetNtracks()    );
	    Digi->SetNtracksBack((*THC)[i]->GetNtracksBack());
	    Digi->SetNtracksPrim((*THC)[i]->GetNtracksPrim());
	    Digi->SetNhits(1);
	    ThitID[NChannel+j] = DigitsCollection->insert(Digi) -1;	


	    //   Digit position information must be used for debugging only!
	    //   Don't use in the real analysis!
	    //-----------------------------------------
            #ifdef DAMPESW_DEBUG
	    if(j==0 && !IsFloat){
	      Digi->SetDigitX((*THC)[i]->GetPos().x());
	      Digi->SetDigitY((*THC)[i]->GetPos().y());
	      Digi->SetDigitZ((*THC)[i]->GetPos().z());
	    }
	    else{
	      Digi->SetDigitX(-999999.9);
	      Digi->SetDigitY(-999999.9);
	      Digi->SetDigitZ(-999999.9);
	    }
            #endif
	    //-----------------------------------------




	  }
	  else {
	    int oldHit1 = (*DigitsCollection)[ThitID[NChannel+j]]->GetHitID1();
	    int oldHit  = (*DigitsCollection)[ThitID[NChannel+j]]->GetHitID();
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetHitID2(oldHit1);
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetHitID1(oldHit);
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetHitID(i);

	    if(new1 == 2 || new1 == 3 || new2 == 2 || new2 == 3) {
	      G4int oldID = (*DigitsCollection)[ThitID[NChannel+j]]->GetPrimTrackID();
	      int old1 = oldID/100;
	      int old2 = oldID - old1*100;
	      if(old1 != new1 && old1 != new2 && old2 != new1 && old2 != new2) {
		(*DigitsCollection)[ThitID[NChannel+j]]->SetPrimTrackID(oldID*100 + trackID);
		//update number of primary tracks
		G4int oldP = (*DigitsCollection)[ThitID[NChannel+j]]->GetNtracksPrim();
		(*DigitsCollection)[ThitID[NChannel+j]]->SetNtracksPrim(oldP + (*THC)[i]->GetNtracksPrim());
	      }
	    }
	    //update number of hits
	    G4int oldH = (*DigitsCollection)[ThitID[NChannel+j]]->GetNhits();
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetNhits(oldH + 1);
	    
	    //update number of back scattering hits
	    G4int oldHback = (*DigitsCollection)[ThitID[NChannel+j]]->GetNhitsBack();
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetNhitsBack(oldHback + 1);
	    
	    //update number of tracks
	    G4int oldN = (*DigitsCollection)[ThitID[NChannel+j]]->GetNtracks();
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetNtracks(oldN + (*THC)[i]->GetNtracks());
	    
	    //update number of back scattering tracks
	    G4int oldNback = (*DigitsCollection)[ThitID[NChannel+j]]->GetNtracksBack();
	    (*DigitsCollection)[ThitID[NChannel+j]]->SetNtracksBack(oldNback + (*THC)[i]->GetNtracksBack());
	    
	  }

	  G4double EChannel         = (*DigitsCollection)[ThitID[NChannel+j]]->GetEnergy();
	  G4double EChannelPrim     = (*DigitsCollection)[ThitID[NChannel+j]]->GetEnergyPrim();
	  //G4double ADCcountsChannel = (*DigitsCollection)[ThitID[NChannel+j]]->GetADCcounts();
	  //G4double track    = (*DigitsCollection)[ThitID[NChannel+j]]->GetPrimTrackID();
	  
	  (*DigitsCollection)[ThitID[NChannel+j]]->SetEnergy(Edep + EChannel);
	  (*DigitsCollection)[ThitID[NChannel+j]]->SetEnergyPrim(EdepPrim + EChannelPrim);
	  //(*DigitsCollection)[ThitID[NChannel+j]]->SetADCcounts(ADCcounts + ADCcountsChannel);
	  /***
	      G4cout << " " << IsX << "|" << NPlane  << "|" << BlockNumber << "|" << ladderNumber << "|" << stripNumber 
	      << "|" << channelNumberLayer << "|" << channelNumberLayer+j << "|" <<  ESil << "|"  << EChannel 
	      << "|"  << Edep << "|"  << track << DmpLogEndl;
	  ***/
	}
	
      }
  }
/***
  if (THC) {
    G4cout << "Number of STK digits in this event =  " 
	   << DigitsCollection->entries()  
	   << DmpLogEndl;
  }
  ***/



/*

  // *
  // *  Add random noise to channels
  // *
#ifdef ADD_RANDOM_NOISE
  for (G4int i=0;i<NbOfTKRChannels;i++)
  {
    G4double noise = 0 ; // TODO: implement gaussian noise
    if(ThitID[i] == -1){

    }
    else{
      G4double ADCcountsChannel = (*DigitsCollection)[ThitID[i]]->GetADCcounts();
      (*DigitsCollection)[ThitID[i]]->SetADCcounts(noise + ADCcountsChannel);
    }
  };
#endif

*/


  StoreDigiCollection(DigitsCollection);    
  
}









