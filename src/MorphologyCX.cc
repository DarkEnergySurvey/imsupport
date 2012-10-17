#include "MorphologyCX.h"
#include "ImageMorphology.hh"
//#include <fstream>
extern "C" {
  int GetObjectRadii(float *image,short *mask,int Nx,int Ny,float *object_x,float *object_y,
		     int nobjects,float object_level,int maxrad,
		     int minpix,short RejectionMask,short AcceptMask,int *radii){
    //    float star_level = sky + level*skysigma;
    //    std::ofstream Ouf;
    //    Ouf.open("radial_profile");
    for(int o = 0;o < nobjects;o++){
      radii[o] = 0;
      Morph::IndexType x = static_cast<Morph::IndexType>(object_x[o]+.5);
      Morph::IndexType y = static_cast<Morph::IndexType>(object_y[o]+.5);
      Morph::IndexType index = y*Nx + x;
      std::vector<Morph::ImageDataType> energy;
      std::vector<Morph::IndexType> npix;
      Morph::RadialDistribution(image,mask,Nx,Ny,index,maxrad,RejectionMask,AcceptMask,energy,npix);
      //      if(o == 0){
      //	for(int i = 0;i < maxrad;i++)
      //	  Ouf << i << "  " << energy[i] << std::endl;
      //	Ouf.close();
      //      }
      std::vector<Morph::ImageDataType>::iterator ei = energy.begin();
      std::vector<Morph::IndexType>::iterator npi    = npix.begin();
      while(ei != energy.end()){
	if(*npi >= minpix && (*ei < object_level)){
	  radii[o] = (ei - energy.begin());
	  ei = energy.end();
	}
	else {
	  ei++;
	  npi++;
	}
      }
      if(radii[o] == 0)
	radii[o] = maxrad;
      //      std::cout << "Radius at (" << x << "," << y << ") found to be " << radii[o] << std::endl;
    }
    return(0);
  };
  
  int SetRadialMask(short *mask,int Nx,int Ny,float *object_x, float *object_y,
		    int nobjects,int *radii,short Mask){
    for(int o = 0; o < nobjects;o++){
      Morph::IndexType x = static_cast<Morph::IndexType>(object_x[o]+.5);
      Morph::IndexType y = static_cast<Morph::IndexType>(object_y[o]+.5);
      Morph::IndexType index = y*Nx + x;
      Morph::IndexType radius = radii[o];
      Morph::IndexType x0 = x - radius;
      Morph::IndexType y0 = y - radius;
      Morph::IndexType x1 = x + radius;
      Morph::IndexType y1 = y + radius;
      double rad = static_cast<double>(radius);
      if(x0 < 0) x0 = 0;
      if(x1 >= Nx) x1 = Nx -1;
      if(y0 < 0) y0 = 0;
      if(y1 >= Ny) y1 = Ny -1;
      for(int yb=y0;yb<=y1;yb++){
	for(int xb=x0;xb<=x1;xb++){
	  Morph::IndexType i = yb*Nx+xb;
	  double dx = static_cast<double>(xb - x);
	  double dy = static_cast<double>(yb - y);
	  double dist = std::sqrt(dx*dx + dy*dy);
	  if(dist <= rad){
	    mask[i] |= Mask;
	  }
	}
      }
    }
    return(0);
  };
  
};
