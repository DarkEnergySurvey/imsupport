#ifndef __FITS_TOOLS__
#define __FITS_TOOLS__
///
/// \file
/// \brief Quick tool for interacting with FITS data
/// \author Mike Campbell (mtcampbe@illinois.edu)
/// 
#include <memory>
#include <cstring>
//#include <algorithm>

// System Tools
#include "System.hh"

// Interface to legacy constructs
#include "LegacyInterface.hh"

// CCFITS
#include <CCfits>

namespace FitsTools {

  template<typename PixelType>
  class ImageData {
  private:
    PixelType *_data;
    size_t _npix;
    bool _mine;
    unsigned int _i;
    unsigned int _j;
  public:
    ImageData(): _data(NULL), _npix(0),_mine(true),_i(0),_j(0) {};
    ImageData(size_t npix): 
      _npix(npix),_mine(true),_i(npix) 
    {assert(npix >= 0); _data = new PixelType [npix]; };
    ImageData(PixelType *data,unsigned int i,unsigned int j=0): 
      _data(data), _mine(false), _i(i), _j(j)
    {
      _npix = i;
      if(j > 0)
	_npix *= j;
      assert(_npix >= 0);
    };
    ImageData(unsigned int i, unsigned int j): 
      _mine(true), _i(i), _j(j)
    {
      _npix = 0;
      _npix = i;
      if(j > 0)
	_npix *= j;
      assert(_npix > 0);
      _data = new PixelType [_npix];
    };
    void Init(){Destroy();};
    void Init(size_t npix)
    { 
      Destroy(); 
      _npix = npix;
      _mine = true; 
      assert(_npix > 0);
      _data = new PixelType [_npix]; 
    };
    void Init(PixelType *data,unsigned int i, unsigned int j)
    {
      Destroy();
      _data = data;
      _mine = false;
      _i = i;
      _j = j;   
      _npix = i;
      if(_j > 0) _npix *= _j;
      assert(_npix >= 0);
    };
    void Init(unsigned int i,unsigned int j)
    {
      Destroy();
      _mine = true;
      _npix = i;
      _i = i;
      _j = j;
      if(j > 0)
	_npix *= j;
      assert(_npix > 0);
      _data = new PixelType [_npix];
    };
    void Destroy(){ if(_mine) delete [] _data; _data=NULL; _npix = 0;};
    PixelType *Data() { return _data; };
    size_t NPix() { return _npix; };
    const PixelType *Data() const { return _data; };
    ~ImageData(){ if(_mine) delete [] _data; };
    inline void ScaleImage(PixelType scale){for(unsigned int i = 0;i < _npix;i++) _data[i] *= scale; }
    ImageData<PixelType> *SubImg(unsigned int x,unsigned int y,unsigned int ncol,unsigned int nrow)
    {
      assert((x >= 0)&&(x < _i)&&(y >= 0)&&(y < _j));
      unsigned int ymax = y + nrow - 1;
      unsigned int xmax = x + ncol - 1;
      assert((xmax < _i)&&(ymax < _j));
      ImageData<PixelType> *retval = new ImageData<PixelType>(nrow*ncol);
      if(retval){
	unsigned int n = 0;
	for(unsigned int j = y;j <= ymax;j++){
	  unsigned int index = j*_i;
	  for(unsigned int i = x;i <= xmax;i++){
	    index = index + i;
	    retval->_data[n++] = this->_data[index];
	  }
	}
	retval->_i = ncol;
	retval->_j = nrow;
	retval->_npix = n;
	retval->_mine = true;
      }
      return(retval);
    };
  };

  class FitsImage
  {
  private:
    std::ostream               *Out;
    char                       **exclist;
    int                        nexcl;
    std::vector<std::string>   _Headers;
    bool                       _myimage;
    desimage                   *_des;
  public:
    FitsImage(): Out(NULL), exclist(NULL), nexcl(0), _myimage(true) {
      _des = new desimage;
      init_desimage(_des); 
    };
    const std::string &ImageHeader() const { return(_Headers[_des->unit]); };
    std::string &ImageHeader() { return(_Headers[_des->unit]); };
    void AppendImageHeader(const std::string &entry){
      std::ostringstream Ostr;
      Ostr << entry;
      int roomtogo = 80 - entry.size();
      if(roomtogo > 0){
	while(roomtogo--)
	  Ostr << " ";
      }
      Ostr << std::endl;
      _Headers[_des->unit] += Ostr.str();
    };
    desimage *DES() { return _des; };
    std::vector<std::string> &Headers() { return (_Headers); };
    std::string &Header(unsigned int num) { 
      assert(num < _Headers.size()); 
      return(_Headers[num]); 
    };
    std::string GetHeader(unsigned int num) { assert(num < _Headers.size()); return(_Headers[num]); };
    void SetOutStream(std::ostream &ostr) { Out = &ostr; };
    void SetExclusions(char **elist,int nel){ exclist=elist; nexcl=nel; };
    int Close(){ int status = 0;return(fits_close_file(_des->fptr,&status)); };
    char **MakeCArgList(std::vector<std::string> &stringlist){
      char **retval = NULL;
      if(!stringlist.empty()){
	retval = new char *[stringlist.size()];
	std::vector<std::string>::iterator sli = stringlist.begin();
	int n = 0;
	while(sli != stringlist.end()){
	  std::string argstring(*sli++);
	  std::string::size_type nchar = argstring.size();
	  nchar++;
	  retval[n] = new char [nchar];
	  std::strncpy(retval[n++],argstring.c_str(),nchar);
	}
      }
      return(retval);
    }
    int WriteTableColumn(int colnum,int nelem,int datatype,void *array,int flag_verbose=0){
      if(!Out)
	Out = &std::cout;
      int status = 0;
      //      long nrows = 0;
//       fits_get_num_rows(this->_des.fptr, &nrows, &status);
//       if(nrows < nelem){
// 	nrows = nelem - nrows;
// 	fits_insert_rows(this->_des.fptr, 0, nrows, &status);
//       }
      if(fits_write_col(this->_des->fptr,datatype,colnum,1,1,nelem, array,&status)){
	fits_report_error(stderr,status);
	*Out << "FitsTools::FitsImage::WriteTableColumn(): Error: Could not write FITS table column " 
	     << this->_des->name << std::endl;
	return(1);
      }
      if(flag_verbose){
	*Out << "FitsTools::FitsImage::WriteTableColumn(): Wrote FITS table column in " 
	     << this->_des->name << std::endl;
      }
      return(0);
    }
    int MakeTable(int nfields,std::vector<std::string> &field_names,std::vector<std::string> &field_types,
		  std::vector<std::string> &field_units,const std::string &extension_name,int flag_verbose){
      
      if(!Out)
	Out = &std::cout;
      int status = 0;
      if(fits_create_tbl(this->_des->fptr, BINARY_TBL,0,nfields,
			 MakeCArgList(field_names),MakeCArgList(field_types), 
			 MakeCArgList(field_units),
			 extension_name.c_str(), &status)){
	*Out << "FitsTools::FitsImage::MakeTable(): Error: Could not make FITS table in " 
	     << this->_des->name << std::endl;
	return(1);
      }
      if(flag_verbose){
	*Out << "FitsTools::FitsImage::MakeTable(): Created FITS table in " 
	     << this->_des->name << std::endl;
      }
      return(0);
    }
    int CreateFile(const std::string &filename,bool replace=false,int flag_verbose=0)
    {
      if(!Out)
	Out = &std::cout;
      if (mkpath(filename.c_str(),flag_verbose)) {
	*Out << "FitsTools::FitsImage::CreateFile(): Error: Could not make path for output, "
	     << filename << std::endl;
	return(1);
      }
      else if (flag_verbose){
	*Out << "FitsTools::FitsImage::CreateFile(): Message: Created path to " 
	     << filename << std::endl;
      }
      if(replace && Sys::FILEEXISTS(filename))
	unlink(filename.c_str());
      int status = 0;
      std::strncpy(this->_des->name,filename.c_str(),filename.size()+1);
      if (fits_create_file(&(this->_des->fptr),filename.c_str(),&status)) {
	*Out << "FitsTools::FitsImage::CreateFile(): Error: Could not create FITS file, " 
	     << filename << std::endl;
	return(1);
      }
      if (flag_verbose) {
	*Out << "FitsTools::FitsImage::CreateFile(): Message: Opened FITS file, "
	     << filename << std::endl;
      }
      return(0);
    }
    int Write(const std::string &filename,bool replace=false,int flag_verbose=0)
    {
      if(!Out)
	Out = &std::cout;
      if (mkpath(filename.c_str(),flag_verbose)) {
	*Out << "FitsTools::FitsImage::Write(): Error: Could not make path for output, "
	     << filename << std::endl;
	return(1);
      }
      else if (flag_verbose){
	*Out << "FitsTools::FitsImage::Write(): Message: Created path to " 
	     << filename << std::endl;
      }
      if(replace && Sys::FILEEXISTS(filename))
	unlink(filename.c_str());
      int status = 0;
      std::strncpy(this->_des->name,filename.c_str(),filename.size()+1);
      if (fits_create_file(&(this->_des->fptr),filename.c_str(),&status)) {
	*Out << "FitsTools::FitsImage::Write(): Error: Could not create FITS file, " 
	     << filename << std::endl;
	return(1);
      }
      if (flag_verbose) {
	*Out << "FitsTools::FitsImage::Write(): Message: Opened FITS file, "
	     << filename << std::endl;
      }
      status = 0;
      if(this->_des->image){
	if(WriteImage()){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write primary image."
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote primary image."
	       << std::endl;
	}
	if(WriteHeader(_des->unit)){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write header for image." 
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote primary header."
	       << std::endl;
	}
      }
      if(_des->mask){
	int status = 0;
	// Write mask image
	if(WriteMask()){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write mask."
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote mask."
	       << std::endl;
	}
	if(WriteHeader(_des->maskunit)){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write header for mask." 
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote mask header."
	       << std::endl;
	}
      }
      if(_des->varim){
	// Write weight image
	if(WriteWeight()){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write weight image."
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote weight image."
	       << std::endl;
	}
	if(WriteHeader(_des->varunit)){
	  *Out << "FitsTools::FitsImage::Write(): Error: Failed to write header for weight image." 
	       << std::endl;
	  return(1);
	}
	if(flag_verbose){
	  *Out << "FitsTools::FitsImage::Write(): Message: Wrote weight header."
	       << std::endl;
	}
      }
      return(0);
    };
    int WriteImage()
    {
      if(!Out)
	Out = &std::cout;
      // Write primary image
      int status = 0;
      if (fits_create_img(this->_des->fptr,FLOAT_IMG,2,this->_des->axes,&status)) {
	*Out << "FitsTools::FitsImage::WriteImage(): Error: Could not create FITS image extension."
	     << std::endl;
	return(1);
      }
      // Write image data
      if (fits_write_img(_des->fptr,TFLOAT,1,_des->npixels,_des->image,&status)) {
	*Out << "FitsTools::FitsImage::WriteImage(): Error: Failed to write image."
	     << std::endl;
	return(1);
      }
      return(0);
    };
    int WriteWeight()
    {
      if(!Out)
	Out = &std::cout;
      int status = 0;
      if(_des->varim){
	if (fits_create_img(this->_des->fptr,FLOAT_IMG,2,this->_des->axes,&status)) {
	  *Out << "FitsTools::FitsImage::WriteImage(): Error: Could not create FITS weight extension."
	       << std::endl;
	  return(1);
	}
	// Write image data
	if (fits_write_img(_des->fptr,TFLOAT,1,_des->npixels,_des->varim,&status)) {
	  *Out << "FitsTools::FitsImage::WriteImage(): Error: Failed to write image."
	       << std::endl;
	  return(1);
	}
      }
      return(0);
    };
    int WriteMask()
    {
      if(!Out)
	Out = &std::cout;
      int status = 0;
      if(_des->mask){
	if (fits_create_img(this->_des->fptr,USHORT_IMG,2,this->_des->axes,&status)) {
	  *Out << "FitsTools::FitsImage::WriteImage(): Error: Could not create FITS mask extension."
	       << std::endl;
	  return(1);
	}
	// Write image data
	if (fits_write_img(_des->fptr,TUSHORT,1,_des->npixels,_des->mask,&status)) {
	  *Out << "FitsTools::FitsImage::WriteImage(): Error: Failed to write image."
	       << std::endl;
	  return(1);
	}
      }
      return(0);
    };
    int WriteHeader(unsigned int unitno)
    {
      int status = 0;
      if(!Out)
	Out = &std::cout;
      assert(unitno < _Headers.size());
      std::string line;
      std::istringstream Istr(_Headers[unitno]);
      int nlines = 0;
      while(std::getline(Istr,line)){
	if(line.size() > 1)
	  nlines++;
      }
      Istr.clear();
      Istr.str(_Headers[unitno]);
      // Not sure if this is actually necessary but .... reserve header space
      if (fits_set_hdrsize(this->_des->fptr,nlines+5,&status)) {
	*Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not reserve FITS header space for "
	     << nlines << " entries." << std::endl;
	return(1);
      }
      int hdrtot = 0;
      int hdrpos = 0;
      if (fits_get_hdrpos(_des->fptr,&hdrtot,&hdrpos,&status)) {
	*Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not get header position."
	     << std::endl;
	return(1);
      }
      nlines=1;
      std::string des_extension_type;
      while(std::getline(Istr,line)){
	// Let's write DES_EXT last.  It looks better.
	if(line.substr(0,7) == "DES_EXT"){
	  des_extension_type=line;
	}
	else if(line.substr(0,7) == "COMMENT"){
	  std::string::size_type x = line.find("format is defined");
	  if(x == std::string::npos)
	    x = line.find("and Astrophysics");
	  if(x == std::string::npos){
	    if(fits_insert_record(_des->fptr,hdrtot+nlines++,(char *)line.c_str(),&status)){
	      *Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not insert comment record into header."
		   << std::endl;
	    }
	  }
	}
	else if(line.substr(0,7) == "HISTORY"){
	  //	  std::cout << "History Item: " << line.substr(8) << std::endl;
	  if(fits_write_history(_des->fptr,(char *)line.substr(8).c_str(),&status)){
	    *Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not insert history record into header."
		 << std::endl;
	    return(1);
	  }
	}
	else if(line.size() > 1){
 	  if(line.substr(0,8) == "FILENAME"){
 	    std::ostringstream Ostr;
 	    Ostr << "FILENAME= '" << Util::stripdirs(this->_des->name) << "'";
 	    int space_left_over = 80 - Ostr.str().size();
 	    while(space_left_over-- > 0)
 	      Ostr << " ";
 	    	    Ostr << std::endl;
 	    line = Ostr.str();
 	  }
	  if (fits_insert_record(_des->fptr,hdrtot+nlines++,
				 (char *)line.c_str(),&status)) {
	    *Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not insert record into header."
		 << std::endl;
	    return(1);
	  } 
	}
      }							
      // Write the DES_EXT
      if (fits_insert_record(_des->fptr,hdrtot+nlines++,
			     (char *)des_extension_type.c_str(),&status)) {
	*Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not insert end record into header."
	     << std::endl;
	return(1);
      }
      // END the header
      //      if (fits_insert_record(_des->fptr,hdrtot+nlines++,
      //			     (char *)"END",&status)) {
      //	*Out << "FitsTools::FitsImage::WriteHeader(): Error: Could not insert end record into header."
      //	     << std::endl;
      //	return(1);
      //      }
      return(0);
    };
    int Read(const std::string &filename,int flag_verbose)
    {
      //      if(flag_verbose)
      //	std::cout << "verbose!" << std::endl;
      if(!Out)
	Out = &std::cout;
      //      else
      //	std::cout << "Out is set!" << std::endl;
      std::string fname(filename);
      if(!Sys::FILEEXISTS(fname)){
	fname.assign(filename+".fz");
	if(!Sys::FILEEXISTS(fname)){
	  fname.assign(filename+".gz");
	  if(!Sys::FILEEXISTS(fname)){
	    fname.assign(filename+".fits.fz");
	    if(!Sys::FILEEXISTS(fname)){
	      fname.assign(filename+".fits.gz");
	      if(!Sys::FILEEXISTS(fname)){
		*Out << "FitsTools::readImage: Could not find FITS file for " << filename << "." << std::endl;
		return(1);
	      }
	    }
	  }
	}
      }
      if(flag_verbose){
	*Out << "FitsTools::readImage: Found FITS file " << fname << "." << std::endl;
      }
      std::strcpy(this->_des->name,fname.c_str());
      rd_desimage(this->_des,READONLY,flag_verbose);
      char *zeroheader = NULL;
      int numzerokeys = 0;
      int status = 0;
      char *exclkey = NULL;
      int nexc = 0;
      int hdutype;
      if(flag_verbose)
	*Out << "FitsTools::readImage: Found " << this->_des->hdunum << " header units." << std::endl
	     << "FitsTools::readImage: Image unit = " << this->_des->unit << std::endl;
      for(int i = 1;i <= this->_des->hdunum;i++){
	if (fits_movabs_hdu(this->_des->fptr,i,&hdutype,&status)) {
	  // 	sprintf(event,"Move to HDU=1 failed: %s",bias.name);
	  // 	reportevt(flag_verbose,STATUS,5,event);
	  // 	printerror(status);
	  *Out << "FitsTools::readImage: Move to HDU= " << i << " failed: " << this->_des->name << std::endl;
	  return(1);
	}
	if(flag_verbose)
	  *Out << "FitsTools::readImage: Header(" << i << ") is of type " << hdutype << std::endl;
	if (fits_hdr2str(this->_des->fptr,0,this->exclist,this->nexcl,&zeroheader,&numzerokeys,&status)) {
	  *Out << "FitsTools::readFitsFile could not read header information from " 
		    << this->_des->name << "." << std::endl;
	  return(1);
	}
	std::string header_zero(zeroheader);
	std::ostringstream Ostr;
	for(int i = 0;i < numzerokeys;i++){
	  std::string header_record = header_zero.substr(i*80,80);
	  if(!header_record.empty() && (header_record.substr(0,3) != "END"))
	    Ostr << header_record << std::endl;
	}
	this->_Headers.push_back(Ostr.str());
	//	if(flag_verbose > 2) 
	//	  *Out << Ostr.str() << std::endl;
      }
      return(0);
    };
    int ReadSubImage(const std::string &filename,std::vector<long> &lx,std::vector<long> &ux,int flag_verbose)
    {
      //      if(flag_verbose)
      //	std::cout << "verbose!" << std::endl;
      if(!Out)
	Out = &std::cout;
      //      else
      //	std::cout << "Out is set!" << std::endl;
      std::string fname(filename);
      if(!Sys::FILEEXISTS(fname)){
	fname.assign(filename+".fz");
	if(!Sys::FILEEXISTS(fname)){
	  fname.assign(filename+".gz");
	  if(!Sys::FILEEXISTS(fname)){
	    fname.assign(filename+".fits.fz");
	    if(!Sys::FILEEXISTS(fname)){
	      fname.assign(filename+".fits.gz");
	      if(!Sys::FILEEXISTS(fname)){
		*Out << "FitsTools::readImage: Could not find FITS file for " << filename << "." << std::endl;
		return(1);
	      }
	    }
	  }
	}
      }
      if(flag_verbose){
	*Out << "FitsTools::readImage: Found FITS file " << fname << "." << std::endl;
      }
      std::strcpy(this->_des->name,fname.c_str());
      rd_dessubimage(this->_des,&lx[0],&ux[0],READONLY,flag_verbose);
      char *zeroheader = NULL;
      int numzerokeys = 0;
      int status = 0;
      char *exclkey = NULL;
      int nexc = 0;
      int hdutype;
      if(flag_verbose)
	*Out << "FitsTools::readImage: Found " << this->_des->hdunum << " header units." << std::endl
	     << "FitsTools::readImage: Image unit = " << this->_des->unit << std::endl;
      for(int i = 1;i <= this->_des->hdunum;i++){
	if (fits_movabs_hdu(this->_des->fptr,i,&hdutype,&status)) {
	  // 	sprintf(event,"Move to HDU=1 failed: %s",bias.name);
	  // 	reportevt(flag_verbose,STATUS,5,event);
	  // 	printerror(status);
	  *Out << "FitsTools::readImage: Move to HDU= " << i << " failed: " << this->_des->name << std::endl;
	  return(1);
	}
	if(flag_verbose)
	  *Out << "FitsTools::readImage: Header(" << i << ") is of type " << hdutype << std::endl;
	if (fits_hdr2str(this->_des->fptr,0,this->exclist,this->nexcl,&zeroheader,&numzerokeys,&status)) {
	  *Out << "FitsTools::readFitsFile could not read header information from " 
		    << this->_des->name << "." << std::endl;
	  return(1);
	}
	std::string header_zero(zeroheader);
	std::ostringstream Ostr;
	for(int i = 0;i < numzerokeys;i++){
	  std::string header_record = header_zero.substr(i*80,80);
	  if(!header_record.empty() && (header_record.substr(0,3) != "END"))
	    Ostr << header_record << std::endl;
	}
	this->_Headers.push_back(Ostr.str());
	//	if(flag_verbose > 2) 
	//	  *Out << Ostr.str() << std::endl;
      }
      return(0);
    };
  };
  
  
  bool HeaderKeyExists(const std::string &header,const std::string &key){
    std::istringstream Istr(header);
    std::string line;
    while(std::getline(Istr,line)){
      std::string::size_type x = line.find("=");
      std::string keyword;
      std::istringstream Istr1(line.substr(0,x));
      Istr1 >> keyword;
      if(keyword == key){
	return(true);
      } 
    }
    return(false);
  }

  template<typename ValueType>  
  ValueType GetHeaderValue(const std::string &header,const std::string  &key)
  {
    ValueType retval;
    std::string line;
    std::istringstream Istr(header);
    while(std::getline(Istr,line)){
      std::string::size_type x = line.find("=");
      std::string keyword;
      std::istringstream Istr1(line.substr(0,x));
      Istr1 >> keyword;
      if(keyword == key){
	std::istringstream Istr2(line.substr(x+1));
	Istr2 >> retval;
	return(retval);
      }
    }
    return(retval);
  }

  template<typename ValueType>  
  int PutHeaderValue(std::string &header,const std::string &key,const ValueType &value)
  {
    int retval = 1;
    std::string line;
    std::ostringstream Ostr;
    std::ostringstream Hstr;
    std::istringstream Istr(header);
    while(std::getline(Istr,line)){
      std::string::size_type x = line.find("=");
      std::string keyword_real = line.substr(0,x);
      std::string::size_type y = line.find(" / ");
      std::string comment = line.substr(y+2);
      std::istringstream Istr2(keyword_real);
      std::string keyword;
      Istr2 >> keyword;
      if(keyword == key){
	Ostr << keyword_real <<  "= " << std::right << std::setw(20) << value << " / " << comment;
	line.assign(Ostr.str());
	retval = 0;
      }
      Hstr << line << std::endl;
    }
    header.assign(Hstr.str());
    return(retval);
  }
};

#endif
