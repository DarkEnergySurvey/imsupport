/* image reading and writing subroutines */
/*$Id$*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "imsupport.h"

#define VERBOSE 0

void printerror(status)
        int status;
{
	printf("  **** FITS Error ****\n");
        fits_report_error(stderr,status);
        exit(0);
}



void init_desimage(desimage *image)
{
  /* **************************************** */
  /* ****** set pointers to be NULL ********* */
  /* **************************************** */
  image->image     = NULL;
  image->varim     = NULL;
  image->mask      = NULL;
  /* hardwire these FITS parameters */
  image->fpixel    = 1;
  image->nullval   = 0.0;
  image->shnullval = 0;
  image->npixels   = 0;
  image->nfound    = 0; 
}

void destroy_desimage(desimage *image)
{
  if(image->image)
    free(image->image);
  if(image->varim)
    free(image->varim);
  if(image->mask)
    free(image->mask);
  init_desimage(image);
}

// (Safely) destroys any data in destination.
void copy_desimage(desimage *destination,desimage *source)
{
  destroy_desimage(destination);
  destination->fptr = source->fptr;
  strncpy(destination->name,source->name,1000);
  strncpy(destination->biasseca,source->biasseca,100);
  strncpy(destination->ampseca,source->ampseca,100);
  strncpy(destination->dataseca,source->dataseca,100);
  strncpy(destination->biassecb,source->biassecb,100);
  strncpy(destination->ampsecb,source->ampsecb,100);
  strncpy(destination->datasecb,source->datasecb,100);
  strncpy(destination->trimsec,source->trimsec,100);
  strncpy(destination->datasec,source->datasec,100);
  destination->bscale       = source->bscale;
  destination->bzero        = source->bzero;
  destination->bitpix       = source->bitpix;
  destination->npixels      = source->npixels;
  destination->fpixel       = source->fpixel;
  destination->saturateA    = source->saturateA;
  destination->saturateB    = source->saturateB;
  destination->gainA        = source->gainA;
  destination->gainB        = source->gainB;
  destination->rdnoiseA     = source->rdnoiseA;
  destination->rdnoiseB     = source->rdnoiseB;
  destination->exptime      = source->exptime;
  destination->crpix1       = source->crpix1;
  destination->crpix2       = source->crpix2;
  destination->nfound       = source->nfound;
  destination->hdunum       = source->hdunum;
  destination->unit         = source->unit;
  destination->varunit      = source->varunit;
  destination->maskunit     = source->maskunit;
  destination->nullval      = source->nullval;
  destination->shnullval    = source->shnullval;
  destination->variancetype = source->variancetype;
  memcpy(destination->axes,source->axes,7*sizeof(long));
  memcpy(destination->biassecan,source->biassecan,4*sizeof(int));
  memcpy(destination->biassecbn,source->biassecbn,4*sizeof(int));
  memcpy(destination->ampsecan,source->ampsecan,4*sizeof(int));
  memcpy(destination->ampsecbn,source->ampsecbn,4*sizeof(int));
  memcpy(destination->trimsecn,source->trimsecn,4*sizeof(int));
  memcpy(destination->datasecn,source->datasecn,4*sizeof(int));
  memcpy(destination->datasecan,source->datasecan,4*sizeof(int));
  memcpy(destination->datasecbn,source->datasecbn,4*sizeof(int));
  size_t imsize = source->axes[0] * source->axes[1];
  if(source->image){
    destination->image = (float *)calloc(imsize,sizeof(float));
    memcpy(destination->image,source->image,imsize*sizeof(float));
  }
  if(source->varim){
    destination->varim = (float *)calloc(imsize,sizeof(float));
    memcpy(destination->varim,source->varim,imsize*sizeof(float));
  }
  if(source->mask){
    destination->mask = (short *)calloc(imsize,sizeof(short));
    memcpy(destination->mask,source->mask,imsize*sizeof(short));
  }
}

void rd_desimage(desimage *image,int mode,int flag_verbose)
{
        static int status=0;
	int	anynull,hdu,flag_type=0,hdutype,flag_fpack=0,
	  	imcount,hdunum,i,flag_bpmbad = 0,ypos,xpos,
	  check_image_name(),bitpix,compressed_format=0;
        int      readCounter, readHDUnumFailed, retryDelay, nRetry, done_reading_file;
        int      readCurrentFailed, readGzippedFailed, readFpackedFailed;
	float	saturate;
	char	comment[1000],type[100],compressname[500],name_im[500],
		rootname[500],compressname_im[500],currentimage[500],
		obstype[50],event[10000],
		imtypename[6][10]={"","IMAGE","VARIANCE","BPM","SIGMA","WEIGHT"};
        void    reportevt();
	long    imagesize[2] = {0,0}, bpmsize[2] ;
	int     move_success = 0, imtype_found = 1;
	/* **************************************************************** */
	/* ****************** checking image name ************************* */
	/* **************************************************************** */
	sprintf(currentimage,"%s",image->name);
	check_image_name(currentimage,REQUIRE_FITS,flag_verbose);

	/* **************************************** */
	/* ****** set pointers to be NULL ********* */
	/* **************************************** */
	image->image=NULL;
	image->varim=NULL;
	image->mask=NULL;
	/* hardwire these FITS parameters */
        image->fpixel=1;
	image->nullval=0.0;
	image->shnullval=0;
	image->npixels=0;
	image->nfound = 0;
	/* **************************************************************** */
        /* ********** open the file, check for other flavors ************* */
	/* **************************************************************** */

        readCounter = 0;
        retryDelay = 5;
        nRetry = 20;
	done_reading_file = 0;

        while ((readCounter < nRetry) && !done_reading_file) {
	  sprintf(currentimage,"%s",image->name);	  
	  if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
	    sprintf(currentimage,"%s.gz",image->name);
	    status=0;
	    if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
	      status=0;
	      sprintf(currentimage,"%s.fz",image->name);
	      status=0;
	      if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
		status=0;
		/* If we get here, we have failed to read all three. */
		if(readCounter < nRetry){
		  sleep(retryDelay);
		  readCounter++;
		  sprintf(event,"Images %s(.gz,.fz) failed to read on try %i", image->name, readCounter);
		  reportevt(flag_verbose,STATUS,4,event);
		  printf("  **** FITS Error ****\n");
		  fits_report_error(stderr,status);
		}
		else {
		  sprintf(event,"Images %s(.gz,.fz) failed to read on final try %i", image->name, readCounter);
		  reportevt(flag_verbose,STATUS,5,event);
		  printf("  **** FITS Error ****\n");
		  fits_report_error(stderr,status);
		  //		  printerror(status);
		}
	      }
	      else {
		flag_fpack=1;
		compressed_format = 1;
		done_reading_file = 1;
	      }
	    }
	    else
	      done_reading_file = 1;
	  }
	  else{
	    if(!strncmp(&currentimage[strlen(currentimage)-3],".fz",3)){
	      flag_fpack = 1;
	      compressed_format = 1;
	    } 
	    done_reading_file = 1;
	  }
	}
	/* report altered filename if needed */
	if (strcmp(image->name,currentimage)) {
	  sprintf(event,"Image %s mapped into %s",image->name,
	    currentimage);
	  reportevt(flag_verbose,STATUS,1,event);
	}
	
	/* **************************************************************** */
	/* ****************** determine how many extensions *************** */
	/* **************************************************************** */
	if (fits_get_num_hdus(image->fptr,&(image->hdunum),&status)) {
	  sprintf(event,"Reading HDUNUM failed: %s",currentimage);
	  reportevt(flag_verbose,STATUS,5,event);
	  printerror(status);
	}
	  
	/* **************************************************************** */
	/* ********* cycle through the HDU's loading as appropriate ******* */
	/* **************************************************************** */
	image->unit = compressed_format;
	for (hdu=flag_fpack;hdu<image->hdunum;hdu++) {
	  if (hdu>0) 
	    if (fits_movrel_hdu(image->fptr,1,&hdutype,&status))  {
	      sprintf(event,"Move forward 1 HDU failed: %s",image->name);
	      reportevt(flag_verbose,STATUS,5,event);
	      printerror(status);
	    }
	  /*
	    this does not work for fz file hence replacing with fits_get_img_size 
	    if (fits_read_keys_lng(image->fptr,"NAXIS",1,2,image->axes,
	    &image->nfound,&status))
	  */
	  if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
	    image->axes,&status)) {
	    sprintf(event,"NAXIS read failed: %s",currentimage);
	    reportevt(flag_verbose,STATUS,5,event);
	    printerror(status);
	  }
	  if (image->nfound>2) {
	    sprintf(event,"Image dimension too large (%s,%d)",
	     currentimage,image->nfound);
	    reportevt(flag_verbose,STATUS,5,event);
	    exit(0);
	  }
	  if (flag_verbose==3) {
	    sprintf(event,"Opened %s: %d HDU (%ldX%ld)",
	     currentimage,image->hdunum,image->axes[0],image->axes[1]);
	    reportevt(flag_verbose,STATUS,1,event);
	  }
	  if (fits_read_key_str(image->fptr,"DES_EXT",type,comment,&status)==
	      KEY_NO_EXIST) {
	    status=0; /* reset status flag */
	    if (flag_verbose) {
	      sprintf(event,"No DES_EXT keyword found in %s, trying next HDU before giving up.",
		      image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	    }
	    imtype_found = 0;
	    move_success = 0;
	    /* Let's try moving to the next HDU before giving up on DES_EXT */
	    if (fits_movrel_hdu(image->fptr,1,&hdutype,&status))  {
	      sprintf(event,"Test of move forward 1 HDU failed: %s",image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	      status=0;
	      //	      printerror(status);
	    }
	    else {
	      move_success = 1;
	      if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
				     image->axes,&status)) {
		sprintf(event,"Image parameter read failed: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	      }
	      if (image->nfound>2) {
		sprintf(event,"Image dimension too large (%s,%d)",
			currentimage,image->nfound);
		reportevt(flag_verbose,STATUS,5,event);
		exit(0);
	      }
	      if (fits_read_key_str(image->fptr,"DES_EXT",type,comment,&status)==
		  KEY_NO_EXIST) {
		imtype_found = 0;
		status=0; /* reset status flag */
		/* move back to the original hdu */
		if (fits_movrel_hdu(image->fptr,-1,&hdutype,&status))  {
		  sprintf(event,"Move backward 1 HDU failed: %s",image->name);
		  reportevt(flag_verbose,STATUS,5,event);
		  printerror(status);
		}
		/*  RE-Get the image parameters, because we've overwritten them 
		 *  in the attempt to look at the next HDU.
		 */
		if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
				       image->axes,&status)) {
		  sprintf(event,"Image parameter read failed: %s",currentimage);
		  reportevt(flag_verbose,STATUS,5,event);
		  printerror(status);
		}
	      }
	      else
		imtype_found = 1;
	    }
	  }
	  if(!imtype_found){
	    if (flag_verbose) {
	      sprintf(event,"No DES_EXT keyword found in %s",
		      image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	    }
	    if (hdu==0) {
	      sprintf(type,"IMAGE"); 
	      if (flag_verbose) {
		sprintf(event,"Assuming this is IMAGE extension");
		reportevt(flag_verbose,STATUS,3,event);
	      }
	    }
	    else if (hdu==1) {
	      sprintf(type,"WEIGHT"); 
	      if (flag_verbose) {
		sprintf(event,"Assuming this is WEIGHT extension");
		reportevt(flag_verbose,STATUS,3,event);
	      }
	    }
	  }
	  else if (move_success){
	    if (flag_verbose) {
	      reportevt(flag_verbose,STATUS,3,"Found DES_EXT in next HDU. Image must be compressed.");
	    }
	    hdu++;
	    compressed_format = 1;
	  }
	  if (!strcmp(type,"IMAGE")){
	    flag_type=DES_IMAGE;
	    image->unit = hdu;
	  }
	  else if (!strcmp(type,"MASK") || !strcmp(type,"BPM")){ 
	    flag_type=DES_MASK;
	    image->maskunit = hdu;
	  }
	  /* following type is now deprecated */
	  else if (!strcmp(type,"VARIANCE")){
	    image->varunit = hdu;
	    flag_type=DES_VARIANCE;
	  }
	  /* these are replacement types for VARIANCE */
	  else if (!strcmp(type,"SIGMA")){
	    image->varunit = hdu;
	    flag_type=DES_SIGMA;
	  }
	  else if (!strcmp(type,"WEIGHT")){
	    image->varunit = hdu;
	    flag_type=DES_WEIGHT;
	  }


	  /* **************************************************************** */
	  /* ************** Current extension is IMAGE extension ************ */
	  /* **************************************************************** */
   	  if (flag_type==DES_IMAGE) {  /* standard image in this extension */
     	    imagesize[0] = image->axes[0];
	    imagesize[1] = image->axes[1];
	    image->npixels  = image->axes[0]*image->axes[1];
	    if(compressed_format){
	      if (fits_read_key_lng(image->fptr,"ZBITPIX",&(image->bitpix),
				    comment,&status)) {
		sprintf(event,"BITPIX missing in header: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);	
	      }
	    }
	    else{
	      if (fits_read_key_lng(image->fptr,"BITPIX",&(image->bitpix),
				    comment,&status)) {
		sprintf(event,"BITPIX missing in header: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);	
	      }
	    }
	    image->bscale = 0;
	    image->bzero = 0;
	    if (fits_read_key_lng(image->fptr,"BSCALE",&(image->bscale),
	      comment,&status)==KEY_NO_EXIST) status=0;
	    if (fits_read_key_lng(image->fptr,"BZERO",&(image->bzero),comment,
	      &status)) status=0;
	    /* grab OBSTYPE if it is present */
	    if (fits_read_key_str(image->fptr,"OBSTYPE",obstype,
	      comment,&status)==KEY_NO_EXIST) {
	      status=0;
	    }
	    if (!strcmp(obstype,"raw_obj") || !strcmp(obstype,"raw_dflat")
	      || !strcmp(obstype,"raw_dark") || !strcmp(obstype,"raw_bias")) {
	      /* grab SATURATA and SATURATB if they are present */
	      if (fits_read_key_flt(image->fptr,"SATURATA",
	        &(image->saturateA),comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->saturateA=0.0;
	        sprintf(event,"SATURATA missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"SATURATB",
	        &(image->saturateB),comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->saturateB=0.0;
	 	sprintf(event,"SATURATB missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      /* grab GAINA and GAINB if they are present */
	      if (fits_read_key_flt(image->fptr,"GAINA",&(image->gainA),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->gainA=0.0;
	        sprintf(event,"GAINA missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"GAINB",&(image->gainB),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->gainB=0.0;
	 	sprintf(event,"GAINB missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	        
	      /* grab RDNOISEA and RDNOISEB if they are present */
	      if (fits_read_key_flt(image->fptr,"RDNOISEA",&(image->rdnoiseA),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->rdnoiseA=0.0;
	        sprintf(event,"RDNOISEA missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"RDNOISEB",&(image->rdnoiseB),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->rdnoiseB=0.0;
	        sprintf(event,"RDNOISEB missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }

	      /* grab CRPIX1 and CRPIX2 if they are present */
	      if (fits_read_key_flt(image->fptr,"CRPIX1",&(image->crpix1),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix1=0.0;
	        sprintf(event,"CRPIX1 missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"CRPIX2",&(image->crpix2),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix2=0.0;
	        sprintf(event,"CRPIX2 missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }

              /*obtain exposure time if need to apply dar current subtraction */
              if (fits_read_key_flt(image->fptr,"EXPTIME",&(image->exptime),comment,
				    &status)==KEY_NO_EXIST) {
	        status=0;
                image->exptime = 0.0 ;
                sprintf(event,"EXPTIME missing: %s\n",currentimage);
                reportevt(flag_verbose,STATUS,3,event);
              }
	      else{
                sprintf(event,"CCD EXPTIME = %.1f\n",image->exptime);
                reportevt(flag_verbose,STATUS,3,event);
	      }
	  
	      /* Echo what has been read if verbose level appropriate */
	      /* currently not echoing exposure time */
	      if (flag_verbose==3) {
	        sprintf(event,"BITPIX=%ld BSCALE=%ld BZERO=%ld OBSTYPE=%s",
	        image->bitpix,image->bscale,image->bitpix,obstype);
		reportevt(flag_verbose,STATUS,1,event);
	        sprintf(event,"Image=%s & AmpA: Saturate=%.1f & Rdnoise=%.2f & Gain=%.3f & ",
		  image->name,image->saturateA,image->rdnoiseA,image->gainA);
	        sprintf(event,"%s AmpB: Saturate=%.1f & Rdnoise=%.2f & Gain=%.3f",
		  event,image->saturateB,image->rdnoiseB,image->gainB);
		reportevt(flag_verbose,QA,1,event);
	      }
	    }
	    else { /* not raw data */
	      if (flag_verbose==3) {
	        sprintf(event,"OBSTYPE: %s",obstype);
		  reportevt(flag_verbose,STATUS,1,event);
	      }
	      /* see if there is a SATURATE keyword */
	      if (fits_read_key_flt(image->fptr,"SATURATE",&saturate,
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	      }
	      else if (flag_verbose==3) {
	        sprintf(event,"CCD: Saturate= %.1f",saturate);
	        reportevt(flag_verbose,STATUS,1,event);
	      }
	      /* grab CRPIX1 and CRPIX2 if they are present */
	      if (fits_read_key_flt(image->fptr,"CRPIX1",&(image->crpix1),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix1=0.0;
	        sprintf(event,"CRPIX1 missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"CRPIX2",&(image->crpix2),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix2=0.0;
	        sprintf(event,"CRPIX2 missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }

              /*obtain exposure time if need to apply dar current subtraction */
              if (fits_read_key_flt(image->fptr,"EXPTIME",&(image->exptime),comment,
				    &status)==KEY_NO_EXIST) {
	        status=0;
                image->exptime = 0.0 ;
                sprintf(event,"EXPTIME missing: %s\n",currentimage);
                reportevt(flag_verbose,STATUS,3,event);
              }
	    }
	    /* image type assumed to be FLOAT */
	    image->image=(float *)calloc(image->npixels,sizeof(float));
	    if (image->image==NULL) {
	      sprintf(event,"Calloc of image->image failed");
	      reportevt(flag_verbose,STATUS,5,event);
	    }
	    status=0;
            if (fits_read_img(image->fptr,TFLOAT,image->fpixel,image->npixels,
	        &image->nullval,image->image,&anynull,&status)) {
	        sprintf(event,"%s read failed: %s",
		  imtypename[flag_type],currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	    }
	    if (flag_verbose==3) {
	      sprintf(event,"Read %s extension",imtypename[flag_type]);
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
 
	  /* **************************************************************** */
	  /* ************** Current extension is MASK extension ************* */
	  /* **************************************************************** */
	  if (flag_type==DES_MASK) {  /* image mask in this extension */
	    bpmsize[0] = image->axes[0] ;
	    bpmsize[1] = image->axes[1];
	    if (image->hdunum==1) image->npixels = bpmsize[0]*bpmsize[1];
	    image->mask=(short *)calloc(image->npixels,sizeof(short));
	    if (image->mask==NULL) {
		sprintf(event,"Calloc failed for image->mask");
		reportevt(flag_verbose,STATUS,5,event);
		exit(0);
	    }
	    /* ****** MASK is same size as IMAGE ****** */
	    if (((bpmsize[0]==imagesize[0]) && (bpmsize[1]==imagesize[1])) 
	      || (image->hdunum ==1)) {
	      if (fits_read_img(image->fptr,TUSHORT,image->fpixel,
		 image->npixels,
		  &image->shnullval,image->mask,&anynull,&status)) {
	        sprintf(event,"%s image read failed: %s",
		  imtypename[flag_type],currentimage);
	        reportevt(flag_verbose,STATUS,5,event);
	        printerror(status);  
	      }
	      if (flag_verbose==3) {
	        sprintf(event,"Read %s extension",imtypename[flag_type]);
	        reportevt(flag_verbose,STATUS,1,event);
	      }
	    }
	    else { /* MASK is different size from image */
	      /* NOTE:  this should not happen, but in DC4 we had a bunch */
	      /* buggy remap image masks and we added this to handle them */
	      /* recreate a BPM that is the same size as ORIGINAL IMAGE*/
	      for (i=0;i<image->npixels;i++) {
		image->mask[i] = 0;
		xpos = i%imagesize[0] ;
		ypos = i/imagesize[0] ;
		/* set BPM near edges */
		if (xpos<2 || ypos<2)image->mask[i]=1 ;
		if (abs(xpos-imagesize[0])<=2 || abs(ypos-imagesize[1])<=2)
		  image->mask[i]=1 ;
	      }
	      sprintf(event,"Size of BPM is different from the main image");
	      reportevt(flag_verbose,STATUS,4,event);
	      flag_bpmbad = 1;
	      sprintf(event,"Resetting BPM values at image edge to be 1");
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
	  /* **************************************************************** */
	  /* *********** Current extension is VARIANCE extension ************ */
	  /* **************************************************************** */
	  if (flag_type==DES_VARIANCE || flag_type==DES_WEIGHT ||
	    flag_type==DES_SIGMA) {  /* weight map in this extension */
	    if ((image->npixels)==0)
	      image->npixels=image->axes[0]*image->axes[1];
	    image->varim=(float *)calloc(image->npixels,sizeof(float));
	    /* set the variance type in image structure */
	    image->variancetype=flag_type;
	    status=0;
            if (fits_read_img(image->fptr,TFLOAT,image->fpixel,image->npixels,
	        &image->nullval,image->varim,&anynull,&status)) {
		sprintf(event,"%s image read failed: %s",
		  imtypename[image->variancetype],currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	    }
	    if (flag_verbose==3) {
	      sprintf(event,"Read %s extension",
	  	imtypename[image->variancetype]);
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
	} /* end of HDU loop */


	/* **************************************************************** */
	/* *************  Setting VARIM saturated pixels ****************** */
	/* **************  in case where BPM wrong size ******************* */
	/* **************************************************************** */
	/* ******************** TEMPORARY DC4 FIX ************************* */
	/* **************************************************************** */

	if (flag_bpmbad){
	  sprintf(event,"Resetting  VARIM values to 0 for saturated pixels");
	  reportevt(flag_verbose,STATUS,1,event);
	  for (i=0;i<image->npixels;i++) 
	      if (image->image[i] > saturate) image->varim[i]=0.0;
          if ((image->axes[0]!=imagesize[0])||(image->axes[1]!=imagesize[1])) {
	    image->axes[0]=imagesize[0];
	    image->axes[1]=imagesize[1] ;
	  }
	}	
}

void rd_dessubimage(desimage *image,long *lx,long *ux,int mode,int flag_verbose)
{
        static int status=0;
	int	anynull,hdu,flag_type=0,hdutype,flag_fpack=0,
	  	imcount,hdunum,i,flag_bpmbad = 0,ypos,xpos,
	  check_image_name(),bitpix,compressed_format=0;
        int      readCounter, readHDUnumFailed, retryDelay, nRetry, done_reading_file;
        int      readCurrentFailed, readGzippedFailed, readFpackedFailed;
	float	saturate;
	char	comment[1000],type[100],compressname[500],name_im[500],
		rootname[500],compressname_im[500],currentimage[500],
		obstype[50],event[10000],
		imtypename[6][10]={"","IMAGE","VARIANCE","BPM","SIGMA","WEIGHT"};
        void    reportevt();
	long    imagesize[2] = {0,0}, bpmsize[2] ;
	long    inc[2] = {1,1};
	int     move_success = 0, imtype_found = 1;
	/* **************************************************************** */
	/* ****************** checking image name ************************* */
	/* **************************************************************** */
	sprintf(currentimage,"%s",image->name);
	check_image_name(currentimage,REQUIRE_FITS,flag_verbose);

	/* **************************************** */
	/* ****** set pointers to be NULL ********* */
	/* **************************************** */
	image->image=NULL;
	image->varim=NULL;
	image->mask=NULL;
	/* hardwire these FITS parameters */
        image->fpixel=1;
	image->nullval=0.0;
	image->shnullval=0;
	image->npixels=0;
	image->nfound = 0;
	/* **************************************************************** */
        /* ********** open the file, check for other flavors ************* */
	/* **************************************************************** */

        readCounter = 0;
        retryDelay = 5;
        nRetry = 20;
	done_reading_file = 0;

        while ((readCounter < nRetry) && !done_reading_file) {
	  sprintf(currentimage,"%s",image->name);	  
	  if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
	    sprintf(currentimage,"%s.gz",image->name);
	    status=0;
	    if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
	      status=0;
	      sprintf(currentimage,"%s.fz",image->name);
	      status=0;
	      if (fits_open_file(&image->fptr,currentimage,mode,&status)) {
		status=0;
		/* If we get here, we have failed to read all three. */
		if(readCounter < nRetry){
		  sleep(retryDelay);
		  readCounter++;
		  sprintf(event,"Images %s(.gz,.fz) failed to read on try %i", image->name, readCounter);
		  reportevt(flag_verbose,STATUS,4,event);
		  printf("  **** FITS Error ****\n");
		  fits_report_error(stderr,status);
		}
		else {
		  sprintf(event,"Images %s(.gz,.fz) failed to read on final try %i", image->name, readCounter);
		  reportevt(flag_verbose,STATUS,5,event);
		  printf("  **** FITS Error ****\n");
		  fits_report_error(stderr,status);
		  //		  printerror(status);
		}
	      }
	      else {
		flag_fpack=1;
		compressed_format = 1;
		done_reading_file = 1;
	      }
	    }
	    else
	      done_reading_file = 1;
	  }
	  else
	    done_reading_file = 1;
	}
	/* report altered filename if needed */
	if (strcmp(image->name,currentimage)) {
	  sprintf(event,"Image %s mapped into %s",image->name,
	    currentimage);
	  reportevt(flag_verbose,STATUS,1,event);
	}
	
	/* **************************************************************** */
	/* ****************** determine how many extensions *************** */
	/* **************************************************************** */
	if (fits_get_num_hdus(image->fptr,&(image->hdunum),&status)) {
	  sprintf(event,"Reading HDUNUM failed: %s",currentimage);
	  reportevt(flag_verbose,STATUS,5,event);
	  printerror(status);
	}
	  
	/* **************************************************************** */
	/* ********* cycle through the HDU's loading as appropriate ******* */
	/* **************************************************************** */
	image->unit = compressed_format;
	for (hdu=flag_fpack;hdu<image->hdunum;hdu++) {
	  if (hdu>0) 
	    if (fits_movrel_hdu(image->fptr,1,&hdutype,&status))  {
	      sprintf(event,"Move forward 1 HDU failed: %s",image->name);
	      reportevt(flag_verbose,STATUS,5,event);
	      printerror(status);
	    }
	  /*
	    this does not work for fz file hence replacing with fits_get_img_size 
	    if (fits_read_keys_lng(image->fptr,"NAXIS",1,2,image->axes,
	    &image->nfound,&status))
	  */
	  if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
	    image->axes,&status)) {
	    sprintf(event,"NAXIS read failed: %s",currentimage);
	    reportevt(flag_verbose,STATUS,5,event);
	    printerror(status);
	  }
	  if (image->nfound>2) {
	    sprintf(event,"Image dimension too large (%s,%d)",
	     currentimage,image->nfound);
	    reportevt(flag_verbose,STATUS,5,event);
	    exit(0);
	  }
	  imagesize[0] = ux[0] - lx[0] + 1;
	  imagesize[1] = ux[1] - lx[1] + 1;
	  if (flag_verbose==3) {
	    sprintf(event,"Opened %s: %d HDU (%ldX%ld)",
	     currentimage,image->hdunum,image->axes[0],image->axes[1]);
	    reportevt(flag_verbose,STATUS,1,event);
	    sprintf(event,"Reading subset of %s: %d HDU (%ldX%ld)",
	     currentimage,image->hdunum,imagesize[0],imagesize[1]);
	    reportevt(flag_verbose,STATUS,1,event);
	  }
	  if (fits_read_key_str(image->fptr,"DES_EXT",type,comment,&status)==
	      KEY_NO_EXIST) {
	    status=0; /* reset status flag */
	    if (flag_verbose) {
	      sprintf(event,"No DES_EXT keyword found in %s, trying next HDU before giving up.",
		      image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	    }
	    imtype_found = 0;
	    move_success = 0;
	    /* Let's try moving to the next HDU before giving up on DES_EXT */
	    if (fits_movrel_hdu(image->fptr,1,&hdutype,&status))  {
	      sprintf(event,"Test of move forward 1 HDU failed: %s",image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	      status=0;
	      //	      printerror(status);
	    }
	    else {
	      move_success = 1;
	      if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
				     image->axes,&status)) {
		sprintf(event,"Image parameter read failed: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	      }
	      if (image->nfound>2) {
		sprintf(event,"Image dimension too large (%s,%d)",
			currentimage,image->nfound);
		reportevt(flag_verbose,STATUS,5,event);
		exit(0);
	      }
	      if (fits_read_key_str(image->fptr,"DES_EXT",type,comment,&status)==
		  KEY_NO_EXIST) {
		imtype_found = 0;
		status=0; /* reset status flag */
		/* move back to the original hdu */
		if (fits_movrel_hdu(image->fptr,-1,&hdutype,&status))  {
		  sprintf(event,"Move backward 1 HDU failed: %s",image->name);
		  reportevt(flag_verbose,STATUS,5,event);
		  printerror(status);
		}
		/*  RE-Get the image parameters, because we've overwritten them 
		 *  in the attempt to look at the next HDU.
		 */
		if (fits_get_img_param(image->fptr,3,&bitpix,&(image->nfound),
				       image->axes,&status)) {
		  sprintf(event,"Image parameter read failed: %s",currentimage);
		  reportevt(flag_verbose,STATUS,5,event);
		  printerror(status);
		}
	      }
	      else
		imtype_found = 1;
	    }
	  }
	  if(!imtype_found){
	    if (flag_verbose) {
	      sprintf(event,"No DES_EXT keyword found in %s",
		      image->name);
	      reportevt(flag_verbose,STATUS,3,event);
	    }
	    if (hdu==0) {
	      sprintf(type,"IMAGE"); 
	      if (flag_verbose) {
		sprintf(event,"Assuming this is IMAGE extension");
		reportevt(flag_verbose,STATUS,3,event);
	      }
	    }
	    else if (hdu==1) {
	      sprintf(type,"WEIGHT"); 
	      if (flag_verbose) {
		sprintf(event,"Assuming this is WEIGHT extension");
		reportevt(flag_verbose,STATUS,3,event);
	      }
	    }
	  }
	  else if (move_success){
	    if (flag_verbose) {
	      reportevt(flag_verbose,STATUS,3,"Found DES_EXT in next HDU. Image must be compressed.");
	    }
	    hdu++;
	    compressed_format = 1;
	  }
	  if (!strcmp(type,"IMAGE")){
	    flag_type=DES_IMAGE;
	    image->unit = hdu;
	  }
	  else if (!strcmp(type,"MASK") || !strcmp(type,"BPM")){ 
	    flag_type=DES_MASK;
	    image->maskunit = hdu;
	  }
	  /* following type is now deprecated */
	  else if (!strcmp(type,"VARIANCE")){
	    image->varunit = hdu;
	    flag_type=DES_VARIANCE;
	  }
	  /* these are replacement types for VARIANCE */
	  else if (!strcmp(type,"SIGMA")){
	    image->varunit = hdu;
	    flag_type=DES_SIGMA;
	  }
	  else if (!strcmp(type,"WEIGHT")){
	    image->varunit = hdu;
	    flag_type=DES_WEIGHT;
	  }


	  /* **************************************************************** */
	  /* ************** Current extension is IMAGE extension ************ */
	  /* **************************************************************** */
   	  if (flag_type==DES_IMAGE) {  /* standard image in this extension */
     	    imagesize[0] = ux[0] - lx[0] + 1;
	    imagesize[1] = ux[1] - lx[1] + 1;
	    image->axes[0] = imagesize[0];
	    image->axes[1] = imagesize[1];
	    image->npixels  = image->axes[0]*image->axes[1];
	    if(compressed_format){
	      if (fits_read_key_lng(image->fptr,"ZBITPIX",&(image->bitpix),
				    comment,&status)) {
		sprintf(event,"ZBITPIX missing in header: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);	
	      }
	    }
	    else{
	      if (fits_read_key_lng(image->fptr,"BITPIX",&(image->bitpix),
				    comment,&status)) {
		sprintf(event,"BITPIX missing in header: %s",currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);	
	      }
	    }
	    image->bscale = 0;
	    image->bzero = 0;
	    if (fits_read_key_lng(image->fptr,"BSCALE",&(image->bscale),
	      comment,&status)==KEY_NO_EXIST) status=0;
	    if (fits_read_key_lng(image->fptr,"BZERO",&(image->bzero),comment,
	      &status)) status=0;
	    /* grab OBSTYPE if it is present */
	    if (fits_read_key_str(image->fptr,"OBSTYPE",obstype,
	      comment,&status)==KEY_NO_EXIST) {
	      status=0;
	    }
	    if (!strcmp(obstype,"raw_obj") || !strcmp(obstype,"raw_dflat")
	      || !strcmp(obstype,"raw_dark") || !strcmp(obstype,"raw_bias")) {
	      /* grab SATURATA and SATURATB if they are present */
	      if (fits_read_key_flt(image->fptr,"SATURATA",
	        &(image->saturateA),comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->saturateA=0.0;
	        sprintf(event,"SATURATA missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"SATURATB",
	        &(image->saturateB),comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->saturateB=0.0;
	 	sprintf(event,"SATURATB missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      /* grab GAINA and GAINB if they are present */
	      if (fits_read_key_flt(image->fptr,"GAINA",&(image->gainA),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->gainA=0.0;
	        sprintf(event,"GAINA missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"GAINB",&(image->gainB),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->gainB=0.0;
	 	sprintf(event,"GAINB missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	        
	      /* grab RDNOISEA and RDNOISEB if they are present */
	      if (fits_read_key_flt(image->fptr,"RDNOISEA",&(image->rdnoiseA),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->rdnoiseA=0.0;
	        sprintf(event,"RDNOISEA missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      if (fits_read_key_flt(image->fptr,"RDNOISEB",&(image->rdnoiseB),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->rdnoiseB=0.0;
	        sprintf(event,"RDNOISEB missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }

	      /* grab CRPIX1 and CRPIX2 if they are present */
	      if (fits_read_key_flt(image->fptr,"CRPIX1",&(image->crpix1),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix1=0.0;
	        sprintf(event,"CRPIX1 missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      else {
		if(flag_verbose){
		  sprintf(event,"Reset CRPIX1 from %f to %f",image->crpix1,image->crpix1-lx[0]+1);
		  reportevt(flag_verbose,STATUS,3,event);
		}
		image->crpix1 = image->crpix1 - lx[0] + 1;
	      }
	      if (fits_read_key_flt(image->fptr,"CRPIX2",&(image->crpix2),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix2=0.0;
	        sprintf(event,"CRPIX2 missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	      else {
		if(flag_verbose){
		  sprintf(event,"Reset CRPIX2 from %f to %f",image->crpix2,image->crpix2 - lx[1]+1);
		  reportevt(flag_verbose,STATUS,3,event);
		}
		image->crpix2 = image->crpix2 - lx[1] + 1;
	      }
              /*obtain exposure time if need to apply dar current subtraction */
              if (fits_read_key_flt(image->fptr,"EXPTIME",&(image->exptime),comment,
				    &status)==KEY_NO_EXIST) {
	        status=0;
                image->exptime = 0.0 ;
                sprintf(event,"EXPTIME missing: %s\n",currentimage);
                reportevt(flag_verbose,STATUS,3,event);
              }
	      else{
                sprintf(event,"CCD: EXPTIME = %f\n",image->exptime);
                reportevt(flag_verbose,STATUS,3,event);
	      }
	      /* Echo what has been read if verbose level appropriate */
	      /* currently not echoing exposure time */
	      if (flag_verbose==3) {
	        sprintf(event,"BITPIX=%ld BSCALE=%ld BZERO=%ld OBSTYPE=%s",
	        image->bitpix,image->bscale,image->bitpix,obstype);
		reportevt(flag_verbose,STATUS,1,event);
	        sprintf(event,"Image=%s & AmpA: Saturate=%.1f & Rdnoise=%.2f & Gain=%.3f & ",
		  image->name,image->saturateA,image->rdnoiseA,image->gainA);
	        sprintf(event,"%s AmpB: Saturate=%.1f & Rdnoise=%.2f & Gain=%.3f",
		  event,image->saturateB,image->rdnoiseB,image->gainB);
		reportevt(flag_verbose,QA,1,event);
	      }
	    }
	    else { /* not raw data */
	      if (flag_verbose==3) {
	        sprintf(event,"OBSTYPE: %s",obstype);
		  reportevt(flag_verbose,STATUS,1,event);
	      }
	      /* see if there is a SATURATE keyword */
	      if (fits_read_key_flt(image->fptr,"SATURATE",&saturate,
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	      }
	      else if (flag_verbose==3) {
	        sprintf(event,"CCD: Saturate= %.1f",saturate);
	        reportevt(flag_verbose,STATUS,1,event);
	      }
	      /* grab CRPIX1 and CRPIX2 if they are present */
	      if (fits_read_key_flt(image->fptr,"CRPIX1",&(image->crpix1),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix1=0.0;
	        sprintf(event,"CRPIX1 missing: %s",currentimage);
		reportevt(flag_verbose,STATUS,3,event);
	      }
	      else {
		if(flag_verbose){
		  sprintf(event,"Reset CRPIX1 from %f to %f",image->crpix1,image->crpix1-lx[0]+1);
		  reportevt(flag_verbose,STATUS,3,event);
		}
		image->crpix1 = image->crpix1 - lx[0] + 1;
	      }
	      if (fits_read_key_flt(image->fptr,"CRPIX2",&(image->crpix2),
	        comment,&status)==KEY_NO_EXIST) {
	        status=0;
	        image->crpix2=0.0;
	        sprintf(event,"CRPIX2 missing: %s",currentimage);
	        reportevt(flag_verbose,STATUS,3,event);
	      }
	      else {
		if(flag_verbose){
		  sprintf(event,"Reset CRPIX2 from %f to %f",image->crpix2,image->crpix2 - lx[1]+1);
		  reportevt(flag_verbose,STATUS,3,event);
		}
		image->crpix2 = image->crpix2 - lx[1] + 1;
	      }
              /*obtain exposure time if need to apply dar current subtraction */
              if (fits_read_key_flt(image->fptr,"EXPTIME",&(image->exptime),comment,
				    &status)==KEY_NO_EXIST) {
	        status=0;
                image->exptime = 0.0 ;
                sprintf(event,"EXPTIME missing: %s\n",currentimage);
                reportevt(flag_verbose,STATUS,3,event);
              }
	      else{
                sprintf(event,"CCD: EXPTIME = %.1f\n",image->exptime);
                reportevt(flag_verbose,STATUS,3,event);
	      }
	    }
	    /* image type assumed to be FLOAT */
	    image->image=(float *)calloc(image->npixels,sizeof(float));
	    if (image->image==NULL) {
	      sprintf(event,"Calloc of image->image failed");
	      reportevt(flag_verbose,STATUS,5,event);
	    }
	    status=0;
	    inc[0] = inc[1] = 1;
            if (fits_read_subset(image->fptr,TFLOAT,lx,ux,inc,
	        &image->nullval,image->image,&anynull,&status)) {
	        sprintf(event,"%s read failed: %s",
		  imtypename[flag_type],currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	    }
	    if (flag_verbose==3) {
	      sprintf(event,"Read %s extension",imtypename[flag_type]);
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
 
	  /* **************************************************************** */
	  /* ************** Current extension is MASK extension ************* */
	  /* **************************************************************** */
	  if (flag_type==DES_MASK) {  /* image mask in this extension */
     	    imagesize[0] = ux[0] - lx[0] + 1;
	    imagesize[1] = ux[1] - lx[1] + 1;
	    image->axes[0] = imagesize[0];
	    image->axes[1] = imagesize[1];
	    bpmsize[0] = image->axes[0];
	    bpmsize[1] = image->axes[1];
	    if (image->hdunum==1) image->npixels = bpmsize[0]*bpmsize[1];
	    image->mask=(short *)calloc(image->npixels,sizeof(short));
	    if (image->mask==NULL) {
		sprintf(event,"Calloc failed for image->mask");
		reportevt(flag_verbose,STATUS,5,event);
		exit(0);
	    }
	    /* ****** MASK is same size as IMAGE ****** */
	    if (((bpmsize[0]==imagesize[0]) && (bpmsize[1]==imagesize[1])) 
	      || (image->hdunum ==1)) {
	      if (fits_read_subset(image->fptr,TUSHORT,lx,ux,inc,
		  &image->shnullval,image->mask,&anynull,&status)) {
	        sprintf(event,"%s image read failed: %s",
		  imtypename[flag_type],currentimage);
	        reportevt(flag_verbose,STATUS,5,event);
	        printerror(status);  
	      }
	      if (flag_verbose==3) {
	        sprintf(event,"Read %s extension",imtypename[flag_type]);
	        reportevt(flag_verbose,STATUS,1,event);
	      }
	    }
	    else { /* MASK is different size from image */
	      /* NOTE:  this should not happen, but in DC4 we had a bunch */
	      /* buggy remap image masks and we added this to handle them */
	      /* recreate a BPM that is the same size as ORIGINAL IMAGE*/
	      for (i=0;i<image->npixels;i++) {
		image->mask[i] = 0;
		xpos = i%imagesize[0] ;
		ypos = i/imagesize[0] ;
		/* set BPM near edges */
		if (xpos<2 || ypos<2)image->mask[i]=1 ;
		if (abs(xpos-imagesize[0])<=2 || abs(ypos-imagesize[1])<=2)
		  image->mask[i]=1 ;
	      }
	      sprintf(event,"Size of BPM is different from the main image");
	      reportevt(flag_verbose,STATUS,4,event);
	      flag_bpmbad = 1;
	      sprintf(event,"Resetting BPM values at image edge to be 1");
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
	  /* **************************************************************** */
	  /* *********** Current extension is VARIANCE extension ************ */
	  /* **************************************************************** */
	  if (flag_type==DES_VARIANCE || flag_type==DES_WEIGHT ||
	    flag_type==DES_SIGMA) {  /* weight map in this extension */
     	    imagesize[0] = ux[0] - lx[0] + 1;
	    imagesize[1] = ux[1] - lx[1] + 1;
	    image->axes[0] = imagesize[0];
	    image->axes[1] = imagesize[1];
	    if ((image->npixels)==0)
	      image->npixels=image->axes[0]*image->axes[1];
	    image->varim=(float *)calloc(image->npixels,sizeof(float));
	    /* set the variance type in image structure */
	    image->variancetype=flag_type;
	    status=0;
            if (fits_read_subset(image->fptr,TFLOAT,lx,ux,inc,
	        &image->nullval,image->varim,&anynull,&status)) {
		sprintf(event,"%s image read failed: %s",
		  imtypename[image->variancetype],currentimage);
		reportevt(flag_verbose,STATUS,5,event);
		printerror(status);
	    }
	    if (flag_verbose==3) {
	      sprintf(event,"Read %s extension",
	  	imtypename[image->variancetype]);
	      reportevt(flag_verbose,STATUS,1,event);
	    }
	  }
	} /* end of HDU loop */


	/* **************************************************************** */
	/* *************  Setting VARIM saturated pixels ****************** */
	/* **************  in case where BPM wrong size ******************* */
	/* **************************************************************** */
	/* ******************** TEMPORARY DC4 FIX ************************* */
	/* **************************************************************** */

	if (flag_bpmbad){
	  sprintf(event,"Resetting  VARIM values to 0 for saturated pixels");
	  reportevt(flag_verbose,STATUS,1,event);
	  for (i=0;i<image->npixels;i++) 
	      if (image->image[i] > saturate) image->varim[i]=0.0;
          if ((image->axes[0]!=imagesize[0])||(image->axes[1]!=imagesize[1])) {
	    image->axes[0]=imagesize[0];
	    image->axes[1]=imagesize[1] ;
	  }
	}	
}



/* check whether name conforms to DESDM standards */
int check_image_name(name,mode,flag_verbose)
	char name[];
	int mode,flag_verbose;
{
	char	event[1000];
	void	reportevt();

	/* does file have suffix consistent with supported file flavors */
	if (!strncmp(&(name[strlen(name)-5]),".fits",5)) 
	  return(FLAVOR_FITS);
        else if (!strncmp(&(name[strlen(name)-8]),".fits.gz",8)) 
	  return(FLAVOR_GZ);
        else if (!strncmp(&(name[strlen(name)-8]),".fits.fz",8)) 
	  return(FLAVOR_FZ);
	else {
	  if (mode==REQUIRE_FITS) {
	    sprintf(event,
	      "fits or fits.gz or fits.fz image name suffix required:  %s",
	      name);
	    reportevt(flag_verbose,STATUS,5,event);
	    exit(0);
	  }
	  else if (mode==CHECK_FITS) {
	    return(0);
	  }
	  else {
	    sprintf(event,"Unsupported mode %d in check_image_name",mode);
	    reportevt(flag_verbose,STATUS,5,event);
	    exit(0);
	  }
	}
}

/* retrieve basic image information from header */
void readimsections(data,flag_verbose)
	desimage *data;
	int flag_verbose;
{
	int	hdu;
	static int status;
	char	comment[200],event[10000];
	void	decodesection();
	
	
	  /* get the BIASSEC information */
	  if (fits_read_key_str((data->fptr),"BIASSECA",(data->biasseca),
	    comment,&status)==KEY_NO_EXIST) {
	    printf("  Keyword BIASSECA not defined in %s\n",data->name);
	    printerror(status);
	  }
	  if (flag_verbose) printf("    BIASSECA = %s\n",(data->biasseca));	  
	  decodesection((data->biasseca),(data->biassecan),flag_verbose);
	
	  /* get the AMPSEC information */
	  if (fits_read_key_str(data->fptr,"AMPSECA",data->ampseca,comment,
	    &status) ==KEY_NO_EXIST) {
	    printf("  Keyword AMPSECA not defined in %s\n",data->name);
	    printerror(status);
	  }
	  if (flag_verbose) printf("    AMPSECA  = %s\n",data->ampseca);	  
	  decodesection(data->ampseca,data->ampsecan,flag_verbose);
	
	  /* get the BIASSEC information */
	  if (fits_read_key_str(data->fptr,"BIASSECB",data->biassecb,comment,
	    &status)==KEY_NO_EXIST) {
	    printf("  Keyword BIASSECB not defined in %s\n",data->name);
	    printerror(status);
	  }
	  if (flag_verbose) printf("    BIASSECB = %s\n",data->biassecb);	  
	  decodesection(data->biassecb,data->biassecbn,flag_verbose);
	
	  /* get the AMPSEC information */
	  if (fits_read_key_str(data->fptr,"AMPSECB",data->ampsecb,comment,
	    &status)==KEY_NO_EXIST) {
	    printf("  Keyword AMPSECB not defined in %s\n",data->name);
	    printerror(status);
	  }
	  if (flag_verbose) printf("    AMPSECB  = %s\n",data->ampsecb);	  
	  decodesection(data->ampsecb,data->ampsecbn,flag_verbose);

	  /* get the TRIMSEC information */
	  if (fits_read_key_str(data->fptr,"TRIMSEC",data->trimsec,comment,
	    &status) ==KEY_NO_EXIST) {
	    printf("  Keyword TRIMSEC not defined in %s\n",data->name);
	    printerror(status);
	  }
	  if (flag_verbose) printf("    TRIMSEC  = %s\n",data->trimsec);	  
	  decodesection(data->trimsec,data->trimsecn,flag_verbose);

	  /* get the DATASEC information */
	  if (fits_read_key_str(data->fptr,"DATASEC",data->datasec,comment,
	    &status) ==KEY_NO_EXIST) {
	      printf("  Keyword DATASEC not defined in %s\n",
		data->name);
	      printerror(status);
	  }
	  if (flag_verbose) printf("    DATASEC  = %s\n",data->datasec);	  
	  decodesection(data->datasec,data->datasecn,flag_verbose);
}



/* decode section string */
void decodesection(name,numbers,flag_verbose)
	char name[];
	int numbers[],flag_verbose;
{
	int	i,len;
	char	tempname[100],event[1000];
	void	reportevt();
	

	sprintf(tempname,"%s",name);
	len=strlen(tempname);
	for (i=0;i<len;i++) {
	  if (strncmp(&tempname[i],"0",1)<0 || strncmp(&tempname[i],"9",1)>0) 
	    tempname[i]=32; 
	} 
	sscanf(tempname,"%d %d %d %d",numbers,numbers+1,numbers+2,numbers+3);
	if (flag_verbose==3) {
	  sprintf(event,"Interpreted sections %s as %d %d %d %d",name,
	    numbers[0],numbers[1],numbers[2],numbers[3]);
	  reportevt(flag_verbose,STATUS,1,event);
	}
}


void mkdecstring(dec,answer,decd,decm,decs)
  float dec,*decs;
  int *decd,*decm;
  char answer[];
{
	if (dec>90.0 || dec<-90.0) {
	  printf("** Declination out of bounds:  %8.4f\n",dec);
	  exit(0);
	}
	*decd=(int)fabs(dec);
	*decm=(int)((fabs(dec)-(*decd))*60.0);
	*decs=((fabs(dec)-(*decd))*3600.0-(*decm)*60.0);
	if (dec<0.0) (*decd)*=-1;
	sprintf(answer,"%03d:%02d:%04.1f",*decd,*decm,*decs);
}

void mkrastring(ra,answer,rah,ram,ras)
  float ra,*ras;
  int *rah,*ram;
  char answer[];
{
	if (ra>=360.0) ra-=360.0;
	if (ra<0.0) ra+=360.0;
	ra/=15.0;  /* convert from degrees to hours */
	*rah=(int)ra;
	*ram=(int)((ra-(*rah))*60.0);
	*ras=((ra-(*rah))*3600.0-(*ram)*60.0);
	sprintf(answer,"%02d:%02d:%04.1f",*rah,*ram,*ras);

}

double raconvert(rastring,rah,ram,ras)
	char rastring[];
	int *rah,*ram;
	float *ras;
{
	int	i,len;
	double	val;
	char	tmp[200];
	
	sprintf(tmp,"%s",rastring);
	len=strlen(rastring);
	for (i=0;i<len;i++) 
	  if (!strncmp(&(tmp[i]),":",1) || !strncmp(&(tmp[i]),"'",1)) tmp[i]=32;
	sscanf(tmp,"%d %d %f",rah,ram,ras);
	val=(*rah)+(*ram)/60.0+(*ras)/3600.0;
	val*=15.0;
	if (VERBOSE) printf("%s %f\n",tmp,val);
	return(val);
}

double decconvert(decstring,decd,decm,decs)
	char decstring[];
	int *decd,*decm;
	float *decs;
{
	int	i,len;
	double	val;
	char	tmp[200];
	
	sprintf(tmp,"%s",decstring);
	len=strlen(decstring);
	for (i=0;i<len;i++) 
	  if (!strncmp(&(tmp[i]),":",1) || !strncmp(&(tmp[i]),"'",1)) tmp[i]=32;
	sscanf(tmp,"%d %d %f",decd,decm,decs);
	if ((*decd)>=0.0) val=(*decd)+(*decm)/60.0+(*decs)/3600.0;
	else val=(*decd)-(*decm)/60.0-(*decs)/3600.0;
	if (VERBOSE) printf("%s  %f\n",tmp,val);
	return(val);
}

void headercheck(image,filter,ccdnum,keyword,flag_verbose)
	desimage *image;
	char filter[],keyword[];
	int *ccdnum;
{

	char	comment[1000],value[1000],newfilter[1000]="",
		event[10000];
	int    hdutype,hdunum,check_image_name();
	long newccdnum;
	FILE *fp ;
	static int status=0;
	void	printerror(),reportevt();
	
	hdunum = 1;
	/*check to see if input file is a .fz file*/
	if (check_image_name(image->name,REQUIRE_FITS,flag_verbose)==FLAVOR_FITS)
	  {
	    fp = fopen(image->name,"r");
	    if(fp==NULL)
	      {hdunum =2;} 
	    else
	      {fclose(fp);}  
	  }  
	   else hdunum =2 ;
        /* make sure we are in the 1st extension */
        if (fits_movabs_hdu(image->fptr,hdunum,&hdutype,&status)) {
          sprintf(event,"Move to HDU=%d failed: %s",hdunum,image->name);
          reportevt(flag_verbose,STATUS,5,event);
          printerror(status);
        }

	/* Issue a status 4 if keyword matches DESPUPC  */
/*  From Joe:  I cannot understand why these are in here... there must be an error?
	if (!strcmp(keyword,"DESPUPC")) {
	  if (fits_read_keyword(image->fptr,keyword,value,comment,&status)==
	    KEY_NO_EXIST) {
	    sprintf(event,"Image does not contain keyword %s: %s",
	      keyword,image->name);
	    reportevt(flag_verbose,STATUS,4,event);
	    status = 0 ;
	    return ;
	  }
	}
*/

	/* if keyword matches DESMKPUP also look for  DESPUPL */
/*
	if (!strcmp(keyword,"DESMKPUP")) {
	  if (fits_read_keyword(image->fptr,keyword,value,comment,&status)==
	    KEY_NO_EXIST)
	    { 
	      status = 0;
	      if (fits_read_keyword(image->fptr,"DESPUPL",value,comment,&status)==
		  KEY_NO_EXIST)
		{
		  sprintf(event,"Image does not contain keyword %s: %s",
			  keyword,image->name);
		  reportevt(flag_verbose,STATUS,5,event);
		  exit(0);
		}
	    }
	      return ;
	}
*/
	/* check test keyword to confirm imagetype  */
	if (strcmp(keyword,"NOCHECK")) {
	  if (fits_read_keyword(image->fptr,keyword,value,comment,&status)==
	    KEY_NO_EXIST) {
	    sprintf(event,"Image does not contain keyword %s: %s",
	      keyword,image->name);
	    reportevt(flag_verbose,STATUS,5,event);
	    exit(0);
	  }
	}

	/* check the FILTER keyword value */
	if (strcmp(filter,"NOCHECK")) {
	  if (fits_read_key_str(image->fptr,"FILTER",newfilter,comment,
	    &status)==KEY_NO_EXIST) {
	    sprintf(event,"Image %s does not have FILTER keyword",
	      image->name);
	    reportevt(flag_verbose,STATUS,4,event);
	    status=0;
	  }

	  /* clean up filter */
	  /* simply transfer filter if it hasn't already been read */
	  if (!strlen(filter)) sprintf(filter,"%s",newfilter);
	  else {
/*          RAG 2012Nov26: changed comparison so that it matches first character of the FILTER
                           keyword, to prevent detailed filter information from throwing a STATUS4 */
/*	    if (strncmp(filter,newfilter,strlen(filter))) { */
	    if (strncmp(filter,newfilter,1)) { 
	      sprintf(event,"Image %s FILTER %s != current filter %s",
	        image->name,newfilter,filter);
	      reportevt(flag_verbose,STATUS,4,event);
	      status=0;
	    }
	  }
	}


	/* check the CCDNUM keyword value */
	if (fits_read_key_lng(image->fptr,"CCDNUM",&newccdnum,comment,&status)==
	  KEY_NO_EXIST) {
	  sprintf(event,"Image %s does not have CCDNUM keyword",
	    image->name);
	  reportevt(flag_verbose,STATUS,4,event);
	  status=0;
	}
	/* simply transfer ccdnum if it hasn't already been read */
	if (!(*ccdnum)) *ccdnum=newccdnum;
	else {
	  if (*ccdnum!=newccdnum) {
	    sprintf(event,"Image CCDNUM doesn't match %ld vs %d: %s",
	      newccdnum,*ccdnum,image->name);
	    reportevt(flag_verbose,STATUS,4,event);
	  }
	}
}

#define IMAGE 0
#define BPM 1
#define VARIANCE 2
/* make basic checks of structure of remap images */
int check_remap_image(flag_verbose,name,pixelscale,hdu_im,hdu_wt,hdu_bpm)
	int flag_verbose,*hdu_im,*hdu_wt,*hdu_bpm;
	char name[];
	float pixelscale;
{
	char	event[1000],typestring[3][80],type[80],comment[200];
	fitsfile *fptr;
	int	status=0,hdunum,flag_image=0,hdu,hdutype,nfound=0,anynull=0;
	float	nullval=0.0,*image,cd1_1,cd1_2,cd2_1,cd2_2,pixscaleratio1,
		pixscaleratio2;
	long	axes[7],npixels,fpixel;
	void	reportevt();

	/* initialize header types */
	sprintf(typestring[IMAGE],"IMAGE");
	sprintf(typestring[BPM],"MASK");
	sprintf(typestring[VARIANCE],"WEIGHT");
	pixelscale/=3600.0;
	*hdu_im=*hdu_wt=*hdu_bpm=-1;

	/* ***************************************************** */
	/* *********** check if the remap image exists ********* */
	/* ***************************************************** */
	if(fits_open_file(&fptr,name,READONLY,&status)) {
          sprintf(event,"Image discarded (decompress failed): %s",name);
       	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}


	/* ***************************************************** */
	/* ***************** remap image exists **************** */
	/* ***************************************************** */

	/* extract the pixel scale and compare to expectation */
        if (fits_read_key_flt(fptr,"CD1_1",&cd1_1,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD1_1 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD1_2",&cd1_2,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD1_2 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD2_1",&cd2_1,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD2_1 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD2_2",&cd2_2,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD2_2 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	pixscaleratio1=sqrt(Squ(cd1_1)+Squ(cd1_2))/pixelscale;
	pixscaleratio2=sqrt(Squ(cd2_1)+Squ(cd2_2))/pixelscale;
	if (pixscaleratio1<0.9 || pixscaleratio1>1.1 || pixscaleratio2<0.9 ||
	  pixscaleratio2>1.1) {
          sprintf(event,"Image discarded (Pixel scale wrong %.2f %.2f): %s",
	    pixscaleratio1,pixscaleratio2,name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}

	/* confirm that it has not more than 4 HDUs */
	if (fits_get_num_hdus(fptr,&hdunum,&status)) {
	  sprintf(event,"Image discarded (HDUNUM read failed): %s",
	    name);
	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	if (hdunum>4) {
	  sprintf(event,"Image discarded (HDUNUM=%d): %s",
	    hdunum,name);
	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	else for (hdu=0;hdu<hdunum;hdu++) {
          if (hdu>0) if (fits_movrel_hdu(fptr,1,&hdutype,&status))  {
            sprintf(event,"Image discarded (HDU movrel failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
          }
	  /* confirm that DES header markers are present */
          if (fits_read_key_str(fptr,"DES_EXT",type,comment,&status)==
            KEY_NO_EXIST) {
            sprintf(event,"Image discarded (No DES_EXT keyword found): %s",
	      name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  if (!strcmp(type,typestring[IMAGE])) *hdu_im=hdu;
	  else if (!strcmp(type,typestring[BPM])) *hdu_bpm=hdu;
	  else if (!strcmp(type,typestring[VARIANCE])) *hdu_wt=hdu;
	  else {
            sprintf(event,"Image discarded (HDU = %d  DES_EXT = %s): %s",
	      hdu,type,name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  /* extract the dimension of the image */
	  if (fits_read_keys_lng(fptr,"NAXIS",1,2,axes,&nfound,&status)) {
            sprintf(event,"Image discarded (NAXIS read failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  if (nfound!=2) {
            sprintf(event,"Image discarded (Dimension wrong %d): %s",
	      nfound,name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  /* do a test read of the last row of the image */
	  fpixel=1;
	  npixels=axes[0]*axes[1];
	  image = (float *) calloc(npixels,sizeof(float));
	  /*
	  fpixel=axes[0]*(axes[1]-1);
	  npixels=axes[0];
	  if (npixels>4096) npixels=4096;
	  */
	  if (fits_read_img(fptr,TFLOAT,fpixel,npixels,&nullval,image,&anynull,
	    &status)) {
            sprintf(event,"Image discarded (read failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    free(image);
	    return(1);
	  }
	  free(image);
	}
        /* close remap image */
        if(fits_close_file(fptr,&status)) {
          sprintf(event,"File close failed: %s",name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
        }

	return(0);
}


#undef VERBOSE
#undef IMAGE
#undef BPM
#undef VARIANCE

#define IMAGE 0
#define BPM 1
#define VARIANCE 2
#define EXPOSURE 3
/* This is the same as check_remap_image() except that the cp remap images now have four extentions */
int cp_check_remap_image(flag_verbose,name,pixelscale,hdu_im,hdu_wt,hdu_bpm,hdu_exp)
     int flag_verbose,*hdu_im,*hdu_wt,*hdu_bpm, *hdu_exp;
	char name[];
	float pixelscale;
{
	char	event[1000],typestring[3][80],type[80],comment[200];
	fitsfile *fptr;
	int	status=0,hdunum,flag_image=0,hdu,hdutype,nfound=0,anynull=0;
	float	nullval=0.0,*image,cd1_1,cd1_2,cd2_1,cd2_2,pixscaleratio1,
		pixscaleratio2;
	long	axes[7],npixels,fpixel;
	void	reportevt();

	/* initialize header types */
	sprintf(typestring[IMAGE],"IMAGE");
	sprintf(typestring[BPM],"MASK");
	sprintf(typestring[VARIANCE],"WEIGHT");
	sprintf(typestring[EXPOSURE],"EXPOSURE");
	pixelscale/=3600.0;
	*hdu_im=*hdu_wt=*hdu_bpm=*hdu_exp=-1;

	/* ***************************************************** */
	/* *********** check if the remap image exists ********* */
	/* ***************************************************** */
	if(fits_open_file(&fptr,name,READONLY,&status)) {
          sprintf(event,"Image discarded (decompress failed): %s",name);
       	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}


	/* ***************************************************** */
	/* ***************** remap image exists **************** */
	/* ***************************************************** */

	/* extract the pixel scale and compare to expectation */
        if (fits_read_key_flt(fptr,"CD1_1",&cd1_1,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD1_1 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD1_2",&cd1_2,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD1_2 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD2_1",&cd2_1,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD2_1 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
        if (fits_read_key_flt(fptr,"CD2_2",&cd2_2,comment,&status)==
          KEY_NO_EXIST) {
          sprintf(event,"Image discarded (No CD2_2 keyword found): %s",
	    name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	pixscaleratio1=sqrt(Squ(cd1_1)+Squ(cd1_2))/pixelscale;
	pixscaleratio2=sqrt(Squ(cd2_1)+Squ(cd2_2))/pixelscale;
	if (pixscaleratio1<0.9 || pixscaleratio1>1.1 || pixscaleratio2<0.9 ||
	  pixscaleratio2>1.1) {
          sprintf(event,"Image discarded (Pixel scale wrong %.2f %.2f): %s",
	    pixscaleratio1,pixscaleratio2,name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}

	/* confirm that it has four HDUs */
	if (fits_get_num_hdus(fptr,&hdunum,&status)) {
	  sprintf(event,"Image discarded (HDUNUM read failed): %s",
	    name);
	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	if (hdunum!=4) {
	  sprintf(event,"Image discarded (HDUNUM=%d): %s",
	    hdunum,name);
	  reportevt(flag_verbose,STATUS,4,event);
	  return(1);
	}
	else for (hdu=0;hdu<hdunum;hdu++) {
          if (hdu>0) if (fits_movrel_hdu(fptr,1,&hdutype,&status))  {
            sprintf(event,"Image discarded (HDU movrel failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
          }
	  /* confirm that DES header markers are present */
          if (fits_read_key_str(fptr,"DES_EXT",type,comment,&status)==
            KEY_NO_EXIST) {
            sprintf(event,"Image discarded (No DES_EXT keyword found): %s",
	      name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  if (!strcmp(type,typestring[IMAGE])) *hdu_im=hdu;
	  else if (!strcmp(type,typestring[BPM])) *hdu_bpm=hdu;
	  else if (!strcmp(type,typestring[VARIANCE])) *hdu_wt=hdu;
	  else if (!strcmp(type,typestring[EXPOSURE])) *hdu_exp=hdu; 
	  else {
            sprintf(event,"Image discarded (HDU = %d  DES_EXT = %s): %s",
	      hdu,type,name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  /* extract the dimension of the image */
	  if (fits_read_keys_lng(fptr,"NAXIS",1,2,axes,&nfound,&status)) {
            sprintf(event,"Image discarded (NAXIS read failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  if (nfound!=2) {
            sprintf(event,"Image discarded (Dimension wrong %d): %s",
	      nfound,name);
            reportevt(flag_verbose,STATUS,4,event);
	    return(1);
	  }
	  /* do a test read of the last row of the image */
	  fpixel=1;
	  npixels=axes[0]*axes[1];
	  image = (float *) calloc(npixels,sizeof(float));
	  /*
	  fpixel=axes[0]*(axes[1]-1);
	  npixels=axes[0];
	  if (npixels>4096) npixels=4096;
	  */
	  if (fits_read_img(fptr,TFLOAT,fpixel,npixels,&nullval,image,&anynull,
	    &status)) {
            sprintf(event,"Image discarded (read failed): %s",name);
            reportevt(flag_verbose,STATUS,4,event);
	    free(image);
	    return(1);
	  }
	  free(image);
	}
        /* close remap image */
        if(fits_close_file(fptr,&status)) {
          sprintf(event,"File close failed: %s",name);
          reportevt(flag_verbose,STATUS,4,event);
	  return(1);
        }

	return(0);
}


#undef VERBOSE
#undef IMAGE
#undef BPM
#undef VARIANCE
#undef EXPOSURE


