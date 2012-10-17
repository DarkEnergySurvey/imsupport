///
/// \file 
/// \brief Legacy interface implementation
/// \author Mike Campbell (mtcampbe@illinois.edu)
///
#include "LegacyInterface.hh"


namespace LegacyInterface {

  void ReportMessage(int verb,int type,int level, const std::string &message)
  {
    std::istringstream Istr(message);
    std::string line;
    while(std::getline(Istr,line))
      reportevt(verb,type,level,(char *)line.c_str());
  }

}
