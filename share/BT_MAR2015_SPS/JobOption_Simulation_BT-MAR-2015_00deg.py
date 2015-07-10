import DMPSW 
import libDmpSimu as  DmpSim
import os

#-------------------------------------------------------------------
# IOSvc options
#DMPSW.IOSvc.Set("OutData/FileName","DmpSimu_Electron")   
#DMPSW.IOSvc.Set("OutData/Tag","Sim")  


DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG            
SimAlg = DmpSim.DmpSimAlg()       #@ Get instance of DAMPE simulation tool (interface to geant4)
#SimAlg.Set("GeometryMainGdmlName","Geometry/BeamTestNov2014_SPS/DAMPE_BT_SPS_2014.gdml")
SimAlg.Set("MacFile", os.environ['DMPSWSYS'] + "/share/TestRelease/BT_MAR2015_SPS/electron_00deg.mac")
DMPSW.AlgMgr.Append(SimAlg)




VisualizationMode =  True
if VisualizationMode:
    #@ Visualization mode (uncomment me to run visualization)
    DMPSW.IOSvc.Set("OutData/FileName", "tmp")
    SimAlg.Set("Visualization", "True")   
else:
    #@ Simulation mode (uncomment me to run simulation)
    DMPSW.IOSvc.Set("OutData/FileName","DmpSimu")
    DMPSW.Core.Set("EventNumber","10")



SimuCfgPsr = DmpSim.SimuCfgParser()
SimuCfgPsr.Set("ConfigFile", "dmpSimuSPS2015MAR.cfg")
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

