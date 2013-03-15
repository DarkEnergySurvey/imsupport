///
/// \file 
/// \brief Implementation of morphology-like image processing functions
/// \author Mike Campbell (mtcampbe@illinois.edu)
///
#include "ImageMorphology.hh"


namespace Morph {
  

  int MaskBadPixelData(Morph::ImageDataType *image,
		       Morph::MaskDataType *mask,
		       Morph::IndexType Nx,Morph::IndexType Ny,
		       Morph::MaskDataType bad_pixel_mask)
  {
    Morph::IndexType nbad_pix = 0;
    Morph::IndexType npix = Nx*Ny;
    for(Morph::IndexType pixi = 0;pixi < npix;pixi++)
      if(image[pixi] != image[pixi]){
	mask[pixi] |= bad_pixel_mask;
	nbad_pix++;
      }
    return(nbad_pix);
  }


  bool BoxesCollide(Morph::BoxType &box1,Morph::BoxType &box2,Morph::BoxType &cbox)
  {
    if(((box1[0] >= box2[0]) && (box1[0] <= box2[1])) ||
       ((box1[1] >= box2[0]) && (box1[1] <= box2[1]))){
      if(((box1[2] >= box2[2]) && (box1[2] <= box2[3])) ||
	 ((box1[3] >= box2[2]) && (box1[3] <= box2[3]))){
	cbox.resize(4);
	cbox[0] = (box1[0] > box2[0] ? box1[0] : box2[0]);
	cbox[1] = (box1[1] < box2[1] ? box1[1] : box2[1]);
	cbox[2] = (box1[2] > box2[2] ? box1[2] : box2[2]);
	cbox[3] = (box1[3] < box2[3] ? box1[3] : box2[3]);
	return(true);
      }
    }
    return(false);
  }

  void CollideBoxes(std::vector<Morph::BoxType>   &inboxes,
		    std::vector<Morph::BoxType>   &refboxes,
		    std::vector<Morph::IndexType> &collision_indices,
		    std::vector<Morph::BoxType>   &collided_boxes)
  {
    collision_indices.resize(0);
    collided_boxes.resize(0);
    std::vector<Morph::BoxType>::iterator ibi = inboxes.begin();
    while(ibi != inboxes.end()){
      Morph::BoxType &box = *ibi++;
      std::vector<Morph::BoxType>::iterator rbi = refboxes.begin();
      bool found = false;
      while(rbi != refboxes.end() && !found){
	Morph::BoxType &refbox = *rbi++;
	Morph::IndexType rbindex = rbi - refboxes.begin();
	Morph::BoxType collision_box;
	if(BoxesCollide(box,refbox,collision_box)){
	  found = true;
	  collision_indices.push_back(rbindex);
	  collided_boxes.push_back(collision_box);
	}
      }
      if(!found){
	collision_indices.push_back(0);
	Morph::BoxType collision_box(4,0);
	collided_boxes.push_back(collision_box);
      }
    }
  }

  void GetBlobBoundingBox(std::vector<Morph::IndexType> &blob,
			  Morph::IndexType Nx,Morph::IndexType Ny,
			  std::vector<Morph::IndexType> &box)
  {
    box.resize(4,0);
    box[0] = Nx;
    box[2] = Ny;
    std::vector<Morph::IndexType>::iterator bi = blob.begin();
    while(bi != blob.end()){
      Morph::IndexType index = *bi++;
      Morph::IndexType x = index%Nx;
      Morph::IndexType y = index/Nx;
      if(x < box[0]) box[0] = x;
      if(x > box[1]) box[1] = x;
      if(y < box[2]) box[2] = y;
      if(y > box[3]) box[3] = y;
    }
  };


  void GetSEAttributes(std::vector<Morph::IndexType> &structuring_element,
		       Morph::IndexType Nx,Morph::IndexType &npix_struct,
		       Morph::IndexType &border_y_minus,
		       Morph::IndexType &border_y_plus,
		       Morph::IndexType &border_x_minus,
		       Morph::IndexType &border_x_plus)
  {
    npix_struct = structuring_element.size();
    Morph::IndexType border = structuring_element[0]/Nx;
    border_y_minus = (border > 0 ? border : -border);
    border = structuring_element[npix_struct-1]/Nx;
    border_y_plus  = (border > 0 ? border : -border);
    border_x_minus = 0;
    border_x_plus  = 0;
    std::vector<Morph::IndexType>::iterator selIt = 
      structuring_element.begin();
    while(selIt != structuring_element.end()){
      Morph::IndexType offset = *selIt++;
      Morph::IndexType yoffset = offset/Nx;
      if(yoffset==0 && (std::abs(static_cast<double>(offset)) < 
			static_cast<double>(Nx)/2.0)){
	if(offset < border_x_minus)
	  border_x_minus = offset;
	if(offset > border_x_plus )
	  border_x_plus = offset;
      }
    }
    border_x_minus = (border_x_minus > 0 ? border_x_minus :
		      -border_x_minus);
  };
  
  
  void DilateMask(Morph::MaskDataType *mask,
		  Morph::IndexType Nx,
		  Morph::IndexType Ny,
		  std::vector<Morph::IndexType> &structuring_element,
		  Morph::MaskDataType BitMask)
  {
    // Set up by getting image size, copying current mask, and determining borders 
    // for the structuring element to speed up the loops.
    //    Morph::IndexType Nx = image->axes[0];
    //    Morph::IndexType Ny = image->axes[1];
    Morph::IndexType npixels = Nx*Ny;
    Morph::IndexType npix_struct = 0;
    Morph::IndexType border_y_minus = 0;
    Morph::IndexType border_y_plus  = 0;
    Morph::IndexType border_x_minus = 0;
    Morph::IndexType border_x_plus  = 0;
    std::vector<Morph::MaskDataType> temp_mask(&mask[0],&mask[npixels-1]);
    Morph::GetSEAttributes(structuring_element,Nx,npix_struct,border_y_minus,
			   border_y_plus,border_x_minus,border_x_plus);
    // Do the slow parts (i.e. parts near image borders)
    //
    // Slow Part 1 [1:Nx,1:border_y]
    if(border_y_minus > 0){
      for(Morph::IndexType y = 0;y < border_y_minus;y++){
	for(Morph::IndexType x = 0;x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = 
	    structuring_element.begin();
	  while(selIt != structuring_element.end() && !(mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels))
	      mask[index] |= (temp_mask[ind]&BitMask);
	  }
	}
      }
    }
    // Slow Part 2.1 [1:border_x,border_y_minus:Ny-border_y_plus]
    if(border_x_minus > 0){
      Morph::IndexType ylimit = Ny - border_y_plus;
      for(Morph::IndexType y = border_y_minus;y < ylimit;y++){
	for(Morph::IndexType x = 0;x < border_x_minus;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = 
	    structuring_element.begin();
	  while(selIt != structuring_element.end() && !(mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels))
	      mask[index] |= (temp_mask[ind]&BitMask);
	  }
	}
      }
    }
    // Slow Part 2.2 [Nx-border_x:Nx,border_y:Ny-border_y]
    if(border_x_plus > 0){
      Morph::IndexType ylimit = Ny - border_y_plus;
      for(Morph::IndexType y = border_y_minus;y < ylimit;y++){
	for(Morph::IndexType x = (Nx-border_x_plus);x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
	  while(selIt != structuring_element.end() && !(mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels))
	      mask[index] |= (temp_mask[ind]&BitMask);
	  }
	}
      }
    }
    // Slow Part 3 [1:Nx,Ny-border_y:Ny]
    if(border_y_plus > 0){
      for(Morph::IndexType y = (Ny-border_y_plus);y < Ny;y++){
	for(Morph::IndexType x = 0;x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
	  while(selIt != structuring_element.end() && !(mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels))
	      mask[index] |= (temp_mask[ind]&BitMask);
	  }
	}
      }
    }
    // Fast Part, main part of image [border_x:Nx-border_x,border_y:Ny-border_y]
    if(border_y_minus > 0 || border_y_plus > 0 ||
       border_x_minus > 0 || border_x_plus > 0){
      Morph::IndexType ylimit = Ny-border_y_plus;
      Morph::IndexType xlimit = Nx-border_x_plus;
      for(Morph::IndexType y = border_y_minus; y < ylimit;y++){
	for(Morph::IndexType x = border_x_minus;x < xlimit;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
	  while(selIt != structuring_element.end() && !(mask[index]&BitMask))
	    mask[index] |= (temp_mask[index+*selIt++]&BitMask);
	}
      }
    }
  }
				      
  void ErodeMask(Morph::MaskDataType *mask,
		 Morph::IndexType Nx,
		 Morph::IndexType Ny,
		 std::vector<Morph::IndexType> &structuring_element,
		 Morph::MaskDataType BitMask)
  {
    // Set up by getting image size, copying current mask, and determining borders 
    // for the structuring element to speed up the loops.
    Morph::IndexType npixels = Nx*Ny;
    Morph::IndexType npix_struct = 0;
    Morph::IndexType border_y_minus = 0;
    Morph::IndexType border_y_plus  = 0;
    Morph::IndexType border_x_minus = 0;
    Morph::IndexType border_x_plus  = 0;
    std::vector<Morph::IndexType> trimmed_structuring_element;
    std::vector<Morph::MaskDataType> temp_mask(&mask[0],&mask[npixels-1]);
    Morph::GetSEAttributes(structuring_element,Nx,npix_struct,border_y_minus,
			   border_y_plus,border_x_minus,border_x_plus);

    // Do the slow parts (i.e. parts near image borders)
    //
    // Slow Part 1 [1:Nx,1:border_y]
    if(border_y_minus > 0){
      for(Morph::IndexType y = 0;y < border_y_minus;y++){
	for(Morph::IndexType x = 0;x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  Morph::TrimStructuringElement(index,Nx,Ny,structuring_element,trimmed_structuring_element);
	  std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	  while(selIt != trimmed_structuring_element.end() && (mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels) && !(temp_mask[ind]&BitMask))
	      mask[index] ^= BitMask;
	  }
	}
      }
    }
    // Slow Part 2.1 [1:border_x,border_y_minus:Ny-border_y_plus]
    if(border_x_minus > 0){
      Morph::IndexType ylimit = Ny - border_y_plus;
      for(Morph::IndexType y = border_y_minus;y < ylimit;y++){
	for(Morph::IndexType x = 0;x < border_x_minus;x++){
	  Morph::IndexType index = y*Nx + x;
	  Morph::TrimStructuringElement(index,Nx,Ny,structuring_element,trimmed_structuring_element);
	  std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	  while(selIt != trimmed_structuring_element.end() && (mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels) && !(temp_mask[ind]&BitMask))
	      mask[index] ^= BitMask;
	  }
	}
      }
    }
    // Slow Part 2.2 [Nx-border_x:Nx,border_y:Ny-border_y]
    if(border_x_plus > 0){
      Morph::IndexType ylimit = Ny - border_y_plus;
      for(Morph::IndexType y = border_y_minus;y < ylimit;y++){
	for(Morph::IndexType x = (Nx-border_x_plus);x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  Morph::TrimStructuringElement(index,Nx,Ny,structuring_element,trimmed_structuring_element);
	  std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	  while(selIt != trimmed_structuring_element.end() && (mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels) && !(temp_mask[ind]&BitMask))
	      mask[index] ^= BitMask;
	  }
	}
      }
    }
    // Slow Part 3 [1:Nx,Ny-border_y:Ny]
    if(border_y_plus > 0){
      for(Morph::IndexType y = (Ny-border_y_plus);y < Ny;y++){
	for(Morph::IndexType x = 0;x < Nx;x++){
	  Morph::IndexType index = y*Nx + x;
	  Morph::TrimStructuringElement(index,Nx,Ny,structuring_element,trimmed_structuring_element);
	  std::vector<Morph::IndexType>::iterator selIt = trimmed_structuring_element.begin();
	  while(selIt != trimmed_structuring_element.end() && (mask[index]&BitMask)){
	    Morph::IndexType ind = index + *selIt++;
	    if((ind >= 0) && (ind < npixels) && !(temp_mask[ind]&BitMask))
	      mask[index] ^= BitMask;
	  }
	}
      }
    }
    // Fast Part, main part of image [border_x:Nx-border_x,border_y:Ny-border_y]
    if(border_y_minus > 0 || border_y_plus > 0 ||
       border_x_minus > 0 || border_x_plus > 0){
      Morph::IndexType ylimit = Ny-border_y_plus;
      Morph::IndexType xlimit = Nx-border_x_plus;
      for(Morph::IndexType y = border_y_minus; y < ylimit;y++){
	for(Morph::IndexType x = border_x_minus;x < xlimit;x++){
	  Morph::IndexType index = y*Nx + x;
	  std::vector<Morph::IndexType>::iterator selIt = structuring_element.begin();
	  while(selIt != structuring_element.end() && (mask[index]&BitMask)){
	    if(!(temp_mask[index+*selIt]&BitMask))
	      mask[index] ^= BitMask;
	    selIt++;
	  }
	}
      }
    }
  }

  void GetBlobs(Morph::MaskDataType *mask,
		Morph::IndexType Nx,Morph::IndexType Ny,
		Morph::MaskDataType BlobMask,
		std::vector<Morph::IndexType> &blob_image,
		std::vector<std::vector<Morph::IndexType> > &blobs)
  {
    Morph::IndexType npix = Nx*Ny;
    blob_image.resize(npix,0);
    blobs.resize(0);
    std::vector<bool> already_processed(npix,false);
    Morph::IndexType nblob = 0;
    for(Morph::IndexType p = 0;p < npix;p++){
      Morph::IndexType y = p/Nx;
      Morph::IndexType x = p%Nx;
      if(!already_processed[p]){
	already_processed[p] = true;
	if(mask[p] & BlobMask){
	  blob_image[p] = ++nblob;
	  Morph::IndexType current_pixel = p;
	  Morph::IndexType blob_pixel = 0;
	  Morph::IndexType bx = x;
	  Morph::IndexType by = y;
	  std::vector<Morph::IndexType> blob(1,current_pixel);
	  while(blob_pixel != static_cast<Morph::IndexType>(blob.size())){
	    if((by - 1) > 0){
	      Morph::IndexType index = blob[blob_pixel] - Nx;
	      if((mask[index]&BlobMask) && !already_processed[index]){
		blob.push_back(index);
		blob_image[index] = nblob;
		already_processed[index] = true;
	      }
	      if((bx - 1) > 0){
		if((mask[index-1]&BlobMask) && !already_processed[index-1]){
		  blob.push_back(index-1);
		  blob_image[index-1] = nblob;
		  already_processed[index-1] = true;
		}
	      }
	      if((bx + 1) < Nx){
		if((mask[index+1]&BlobMask) && !already_processed[index+1]){
		  blob.push_back(index+1);
		  blob_image[index+1] = nblob;
		  already_processed[index+1] = true;
		}
	      }
	    }
	    if((bx+1) < Nx){
	      if((mask[blob[blob_pixel]+1]&BlobMask) && !already_processed[blob[blob_pixel]+1]){
		blob.push_back(blob[blob_pixel]+1);
		blob_image[blob[blob_pixel]+1] = nblob;
		already_processed[blob[blob_pixel]+1] = true;
	      }
	    }
	    if((by+1) < Ny){
	      Morph::IndexType index = blob[blob_pixel]+Nx;
	      if(mask[index]&BlobMask && !already_processed[index]){
		blob.push_back(index);
		blob_image[index] = nblob;
		already_processed[index] = true;
	      }	      
	      if((bx - 1) >= 0){
		if(mask[index-1]&BlobMask && !already_processed[index-1]){
		  blob.push_back(index-1);
		  blob_image[index-1] = nblob;
		  already_processed[index-1] = true;
		}
	      }
	      if((bx + 1) < Nx){
		if(mask[index+1]&BlobMask && !already_processed[index+1]){
		  blob.push_back(index+1);
		  blob_image[index+1] = nblob;
		  already_processed[index+1] = true;
		}		
	      }
	    }
	    blob_pixel++;
	    if(blob_pixel < static_cast<Morph::IndexType>(blob.size())){
	      bx = blob[blob_pixel]%Nx;
	      by = blob[blob_pixel]/Nx;
	    }
	  }
	  blobs.push_back(blob);
	}
      }
    }
  }
  // RejectionMask typically = BADPIX_SATURATE | BADPIX_CRAY | BADPIX_BPM | BADPIX_STAR | BADPIX_TRAIL
  // AcceptMask = BADPIX_INTERP
  // tol = 1e-3
  //
  int GetSky(Morph::ImageDataType *image,Morph::MaskDataType *mask, Morph::IndexType Nx, Morph::IndexType Ny,
	     Morph::IndexType minpix,Morph::IndexType maxiter,double ground_rejection_factor,double tol,
	     Morph::MaskDataType RejectionMask, Morph::MaskDataType AcceptMask,
	     Morph::StatType &image_stats,Morph::IndexType &npix,Morph::IndexType &niter,std::ostream *OStr){
    
    Morph::BoxType image_box(4,0);
    image_box[0] = 0;
    image_box[1] = Nx-1;
    image_box[2] = 0;
    image_box[3] = Ny-1;
    
  
    // Get global image statistics
    Morph::BoxStats(image,mask,Nx,Ny,image_box,RejectionMask,
		    AcceptMask,image_stats,npix);
    niter = 0;
    if(maxiter == 0)
      return(0);
    double last_mean = image_stats[Image::IMMEAN];
    while((niter < maxiter)){ 
      niter++;
      Morph::ImageDataType ground_rejection_level = image_stats[Image::IMMEAN] + 
	ground_rejection_factor*image_stats[Image::IMSIGMA];
      // Get stats again, but reject high pixels
      Morph::BoxStats(image,mask,Nx,Ny,image_box,RejectionMask,
		      AcceptMask,0,ground_rejection_level,
		      image_stats,npix);
      if(npix < minpix){
	if(OStr){
	  *OStr << "Morph::GetSky:Error: ran out of pixels(" << npix << ") after " << niter 
		<< (niter==1 ? " iteration." : " iterations.") << std::endl; 
	}
	return(1);
      }
      double residual = std::abs(image_stats[Image::IMMEAN]-last_mean);
      if(residual < tol){
	//	if(OStr){
	//	  *OStr << "Morph::GetSky statistics converged after NITER=" << niter 
	//		<< (niter==1 ? " iteration." : " iterations.") << std::endl; 
	//	}
	return(0);
      }
      last_mean = image_stats[Image::IMMEAN];
    }
    if(OStr){
      *OStr << "Morph::GetSky:Error: statistics did not converge after " << niter << " iterations.";
      return(1);
    } 
    return(0);
  }

};				      


