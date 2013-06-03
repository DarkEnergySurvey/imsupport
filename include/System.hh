///
/// \file
/// \ingroup support
/// \brief Unix System Tools interface
/// \author Mike Campbell (mtcampbe@illinois.edu)
///
#ifndef _SYSTEM_UTIL_H_
#define _SYSTEM_UTIL_H_

//#include <ostream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "primitive_utilities.hh"

namespace Sys {
  std::string LogTime();
  void TokenizePath(std::vector<std::string> rv,const std::string &path);
  int TempFileName(std::string &stub);
  const std::string Hostname(bool longname = false);
  const std::string StripDirs(const std::string &);
  const std::string CWD();
  void SafeRemove(const std::string &fname,const std::string &ext);
  int ChDir(const std::string &path);
  bool FILEEXISTS(const std::string &fname);
  bool ISDIR(const std::string &fname);
  bool ISLINK(const std::string &fname);
  int CreateDirectory(const std::string &fname);
  const std::string ResolveLink(const std::string &path);
  class Directory : public std::vector<std::string>
  {
  private:
    std::string _path;
    bool   _good;
    DIR    *_dir;
  public:
    Directory(const std::string &s = "");
    ~Directory();
    int open(const std::string &s = "");
    void close();
    operator void* ();
    bool operator ! ();
  };


  class Environment : public std::vector< std::pair<std::string,std::string> >
  {
  public:
    Environment();
    int SetEnv(const std::string &,const std::string &,bool);
    void UnSetEnv(const std::string &);
#ifndef DARWIN
    int ClearEnv();
#endif
    const std::string GetEnv(const std::string &) const;
    std::string &GetEnv(const std::string &);
    int PutEnv(char *);
    void Refresh();
    char **GetRawEnv();
  private:
    void init();
    std::string empty_string;
  };

  inline double
  Time()
  {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    double t = tv.tv_sec + tv.tv_usec/1000000.;
    return(t);
  };
};
#endif
