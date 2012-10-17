#ifndef __MORPHOLOGYCX_H__
#define __MORPHOLOGYCX_H__

extern "C" {
  int GetObjectRadii(float *image,short *mask,int Nx,int Ny,float *object_x,float *object_y,
		     int nobjects,float object_level,int maxrad,int minpix,
		     short RejectionMask,short AcceptMask,int *radii);
  int SetRadialMask(short *mask,int Nx,int Ny,float *object_x,float *object_y,
		    int nobjects,int *radii,short Mask);
  
};
#endif
