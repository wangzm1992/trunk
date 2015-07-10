import DMPSW 
import libDmpSimu as  DmpSim
import os

#-------------------------------------------------------------------
# IOSvc options
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu_Electron")   
#DMPSW.IOSvc.Set("OutData/Tag","Sim")  


DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG            
SimAlg = DmpSim.DmpSimAlg()       #@ Get instance of DAMPE simulation tool (interface to geant4)
#SimAlg.Set("GeometryMainGdmlName","Geometry/BeamTestOctober2014/DAMPE_BT_OCTOBER2014.gdml")
SimAlg.Set("MacFile", os.environ['DMPSWSYS'] + "/share/TestRelease/BT_OCT2014/electron_00deg.mac")
DMPSW.AlgMgr.Append(SimAlg)


#@ Simulation mode
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu")
#DMPSW.Core.Set("EventNumber","10")

#@ Visualization mode
DMPSW.IOSvc.Set("OutData/FileName", "tmp")
SimAlg.Set("Visualization", "True")   



SimuCfgPsr = DmpSim.SimuCfgParser()
SimuCfgPsr.Set("ConfigFile", "dmpSimuPS2014OCT.cfg")
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

