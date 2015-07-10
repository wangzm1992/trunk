#! /usr/bin/python
import DMPSW 
import libDmpSimu as  DmpSim
import os

#-------------------------------------------------------------------
# IOSvc options
DMPSW.IOSvc.Set("OutData/FileName","DmpSimu")   
DMPSW.IOSvc.Set("OutData/Tag","Sim")    

DMPSW.Core.Set("LogLevel","INFO")    #@ Possible options are:  NONE, ERROR, WARNING, INFO, DEBUG
DMPSW.Core.Set("EventNumber","1000")            
SimAlg   = DmpSim.DmpSimAlg()                     #@ Get instance of DAMPE simulation tool (interface to geant4)
DMPSW.AlgMgr.Append(SimAlg)
#SimAlg.Set("GeometryMainGdmlName","Geometry/DAMPE.gdml") <-- obsolete. Geometry is set from the configuration file (see below)




#@ Geant4 macro
#@
#@         Note, that "Run" commands must be removed from the Macro, since run parameters are defined in the joboption file!
#@         (see f.ex. DMPSW.Core.Set("EventNumber","1000")     command)
#@
SimAlg.Set("MacFile", os.environ['DMPSWSYS'] + "/share/TestRelease/test_stkkalman_muons.mac")


#@ Geant4 random seed
#SimAlg.Set("RandomSeed", "1234567") # <-- uncomment this to set up a random seed 


#@ Simulation configuration file
#@   The config file used: Simulation/share/dmpSimu.cfg
#@   This configuration file is installed to $DMPSWSYS/share/Configuration/dmpSimu.cfg
#@   To use your custom configuration, replace the default config file $DMPSWSYS/share/Configuration/dmpSimu.cfg with the one of your choice
#@
SimuCfgPsr = DmpSim.SimuCfgParser()
DMPSW.SvcMgr.Append(SimuCfgPsr)
DMPSW.SvcMgr.Append(DMPSW.GeoMgr)

DMPSW.Core.Initialize()
DMPSW.Core.Run()
DMPSW.Core.Finalize()

