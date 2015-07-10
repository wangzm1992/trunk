import DMPSW 
import libDmpSimu as  DmpSim
import os

#-------------------------------------------------------------------
# IOSvc options
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu_Electron")   
#DMPSW.IOSvc.Set("OutData/Tag","Sim")  
#DMPSW.IOSvc.Set("OutData/NoOutput", "true")
DMPSW.IOSvc.Set("OutData/FileName","DmpSimu")
  
DMPSW.Core.Set("EventNumber","10")   
DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG            
SimAlg = DmpSim.DmpSimAlg()       #@ Get instance of DAMPE simulation tool (interface to geant4)
SimAlg.Set("GeometryMainGdmlName","DAMPE_BT_OCTOBER2014_30deg.gdml")
SimAlg.Set("MacFile", os.environ['DMPSWSYS'] + "/share/TestRelease/BT_OCT2014/electron_30deg.mac")
#SimAlg.Set("Visualization", "True")
DMPSW.AlgMgr.Append(SimAlg)

SimuCfgPsr = DmpSim.SimuCfgParser()
SimuCfgPsr.Set("ConfigFile", "dmpSimuPS2014OCT.cfg")
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

