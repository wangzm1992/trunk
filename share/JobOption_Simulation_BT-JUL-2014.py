import DMPSW 
import libDmpSimu as  DmpSim


#-------------------------------------------------------------------
# IOSvc options
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu_Electron_new")   
#DMPSW.IOSvc.Set("OutData/Tag","Sim")    
DMPSW.IOSvc.Set("OutData/FileName", "tmp")

DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG
SimAlg   = DmpSim.DmpSimAlg()       #@ Get instance of DAMPE simulation tool (interface to geant4)
#SimAlg.Set("GeometryMainGdmlName","Geometry/DAMPE_BT_JULY2014.gdml")
SimAlg.Set("STKsimulation", "off")
SimAlg.Set("BGOsimulation", "off")
SimAlg.Set("NUDsimulation", "off")
SimAlg.Set("PSDsimulation", "off")
SimAlg.Set("SATsimulation", "off")
SimAlg.Set("Visualization", "True")
DMPSW.AlgMgr.Append(SimAlg)


SimuCfgPsr = DmpSim.SimuCfgParser()
SimuCfgPsr.Set("ConfigFile", "dmpSimuPS2014JUL.cfg")
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

