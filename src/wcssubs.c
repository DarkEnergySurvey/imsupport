/*
**
** wcssubs.c
**
** DESCRIPTION:
**     WCS conversion routines based on Emmanuel Bertin's terrapix code.
**     These routines provide an easy interface into the terapix wcs code
**     for use in the DES processing codes.
**
** AUTHOR:  Tony Darnell (tdarnell@uiuc.edu)
**
** Last commit:
**     $Rev$
**     $LastChangedBy$
**     $LastChangedDate$
**
*/

//#include "imageproc.h"
#include "fits/fitscat.h"
#include "imsupport.h"
#include "fitswcs.h"
#include "define.h"
#include "prefs.h"

/*
** Calculates the wcs coordinates of the four corners of an image.
** Uses PV distortion coeffs and any wcs information in the header
*/
void wcsCorners(char *catname, double retwcs[4][2], int verboseFlag){

  double rawpos[2],wcspos[2];

  catstruct *cat;
  tabstruct *tab, *imatab;
  wcsstruct *wcsin;

  if (!(cat = read_cat(catname)))
     error(EXIT_FAILURE, "*Error*: No such file: ", catname);

  tab = cat->tab;

  wcsin = read_wcs(tab);

  if (verboseFlag){
    printf("naxis--> %d\n",wcsin->naxis);
    printf("naxis1--> %d\n",wcsin->naxisn[0]);
    printf("naxis2--> %d\n",wcsin->naxisn[1]);
    printf("ctype1--> %s\n",wcsin->ctype[0]);
    printf("ctype2--> %s\n",wcsin->ctype[1]);
    printf("cunit1--> %s\n",wcsin->cunit[0]);
    printf("cunit2--> %s\n",wcsin->cunit[1]);
    printf("crval1--> %e\n",wcsin->crval[0]);
    printf("crval2--> %e\n",wcsin->crval[1]);
    printf("cdelt1--> %e\n",wcsin->cdelt[0]);
    printf("cdelt2--> %e\n",wcsin->cdelt[1]);
    printf("crpix1--> %e\n",wcsin->crpix[0]);
    printf("crpix2--> %e\n",wcsin->crpix[1]);
    printf("crder1--> %e\n",wcsin->crder[0]);
    printf("crder2--> %e\n",wcsin->crder[1]);
    printf("csyer1--> %e\n",wcsin->csyer[0]);
    printf("csyer2--> %e\n",wcsin->csyer[1]);
    printf("cd1_1 --> %e\n",wcsin->cd[0]);
    printf("cd2_1 --> %e\n",wcsin->cd[1]);
    printf("cd1_2 --> %e\n",wcsin->cd[2]);
    printf("cd2_2 --> %e\n\n",wcsin->cd[3]);

    printf("nprojp--> %d\n",wcsin->nprojp);
    printf("projp1--> %e\n",wcsin->projp[0]);
    printf("projp2--> %e\n",wcsin->projp[1]);
    printf("projp3--> %e\n",wcsin->projp[2]);
    printf("projp4--> %e\n",wcsin->projp[3]);
    printf("projp5--> %e\n",wcsin->projp[4]);
    printf("projp6--> %e\n",wcsin->projp[5]);
    printf("projp7--> %e\n",wcsin->projp[6]);
    printf("projp8--> %e\n",wcsin->projp[7]);
    printf("projp9--> %e\n",wcsin->projp[8]);
    printf("projp10--> %e\n",wcsin->projp[9]);
    printf("projp11--> %e\n\n",wcsin->projp[10]);

    printf("projp12--> %e\n",wcsin->projp[100]);
    printf("projp13--> %e\n",wcsin->projp[101]);
    printf("projp14--> %e\n",wcsin->projp[102]);
    printf("projp15--> %e\n",wcsin->projp[103]);
    printf("projp16--> %e\n",wcsin->projp[104]);
    printf("projp17--> %e\n",wcsin->projp[105]);
    printf("projp18--> %e\n",wcsin->projp[106]);
    printf("projp19--> %e\n",wcsin->projp[107]);
    printf("projp20--> %e\n",wcsin->projp[108]);
    printf("projp21--> %e\n",wcsin->projp[109]);
    printf("projp22--> %e\n",wcsin->projp[110]);
    
    printf("wcsmin1--> %f\n",wcsin->wcsmin[0]);
    printf("wcsmin2--> %f\n",wcsin->wcsmin[1]);
    printf("wcsmax1--> %f\n",wcsin->wcsmax[0]);
    printf("wcsmax1--> %f\n",wcsin->wcsmax[1]);
    printf("wcsscale1--> %f\n",wcsin->wcsscale[0]);
    printf("wcsscale2--> %f\n",wcsin->wcsscale[1]);
    printf("wcsmaxradius--> %f\n",wcsin->wcsmaxradius);
    printf("outmin--> %f\n",wcsin->outmin);
    printf("outmax--> %f\n",wcsin->outmax);
    printf("lat--> %d\n",wcsin->lat);
    printf("long--> %d\n",wcsin->lng);
    printf("r0--> %f\n",wcsin->r0);
    printf("lindet--> %f\n",wcsin->lindet);
    printf("chirality--> %d\n",wcsin->chirality);
    printf("ap2000--> %f\n",wcsin->ap2000);
    printf("dp2000--> %f\n",wcsin->dp2000);
    printf("ap1950--> %f\n",wcsin->ap1950);
    printf("dp1950--> %f\n",wcsin->dp1950);
    printf("obsdate--> %f\n",wcsin->obsdate);
    printf("equinox--> %f\n",wcsin->equinox);
    printf("epoch--> %f\n",wcsin->epoch);
    printf("radecsys--> %f\n",wcsin->radecsys);
    printf("celsys--> %f\n",wcsin->celsys);
    printf("celsysmat1--> %f\n",wcsin->celsysmat[0]);
    printf("celsysmat2--> %f\n",wcsin->celsysmat[1]);
    printf("celsysmat3--> %f\n",wcsin->celsysmat[2]);
    printf("celsysmat4--> %f\n",wcsin->celsysmat[3]);
    printf("celsysconvflag--> %d\n",wcsin->celsysconvflag);
  }

  /* lower left corner */
  wcspos[0]=0.0;
  wcspos[1]=0.0;
  rawpos[0] = 0.0;
  rawpos[1] = 0.0;
  raw_to_wcs(wcsin,rawpos,wcspos);
  retwcs[0][0]=wcspos[0];
  retwcs[0][1]=wcspos[1];

  /* upper left corner */
  rawpos[0] = 0;
  rawpos[1] = (float) wcsin->naxisn[1];

  raw_to_wcs(wcsin,rawpos,wcspos);
  retwcs[1][0]=wcspos[0];
  retwcs[1][1]=wcspos[1];

  /* upper right corner */
  rawpos[0] = (float) wcsin->naxisn[0];
  rawpos[1] = (float) wcsin->naxisn[1];

  raw_to_wcs(wcsin,rawpos,wcspos);
  retwcs[2][0]=wcspos[0];
  retwcs[2][1]=wcspos[1];

  /* lower right corner */
  rawpos[0] = (float) wcsin->naxisn[0];
  rawpos[1] = 0.0;

  raw_to_wcs(wcsin,rawpos,wcspos);
  retwcs[3][0]=wcspos[0];
  retwcs[3][1]=wcspos[1];

  if (verboseFlag){
    printf("*** Lower left corner (0,0) ***\n");
    printf("RA:  %f \nDEC: %f\n",retwcs[0][0],retwcs[0][1]);

    printf("*** upper left corner (0,%d) ***\n",wcsin->naxisn[1]);
    printf("RA:  %f \nDEC: %f\n",retwcs[1][0],retwcs[1][1]);

    printf("*** upper right corner (%d,%d) ***\n",
        wcsin->naxisn[0],wcsin->naxisn[1]);
    printf("RA:  %f \nDEC: %f\n",retwcs[2][0],retwcs[2][1]);

    printf("*** lower right corner (%d,0) ***\n",wcsin->naxisn[0]);
    printf("RA:  %f \nDEC: %f\n",retwcs[3][0],retwcs[3][1]);
  }

  /* printf("%f\n",wcsin->projp[10]); */

}

/*
** xy2wcs
**
** Convert a pixel location from an image to its wcs coordinates
**
*/
void xy2wcs(char *catname, double x, double y, double *wcspos, int verboseFlag){

  double rawpos[2];

  catstruct *cat;
  tabstruct *tab;
  wcsstruct *wcsin;

  rawpos[0] = x;
  rawpos[1] = y;

  /* Read file */
  if (!(cat = read_cat(catname)))
     error(EXIT_FAILURE, "*Error*: No such file: ", catname);

  tab = cat->tab;

  /* Read wcs data structure */
  wcsin = read_wcs(tab);

  /* Do the conversion */
  raw_to_wcs(wcsin,rawpos,wcspos);

  /* Print the results */
  if (verboseFlag){
    printf ("%d,%d => %f,%f\n",x,y,wcspos[0],wcspos[1]);
  }

}

/*
** wcs2xy
**
** Convert wcs coordinates to a pixel x,y
**
*/
void wcs2xy(char *catname, double *wcspos, double *rawpos, int verboseFlag){

  catstruct *cat;
  tabstruct *tab;
  wcsstruct *wcsin;

  /* Read file */
  if (!(cat = read_cat(catname)))
     error(EXIT_FAILURE, "*Error*: No such file: ", catname);

  tab = cat->tab;

  /* Read wcs data structure */
  wcsin = read_wcs(tab);

  /* Do the conversion */
  wcs_to_raw(wcsin,wcspos,rawpos);

  /* Print the results */
  if (verboseFlag){
    printf ("%f,%f => %f,%f\n",wcspos[0],wcspos[1],rawpos[0],rawpos[1]);
  }

}
