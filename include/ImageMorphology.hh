#ifndef __IMAGE_MORPHOLOGY__
#define __IMAGE_MORPHOLOGY__
///
/// \file 
/// \brief Morphology namespace main header
/// \author Mike Campbell (mtcampbe@illinois.edu)
///
/// Interface definitions for morphology-like operations 
#include <vector>
#include <list>
#include <sstream>
#include <string>
//#include <cmath>
#include <limits>
#include <cassert>
#include <iostream>

//#include "Image.hh"
#include "NRFuncs.hh"

#ifndef _Image_HH_
#define _Image_HH_
/// \namespace Image
/// \brief Image namespace specification
///
/// Quick and dirty convenience image-specific constructs
namespace Image {

  enum { IMMIN, IMMAX, IMMEAN, IMSIGMA, IMMEDIAN, IMMODE, IMFWHM };
  class Stats : public std::vector<double>
  {
  public:
    Stats(){ this->resize(7); };
  };

};
#endif

/// 
/// \namespace Morph
/// \brief Morphology-type operations with images and bitmasks
///
/// This namespace holds all the operations and helper functions
/// for morphological-like processing and filtering.
///
namespace Morph {
  typedef float ImageDataType;
  typedef short MaskDataType;
  typedef long  IndexType;
  typedef Image::Stats StatType;
  typedef std::vector<Morph::IndexType> BlobType; 
  typedef std::vector<Morph::IndexType> BoxType;
  typedef std::pair<int,int> ImageOffsetType;
  typedef std::vector<Morph::BlobType> BlobsType;

  class StructuringElement : public std::vector<Morph::ImageOffsetType>
  {
  public:
    inline int Xoffset(const Morph::IndexType &index) const { return((*this)[index].first);  };
    inline int Yoffset(const Morph::IndexType &index) const { return((*this)[index].second); };
    Morph::IndexType NPix() const { return(this->size()); };
    int MinXOffset() const
    {
      int xmin = 0;
      std::vector<Morph::ImageOffsetType>::const_iterator it = this->begin();
      while(it != this->end()){
	if(it->first < xmin) xmin = it->first;
	it++;
      }
      return(xmin);
    };
    int MaxXOffset() const
    {
      int xmax = 0;
      std::vector<Morph::ImageOffsetType>::const_iterator it = this->begin();
      while(it != this->end()){
	if(it->first > xmax) xmax = it->first;
	it++;
      }
      return(xmax);
    };
    int MinYOffset() const
    {
      int ymin = 0;
      std::vector<Morph::ImageOffsetType>::const_iterator it = this->begin();
      while(it != this->end()){
	if(it->second < ymin) ymin = it->second;
	it++;
      }
      return(ymin);
    };
    int MaxYOffset() const
    {
      int ymax = 0;
      std::vector<Morph::ImageOffsetType>::const_iterator it = this->begin();
      while(it != this->end()){
	if(it->first > ymax) ymax = it->second;
	it++;
      }
      return(ymax);
    };
  };  

  typedef StructuringElement StructuringElementType;
 
  class ImageFilter
  {
  public:
    ImageFilter() : InputImageBuffer(NULL), InputMaskBuffer(NULL), 
		    OutputImageBuffer(NULL), OutputMaskBuffer(NULL),
		    ReferenceMaskBuffer(NULL)
    {
      input_image_buffer_is_mine  = false;
      input_mask_buffer_is_mine   = false;
      output_mask_buffer_is_mine  = false;
      output_image_buffer_is_mine = false;
    };
    virtual void SetInputImageData(Morph::ImageDataType *InputImageData)  
    {
      if(InputImageBuffer && input_image_buffer_is_mine)
	delete [] InputImageBuffer;
      input_image_buffer_is_mine = false;
      InputImageBuffer  = InputImageData; 
    };
    virtual void SetOutputImageData(Morph::ImageDataType *InputImageData) 
    {
      if(OutputImageBuffer && output_image_buffer_is_mine)
	delete [] OutputImageBuffer;
      output_image_buffer_is_mine = false;
      OutputImageBuffer = InputImageData; 
    };
    virtual void SetInputMaskData(Morph::MaskDataType *InputImageData)   
    { 
      if(InputMaskBuffer && input_mask_buffer_is_mine)
	delete [] InputMaskBuffer;
      input_mask_buffer_is_mine = false;
      InputMaskBuffer   = InputImageData; 
    };
    virtual void SetReferenceMaskData(Morph::MaskDataType *InputImageData)   
    { 
      ReferenceMaskBuffer   = InputImageData; 
    };
    virtual void SetOutputMaskData(Morph::MaskDataType *InputImageData)  
    { 
      if(OutputMaskBuffer && output_mask_buffer_is_mine)
	delete [] OutputMaskBuffer;
      output_mask_buffer_is_mine = false;
      OutputMaskBuffer  = InputImageData; 
    };
    virtual void Initialize()
    {
      if(nx > 0)
	ny = npix/nx;
      if(!InputImageBuffer && (npix > 0)) {
	InputImageBuffer = new Morph::ImageDataType [npix];
	if(!OutputImageBuffer)
	  OutputImageBuffer = InputImageBuffer;
	input_image_buffer_is_mine = true;
      }
      if(!InputMaskBuffer && (npix > 0)) {
	InputMaskBuffer = new Morph::MaskDataType [npix];
	if(!OutputMaskBuffer)
	  OutputMaskBuffer = InputMaskBuffer;
	input_mask_buffer_is_mine = true;
      }
      if(!OutputMaskBuffer && (npix > 0)){
	OutputMaskBuffer = new Morph::MaskDataType [npix];
	output_mask_buffer_is_mine = true;
      }
      if(!OutputImageBuffer && (npix > 0)){
	OutputImageBuffer = new Morph::ImageDataType [npix];
	output_image_buffer_is_mine = true;
      }
    };
    virtual void Initialize(Morph::ImageDataType *InputImageData,Morph::MaskDataType *InputMaskData)
    {
      Destroy();
      InputImageBuffer = InputImageData;
      InputMaskBuffer  = InputMaskData;
      Initialize();
    };
    virtual Morph::IndexType NumberOfPixelsInX() const { return(nx);      };
    virtual Morph::IndexType NumberOfPixels()    const { return(npix);    };
    virtual Morph::IndexType NumberOfPixelsInY() const { return(ny);      };
    virtual void SetProcessingRejectionMask(Morph::MaskDataType InputMaskData) { ProcessingRejectionMask = InputMaskData; };
    virtual void SetProcessingAcceptMask(Morph::MaskDataType InputMaskData)    { ProcessingAcceptMask    = InputMaskData; };
    virtual void SetReferenceMask(Morph::MaskDataType InputMaskData)           { ReferenceMask           = InputMaskData; };
    virtual void SetDataRejectionMask(Morph::MaskDataType InputMaskData)       { DataRejectionMask       = InputMaskData; };
    virtual void SetDataAcceptMask(Morph::MaskDataType InputMaskData)          { DataAcceptMask          = InputMaskData; };
    virtual void SetNumberOfPixelsInX(Morph::IndexType xsize)                  { nx                      = xsize;         };
    virtual void SetNumberOfPixels(Morph::IndexType input_npix)                { npix                    = input_npix;    };
    inline virtual Morph::ImageDataType &PixelValue(Morph::IndexType pixel_index) 
    { return(InputImageBuffer[pixel_index]); };
    inline virtual Morph::ImageDataType PixelValue(Morph::IndexType pixel_index) const
    { return(InputImageBuffer[pixel_index]); };
    inline virtual Morph::MaskDataType &MaskValue(Morph::IndexType pixel_index) 
    { return(InputMaskBuffer[pixel_index]); };
    inline virtual Morph::MaskDataType MaskValue(Morph::IndexType pixel_index) const 
    { return(InputMaskBuffer[pixel_index]); };
    inline virtual bool PixelIsFlagged(Morph::IndexType pixel_index,Morph::MaskDataType maskval) const
    { return(InputMaskBuffer[pixel_index]&maskval); };
    inline virtual bool ReferenceIsFlagged(Morph::IndexType pixel_index) const
    { if(ReferenceMaskBuffer){
	return(ReferenceMaskBuffer[pixel_index]&ReferenceMask);
      } else {
	return(false); 
      }
    };
    inline void FlagPixel(Morph::IndexType pixel_index,Morph::MaskDataType maskval) 
    { OutputMaskBuffer[pixel_index] |= maskval; };
    inline void SetOutputPixelValue(Morph::IndexType pixel_index,Morph::ImageDataType pixval)
    { OutputImageBuffer[pixel_index] = pixval; };
    inline virtual bool PixelShouldBeProcessed(Morph::IndexType pixel_index)
    {
      assert(pixel_index < npix);
      return((!(InputMaskBuffer[pixel_index]&ProcessingRejectionMask)) || 
	     InputMaskBuffer[pixel_index]&ProcessingAcceptMask);
    };
    inline virtual bool PixelDataIsValid(Morph::IndexType pixel_index)
    {
      assert(pixel_index < npix);
      return((!(InputMaskBuffer[pixel_index]&DataRejectionMask)) || 
	     InputMaskBuffer[pixel_index]&DataAcceptMask);
    };
    inline int ResolveAbsoluteOffset(const Morph::ImageOffsetType &component_offset)
    {
      return(component_offset.second*nx + component_offset.first);
    };
    virtual void Destroy()
    {
      input_mask_buffer_is_mine    = false;
      output_mask_buffer_is_mine   = false;
      input_image_buffer_is_mine   = false;
      output_image_buffer_is_mine  = false;
      if(InputImageBuffer && input_image_buffer_is_mine)
	delete [] InputImageBuffer;
      if(OutputImageBuffer && output_image_buffer_is_mine)
	delete [] OutputImageBuffer;
      if(InputMaskBuffer && input_mask_buffer_is_mine)
	delete [] InputMaskBuffer;
      if(OutputMaskBuffer && output_mask_buffer_is_mine)
	delete [] OutputMaskBuffer;
      InputImageBuffer  = NULL;
      OutputImageBuffer = NULL;
      InputMaskBuffer   = NULL;
      OutputMaskBuffer  = NULL;
    };
    virtual ~ImageFilter() { Destroy(); };
  protected:
    Morph::ImageDataType *InputImageBuffer;
    Morph::MaskDataType  *InputMaskBuffer;
    Morph::ImageDataType *OutputImageBuffer;
    Morph::MaskDataType  *OutputMaskBuffer;
    Morph::MaskDataType  *ReferenceMaskBuffer;
    Morph::MaskDataType   ProcessingRejectionMask;
    Morph::MaskDataType   ProcessingAcceptMask;
    Morph::MaskDataType   DataRejectionMask;
    Morph::MaskDataType   DataAcceptMask;
    Morph::MaskDataType   ReferenceMask;
    Morph::IndexType      nx;
    Morph::IndexType      ny;
    Morph::IndexType      npix;
  private:
    bool input_image_buffer_is_mine;
    bool input_mask_buffer_is_mine;
    bool output_image_buffer_is_mine;
    bool output_mask_buffer_is_mine;
  };



  ///
  /// \brief Detect and mask bad pixel values
  /// \param image Pointer to image data
  /// \param mask Pointer to mask image
  /// \param Nx Number of pixels in X
  /// \param Ny Number of pixels in Y
  /// \param bad_pixel_mask Mask value to set for bad pixels
  ///
  /// This function checks an image for nans and infs and 
  /// sets the mask flag for detected pixels. The number of
  /// pixels flagged is returned.
  ///
  int MaskBadPixelData(Morph::ImageDataType *image,
		       Morph::MaskDataType *mask,
		       Morph::IndexType Nx,Morph::IndexType Ny,
		       Morph::MaskDataType bad_pixel_mask);

  ///
  /// \brief Basic erosion
  /// \param mask Pointer to mask image
  /// \param Nx Number of pixels in X
  /// \param Ny Number of pixels in Y
  /// \param structuring_element Structuring element for erosion
  /// \param BitMask The bitmask indicating which bits to erode
  ///
  /// This function takes a structuring element (the structuring 
  /// element is just a sorted list of offsets to the pixels included
  /// in the processing) - and performs an erosion wherein any mask pixel
  /// with the BitMask bits turned on, will turn off if any other pixel 
  /// inside the structuring element has BitMask turned off.
  /// If the square structuring element (i.e. one that includes just the 
  /// 8 neighbors of the given pixel) is used, then erosion will 
  /// erode away at the borders of "globs" of regions where BitMask is "on".
  ///
  void ErodeMask(Morph::MaskDataType *mask,
		 Morph::IndexType Nx,
		 Morph::IndexType Ny,
		 std::vector<Morph::IndexType> &structuring_element,
		 Morph::MaskDataType BitMask);

  ///
  /// \brief Basic dilation
  /// \param mask Pointer to bitmask data
  /// \param Nx Mask size in X
  /// \param Ny Mask size in Y
  /// \param structuring_element Structuring element for erosion
  /// \param BitMask The bitmask indicating which bits to dilate
  ///
  /// This function takes a structuring element (the structuring 
  /// element is just a sorted list of offsets to the pixels included
  /// in the processing) - and performs a dilation wherein any mask pixel
  /// with the BitMask bits turned off, will turn on if any other pixel 
  /// inside the structuring element has BitMask turned on.
  /// If the square structuring element (i.e. one that includes just the 
  /// 8 neighbors of the given pixel) is used, then dilation will 
  /// cause globs of regions where BitMask is "on" to grow.
  ///
  void DilateMask(Morph::MaskDataType *mask,
		  Morph::IndexType Nx,
		  Morph::IndexType Ny,
		  std::vector<Morph::IndexType> &structuring_element,
		  Morph::MaskDataType BitMask);
  

  ///
  /// \brief Basic dilation in the X-direction.
  /// \param mask Pointer to bitmask data
  /// \param Nx Mask size in X
  /// \param Ny Mask size in Y
  /// \param structuring_element Structuring element for erosion
  /// \param length_check size over which search is made for long trails
  /// \param BitMask The bitmask indicating which bits to dilate
  ///
  /// This function takes a structuring element (the structuring 
  /// element is just a sorted list of offsets to the pixels included
  /// in the processing) - and performs a dilation wherein any mask pixel
  /// with the BitMask bits turned off, will turn on if any other pixel 
  /// inside the structuring element has BitMask turned on.  
  ///
  /// A second requirement has been added that requires that a long structure
  /// in the Y-direction also need to be present (adjacent) before the dilation
  /// will occur.
  ///
  void DilateMaskX(Morph::MaskDataType *mask,
		   Morph::IndexType Nx,
		   Morph::IndexType Ny,
		   std::vector<Morph::IndexType> &structuring_element,
		   Morph::IndexType length_check,
		   Morph::MaskDataType BitMask);


  ///
  /// \brief Get basic information about a structuring element
  /// \param structuring_element Input structuring element
  /// \param Nx Size of image on which the S.E. will operate
  /// \param npix_struct Output number of pixels in structuring element
  /// \param border_y_minus Output the left border size in rows  
  /// \param border_y_plus Output the right border size in rows
  /// \param border_x_minus Output the left border size in columns
  /// \param border_x_plus Output the right border size in columns
  /// 
  /// This function takes a structuring elements and determines how many pixels
  /// it includes, and how many columns and rows it covers. These numbers 
  /// indicate where image boundaries intersect with the structuring element, 
  /// so that loops over pixels can treat those parts of the image specially.
  ///
  void GetSEAttributes(std::vector<Morph::IndexType> &structuring_element,
		       Morph::IndexType Nx,Morph::IndexType &npix_struct,
		       Morph::IndexType &border_y_minus,
		       Morph::IndexType &border_y_plus,
		       Morph::IndexType &border_x_minus,
		       Morph::IndexType &border_x_plus);

  ///
  /// \brief Get basic statistics inside structuring element
  /// \param image Pointer to image data
  /// \param mask Pointer to mask data
  /// \param index Indicates pixel being processed (i.e. "the center")
  /// \param structuring_element Structuring element indicating offsets
  /// \param RejectionMask Do not use pixels with bits from this mask
  /// \param ExceptionMask Force exception to rejection for pixels with this
  /// \param min Output minimum pixel value inside the structuring element
  /// \param max Output maximum pixel value inside the structuring element
  /// \param mean Output the mean of pixel value inside the structuring element
  /// \param sigma Output sigma of pixel values inside structuring element
  /// \param npix Output number of pixels used in statistics
  ///
  /// This function retrieves the min,max,mean, and standard deviation for the 
  /// pixel values in the structuring element.  Any pixel (except those with
  /// the ExceptionMask set) with the RejectionMask set is not used.
  ///
  /// \note Right now, this has no input validation; but it gets called a
  ///       bazillion times, so validation would take significant cycles
  /// 
  inline void SEStats(Morph::ImageDataType *image,
		      Morph::MaskDataType *mask,
		      Morph::IndexType index,
		      std::vector<Morph::IndexType> &structuring_element,
		      Morph::MaskDataType RejectionMask,
		      Morph::MaskDataType ExceptionMask,
		      Morph::ImageDataType &min,Morph::ImageDataType &max,
		      Morph::ImageDataType &mean,Morph::ImageDataType &sigma,
		      Morph::IndexType     &npix,
		      Morph::ImageDataType rejection_value,
		      Morph::IndexType     &npix_reject)
  {
    min   =  std::numeric_limits<Morph::ImageDataType>::max();
    max   = -std::numeric_limits<Morph::ImageDataType>::max();
    npix  = 0;
    mean  = 0;
    sigma = 0;
    npix_reject = 0;
    std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
    while(selIt != structuring_element.end()){
      Morph::IndexType sindex = index + *selIt++;
      if(!(mask[sindex]&RejectionMask) || (mask[sindex]&ExceptionMask)){
	if(image[sindex] < rejection_value){
	  mean  += image[sindex];
	  sigma += (image[sindex]*image[sindex]);
	  npix++;
	  if(image[sindex] > max) max = image[sindex];
	  if(image[sindex] < min) min = image[sindex];
	}
	else
	  npix_reject++;
      }
      //      else{
      //	std::cout << "Mask test failed: mask = " << mask[sindex] << std::endl;
      //      }
    }
    if(npix > 0){
      mean /= npix;
      sigma = std::sqrt(sigma/npix - mean*mean);
    }
  }; 

  ///
  /// \brief Get basic statistics inside structuring element
  /// \param image Pointer to image data
  /// \param mask Pointer to mask data
  /// \param index Indicates pixel being processed (i.e. "the center")
  /// \param structuring_element Structuring element indicating offsets
  /// \param RejectionMask Do not use pixels with bits from this mask
  /// \param ExceptionMask Force exception to rejection for pixels with this
  /// \param min Output minimum pixel value inside the structuring element
  /// \param max Output maximum pixel value inside the structuring element
  /// \param mean Output the mean of pixel value inside the structuring element
  /// \param sigma Output sigma of pixel values inside structuring element
  /// \param npix Output number of pixels used in statistics
  ///
  /// This function retrieves the min,max,mean, and standard deviation for the 
  /// pixel values in the structuring element.  Any pixel (except those with
  /// the ExceptionMask set) with the RejectionMask set is not used.
  ///
  /// \note Right now, this has no input validation; but it gets called a
  ///       bazillion times, so validation would take significant cycles
  /// 
  inline void SEStats(Morph::ImageDataType *image,
		      Morph::MaskDataType *mask,
		      Morph::IndexType index,
		      std::vector<Morph::IndexType> &structuring_element,
		      Morph::MaskDataType RejectionMask,
		      Morph::MaskDataType ExceptionMask,
		      Morph::ImageDataType &min,Morph::ImageDataType &max,
		      Morph::ImageDataType &mean,Morph::ImageDataType &sigma,
		      Morph::IndexType     &npix)
  {
    min   =  std::numeric_limits<Morph::ImageDataType>::max();
    max   = -std::numeric_limits<Morph::ImageDataType>::max();
    npix  = 0;
    mean  = 0;
    sigma = 0;
    std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
    while(selIt != structuring_element.end()){
      Morph::IndexType sindex = index + *selIt++;
      if(!(mask[sindex]&RejectionMask) || (mask[sindex]&ExceptionMask)){
	mean  += image[sindex];
	sigma += (image[sindex]*image[sindex]);
	npix++;
	if(image[sindex] > max) max = image[sindex];
	if(image[sindex] < min) min = image[sindex];
      }
      //      else{
      //	std::cout << "Mask test failed: mask = " << mask[sindex] << std::endl;
      //      }
    }
    if(npix > 0){
      mean /= npix;
      sigma = std::sqrt(sigma/npix - mean*mean);
    }
  }; 

  inline void BoxStats(Morph::ImageDataType *image,
		       Morph::MaskDataType *mask,
		       Morph::IndexType Nx,Morph::IndexType Ny,
		       std::vector<Morph::IndexType> &box,
		       Morph::MaskDataType RejectionMask,
		       Morph::MaskDataType ExceptionMask,
		       Morph::StatType &stats,
		       Morph::IndexType &npix)
  {
    stats[Image::IMMIN]   = std::numeric_limits<Morph::ImageDataType>::max();
    stats[Image::IMMAX]   = -std::numeric_limits<Morph::ImageDataType>::max();
    stats[Image::IMMEAN]  = 0;
    stats[Image::IMSIGMA] = 0;
    npix = 0;
    for(Morph::IndexType y = box[2];y <= box[3];y++){
      for(Morph::IndexType x = box[0];x <= box[1];x++){
	Morph::IndexType sindex = y*Nx + x;
	if(!(mask[sindex]&RejectionMask) || (mask[sindex]&ExceptionMask)){
	  stats[Image::IMMEAN]  += image[sindex];
	  stats[Image::IMSIGMA] += (image[sindex]*image[sindex]);
	  npix++;
	  if(image[sindex] > stats[Image::IMMAX]) stats[Image::IMMAX] = image[sindex];
	  if(image[sindex] < stats[Image::IMMIN]) stats[Image::IMMIN] = image[sindex];
	}
      }
    }
    if(npix > 0){
      stats[Image::IMMEAN]  /= npix;
      stats[Image::IMSIGMA]  = std::sqrt(stats[Image::IMSIGMA]/npix - 
					  stats[Image::IMMEAN]*stats[Image::IMMEAN]);
    }
  }; 


  inline void BoxStats(Morph::ImageDataType *image,
		       Morph::MaskDataType *mask,
		       Morph::IndexType Nx,Morph::IndexType Ny,
		       std::vector<Morph::IndexType> &box,
		       Morph::MaskDataType RejectionMask,
		       Morph::MaskDataType ExceptionMask,
		       Morph::ImageDataType min_rejection_value,
		       Morph::ImageDataType max_rejection_value,
		       Morph::StatType &stats,
		       Morph::IndexType &npix)
  {
    stats[Image::IMMIN]   = std::numeric_limits<double>::max();
    stats[Image::IMMAX]   = -std::numeric_limits<double>::max();
    stats[Image::IMMEAN]  = 0;
    stats[Image::IMSIGMA] = 0;
    npix = 0;
    for(Morph::IndexType y = box[2];y <= box[3];y++){
      for(Morph::IndexType x = box[0];x <= box[1];x++){
	Morph::IndexType sindex = y*Nx + x;
	if(!(mask[sindex]&RejectionMask) || (mask[sindex]&ExceptionMask)){
	  if((image[sindex] < max_rejection_value) && (image[sindex] > min_rejection_value)){ 
	    stats[Image::IMMEAN]  += image[sindex];
	    stats[Image::IMSIGMA] += (image[sindex]*image[sindex]);
	    npix++;
	    if(image[sindex] > stats[Image::IMMAX]) stats[Image::IMMAX] = image[sindex];
	    if(image[sindex] < stats[Image::IMMIN]) stats[Image::IMMIN] = image[sindex];
	  }
	}
      }
    }
    if(npix > 0){
      stats[Image::IMMEAN]  /= npix;
      stats[Image::IMSIGMA]  = std::sqrt(stats[Image::IMSIGMA]/npix - 
					 stats[Image::IMMEAN]*stats[Image::IMMEAN]);
    }
  }; 

  int GetSky(Morph::ImageDataType *image,Morph::MaskDataType *mask, Morph::IndexType Nx, Morph::IndexType Ny,
	     Morph::IndexType minpix,Morph::IndexType maxiter,double ground_rejection_factor,double tol,
	     Morph::MaskDataType RejectionMask, Morph::MaskDataType AcceptMask,
	     Morph::StatType &image_stats,Morph::IndexType &npix,Morph::IndexType &niter,std::ostream *OStr=NULL);
  int GetSkyBox(Morph::ImageDataType *image,Morph::MaskDataType *mask, Morph::BoxType &box,Morph::IndexType Nx, 
		Morph::IndexType Ny,Morph::IndexType minpix,Morph::IndexType maxiter,double ground_rejection_factor,double tol,
		Morph::MaskDataType RejectionMask, Morph::MaskDataType AcceptMask,
		Morph::StatType &image_stats,Morph::IndexType &npix,Morph::IndexType &niter,std::ostream *OStr=NULL);
  
  ///
  /// \brief Trim structuring element for image borders
  /// \param index Indicates pixel being processed (i.e. "the center")
  /// \param npix  Indicates number of pixels in image
  /// \param structuring_element Specifies the master structuring element
  /// \param trimmed_sel Output the trimmed structuring element
  ///
  /// Convenience function used near image borders to trim the structuring
  /// element so that it can be used with other utilities which count on
  /// every index in the structuring element to be good.
  ///
  inline void 
  TrimStructuringElement(Morph::IndexType &index,
			 Morph::IndexType Nx,
			 Morph::IndexType Ny,
			 std::vector<Morph::IndexType> &structuring_element,
			 std::vector<Morph::IndexType> &trimmed_sel)
  {
    trimmed_sel.resize(0);
    trimmed_sel.reserve(structuring_element.size());
    std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
    while(selIt != structuring_element.end()){
      Morph::IndexType offset = *selIt++;
      Morph::IndexType xoffset = offset%Nx;
      Morph::IndexType yoffset = offset/Nx;
      Morph::IndexType cxoffset = index%Nx;
      Morph::IndexType cyoffset = index/Nx;
      Morph::IndexType newx = cxoffset + xoffset;
      Morph::IndexType newy = cyoffset + yoffset; 
      if((newx >= 0) && (newx < Nx) &&
	 (newy >= 0) && (newy < Ny))
	trimmed_sel.push_back(offset);
	//      Morph::IndexType sindex = index + offset;
	//      if(sindex >= 0 && sindex < npix)
	//	trimmed_sel.push_back(offset);
    }
  };

  ///
  /// \brief Create blobs of pixels with matching bitmask
  /// \param mask Pointer to mask data
  /// \param Nx  Indicates number of pixels in X
  /// \param Ny  Indicates number of pixels in Y
  /// \param BlobMask Bitmask for accepting into blob
  /// \param blob_image Output image indicates blob index for each pixel (0 = not part of any blob)
  /// \param blobs Output vector of blobs wherein each is a vector of pixels indices contained in the blob
  ///
  /// GetBlobs creates blobs of touching pixels whos bitmask match the
  /// specified BlobMask.
  ///
  void GetBlobs(Morph::MaskDataType *mask,
		Morph::IndexType Nx,Morph::IndexType Ny,
		Morph::MaskDataType BlobMask,
		std::vector<Morph::IndexType> &blob_image,
		std::vector<std::vector<Morph::IndexType> > &blobs);

  ///
  /// \brief Create blobs of pixels with matching bitmask and not matching reject mask
  /// \param mask Pointer to mask data
  /// \param Nx  Indicates number of pixels in X
  /// \param Ny  Indicates number of pixels in Y
  /// \param BlobMask Bitmask for accepting into blob
  /// \param RejectMask Bitmask for rejecting pixel
  /// \param AcceptMask Bitmask for accepting pixel
  /// \param blob_image Output image indicates blob index for each pixel (0 = not part of any blob)
  /// \param blobs Output vector of blobs wherein each is a vector of pixels indices contained in the blob
  ///
  /// GetBlobs creates blobs of touching pixels whos bitmask match the
  /// specified BlobMask.
  ///
  void GetBlobsWithRejection(Morph::MaskDataType *mask,
			     Morph::IndexType Nx,Morph::IndexType Ny,
			     Morph::MaskDataType BlobMask,
			     Morph::MaskDataType RejectMask,
			     Morph::MaskDataType AcceptMask,
			     std::vector<Morph::IndexType> &blob_image,
			     std::vector<std::vector<Morph::IndexType> > &blobs);

  void GetBlobBoundingBox(std::vector<Morph::IndexType> &blob,
			  Morph::IndexType Nx,Morph::IndexType Ny,
			  std::vector<Morph::IndexType> &box);



  class ImageErodeOp {
  private:
    //    std::vector<Morph::BlobType> &_blobs;
    Morph::StatType              &_stats;
    Morph::MaskDataType          _BitMask;
    Morph::MaskDataType          _PixelRejMask;
    Morph::MaskDataType          _PixelAcceptMask;
    float                        _scalefactor;
  public:
    ImageErodeOp(Morph::StatType     &stats,
		 Morph::MaskDataType BitMask,
		 Morph::MaskDataType PixelRejMask,
		 Morph::MaskDataType PixelAcceptMask,
		 float scalefactor) :
      _stats(stats), _BitMask(BitMask), 
      _PixelRejMask(PixelRejMask), _PixelAcceptMask(PixelAcceptMask),
      _scalefactor(scalefactor) {};
    inline void Morph(std::vector<Morph::ImageDataType> &image, 
		      std::vector<Morph::MaskDataType>  &input_mask,
		      std::vector<Morph::MaskDataType>  &output_mask,
		      Morph::IndexType Nx, Morph::IndexType Ny,
		      //		      Morph::IndexType blob_index, 
		      Morph::IndexType pixel,
		      std::vector<Morph::IndexType> trimmed_structuring_element){
      long seed = -17;
      if((input_mask[pixel]&_BitMask) && (output_mask[pixel]&_BitMask)){
	std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	while(selIt != trimmed_structuring_element.end() && (output_mask[pixel]&_BitMask)){
	  Morph::IndexType ind = pixel + *selIt++;
	  if(!(input_mask[ind]&_BitMask) && (!(input_mask[ind]&_PixelRejMask) ||
					     (input_mask[ind]&_PixelAcceptMask)) &&
	     (image[ind] < _stats[Image::IMMEAN] + 
	      _scalefactor*_stats[Image::IMSIGMA])){
	    output_mask[pixel] ^= _BitMask;
	    input_mask[pixel] |= _PixelAcceptMask;
	    image[pixel] = image[ind] + mygasdev(&seed)*_stats[Image::IMSIGMA];
	  }
	}
      }
    }    
  };

  class ImageDilateBlobOp {
  private:
    std::vector<Morph::BlobType> &_blobs;
    std::vector<Morph::StatType> &_stats;
    Morph::MaskDataType          _BitMask;
    Morph::MaskDataType          _PixelRejMask;
    float                        _scalefactor;
  public:
    ImageDilateBlobOp(std::vector<Morph::BlobType> &blobs,
		      std::vector<Morph::StatType> &stats,
		      Morph::MaskDataType BitMask,
		      Morph::MaskDataType PixelRejMask,
		     float scalefactor) :
      _blobs(blobs), _stats(stats), _BitMask(BitMask), 
      _PixelRejMask(PixelRejMask), _scalefactor(scalefactor) {};
    inline void Morph(Morph::ImageDataType *image, Morph::MaskDataType *input_mask,
		      Morph::MaskDataType *output_mask,Morph::IndexType Nx, Morph::IndexType Ny,
		      Morph::IndexType blob_index, Morph::IndexType pixel,
		      std::vector<Morph::IndexType> trimmed_structuring_element){
      if(!(input_mask[pixel]&_BitMask) && !(output_mask[pixel]&_BitMask)){
	std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	while(selIt != trimmed_structuring_element.end() && !(output_mask[pixel]&_BitMask)){
	  Morph::IndexType ind = pixel + *selIt++;
	  if(!(input_mask[ind]&_BitMask) && !(input_mask[ind]&_PixelRejMask) &&
	     (image[ind] < _stats[blob_index][Image::IMMEAN] + 
	      _scalefactor*_stats[blob_index][Image::IMSIGMA])){
	    output_mask[pixel] ^= _BitMask;
	  }
	}
      }    
    }
  };
  
  ///
  /// \brief Image erosion template
  /// \param mask Pointer to mask image
  /// \param Nx Number of pixels in X
  /// \param Ny Number of pixels in Y
  /// \param structuring_element Structuring element for erosion
  /// \param BitMask The bitmask indicating which bits to erode
  ///
  /// This function takes a structuring element (the structuring 
  /// element is just a sorted list of offsets to the pixels included
  /// in the processing) - and performs an erosion wherein any mask pixel
  /// with the BitMask bits turned on, will turn off if any other pixel 
  /// inside the structuring element has BitMask turned off.
  /// If the square structuring element (i.e. one that includes just the 
  /// 8 neighbors of the given pixel) is used, then erosion will 
  /// erode away at the borders of "globs" of regions where BitMask is "on".
  ///
  template<typename MorphOpType>
  void ImageMorphBlob(std::vector<Morph::ImageDataType> &image,
		      std::vector<Morph::MaskDataType>  &mask,
		      Morph::BlobType blob,
		      Morph::IndexType Nx,
		      Morph::IndexType Ny,
		      std::vector<Morph::IndexType> &structuring_element,
		      MorphOpType &MorphOp)
  {
    // Set up by getting image size, copying current mask, and determining borders 
    // for the structuring element to speed up the loops.
    //    Morph::IndexType npixels = Nx*Ny;
    std::vector<Morph::IndexType> trimmed_structuring_element;
    std::vector<Morph::MaskDataType> temp_mask(mask.begin(),mask.end());
    //    std::vector<Morph::BlobType>::iterator bbi = blobs.begin();
    //    while(bbi != blobs.end()){
    //      Morph::IndexType blob_index = bbi-blobs.begin();
    //      Morph::BlobType &blob = *bbi++;
    Morph::BlobType::iterator bi = blob.begin();
    while(bi != blob.end()){
      Morph::IndexType pixel_index = *bi++;
      Morph::TrimStructuringElement(pixel_index,Nx,Ny,structuring_element,trimmed_structuring_element);
      MorphOp.Morph(image,temp_mask,mask,Nx,Ny,pixel_index,trimmed_structuring_element);
    }
  };
  //  };
  
  // Quick utility to grab pixel values and distances for short linear interpolation
  //
  // 1D Search in X and Y for the closest valid pixel whos mask is not in RejectMask
  // or is in AcceptMask.  Record the value, and the distance (in pixels).
  // vals [(x-1) (x+1) (y-1) (y+1) 
  inline void ClosestValidValues(Morph::ImageDataType *image,
				 Morph::MaskDataType  *mask,
				 Morph::IndexType      Nx,
				 Morph::IndexType      Ny,
				 Morph::IndexType      index,
				 Morph::MaskDataType   RejectMask,
				 Morph::MaskDataType   AcceptMask,
				 std::vector<Morph::ImageDataType> &vals,
				 std::vector<Morph::IndexType>     &dist)
  {
    Morph::IndexType y = index/Nx;
    Morph::IndexType x = index%Nx;
    Morph::IndexType sx = x;
    Morph::IndexType sy = y;
    Morph::IndexType sdist = 0;
    bool search_done = false;
    while(!search_done){
      sx--;
      sdist++;
      if(sx < 0){
	dist[0] = 0;
	search_done = true;
      }
      else{
	Morph::IndexType sindex = y*Nx+sx;
	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
	  vals[0] = image[sindex];
	  dist[0] = sdist;
	  search_done = true;
	}
      }
    }
    search_done = false;
    sdist = 0;
    sx = x;
    while(!search_done){
      sx++;
      sdist++;
      if(sx >= Nx){
	dist[1] = 0;
	search_done = true;
      }
      else{
	Morph::IndexType sindex = y*Nx+sx;
	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
	  vals[1] = image[sindex];
	  dist[1] = sdist;
	  search_done = true;
	}
      }
    }
    search_done = false;
    sdist = 0;
    while(!search_done){
      sy++;
      sdist++;
      if(sy >= Ny){
	dist[2] = 0;
	search_done = true;
      }
      else{
	Morph::IndexType sindex = sy*Nx+x;
	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
	  vals[2] = image[sindex];
	  dist[2] = sdist;
	  search_done = true;
	}
      }
    }
    search_done = false;
    sdist = 0;
    while(!search_done){
      sy--;
      sdist++;
      if(sy < 0){
	dist[3] = 0;
	search_done = true;
      }
      else{
	Morph::IndexType sindex = sy*Nx+x;
	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
	  vals[3] = image[sindex];
	  dist[3] = sdist;
	  search_done = true;
	}
      }
    }
    
  };

  // Quick utility to grab pixel values and distances for short linear interpolation
  //
  // 1D Search in X and Y for the closest valid pixel whos mask is not in RejectMask
  // or is in AcceptMask.  Record the value, and the distance (in pixels).
  // vals [(x-1) (x+1) (y-1) (y+1) 
//   inline void ClosestValidValuesNDir(Morph::ImageDataType *image,
// 				     Morph::MaskDataType  *mask,
// 				     Morph::IndexType      Nx,
// 				     Morph::IndexType      Ny,
// 				     Morph::IndexType      index,
// 				     Morph::MaskDataType   RejectMask,
// 				     Morph::MaskDataType   AcceptMask,
// 				     std::vector<Morph::ImageDataType> &rvals,
// 				     std::vector<Morph::IndexType>     &rdist,
// 				     Morph::IndexType Nvals)
//   {
//     Morph::IndexType y = index/Nx;
//     Morph::IndexType x = index%Nx;
//     Morph::IndexType sx = x;
//     Morph::IndexType sy = y;
//     Morph::IndexType sdist = 0;
//     Morph::IndexType curin = 0;
//     bool search_done = false;
//     double aval = 0.0;
//     Morph::IndexType nvals = 0;
//     while(!search_done && (curin < Nvals)){
//       sx--;
//       sdist++;
//       if(sx < 0){
// 	while(curin < Nvals)
// 	  dist[curin++] = 0;
// 	search_done = true;
//       }
//       else{
// 	Morph::IndexType sindex = y*Nx+sx;
// 	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
// 	  vals[curin++] = image[sindex];
// 	  aval += image[sindex];
// 	  nvals++;
// 	  dist[curin++] = sdist;
// 	  //	  search_done = true;
// 	}
//       }
//     }
//     if(nvals > 0)
//       aval = aval/nvals;
//     rval[0] = aval;
//     rdist[0] = dist[0];
//     search_done = false;
//     sdist = 0;
//     curin = 0;
//     aval = 0;
//     nvals = 0;
//     while(!search_done && (curin < Nvals)){
//       sx++;
//       sdist++;
//       if(sx >= Nx){
// 	while(curin < Nvals)
// 	  dist[Nvals+curin++] = 0;
// 	search_done = true;
//       }
//       else{
// 	Morph::IndexType sindex = y*Nx+sx;
// 	if(!(mask[sindex]&RejectMask) || (mask[sindex]&AcceptMask)){
// 	  vals[Nvals+curin++] = image[sindex];
// 	  avals+=image[sindex];
// 	  dist[Nvals+curin++] = sdist;
// 	  //	  search_done = true;
// 	}
//       }
//     }
//     if(nvals > 0)
//       aval /= nvals;
//     rvals[1] = aval;
//     rdist[1] = dist[Nvals];
    
//   };
  void CollideBoxes(std::vector<Morph::BoxType>   &inboxes,
		    std::vector<Morph::BoxType>   &refboxes,
		    std::vector<Morph::IndexType> &collision_indices,
		    std::vector<Morph::BoxType>   &collided_boxes);
  bool BoxesCollide(Morph::BoxType &box1,Morph::BoxType &box2,Morph::BoxType &cbox);

  
  
  // Gives the average pixel value as a function of radius about the pixel at "index."
  // The distribution is returned in "energy" and the number of pixels at each given 
  // radius in "npix".  The return data is arranged like energy[ 0 1 2 3 4 ... radius ]
  //
  inline int RadialDistribution(Morph::ImageDataType *image,
				Morph::MaskDataType  *mask,
				Morph::IndexType      Nx,
				Morph::IndexType      Ny,
				Morph::IndexType      index,
				Morph::IndexType      radius,
				Morph::MaskDataType   RejectMask,
				Morph::MaskDataType   AcceptMask,
				std::vector<Morph::ImageDataType> &energy,
				std::vector<Morph::IndexType>     &npix) {
    Morph::IndexType X  = index%Nx;
    Morph::IndexType Y  = index/Nx;
    Morph::IndexType X0 = X - radius;
    Morph::IndexType X1 = X + radius;
    Morph::IndexType Y0 = Y - radius;
    Morph::IndexType Y1 = Y + radius;
    double R2 = static_cast<double>(radius*radius);;
    energy.resize(radius+1,0);
    npix.resize(radius+1,0);
    if(X0 < 0) X0 = 0;
    if(X1 >= Nx) X1 = Nx - 1;
    if(Y0 < 0) Y0 = 0;
    if(Y1 >= Ny) Y1 = Ny - 1;
    for(Morph::IndexType x = X0;x <= X1;x++){
      Morph::IndexType X2 = (x-X)*(x-X);
      for(Morph::IndexType y=Y0;y <= Y1;y++){
	Morph::IndexType ind = y*Nx + x;
	if(!(mask[ind]&RejectMask) || (mask[ind]&AcceptMask)){
	  Morph::IndexType Y2 = (y-Y)*(y-Y);
	  double dist = static_cast<double>(X2 + Y2);
	  if (dist <= R2){
	    dist = std::sqrt(dist);
	    Morph::IndexType bin = static_cast<Morph::IndexType>(dist+.5);
	    assert(bin <= radius);
	    energy[bin] += image[ind];
	    npix[bin]++;
	  }
	}
      }
    }
    std::vector<Morph::ImageDataType>::iterator ei = energy.begin();
    std::vector<Morph::IndexType>::iterator ii = npix.begin();
    while (ei != energy.end()){
      if(*ii > 0)
	*ei /= static_cast<Morph::ImageDataType>(*ii);
      ei++;
      ii++;
    }
    return(0);
  };

  // Simple level utility that sets a bit based on image values
  // exceeding set thresholds.
  inline void ThresholdImage(Morph::ImageDataType *image,
			     Morph::MaskDataType  *mask,
			     Morph::IndexType      Nx,
			     Morph::IndexType      Ny,
			     Morph::ImageDataType  low_level,
			     Morph::ImageDataType  high_level,
			     Morph::MaskDataType   LowBit,
			     Morph::MaskDataType   HighBit){

    Morph::IndexType npix = Nx*Ny;
    for(unsigned int i = 0;i < npix;i++){
      if(image[i] > high_level)
	mask[i] |= HighBit;
      if(image[i] < low_level)
	mask[i] |= LowBit;
    }
  };
};

#endif
