/* imreadsubs.c */

/* Image structure */
typedef struct {
  char     name[1000];
  char     biasseca[100],biassecb[100],
    ampseca[100], ampsecb[100],
    trimsec[100], datasec[100],
    dataseca[100],datasecb[100];
  long     axes[7],bscale,bzero,bitpix,npixels,fpixel;
  float    saturateA,saturateB,
    gainA,gainB,
    rdnoiseA,rdnoiseB,
    exptime,crpix1,crpix2;
  int      nfound,hdunum,unit,varunit,maskunit;
  int      biassecan[4],biassecbn[4],
    ampsecan[4], ampsecbn[4],
    trimsecn[4], datasecn[4],
    datasecan[4],datasecbn[4];
  fitsfile *fptr;
  float    *image,*varim,nullval;
  int      variancetype;
  short    *mask,shnullval;
} desimage;

/* FITS Catalog structure */
typedef struct {
  char    name[200];
  long    axes[7],bscale,bzero,bitpix,npixels,fpixel;
  float   *image,*varim,nullval;
  short   *mask;
  int     nfound,hdunum;
  int     biassec0n[4],biassec1n[4],
    ampsec0n[4], ampsec1n[4],
    trimsecn[4], datasecn[4];
  long    nrows,*repeat,*width;
  int     ncols,*typecode,hdutype;
} descat;

void printerror(int status);
void init_desimage(desimage *image);
void destroy_desimage(desimage *image);
void copy_desimage(desimage *destination,desimage *src);
void rd_desimage(desimage *image, int mode, int flag_verbose);
void rd_dessubimage(desimage *image, long *lx, long *ux, int mode, int flag_verbose);
int check_image_name(char name[], int mode, int flag_verbose);
void readimsections(desimage *data, int flag_verbose);
void decodesection(char name[], int numbers[], int flag_verbose);
void mkdecstring(double dec, char answer[], int *decd, int *decm, float *decs);
void mkrastring(double ra, char answer[], int *rah, int *ram, float *ras);
double raconvert(char rastring[], int *rah, int *ram, float *ras);
double decconvert(char decstring[], int *decd, int *decm, float *decs);
void headercheck(desimage *image, char filter[], int *ccdnum, char keyword[], int flag_verbose);
int check_remap_image(int flag_verbose, char name[], double pixelscale, int *hdu_im, int *hdu_wt, int *hdu_bpm);
int cp_check_remap_image(int flag_verbose, char name[], double pixelscale, int *hdu_im, int *hdu_wt, int *hdu_bpm, int *hdu_exp);
