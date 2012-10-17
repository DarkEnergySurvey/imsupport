/*$Id$ */
/* reads USNOB catalog from astrostds file. This assumes that the data is in the 2nd HDU*/
//#include "imageproc.h"
#include "imsupport.h"
void  readstdstable(scampfile,ramin,ramax,decmin,decmax,tra,tdec,b2,r2,i2,match)
     char *scampfile ;
     float ramin, ramax,decmin,decmax, r2[],b2[],i2[] ;
     double tra[],tdec[] ;
     int *match ;
{
    fitsfile *fptr;      /* FITS file pointer, defined in fitsio.h */
    char *val, value[1000], nullstr[]="*";
    char keyword[FLEN_KEYWORD], colname[FLEN_VALUE],event[1000];
    int status = 0, j;   /*  CFITSIO status value MUST be initialized to zero!  */
    int hdunum, hdutype, ncols, ii, anynul, dispwidth[1000];
    float r2mag,b2mag,i2mag,tmp ;
    double ra,dec  ;
    int firstcol, lastcol = 0, linewidth;
    long ctr,nrows;
    void reportevt() ;
     
    if (ramax<ramin)
      {/*swap max and min RA,DEC values*/
	tmp = ramax ;
	ramax = ramin;
	ramin= tmp;
      }

    if (decmax<decmin)
      {
    /*swap max and min RA,DEC values*/
	tmp = decmax ;
	decmax = decmin;
	decmin= tmp;
      }
    /*    printf ("Min and max values are %f %f %f %f\n",ramin,ramax,decmin,decmax); */

    if (!fits_open_file(&fptr, scampfile, READONLY, &status))
    {
      if ( fits_get_hdu_num(fptr, &hdunum) == 1 )
          /* This is the primary array;  try to move to the */
          /* first extension and see if it is a table */
          fits_movabs_hdu(fptr, 2, &hdutype, &status);
       else 
          fits_get_hdu_type(fptr, &hdutype, &status); /* Get the HDU type */

      if (hdutype == IMAGE_HDU) 
          printf("Error: this program only displays tables, not images\n");
      else  
      {
        fits_get_num_rows(fptr, &nrows, &status);
        fits_get_num_cols(fptr, &ncols, &status);
        /* find the number of columns that will fit within 80 characters */
          /* print column names as column headers */
	j = 0 ;
	for (ctr = 1; ctr <= nrows && !status; ctr++) {
	      fits_read_col_dbl(fptr,1,ctr, 1, 1, 0.0,&ra, &anynul, &status);
	      fits_read_col_dbl(fptr,2,ctr, 1, 1, 0.0,&dec, &anynul, &status);
	      fits_read_col_flt(fptr,5,ctr, 1, 1, 0.0,&r2mag, &anynul, &status);
	      fits_read_col_flt(fptr,6,ctr, 1, 1, 0.0,&b2mag, &anynul, &status);
	      fits_read_col_flt(fptr,7,ctr, 1, 1, 0.0,&i2mag, &anynul, &status);
	      if ((ramin <= ra) && (ramax >= ra) && (decmin <= dec) && (decmax >= dec) 
		  && (b2mag>1) && (r2mag>1) &&(i2mag>1) && ((b2mag<13) || (r2mag<13) || (i2mag<13)))
		{
		  tra[j] = ra ; 
		  tdec[j] = dec; 
		  r2[j] = r2mag;
		  b2[j] = b2mag ;
		  i2[j] = i2mag ;
		  ++j ;
		}
	}
      fits_close_file(fptr, &status);
      }
 
 
        
 
    *match = j ;
     if (status) fits_report_error(stderr, status); /* print any error message */
    return;
    }
    else
      {sprintf(event,"Unable to open astrostd file %s",scampfile);
	reportevt(2,STATUS,5,event);
      }
	
}
