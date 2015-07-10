import DMPSW 
import libDmpSimu as  DmpSim


#-------------------------------------------------------------------
# IOSvc options
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu_Electron")   
#DMPSW.IOSvc.Set("OutData/Tag","Sim")  
DMPSW.IOSvc.Set("OutData/FileName", "tmp")


DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG            
SimAlg = DmpSim.DmpSimAlg()       #@ Get instance of DAMPE simulation tool (interface to geant4)
#SimAlg.Set("GeometryMainGdmlName","Geometry/BeamTestOctober2014/DAMPE_BT_OCTOBER2014.gdml")
SimAlg.Set("Visualization", "True")
DMPSW.AlgMgr.Append(SimAlg)

SimuCfgPsr = DmpSim.SimuCfgParser()
SimuCfgPsr.Set("ConfigFile", "dmpSimuPS2014OCT.cfg")
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

