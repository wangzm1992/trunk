//
// Author(s):
//  - creation by Andrii Tykhonov, 27/01/2014

#include "DmpSimuStkDigi.hh"
#include "DmpSimuStkDigitizer.hh"
#include "DmpSimuStkDigiEventMaker.hh"
#include "DmpDetectorConstruction.hh"
#include "DmpStkDetectorDescription.hh"
#include "DmpStkLadderAdc.hh"


#include "G4SDManager.hh"
#include "G4DigiManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

#include "DmpLog.h"
#include "DmpCore.h"
#include "DmpIOSvc.h"


#define PAIR_CHARGE           0.00016  // e-hole charge in fC
//#define ADC_PER_CHARGE        (1/0.07) //14.2857  // ADC count per fC
#define SI_IONOZATION_ENERGY  3.6      // eV
#define eV                    0.000001

#define ADC_NOISE_PARAMETER  "AddAdcNoise"


DmpSimuStkDigiEventMaker::DmpSimuStkDigiEventMaker()
:debugEvent(-100) //-100 no event
{
  //set debugEvent to a particular event number for debug printout  
  //debugEvent = -1; //-1 is all events
  //debugEvent = -100; //-100 no event
}

DmpSimuStkDigiEventMaker::~DmpSimuStkDigiEventMaker()
{
	#ifdef DAMPESW_DEBUG
	hito_test_x->Write();
	hito_test_y->Write();
	hito_test_z->Write();
	#endif
}

void DmpSimuStkDigiEventMaker::BeginRun(const G4Run* aRun)
{ 
 
  /*
   *   Book an STK Digit branch
   */
  /*
  tree = aTree;
  run  = aRun;
  StkDigitEvt = new DmpEvtStkDigit();
  tree->Branch("DmpEvtStkDigit",  "DmpEvtStkDigit",  &StkDigitEvt);
  */


  /*
   *   Get number of readout channels
   */
  G4RunManager* runManager = G4RunManager::GetRunManager();
  DmpDetectorConstruction* dmpDetector =
    (DmpDetectorConstruction*)(runManager->GetUserDetectorConstruction());

  G4int NStripsPerLadder = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRStrips();
  NLadderPerBlock  = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRTiles();
  G4int NBlockPerLayer   = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRBlocks();
  G4int NDoubleLayers    = dmpDetector->GetStkDetectorDescription()->GetNbOfTKRLayers();  


  G4int NStripsPerLayer = NStripsPerLadder*NLadderPerBlock*NBlockPerLayer;  
  NbOfROChannels = NStripsPerLayer * NDoubleLayers ;//divided by 2 for readout, but mutiply by 2 for double layer
  DmpLogDebug << "[DmpSimuStkDigiEventMaker::BeginRun] called " << DmpLogEndl;
  DmpLogDebug << "[DmpSimuStkDigiEventMaker::BeginRun]  total number of readout channels : " << NbOfROChannels << DmpLogEndl;

  //*
  //* Config parser
  //*
  fSimuConfig     = (DmpSimuConfigParser*)(gCore->ServiceManager()->Get("DmpSimuConfigParser"));
  if(!fSimuConfig){
    DmpLogError<<"No configuration manager provided! ==> throwing exception!"<<DmpLogEndl;
    throw;
  }

  double CHARGE_PER_ADC = fSimuConfig->GetReal("STK", "CHARGE_PER_ADC", 0.0);
  ADC_PER_CHARGE = 1.0/ CHARGE_PER_ADC;

  fAddStripNoise = false;
  if(fSimuConfig->Find("STK", ADC_NOISE_PARAMETER))
	  fAddStripNoise = fSimuConfig->GetBoolean("STK", ADC_NOISE_PARAMETER, false);
  else
	  DmpLogWarning<<"Following parameter not found in config file: "<<ADC_NOISE_PARAMETER<< DmpLogEndl;
  DmpLogInfo<<"fAddStripNoise = "<<fAddStripNoise<<DmpLogEndl;


  //* Read geometry constants
  fGeometryManager = (DmpGeometryManager*)(gCore->ServiceManager()->Get("DmpGeometryManager"));
  if(!fGeometryManager){
    DmpLogError<<" Geometry manager is not provided!"<<DmpLogEndl;
    DmpLogError<<"    ==> Please consider adding geometry manager (service) to the DAMPE core in your job option:"<<DmpLogEndl;
    DmpLogError<<"        DMPSW.SvcMgr.Append(DMPSW.GeoMgr)"<<DmpLogEndl;
    DmpLogError<<"    ==> Throwing exception!"<<DmpLogEndl;
    throw;
  }
  fNROchannels    = fGeometryManager->getSTKnROstripsLadder();
  fNPlanes        = fGeometryManager->getSTKnplanes();
  fNLaddersTrb    = fGeometryManager->getSTKnLaddersTRB();
  fNTrb           = fGeometryManager->getSTKnTRB();


  //* Initialize common noise
  ladderswithsignal  = new int[fNTrb*fNLaddersTrb];





}

void DmpSimuStkDigiEventMaker::BeginEvent(const G4Event* evt)
{
    //StkDigitEvt->Reset();
    //StkDigitEvt->eventNumber = evt->GetEventID();
    //StkDigitEvt->runNumber = run->GetRunID();
    //StkDigitEvt->NbOfROChannels = NbOfROChannels;
  fStkDigits->Delete();
  fStkDigits->Clear();
  fSTKLadders->Delete();
  fSTKLadders->Clear();

  for(int i=0; i<fNTrb*fNLaddersTrb; i++ ) ladderswithsignal[i] = -1;

}


void DmpSimuStkDigiEventMaker::AddCollections(const G4Run* aRun)
{
  //fDataMgr = (DmpSimSvcDataMgr*)gCore->ServiceManager()->Get("Sim/DataMgr");
  //fStkDigits  = fDataMgr->NewOutCollection(const_cast<char *>("StkDigitsCollection"),  const_cast<char *>("DmpStkDigit"));
  fStkDigits = new TClonesArray("DmpStkDigit",10000);
  fSTKLadders = new TClonesArray("DmpStkLadderAdc",200);
  DmpIOSvc::GetInstance()->AddOutputContainer("StkDigitsCollection",fStkDigits);
  DmpIOSvc::GetInstance()->AddOutputContainer("DmpStkLadderAdcCollection",fSTKLadders);

  #ifdef DAMPESW_DEBUG
  hito_test_x = new TH1F("hito_test_x","hito_test_x",1000000, -1000, 1000);
  hito_test_y = new TH1F("hito_test_y","hito_test_y",1000000, -1000, 1000);
  hito_test_z = new TH1F("hito_test_z","hito_test_z",1000000, -1000, 1000);
  #endif
}



/*
void DmpSimuStkDigiEventMaker::FillStep(const G4Step* aStep)
{
}
*/

void DmpSimuStkDigiEventMaker::FillEvent(const G4Event* evt)
{  
    G4DigiManager * fDM = G4DigiManager::GetDMpointer();
    G4int myDigiCollID = fDM->GetDigiCollectionID("STKDigitsCollection");
    DmpSimuStkDigitsCollection * DC = (DmpSimuStkDigitsCollection*)fDM->GetDigiCollection( myDigiCollID );
    
    DmpSimuStkDigitizer * myDM = (DmpSimuStkDigitizer*)fDM->FindDigitizerModule( "DmpSimuStkDigitizer" );
    G4int *ThitID = myDM->GetThitID();
    
    if(DC) {
        //G4cout << "DmpRootNtupleManager: Total STK Digits " << DC->entries() << DmpLogEndl;
        //G4cout << DmpLogEndl;
        //G4int NbOfTKRChannels =  73728;
        //G4int NbOfTKRChannels =  122880;

        /***
        for (G4int i=0;i<NbOfTKRChannels ;i++) {
        G4int j = (ThitID)[i];
        if(j!=-1) G4cout << "digit on channel " << i << " at collection location " << j << "  energy  " << (*DC)[j]->GetEnergy() <<  DmpLogEndl;
        }
        ***/
        //G4cout << " i|X|Lay|C|E|Eprim/Id   " <<DmpLogEndl;

        //G4int n_digi =  DC->entries();
        G4int NStrip, NPlane, IsX;
        //G4double Energy_Threshold = 10.*keV;
        G4double Energy_Threshold = 0.*keV;
        //for (G4int i=0;i<n_digi;i++) {
        //StkDigitEvt->nSiDigit = 0;
        for (G4int j=0;j<NbOfROChannels;j++) {
            //SiDigit
            G4int i = (ThitID)[j];
            if(i==-1) continue;
            if((*DC)[i]->GetDigiType()!=0) continue;

            G4double EChannel     = (*DC)[i]->GetEnergy();

            // energy threhold
            if(EChannel < Energy_Threshold) continue;

            NStrip = (*DC)[i]->GetStripNumber();
            NPlane = (*DC)[i]->GetPlaneNumber();
            IsX    = (*DC)[i]->GetPlaneType();
            G4double EChannelPrim = (*DC)[i]->GetEnergyPrim();

            //DmpStkDigit* digit=new DmpStkDigit();
            DmpStkDigit *digit = (DmpStkDigit*)fStkDigits->ConstructedAt(fStkDigits->GetLast()+1);

            int stripnm = IsX*1000000 + NPlane*100000 + NStrip;

            digit->SiDigit_e = EChannel;
            digit->SiDigit_e_prim = EChannelPrim;
            digit->SiDigit_strip = stripnm;
            digit->SiDigit_trackID = (*DC)[i]->GetPrimTrackID();
            digit->SiDigit_ntracks = (*DC)[i]->GetNtracks();
            digit->SiDigit_ntracksBack = (*DC)[i]->GetNtracksBack();
            digit->SiDigit_ntracksPrim = (*DC)[i]->GetNtracksPrim();
            digit->SiDigit_nhits =(*DC)[i]->GetNhits();
            digit->SiDigit_nhitsBack = (*DC)[i]->GetNhitsBack();
            digit->SiDigit_hitID = (*DC)[i]->GetHitID();
            digit->SiDigit_hitID1 =(*DC)[i]->GetHitID1();
            digit->SiDigit_hitID2 = (*DC)[i]->GetHitID2();

            //++(StkDigitEvt->nSiDigit);
            //StkDigitEvt->SiDigits.push_back(*digit);


            //--------- the following information is for debugging purposes only!------
            #ifdef DAMPESW_DEBUG
            digit->SiDigitX = (*DC)[i]->GetDigitX();
            digit->SiDigitY = (*DC)[i]->GetDigitY();
            digit->SiDigitZ = (*DC)[i]->GetDigitZ();
            #endif
            //------------------------------------------------------------------------

            //* Fill yet another collection (more compatible with real-data ADC counts)
            //AddLadderAdc(EChannel,stripnm,0,0,0);
            #ifdef DAMPESW_DEBUG
            AddLadderAdc(EChannel,stripnm,(*DC)[i]->GetDigitX(), (*DC)[i]->GetDigitY(), (*DC)[i]->GetDigitZ());
			#else
            AddLadderAdc(EChannel,stripnm);
			#endif






            //G4cout << " " << IsX << "|" << NPlane  << "|" << NStrip << "|"  << EChannel
            //	     << "|"  << EChannelPrim << "|"  << (*DC)[i]->GetPrimTrackID() << DmpLogEndl;
            //G4int NChannel = NStrip + (NPlane*2 + IsX)*6144;
            //G4cout << " location " << i << "  strip number in layer " << NStrip << " channel number "  << NChannel  << DmpLogEndl;
        }
    }


    // Add noise to your ladders
    if (fAddStripNoise)
    {
    	// Loop over all ladders with real signal in event
    	for(int i=0; i<fSTKLadders->GetLast()+1; i++){
    		DmpStkLadderAdc* ladder =  (DmpStkLadderAdc*)  fSTKLadders->ConstructedAt(i);
    		// Loop over all 384 RO strips in the ladder
    		for(int chnl =0; chnl< fNROchannels; chnl++ ){
    			double currentvalue = ladder->GetChannelAdc(chnl);
    			double noise = 0.0; // <===== here's where you implement noise :-)
    			ladder->SetChannelAdc(chnl, currentvalue + noise);
    		}
    	}
    }


    //G4cout << "Total Silicon Digits " << tt_SiDigit_n << DmpLogEndl;
}


G4bool DmpSimuStkDigiEventMaker::passTotalDigiCut(G4int cut)
{  
  return (fStkDigits->GetLast()+1 > cut);
}















// *
// *   These  functions are used to create a LadderAdc collection similar to the one in the real data
// *
int DmpSimuStkDigiEventMaker::GetHardwareLadderID(/*The parameters are from simulation layout*/int isX, int block,	int plane /* 0-5 */, int nl /* 0 -3*/) {
	//int ladderID = (fNPlanes - plane - 1) * NLadderPerBlock - nl + NLadderPerBlock - 1;
	int trbID;
	int isY = !isX;
	if      (block == 3 && isY)
		trbID = 0;
	else if (block == 2 && isY)
		trbID = 1;
	else if (block == 0 && isY)
		trbID = 4;
	else if (block == 1 && isY)
		trbID = 5;
	else if (block == 2 && isX)
		trbID = 2;
	else if (block == 0 && isX)
		trbID = 3;
	else if (block == 1 && isX)
		trbID = 6;
	else if (block == 3 && isX)
		trbID = 7;

	//return  trbID*fNLaddersTrb - (plane - fNPlanes+1 ) * NLadderPerBlock;

	int ladderID;
	if(trbID < 4)
		ladderID =  (fNPlanes -1-plane)*NLadderPerBlock +nl;
	else
		ladderID  = (fNPlanes -1-plane)*NLadderPerBlock + NLadderPerBlock -1 -nl;

	int ladder = trbID*fNLaddersTrb + ladderID;

	return ladder;
}


int DmpSimuStkDigiEventMaker::GetHardwareStripID(/*The parameters are from simulation layout*/ int isX /* 0,1*/, int block /* 0-3*/, int simuChannelID /* 0 -383 */){
	int channel;
	int isY = !isX;
    if( (isX && block == 0) ||
        (isX && block == 2) ||
        (isY && block == 0) ||
        (isY && block == 1))
    {
      channel = simuChannelID;
    }
    else{
      channel = fNROchannels - 1 - simuChannelID;
    }
    return channel;

}

#ifdef DAMPESW_DEBUG
void DmpSimuStkDigiEventMaker::AddLadderAdc(double energy, int stripnm, double x, double y, double z)

#else
void DmpSimuStkDigiEventMaker::AddLadderAdc(double energy, int stripnm)
#endif

{
	STKStripParameters* param = fGeometryManager->getSTKROstripParameters(stripnm);

	/*
	int isX         =  stripnm / 1000000;                                                                          // isX   = [0; 1]
	int plane       = (stripnm - isX*1000000)/100000;                                               			     // plane = [0; 5]
	int block       = (stripnm - isX*1000000 - plane*100000)/m_STKnROStripsBlock;                                  // block = [0; 3]
	int strip       =  stripnm - isX*1000000 - plane*100000 - block*m_STKnROStripsBlock;                           // strip = [0; 384*4 -1]
	int nl          = strip/m_STKnROStripsPerLadder;                                                               // nl    = [0; 3]
	int stripLadder = strip- nl*m_STKnROStripsPerLadder;                                                           // stripLadder = [0; 383]
	int iplane      = plane  * 2 + !(isX);                                                                         // iplane      = [0; 11]
	int iblock      = iplane * m_STKnblocksPerLayer + block;                                                       // iblock      = [0; 47]
	int ladder      = strip/m_STKnROStripsPerLadder + block*m_STKnLaddersBlock + iplane*m_STKnLaddersLayer;        // ladder      = [0; 192]
	int trb         = block*2+!(isX);                                                                              // trb         = [0; 7]
	*/

	int hardwareladderID = GetHardwareLadderID(param->isX, param->block, param->plane, param->nl);
	int hardwareStripID = GetHardwareStripID(param->isX, param->block, param->stripLadder);

	double adc           =  energy / eV  / SI_IONOZATION_ENERGY * PAIR_CHARGE * ADC_PER_CHARGE;


	if(ladderswithsignal[hardwareladderID]==-1){
		int trbid    = hardwareladderID/fNLaddersTrb;
		int ladderid = hardwareladderID%fNLaddersTrb;
		DmpStkLadderAdc* ladder = new((*fSTKLadders)[fSTKLadders->GetLast()+1]) DmpStkLadderAdc();
		ladder->SetLadderID(fNLaddersTrb, trbid, ladderid);
		ladderswithsignal[hardwareladderID] = fSTKLadders->GetLast();
	}

	int index = ladderswithsignal[hardwareladderID];
	DmpStkLadderAdc* ladder = (DmpStkLadderAdc*)  fSTKLadders->ConstructedAt(index);
	ladder->SetChannelAdc(hardwareStripID,adc);

#ifdef DAMPESW_DEBUG
	double center = fGeometryManager->getSTKROStripPositionOnLadder(GetLadderIdSimulationLayout(hardwareladderID), GetStripIdSimulationLayout(hardwareladderID,hardwareStripID));
	int layer  = fGeometryManager->getSTKladderParameters(GetLadderIdSimulationLayout(hardwareladderID))->iplane;
	double globalpos = fGeometryManager->getSTKClusterGlobalPosition(GetLadderIdSimulationLayout(hardwareladderID),center);
	int isX    = fGeometryManager->getSTKisLayerX(layer);
	double posz =fGeometryManager->getSTKlayerZcoordinate(layer);
	if(isX){
		hito_test_x->Fill(globalpos - x);
	}else{
		hito_test_y->Fill(globalpos - y);
	}
	hito_test_z->Fill(posz - z);
#endif


}



#ifdef DAMPESW_DEBUG
int DmpSimuStkDigiEventMaker::GetStripIdSimulationLayout(int ladder, int channelID){
  //* get hardare parameters
    int trbID    = ladder/ fNLaddersTrb;
    int ladderID = ladder %fNLaddersTrb;


    int isX = (trbID/2)%2;
    int isY = !isX;

    //* TRB conversion table TODO: this should be change if the TRB numbering changes
    int block;

    if     (trbID == 0) block = 3;
    else if(trbID == 1) block = 2;
    else if(trbID == 4) block = 0;
    else if(trbID == 5) block = 1;
    else if(trbID == 2) block = 2;
    else if(trbID == 3) block = 0;
    else if(trbID == 6) block = 1;
    else if(trbID == 7) block = 3;

    //* Channel number
    int nchannel;
    if( (isX && block == 0) ||
        (isX && block == 2) ||
        (isY && block == 0) ||
        (isY && block == 1))
    {
      nchannel = channelID;
    }
    else{
      nchannel = fNROchannels - 1 - channelID;
    }

    return nchannel;
}

int DmpSimuStkDigiEventMaker::GetLadderIdSimulationLayout(int ladder /* hardware definition  0 -191 */){

  int trbID    = ladder/ fNLaddersTrb;
  int ladderID = ladder %fNLaddersTrb;

  int fNLaddersBlock = 4;



  int isX = (trbID/2)%2;
  int isY = !isX;

  //* TRB conversion table TODO: this should be change if the TRB numbering changes
  int block;

  if     (trbID == 0) block = 3;
  else if(trbID == 1) block = 2;
  else if(trbID == 4) block = 0;
  else if(trbID == 5) block = 1;
  else if(trbID == 2) block = 2;
  else if(trbID == 3) block = 0;
  else if(trbID == 6) block = 1;
  else if(trbID == 7) block = 3;


  //* 0 -5
  int plane = fNPlanes - 1 - ladderID / fNLaddersBlock;


  int nl; // 0 - 3
  if(trbID < 4) nl = ladderID % fNLaddersBlock;
  else    nl = fNLaddersBlock -1 - ladderID % fNLaddersBlock;


  return fGeometryManager->getSTKLadderID(isX , plane ,  block ,  nl);

}
#endif











