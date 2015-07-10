#! /usr/bin/python
import DMPSW 
import libDmpSimu as  DmpSim


#-------------------------------------------------------------------
# IOSvc options
DMPSW.IOSvc.Set("OutData/FileName", "tmp") 

DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG
SimAlg   = DmpSim.DmpSimAlg()                     #@ Get instance of DAMPE simulation tool (interface to geant4)
#SimAlg.Set("GeometryMainGdmlName","Geometry/DAMPE.gdml")
SimAlg.Set("Visualization", "True")
DMPSW.AlgMgr.Append(SimAlg)

SimuCfgPsr = DmpSim.SimuCfgParser()
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

