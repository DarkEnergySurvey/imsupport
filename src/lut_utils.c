/*$Id$ */
/* Utilities for reading and utilizing look up tables (LUTs) */
#include "imsupport.h"

/* read_linearity_lut: Read a linearity correction look-up table (LUT) for
                    a specific CCD.  Currently the format of this LUT is a 
                    MEF FITS table where each HDU continains the appropriate
                    LUT for a given CCD.  Three columns are expected they are
                    1) a DN value, 
                    2) correction for the given DN value when it occurs for a 
                       pixel that is part of amplifier A,
                    3) similar to 2) but for amplifier B 
            Inputs: lutfile --> file name of FITS table to be read
                    ccdnum  --> ccd number of table desired (HDU=CCDNUM+1)
                    lutx    --> values from LUT for index value 
                    luta    --> values from LUT for amplifier A
                    lutb    --> values from LUT for amplifier B
*/

void  read_linearity_lut(lutfile,ccdnum,lutx,luta,lutb)
     char lutfile[];
     int  ccdnum;
     float **lutx;
     double **luta,**lutb;
{
   fitsfile *fptr;      /* FITS file pointer, defined in fitsio.h */
   char event[1000];
   int status = 0, j;   /*  CFITSIO status value MUST be initialized to zero!  */
   int hdunum, hdutype, ncols, ii, anynul;
   float xval;
   double aval,bval;
   long ctr,nrows;
   void reportevt();

/*   printf(" CCDNUM: %d \n",ccdnum); */

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
         printf("Error: Linear correction has unexpected HDU type, expected a MEF FITS table, found IMAGE_HDU?\n");
         exit(1);
      }else{
         /* Probe table to get size and allocate array space, currently no checks */
         fits_get_num_rows(fptr, &nrows, &status);
         fits_get_num_cols(fptr, &ncols, &status);
         *lutx=(float *)calloc(nrows,sizeof(float));
         *luta=(double *)calloc(nrows,sizeof(double));
         *lutb=(double *)calloc(nrows,sizeof(double));
         if ((lutx==NULL)||(luta==NULL)||(lutb==NULL)){
            sprintf(event,"Calloc of LUT failed");
            reportevt(1,STATUS,5,event);
            exit(1);
         }
         /* Read table */
         j = 0 ;
         for (ctr = 1; ctr <= nrows && !status; ctr++){
            fits_read_col_flt(fptr, 1, ctr, 1, 1, 0.0, &xval, &anynul, &status);
            fits_read_col_dbl(fptr, 2, ctr, 1, 1, 0.0, &aval, &anynul, &status);
            fits_read_col_dbl(fptr, 3, ctr, 1, 1, 0.0, &bval, &anynul, &status);
            (*lutx)[j]=xval;
            (*luta)[j]=aval;
            (*lutb)[j]=bval;
            ++j;
         }
      }
      fits_close_file(fptr, &status);
      if (status) fits_report_error(stderr, status); /* print any error message */
/*      
      j=0;
      for (ctr=1;ctr <= nrows && !status; ctr++){
	 if ((j < 100)||(j%100 == 0)){
            printf("j: %6d x: %7.1f  a: %10.3f  b: %10.3f \n",j,(*lutx)[j],(*luta)[j],(*lutb)[j]);
         }
         j++;
      } 
*/
      return;
   }else{
      sprintf(event,"Unable to open linearity lookup table: %s",lutfile);
      reportevt(2,STATUS,5,event);
      exit(1);
   }
}

/* lut_srch: Generic routine that finds a value based on a lookup table.
             If interpolation is requested then the appropriate calculation
             is made otherwise a nearest neighbor approach is used.
*/
float lut_srch(value,lut,flag_interp)
     const float  value;
     const double lut[];
     const int    flag_interp;
{
   int index;   
   float frac,retval;
   double dl;

/* RAG to make this more generic needs to know the size of the array so 
   that the bounds are not fixed as the are below here */

   if ((value < 0.0)||(value >= 65536.0)){
      retval=value;
/*    printf(" Outside lut range: value=%f  retval=%f  \n",value,retval); */
   }else{
      index=(int)value;
      frac=value-(float)index;
      if (flag_interp){
         dl=lut[index+1]-lut[index];
         retval=(float) (lut[index]+(dl*(double)frac));
      }else{
	if (frac >= 0.5){
           retval=(float)lut[index+1];
        }else{
           retval=(float)lut[index];
        }
      }
/*    printf(" NEAREST value: %.5f index=%d frac=%.5f  lut1=%f lut2=%f  retval=%f  \n",value,index,frac,lut[index],lut[index+1],retval); */
   }
   return(retval);    
}


