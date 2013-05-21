/*$Id: lut_utils.c 9268 2012-10-17 06:12:31Z donaldp $ */
/* Utilities for reading and utilizing look up tables (LUTs) */
//#include "imageproc.h"
#include "imsupport.h"
void  read_lut(lutfile,ccdnum,lutx,luta,lutb)
     char lutfile[];
     int  ccdnum ;
     float *lutx;
     double *luta,*lutb;
{
   fitsfile *fptr;      /* FITS file pointer, defined in fitsio.h */
/*   char *val, value[1000], nullstr[]="*"; */
/*   char keyword[FLEN_KEYWORD], colname[FLEN_VALUE] */
   char event[1000];
   int status = 0, j;   /*  CFITSIO status value MUST be initialized to zero!  */
   int hdunum, hdutype, ncols, ii, anynul;
   float xval;
   double aval,bval;
   long ctr,nrows;
   void reportevt();

   printf(" CCDNUM: %d \n",ccdnum);

   if (!fits_open_file(&fptr, lutfile, READONLY, &status)){
      if ( fits_get_hdu_num(fptr, &hdunum) == 1 ){
         /* This should be the primary array; try to move to the */
         /* nth+1 extension which should correspond to the CCD number requested */
         hdunum=ccdnum+1;
         fits_movabs_hdu(fptr, hdunum, &hdutype, &status);
      }else{
         fits_get_hdu_type(fptr, &hdutype, &status); /* Get the HDU type */
      }

      if (hdutype == IMAGE_HDU){
         printf("Error: this program only displays tables, not images\n");
      }else{
         fits_get_num_rows(fptr, &nrows, &status);
         fits_get_num_cols(fptr, &ncols, &status);
         lutx=(float *)calloc(nrows,sizeof(float));
         luta=(double *)calloc(nrows,sizeof(double));
         lutb=(double *)calloc(nrows,sizeof(double));
         if ((lutx==NULL)||(luta==NULL)||(lutb==NULL)){
            sprintf(event,"Calloc of LUT failed");
            reportevt(1,STATUS,5,event);
            exit(1);
         }

         printf(" nrows: %d  ncols: %d \n",nrows,ncols);
         /* find the number of columns that will fit within 80 characters */
         /* print column names as column headers */
         j = 0 ;
         for (ctr = 1; ctr <= nrows && !status; ctr++){
            fits_read_col_flt(fptr, 1, ctr, 1, 1, 0.0, &xval, &anynul, &status);
            fits_read_col_dbl(fptr, 2, ctr, 1, 1, 0.0, &aval, &anynul, &status);
            fits_read_col_dbl(fptr, 3, ctr, 1, 1, 0.0, &bval, &anynul, &status);
            lutx[j]=xval;
            luta[j]=aval;
            lutb[j]=bval;
            ++j;
         }
      }
      fits_close_file(fptr, &status);
      if (status) fits_report_error(stderr, status); /* print any error message */
/*      
      j=0;
      for (ctr=1;ctr <= nrows && !status; ctr++){
	 if ((j < 100)||(j%100 == 0)){
            printf("j: %6d x: %7.1f  a: %10.3f  b: %10.3f \n",j,lutx[j],luta[j],lutb[j]);
         }
         j++;
      } 
*/
      return;
   }else{
      sprintf(event,"Unable to open non-linearity lookup table: %s",lutfile);
      reportevt(2,STATUS,5,event);
   }
}
