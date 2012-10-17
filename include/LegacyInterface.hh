#ifndef __LEGACY_XF__
#define __LEGACY_XF__
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

///
/// \file
/// \brief Convenience interface between C++ and old C constructs
/// \author Mike Campbell (mtcampbe@illinois.edu)
///
extern "C" {
#include "imutils.h"
  void init_desimage(desimage *image);
  void rd_desimage(desimage *image,int mode,int verbosity);
  void rd_dessubimage(desimage *image,long *lx,long *ux,int mode,int verbosity);
  int mkpath(const char *filename,int flag_verbose);
#if 0 
  void reportevt(int flag_verbose,int type,int level,char *event); // dlp
#endif
  void wcsCorners(char *catname, double retwcs[4][2], int verboseFlag);
  void wcs2xy(char *catname, double *wcspos, double *rawpos, int verboseFlag);
  void  retrievescale(desimage *image,int *scaleregionn,float *scalesort,
		      int flag_verbose,float *scalefactor,
		      float *mode,float *sigma);
  float gasdev(long *seed);
  void shell(int,float *);
  void decodesection(char *name,int *numbers,int flag_verbose);
  void headercheck(desimage *image,char *filter,int *ccdnum,
		   char *keyword,int flag_verbose);
#if 0
  void	printerror(int status);	// dlp
#endif
#if 0
  void image_compare(desimage *image,desimage *stemplate,float *offset,
		     float *rms,float *maxdev,int *npixels,int flag_verbose);
#endif
  char *striparchiveroot(char *filename);
}

namespace LegacyInterface {
  typedef float ImageDataType;
  typedef short MaskType;
  typedef long  IndexType;
  
  ///
  /// \brief Use the "reportevt" utility to send messages to stdout
  ///
  /// This utility takes a string and breaks it up into single lines
  /// for proper output.  May need to revisit this as it will currently
  /// send lines larger than 80 characters.  reportevt may already take
  /// care of that.
  ///
  void ReportMessage(int verb,int type,int level, const std::string &message);
}

namespace LX=LegacyInterface;

#endif
