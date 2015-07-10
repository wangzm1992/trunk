#ifndef DmpSimuStkDigiEventMaker_h
#define DmpSimuStkDigiEventMaker_h 1

#include <vector>
#include "globals.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "TTree.h"
#include "TClonesArray.h"

#include "DmpGeometryManager.h"
#include "DmpEvtStkDigit.hh"
#include "DmpSimuConfigParser.h"
//#include "DmpSimSvcDataMgr.h"
#include "TH1F.h"



class DmpSimuStkDigiEventMaker
{
public:
  
    DmpSimuStkDigiEventMaker();
  ~DmpSimuStkDigiEventMaker();
   
  void AddCollections(const G4Run* aRun);
  //void save();

  void BeginRun(const G4Run* aRun);
  void BeginEvent(const G4Event* evt);
  void FillEvent(const G4Event* evt);
  //void FillStep(const G4Step* aStep);


  // *   These  functions are used to create a LadderAdc collection similar to the one in the real data
  int GetHardwareLadderID(/*The parameters are from simulation layout*/int isX /* 0,1*/, int block,	int plane /* 0-5 */, int nl /* 0 -3*/);
  int GetHardwareStripID(/*The parameters are from simulation layout*/ int isX /* 0,1*/, int block /* 0-3*/, int simuChannelID /* 0 -383 */);
#ifdef DAMPESW_DEBUG
  void AddLadderAdc(double enrgy, int stripnm, double x, double y, double z);
#else
  void AddLadderAdc(double enrgy, int stripnm);
#endif


#ifdef DAMPESW_DEBUG
  int GetStripIdSimulationLayout(int ladder, int channelID);
  int GetLadderIdSimulationLayout(int ladder /* hardware definition  0 -191 */);
#endif


  bool passTotalDigiCut(G4int cut);

 private:
  int debugEvent;
  int NbOfROChannels;
  int NLadderPerBlock;
  double ADC_PER_CHARGE;

  int fNROchannels; // N RO channels per ladder 0 -384, this is not NbOfROChannels!
  int fNPlanes;
  int fNLaddersTrb;
  int fNTrb;
  bool fAddStripNoise;


  //TTree*   tree;
  //const G4Run*   run;
  //DmpEvtStkDigit* StkDigitEvt;

  int* ladderswithsignal;

  //DmpSimSvcDataMgr* fDataMgr;
  TClonesArray* fStkDigits;
  TClonesArray* fSTKLadders; // Collection of output objects
  DmpSimuConfigParser* fSimuConfig;

  //* Read geometry constants
  DmpGeometryManager* fGeometryManager;

  TH1F* hito_test_x;
  TH1F* hito_test_y;
  TH1F* hito_test_z;




};


#endif

