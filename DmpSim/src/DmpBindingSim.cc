/*
 *  $Id: DmpBindingSim.cc, 2014-05-15 00:21:01 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 07/03/2014
*/

#include <boost/python.hpp>

#include "DmpSimAlg.h"
#include "DmpSimuConfigParser.h"
//#include "DmpSimSvcDataMgr.h"


BOOST_PYTHON_MODULE(libDmpSimu){
  using namespace boost::python;

  //* DmpSimAlg
  class_<DmpSimAlg,boost::noncopyable,bases<DmpVAlg> >("DmpSimAlg",init<>());

  //* Configuration parser
  class_<DmpSimuConfigParser,boost::noncopyable,bases<DmpVSvc> >("SimuCfgParser",init<>());

}


