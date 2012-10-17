/*
** ora_utils.c
**
** A package of routines to access the DES database from C programs.
**
** AUTHOR:  Tony Darnell (tdarnell@illinois.edu)
** DATE:    20-July-2009
**
** $Rev::                                             $ Revision of last commit
** $Author::                                          $ Author of last commit
** $Date::                                            $ Date of last commit
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ora_utils.h"

#define DB_USER "DB_USER"
#define DB_PASSWD "DB_PASSWD"
#define DB_SERVER_RW "DB_SERVER"
#define DB_NAME_RW "DB_NAME"

//
// make a db connection
//
int dbconnect() {

  static sword status;
  static text *dbname = (text *) "desdb.cosmology.illinois.edu/des";
  static ub4 ulen,plen,dlen;

  char filename[1000], line[1000], keyword[100], value[100], 
       username[100],password[100];
  FILE *infile;

  sprintf(filename,"%s/.desdm",getenv("HOME"));

  infile = fopen(filename,"r");
  if (infile == NULL){
    printf("Database config file %s not found", filename);
    exit(0);
  } 

  while (fgets(line,1000,infile)!=NULL) {
    sscanf(line,"%s %s",keyword,value);
    if (!strcmp(keyword,DB_USER)) sprintf(username,"%s",value);
    if (!strcmp(keyword,DB_PASSWD)) sprintf(password,"%s",value);
  }

  fclose(infile);

  (void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
                       (dvoid * (*)(dvoid *, size_t)) 0,
                       (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                       (void (*)(dvoid *, dvoid *)) 0 );

  ulen = (ub4) strlen((char *) username);
  plen = (ub4) strlen((char *) password);
  dlen = (ub4) strlen((char *) dbname);

  status = OCIEnvInit(&envhp,OCI_DEFAULT,(size_t) 0,(dvoid **) 0 );


  status = OCIHandleAlloc(envhp,(dvoid **) &errhp,OCI_HTYPE_ERROR,
                          (size_t) 0,(dvoid **) 0);

  status = OCIHandleAlloc(envhp,(dvoid **) &svchp,OCI_HTYPE_SVCCTX,
                          (size_t) 0,(dvoid **) 0);

  status = OCILogon(envhp,errhp,&svchp,username,ulen,password,plen,dbname,dlen);

  checkDBerr(errhp,status);
  
  return(status);
}

int dbdisconnect() {
static sword status;

  status = OCILogoff(svchp,errhp);
  //checkDBerr(errhp,status);

  return(status);
}

void checkDBerr(OCIError *errhp, sword status)
{
  text errbuf[512];
  sb4 errcode = 0;

  switch (status)
  {
  case OCI_SUCCESS:
    break;
  case OCI_SUCCESS_WITH_INFO:
    printf("Error - OCI_SUCCESS_WITH_INFO\n");
    break;
  case OCI_NEED_DATA:
    printf("Error - OCI_NEED_DATA\n");
    break;
  case OCI_NO_DATA:
    printf("Error - OCI_NO_DATA\n");
    //exit(0);
    break;
  case OCI_ERROR:
    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
                        errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
    if (errcode != 1405){
      printf("Error - %.*s\n", 512, errbuf);
    }
    break;
  case OCI_INVALID_HANDLE:
    printf("Error - OCI_INVALID_HANDLE\n");
    break;
  case OCI_STILL_EXECUTING:
    printf("Error - OCI_STILL_EXECUTE\n");
    break;
  case OCI_CONTINUE:
    printf("Error - OCI_CONTINUE\n");
    break;
  default:
    break;
  }
}

imageIdStruct *getCoaddIdsForTile(char *tile, char *run) {

  static OCIBind *bndp0 = (OCIBind *) 0;
  static OCIBind *bndp1 = (OCIBind *) 0;
  static OCIDefine *dfnhp[2];
  static sword status;

  imageIdStruct *coaddIdArr;
  coaddIdArr = (imageIdStruct*) malloc(sizeof(imageIdStruct));

  int numCoadds = 0, i;

  //
  // SQL to get the coadd ids for this tile.  Currently using the 
  // one from the latest run.  The following SQL should return six images
  // (one for each band and the coadd_det, in no particlar order)
  //
  /*
  static text *imageCountSelect =
    (text *)"SELECT count(*) FROM coadd WHERE tilename = :tile "
            "AND run = (SELECT max(run) FROM coadd WHERE "
            "project = 'DES' and tilename = :tile)";

  static text *coaddIdSelect = 
    (text *)"SELECT id FROM coadd " "WHERE tilename = :tile "
            "AND run = (SELECT max(run) FROM coadd WHERE "
            "project = 'DES' and tilename = :tile)";
  static text *imageCountSelect =
    (text *)"SELECT count(*) FROM coadd WHERE tilename = :tile "
            "AND run = '20090809212530_DES2219-4147'";

  static text *coaddIdSelect = 
    (text *)"SELECT id FROM coadd WHERE tilename = :tile "
            "AND run = '20090809212530_DES2219-4147'";
    
  */

  static text *imageCountSelect =
    (text *)"SELECT count(*) FROM coadd WHERE tilename = :tile "
            "AND run = :run";

  static text *coaddIdSelect = 
    (text *)"SELECT id FROM coadd WHERE tilename = :tile "
            "AND run = :run";

  status = OCIStmtPrepare(sthp, errhp, imageCountSelect,
      (ub4) strlen((char *) imageCountSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp0, errhp, (text *) ":tile",
      -1, (dvoid *) tile, (sword) 13, SQLT_STR, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp1, errhp, (text *) ":run",
      -1, (dvoid *) run, (sword) strlen((char *) run)+1, SQLT_STR, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, 1,
        (dvoid *) &numCoadds, sizeof(int), SQLT_INT, (dvoid *) 0, 0,
        (ub2 *)0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtFetch(sthp, errhp, 1, 0, 0);

  printf ("There are %d coadds in this tile: %s\n",numCoadds, tile);

  //
  // Now let's get the coadd ids
  //
  status = OCIStmtPrepare(sthp, errhp, coaddIdSelect,
      (ub4) strlen((char *) coaddIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp0, errhp, (text *) ":tile",
      -1, (dvoid *) tile, (sword) 13, SQLT_STR, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp1, errhp, (text *) ":run",
      -1, (dvoid *) run, (sword) strlen((char *) run)+1, SQLT_STR, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 1,
        (dvoid *) &coaddIdArr[0].imageId, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(imageIdStruct),0,0,0); 
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) numCoadds, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);

  checkDBerr(errhp, status);

  //for (i=0;i < numCoadds;i++){
  //  printf("%d\n",coaddIdArr[i].imageId);
  //}

  return coaddIdArr;

}

int getNumDESTiles(){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[1];
  int numTiles = 0;
  static sword status;

  static text *tileCountSelect =
    (text *)"SELECT count(*) FROM COADDTILE WHERE project='DES'";

  /*
  ** Get the number of DES tiles
  */
  status = OCIStmtPrepare(sthp, errhp, tileCountSelect,
      (ub4) strlen((char *) tileCountSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &numTiles, (sb4) (sizeof(numTiles)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  return(numTiles);

}

int getDESTileId(char *tile){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[1];
  int tileId = 0;
  static sword status;

  static text *tileIdSelect =
    (text *)"SELECT COADDTILE_ID FROM COADDTILE WHERE tilename = :tile";

  /*
  ** Get the tile id
  */
  status = OCIStmtPrepare(sthp, errhp, tileIdSelect,
      (ub4) strlen((char *) tileIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":tile",
      -1, (dvoid *) tile, (sword) 13, SQLT_STR, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &tileId, (sb4) (sizeof(tileId)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);

  checkDBerr(errhp, status);

  return(tileId);

}

double getCoaddZeropoint(int coaddId) {

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[1];
  static sword status;
  double zeropoint;

  printf ("coaddId:  %d\n",coaddId);

  static text *zeroSelect =
    (text *)"SELECT MAG_ZERO FROM ZEROPOINT WHERE IMAGEID = :id";

  /*
  ** Get the zeropoint
  */
  status = OCIStmtPrepare(sthp, errhp, zeroSelect,
      (ub4) strlen((char *) zeroSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &coaddId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &zeropoint, (sb4) (sizeof(zeropoint)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  printf ("In here %f\n",zeropoint);

  return(zeropoint);

}

tileStruct *getDESTiles(tileStruct *desTiles, int numTiles) {

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[7];
  static sword status;

  int i;

  static text *tileSelect =
    (text *)"SELECT coaddtile_id,ra,dec,equinox,pixelsize,npix_ra,npix_dec"
    " FROM COADDTILE WHERE project='DES' order by tilename";

  //
  // Get the tiles
  //
  //desTiles = (tileStruct *) malloc(sizeof(tileStruct) * numTiles);

  status = OCIStmtPrepare(sthp, errhp, tileSelect,
      (ub4) strlen((char *) tileSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  //
  // Set statement handle to get numTiles rows at a time for efficiency
  //
  // ub4 prefetch = numTiles;
  //status = OCIAttrSet((dvoid *)sthp, (ub4) OCI_HTYPE_STMT, 
  //    (dvoid *) &prefetch, 0, (ub4) OCI_ATTR_PREFETCH_ROWS, errhp);
  //checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 1,
        (dvoid *) &desTiles[0].tileId, (sb4) (sizeof(desTiles[0].tileId)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 2,
        (dvoid *) &desTiles[0].ra, (sb4) (sizeof(desTiles[0].ra)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[3], errhp, (ub4) 3,
        (dvoid *) &desTiles[0].dec, (sb4) (sizeof(desTiles[0].dec)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[4], errhp, (ub4) 4,
        (dvoid *) &desTiles[0].equinox, (sb4) (sizeof(desTiles[0].equinox)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[5], errhp, (ub4) 5,
        (dvoid *) &desTiles[0].pixelsize, (sb4) (sizeof(desTiles[0].pixelsize)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[6], errhp, (ub4) 6,
        (dvoid *) &desTiles[0].npixra, (sb4) (sizeof(desTiles[0].npixra)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[7], errhp, (ub4) 7,
        (dvoid *) &desTiles[0].npixdec, (sb4) (sizeof(desTiles[0].npixdec)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineArrayOfStruct(dfnhp[1], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[3], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[4], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[5], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[6], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[7], errhp, sizeof(desTiles[0]),0,0,0); 
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) numTiles, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  //for (i=0;i < numTiles;i++){
  //  printf("%f\n",desTiles[i].pixelsize);
  //}
  //exit(0);

  return (desTiles);

}

imageIdStruct *getImageIdsForCoadd( int coaddId, imageIdStruct *imageIdArr){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[2];
  static sword status;

  //imageIdStruct *imageIdArr;
  //imageIdArr = (imageIdStruct*) malloc(sizeof(imageIdStruct) * 1000);

  int i;

  //
  // SQL to get the coadd ids for this tile.  Currently using the 
  // one from the latest run.  The following SQL should return six images
  // (one for each band and the coadd_det, in no particlar order)
  //
  static text *imgIdSelect = (text *)"SELECT image.id,coadd_src.magzp "
                              "FROM image,coadd_src "
                              "WHERE coadd_src.coadd_imageid=:id "
                              "AND coadd_src.src_imageid = image.id";

  status = OCIStmtPrepare(sthp, errhp, imgIdSelect,
      (ub4) strlen((char *) imgIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &coaddId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 1,
        (dvoid *) &imageIdArr[0].imageId, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 2,
        (dvoid *) &imageIdArr[0].magZP, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineArrayOfStruct(dfnhp[1], errhp, sizeof(imageIdStruct),0,0,0); 
  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(imageIdStruct),0,0,0); 
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) MAXIMGS, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  //checkDBerr(errhp, status);

}

imageInfo *getImageInfoFromDB(int imageId, imageInfo *imageHeader){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[51];
  static sword status;

  int i;

  static text *imgSelect = (text *)"SELECT naxis1,naxis2,band,ra,dec,exptime,"
    "ccd,gaina,gainb,"
    "rdnoisea,rdnoiseb,equinox,wcsdim,ctype1,ctype2,cunit2,cunit2,"
    "crval1,crval2,crpix1,crpix2,"
    "cd1_1,cd1_2,cd2_1,cd2_2,"
    "pv1_0,pv1_1,pv1_2,pv1_3,pv1_4,pv1_5,pv1_6,pv1_7,pv1_8,pv1_9,pv1_10,"
    "pv2_0,pv2_1,pv2_2,pv2_3,pv2_4,pv2_5,pv2_6,pv2_7,pv2_8,pv2_9,pv2_10,"
    "skybrite,skysigma,imagetype,psfscale,fwhm"
    " FROM image WHERE id=:id ";

  status = OCIStmtPrepare(sthp, errhp, imgSelect,
      (ub4) strlen((char *) imgSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  //
  // Set statement handle to get 1000 rows at a time for efficiency
  //
  ub4 prefetch = 1000;
  status = OCIAttrSet((dvoid *)sthp, (ub4) OCI_HTYPE_STMT, 
      (dvoid *) &prefetch, 0, (ub4) OCI_ATTR_PREFETCH_ROWS, errhp);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &imageId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);


  /* Need to bind each column individually */
  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &imageHeader->naxis1, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 2,
        (dvoid *) &imageHeader->naxis2, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 3,
        (dvoid *) &imageHeader->band, (sb4) (sizeof(imageHeader->band)),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);
  status = OCIDefineByPos(sthp, &dfnhp[3], errhp, (ub4) 4,
        (dvoid *) &imageHeader->ra, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[4], errhp, (ub4) 5,
        (dvoid *) &imageHeader->dec, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[5], errhp, (ub4) 6,
        (dvoid *) &imageHeader->exptime, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[6], errhp, (ub4) 7,
        (dvoid *) &imageHeader->ccd, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);
  
  status = OCIDefineByPos(sthp, &dfnhp[7], errhp, (ub4) 8,
        (dvoid *) &imageHeader->gaina, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[8], errhp, (ub4) 9,
        (dvoid *) &imageHeader->gainb, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[9], errhp, (ub4) 10,
        (dvoid *) &imageHeader->rdnoisea, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[10], errhp, (ub4) 11,
        (dvoid *) &imageHeader->rdnoiseb, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[11], errhp, (ub4) 12,
        (dvoid *) &imageHeader->equinox, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[12], errhp, (ub4) 13,
        (dvoid *) &imageHeader->wcsdim, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[13], errhp, (ub4) 14,
        (dvoid *) &imageHeader->ctype1, (sb4) sizeof(imageHeader->ctype1),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[14], errhp, (ub4) 15,
        (dvoid *) &imageHeader->ctype2, (sb4) sizeof(imageHeader->ctype2),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[15], errhp, (ub4) 16,
        (dvoid *) &imageHeader->cunit1, (sb4) sizeof(imageHeader->cunit1),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[16], errhp, (ub4) 17,
        (dvoid *) &imageHeader->cunit2, (sb4) sizeof(imageHeader->cunit2),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[17], errhp, (ub4) 18,
        (dvoid *) &imageHeader->crval1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[18], errhp, (ub4) 19,
        (dvoid *) &imageHeader->crval2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[19], errhp, (ub4) 20,
        (dvoid *) &imageHeader->crpix1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[20], errhp, (ub4) 21,
        (dvoid *) &imageHeader->crpix2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[21], errhp, (ub4) 22,
        (dvoid *) &imageHeader->cd1_1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[22], errhp, (ub4) 23,
        (dvoid *) &imageHeader->cd1_2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[23], errhp, (ub4) 24,
        (dvoid *) &imageHeader->cd2_1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[24], errhp, (ub4) 25,
        (dvoid *) &imageHeader->cd2_2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[25], errhp, (ub4) 26,
        (dvoid *) &imageHeader->pv1_0, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[26], errhp, (ub4) 27,
        (dvoid *) &imageHeader->pv1_1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[27], errhp, (ub4) 28,
        (dvoid *) &imageHeader->pv1_2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[28], errhp, (ub4) 29,
        (dvoid *) &imageHeader->pv1_3, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[29], errhp, (ub4) 30,
        (dvoid *) &imageHeader->pv1_4, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[30], errhp, (ub4) 31,
        (dvoid *) &imageHeader->pv1_5, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[31], errhp, (ub4) 32,
        (dvoid *) &imageHeader->pv1_6, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[32], errhp, (ub4) 33,
        (dvoid *) &imageHeader->pv1_7, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[33], errhp, (ub4) 34,
        (dvoid *) &imageHeader->pv1_8, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[34], errhp, (ub4) 35,
        (dvoid *) &imageHeader->pv1_9, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[35], errhp, (ub4) 36,
        (dvoid *) &imageHeader->pv1_10, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[36], errhp, (ub4) 37,
        (dvoid *) &imageHeader->pv2_0, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[37], errhp, (ub4) 38,
        (dvoid *) &imageHeader->pv2_1, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[38], errhp, (ub4) 39,
        (dvoid *) &imageHeader->pv2_2, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[39], errhp, (ub4) 40,
        (dvoid *) &imageHeader->pv2_3, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[40], errhp, (ub4) 41,
        (dvoid *) &imageHeader->pv2_4, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[41], errhp, (ub4) 42,
        (dvoid *) &imageHeader->pv2_5, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[42], errhp, (ub4) 43,
        (dvoid *) &imageHeader->pv2_6, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[43], errhp, (ub4) 44,
        (dvoid *) &imageHeader->pv2_7, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[44], errhp, (ub4) 45,
        (dvoid *) &imageHeader->pv2_8, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[45], errhp, (ub4) 46,
        (dvoid *) &imageHeader->pv2_9, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[46], errhp, (ub4) 47,
        (dvoid *) &imageHeader->pv2_10, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[47], errhp, (ub4) 48,
        (dvoid *) &imageHeader->skybrite, (sb4) (sizeof(float)),
        SQLT_BFLOAT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[48], errhp, (ub4) 49,
        (dvoid *) &imageHeader->skysigma, (sb4) (sizeof(float)),
        SQLT_BFLOAT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[49], errhp, (ub4) 50,
        (dvoid *) &imageHeader->imageType, 
        (sb4) (sizeof(imageHeader->imageType)),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[50], errhp, (ub4) 51,
        (dvoid *) &imageHeader->psfscale, (sb4) (sizeof(float)),
        SQLT_BFLOAT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[51], errhp, (ub4) 52,
        (dvoid *) &imageHeader->fwhm, (sb4) (sizeof(float)),
        SQLT_BFLOAT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);

  checkDBerr(errhp, status);

}

parentStruct *getImageParentId(int imageId, parentStruct *parent) {

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnp[2];
  static sword status;

  int i;

  //
  // SQL to get the coadd ids for this tile.  Currently using the 
  // one from the latest run.  The following SQL should return six images
  // (one for each band and the coadd_det, in no particlar order)
  //
  static text *imgIdSelect = (text *)"SELECT parentid,imagetype FROM image "
                              "WHERE id=:id ";

  status = OCIStmtPrepare(sthp, errhp, imgIdSelect,
      (ub4) strlen((char *) imgIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &imageId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnp[0], errhp, (ub4) 1,
        (dvoid *) &parent->parentId, (sb4) (sizeof(parent->parentId)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnp[1], errhp, (ub4) 2,
        (dvoid *) &parent->parentType, (sb4) (sizeof(parent->parentType)),
        SQLT_STR, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

}

int getExposureId(int imageId) {

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnp = (OCIDefine *) 0;
  static sword status;

  int exposureId, i;

  //
  // SQL to get the coadd ids for this tile.  Currently using the 
  // one from the latest run.  The following SQL should return six images
  // (one for each band and the coadd_det, in no particlar order)
  //
  static text *imgIdSelect = (text *)"SELECT exposureid FROM image "
                              "WHERE id=:id ";

  status = OCIStmtPrepare(sthp, errhp, imgIdSelect,
      (ub4) strlen((char *) imgIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &imageId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnp, errhp, (ub4) 1,
        (dvoid *) &exposureId, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  return (exposureId);

}

double getImageZeropoint(int imageId, double *coaddZeroPoint) {

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnp = (OCIDefine *) 0;
  static sword status;

  int i;
  double zeroPoint;

  static text *zpIdSelect = (text *)"SELECT mag_zero FROM zeropoint "
                              "WHERE imageid=:id ";

  status = OCIStmtPrepare(sthp, errhp, zpIdSelect,
      (ub4) strlen((char *) zpIdSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIBindByName(sthp, &bndp, errhp, (text *) ":id",
      -1, (dvoid *) &imageId, sizeof(int), SQLT_INT, (dvoid *) 0,
      (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnp, errhp, (ub4) 1,
        (dvoid *) &zeroPoint, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  //checkDBerr(errhp, status);

  *coaddZeroPoint = zeroPoint;

  return;

}








int getNumCoaddObj(){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[1];
  int numCoaddObj = 0;
  static sword status;

  static text *objCountSelect =
    (text *)"SELECT count(*) FROM COADD_OBJECTS, COADD WHERE imageid_i=coadd.id and coadd.run='20100624172007_DES2239-3939'"
        " and spheroid_aspect_world_g is not NAN"
        " and spheroid_aspect_world_r is not NAN"
        " and spheroid_aspect_world_i is not NAN"
        " and spheroid_aspect_world_z is not NAN"
        " and spheroid_aspect_world_y is not NAN"
        " and disk_aspect_world_g is not NAN"
        " and disk_aspect_world_r is not NAN"
        " and disk_aspect_world_i is not NAN"
        " and disk_aspect_world_z is not NAN"
        " and disk_aspect_world_y is not NAN";


  status = OCIStmtPrepare(sthp, errhp, objCountSelect,
      (ub4) strlen((char *) objCountSelect),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &numCoaddObj, (sb4) (sizeof(numCoaddObj)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) 1, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  return(numCoaddObj);

}

fit_modelStruct *getFitModel(fit_modelStruct *fit_model, int numCoaddObj){
  static OCIDefine *dfnhp[56];
  static sword status;
  int i;

  static text *request = (text *) "SELECT COADD_OBJECTS_ID,"
        "ALPHAMODEL_J2000_G,DELTAMODEL_J2000_G,spheroid_reff_world_g,disk_scale_world_g,spheroid_aspect_world_g,disk_aspect_world_g,spheroid_theta_j2000_g,disk_theta_j2000_g,flux_spheroid_g,flux_model_g,"
	"ALPHAMODEL_J2000_R,DELTAMODEL_J2000_R,spheroid_reff_world_r,disk_scale_world_r,spheroid_aspect_world_r,disk_aspect_world_r,spheroid_theta_j2000_r,disk_theta_j2000_r,flux_spheroid_r,flux_model_r,"
        "ALPHAMODEL_J2000_I,DELTAMODEL_J2000_I,spheroid_reff_world_i,disk_scale_world_i,spheroid_aspect_world_i,disk_aspect_world_i,spheroid_theta_j2000_i,disk_theta_j2000_i,flux_spheroid_i,flux_model_i,"
	"ALPHAMODEL_J2000_Z,DELTAMODEL_J2000_Z,spheroid_reff_world_z,disk_scale_world_z,spheroid_aspect_world_z,disk_aspect_world_z,spheroid_theta_j2000_z,disk_theta_j2000_z,flux_spheroid_z,flux_model_z,"
	"ALPHAMODEL_J2000_Y,DELTAMODEL_J2000_Y,spheroid_reff_world_y,disk_scale_world_y,spheroid_aspect_world_y,disk_aspect_world_y,spheroid_theta_j2000_y,disk_theta_j2000_y,flux_spheroid_y,flux_model_y,"
	"zeropoint_g, zeropoint_r, zeropoint_i, zeropoint_z, zeropoint_y"
	" from COADD_OBJECTS,COADD WHERE imageid_i=coadd.id and coadd.run='20100624172007_DES2239-3939'"
        " and spheroid_aspect_world_g is not NAN"
        " and spheroid_aspect_world_r is not NAN"
        " and spheroid_aspect_world_i is not NAN"
        " and spheroid_aspect_world_z is not NAN"
        " and spheroid_aspect_world_y is not NAN"
        " and disk_aspect_world_g is not NAN"
        " and disk_aspect_world_r is not NAN"
        " and disk_aspect_world_i is not NAN"
        " and disk_aspect_world_z is not NAN"
        " and disk_aspect_world_y is not NAN";

  status = OCIStmtPrepare(sthp, errhp, request,
      (ub4) strlen((char *) request),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &fit_model[0].objId, (sb4) (sizeof(int)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 2,
        (dvoid *) &fit_model[0].ra_g, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 3,
        (dvoid *) &fit_model[0].dec_g, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[3], errhp, (ub4) 4,
        (dvoid *) &fit_model[0].spheroid_reff_world_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[4], errhp, (ub4) 5,
        (dvoid *) &fit_model[0].disk_scale_world_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[5], errhp, (ub4) 6,
        (dvoid *) &fit_model[0].spheroid_aspect_world_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[6], errhp, (ub4) 7,
        (dvoid *) &fit_model[0].disk_aspect_world_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[7], errhp, (ub4) 8,
        (dvoid *) &fit_model[0].spheroid_theta_j2000_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[8], errhp, (ub4) 9,
        (dvoid *) &fit_model[0].disk_theta_j2000_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[9], errhp, (ub4) 10,
        (dvoid *) &fit_model[0].flux_spheroid_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[10], errhp, (ub4) 11,
        (dvoid *) &fit_model[0].flux_model_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[11], errhp, (ub4) 12,
        (dvoid *) &fit_model[0].ra_r, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[12], errhp, (ub4) 13,
        (dvoid *) &fit_model[0].dec_r, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[13], errhp, (ub4) 14,
        (dvoid *) &fit_model[0].spheroid_reff_world_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[14], errhp, (ub4) 15,
        (dvoid *) &fit_model[0].disk_scale_world_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[15], errhp, (ub4) 16,
        (dvoid *) &fit_model[0].spheroid_aspect_world_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[16], errhp, (ub4) 17,
        (dvoid *) &fit_model[0].disk_aspect_world_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[17], errhp, (ub4) 18,
        (dvoid *) &fit_model[0].spheroid_theta_j2000_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[18], errhp, (ub4) 19,
        (dvoid *) &fit_model[0].disk_theta_j2000_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[19], errhp, (ub4) 20,
        (dvoid *) &fit_model[0].flux_spheroid_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[20], errhp, (ub4) 21,
        (dvoid *) &fit_model[0].flux_model_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[21], errhp, (ub4) 22,
        (dvoid *) &fit_model[0].ra_i, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[22], errhp, (ub4) 23,
        (dvoid *) &fit_model[0].dec_i, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[23], errhp, (ub4) 24,
        (dvoid *) &fit_model[0].spheroid_reff_world_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[24], errhp, (ub4) 25,
        (dvoid *) &fit_model[0].disk_scale_world_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[25], errhp, (ub4) 26,
        (dvoid *) &fit_model[0].spheroid_aspect_world_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[26], errhp, (ub4) 27,
        (dvoid *) &fit_model[0].disk_aspect_world_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[27], errhp, (ub4) 28,
        (dvoid *) &fit_model[0].spheroid_theta_j2000_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[28], errhp, (ub4) 29,
        (dvoid *) &fit_model[0].disk_theta_j2000_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[29], errhp, (ub4) 30,
        (dvoid *) &fit_model[0].flux_spheroid_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[30], errhp, (ub4) 31,
        (dvoid *) &fit_model[0].flux_model_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[31], errhp, (ub4) 32,
        (dvoid *) &fit_model[0].ra_z, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[32], errhp, (ub4) 33,
        (dvoid *) &fit_model[0].dec_z, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[33], errhp, (ub4) 34,
        (dvoid *) &fit_model[0].spheroid_reff_world_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[34], errhp, (ub4) 35,
        (dvoid *) &fit_model[0].disk_scale_world_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[35], errhp, (ub4) 36,
        (dvoid *) &fit_model[0].spheroid_aspect_world_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[36], errhp, (ub4) 37,
        (dvoid *) &fit_model[0].disk_aspect_world_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[37], errhp, (ub4) 38,
        (dvoid *) &fit_model[0].spheroid_theta_j2000_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[38], errhp, (ub4) 39,
        (dvoid *) &fit_model[0].disk_theta_j2000_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[39], errhp, (ub4) 40,
        (dvoid *) &fit_model[0].flux_spheroid_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[40], errhp, (ub4) 41,
        (dvoid *) &fit_model[0].flux_model_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[41], errhp, (ub4) 42,
        (dvoid *) &fit_model[0].ra_y, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[42], errhp, (ub4) 43,
        (dvoid *) &fit_model[0].dec_y, (sb4) (sizeof(double)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[43], errhp, (ub4) 44,
        (dvoid *) &fit_model[0].spheroid_reff_world_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[44], errhp, (ub4) 45,
        (dvoid *) &fit_model[0].disk_scale_world_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[45], errhp, (ub4) 46,
        (dvoid *) &fit_model[0].spheroid_aspect_world_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[46], errhp, (ub4) 47,
        (dvoid *) &fit_model[0].disk_aspect_world_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[47], errhp, (ub4) 48,
        (dvoid *) &fit_model[0].spheroid_theta_j2000_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[48], errhp, (ub4) 49,
        (dvoid *) &fit_model[0].disk_theta_j2000_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[49], errhp, (ub4) 50,
        (dvoid *) &fit_model[0].flux_spheroid_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[50], errhp, (ub4) 51,
        (dvoid *) &fit_model[0].flux_model_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[51], errhp, (ub4) 52,
        (dvoid *) &fit_model[0].zeropoint_g, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[52], errhp, (ub4) 53,
        (dvoid *) &fit_model[0].zeropoint_r, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[53], errhp, (ub4) 54,
        (dvoid *) &fit_model[0].zeropoint_i, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[54], errhp, (ub4) 55,
        (dvoid *) &fit_model[0].zeropoint_z, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[55], errhp, (ub4) 56,
        (dvoid *) &fit_model[0].zeropoint_y, (sb4) (sizeof(float)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);



  status = OCIDefineArrayOfStruct(dfnhp[0], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[1], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[3], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[4], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[5], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[6], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[7], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[8], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[9], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[10], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[11], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[12], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[13], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[14], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[15], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[16], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[17], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[18], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[19], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[20], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[21], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[22], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[23], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[24], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[25], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[26], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[27], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[28], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[29], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[30], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[31], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[32], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[33], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[34], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[35], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[36], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[37], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[38], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[39], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[40], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[41], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[42], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[43], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[44], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[45], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[46], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[47], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[48], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[49], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[50], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[51], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[52], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[53], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[54], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[55], errhp, sizeof(fit_modelStruct),0,0,0);
  checkDBerr(errhp, status);


  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) numCoaddObj, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);
  return (fit_model);

}



spread_modelStruct *getSpreadModel(spread_modelStruct *spread_model, int numCoaddObj){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[6];
  static sword status;

  int i;

  static text *request =
    (text *)"SELECT COADD_OBJECTS_ID, spread_model_g, spread_model_r, spread_model_i, spread_model_z, spread_model_y"
    " FROM COADD_OBJECTS,COADD WHERE imageid_i=coadd.id and coadd.run='20100624172007_DES2239-3939'"
        " and spheroid_aspect_world_g is not NAN"
        " and spheroid_aspect_world_r is not NAN"
        " and spheroid_aspect_world_i is not NAN"
        " and spheroid_aspect_world_z is not NAN"
        " and spheroid_aspect_world_y is not NAN"
        " and disk_aspect_world_g is not NAN"
        " and disk_aspect_world_r is not NAN"
        " and disk_aspect_world_i is not NAN"
        " and disk_aspect_world_z is not NAN"
        " and disk_aspect_world_y is not NAN";

  status = OCIStmtPrepare(sthp, errhp, request,
      (ub4) strlen((char *) request),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &spread_model[0].objId, (sb4) (sizeof(spread_model[0].objId)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 2,
        (dvoid *) &spread_model[0].g, (sb4) (sizeof(spread_model[0].g)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 3,
        (dvoid *) &spread_model[0].r, (sb4) (sizeof(spread_model[0].r)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[3], errhp, (ub4) 4,
        (dvoid *) &spread_model[0].i, (sb4) (sizeof(spread_model[0].i)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[4], errhp, (ub4) 5,
        (dvoid *) &spread_model[0].z, (sb4) (sizeof(spread_model[0].z)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[5], errhp, (ub4) 6,
        (dvoid *) &spread_model[0].y, (sb4) (sizeof(spread_model[0].y)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineArrayOfStruct(dfnhp[0], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[1], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[3], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[4], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[5], errhp, sizeof(spread_model[0]),0,0,0);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) numCoaddObj, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  return (spread_model);

}



mag_modelStruct *getMagModel(mag_modelStruct *mag_model, int numCoaddObj){

  static OCIBind *bndp = (OCIBind *) 0;
  static OCIDefine *dfnhp[13];
  static sword status;

  int i;

  static text *request =
    (text *)"SELECT COADD_OBJECTS_ID,ALPHAMODEL_J2000_I,DELTAMODEL_J2000_I,MAG_MODEL_G,MAG_MODEL_R,MAG_MODEL_I,MAG_MODEL_Z,MAG_MODEL_Y,MAGERR_MODEL_G,MAGERR_MODEL_R,MAGERR_MODEL_I,MAGERR_MODEL_Z,MAGERR_MODEL_Y"
    " FROM COADD_OBJECTS,COADD WHERE imageid_i=coadd.id and coadd.run='20100624172007_DES2239-3939'"
        " and spheroid_aspect_world_g is not NAN"
        " and spheroid_aspect_world_r is not NAN"
        " and spheroid_aspect_world_i is not NAN"
        " and spheroid_aspect_world_z is not NAN"
        " and spheroid_aspect_world_y is not NAN"
        " and disk_aspect_world_g is not NAN"
        " and disk_aspect_world_r is not NAN"
        " and disk_aspect_world_i is not NAN"
        " and disk_aspect_world_z is not NAN"
        " and disk_aspect_world_y is not NAN";


  status = OCIStmtPrepare(sthp, errhp, request,
      (ub4) strlen((char *) request),
      (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[0], errhp, (ub4) 1,
        (dvoid *) &mag_model[0].objId, (sb4) (sizeof(mag_model[0].objId)),
        SQLT_INT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[1], errhp, (ub4) 2,
        (dvoid *) &mag_model[0].ra, (sb4) (sizeof(mag_model[0].ra)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[2], errhp, (ub4) 3,
        (dvoid *) &mag_model[0].dec, (sb4) (sizeof(mag_model[0].dec)),
        SQLT_BDOUBLE, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[3], errhp, (ub4) 4,
        (dvoid *) &mag_model[0].g, (sb4) (sizeof(mag_model[0].g)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[4], errhp, (ub4) 5,
        (dvoid *) &mag_model[0].r, (sb4) (sizeof(mag_model[0].r)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[5], errhp, (ub4) 6,
        (dvoid *) &mag_model[0].i, (sb4) (sizeof(mag_model[0].i)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[6], errhp, (ub4) 7,
        (dvoid *) &mag_model[0].z, (sb4) (sizeof(mag_model[0].z)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[7], errhp, (ub4) 8,
        (dvoid *) &mag_model[0].y, (sb4) (sizeof(mag_model[0].y)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[8], errhp, (ub4) 9,
        (dvoid *) &mag_model[0].gerr, (sb4) (sizeof(mag_model[0].gerr)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[9], errhp, (ub4) 10,
        (dvoid *) &mag_model[0].rerr, (sb4) (sizeof(mag_model[0].rerr)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[10], errhp, (ub4) 11,
        (dvoid *) &mag_model[0].ierr, (sb4) (sizeof(mag_model[0].ierr)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[11], errhp, (ub4) 12,
        (dvoid *) &mag_model[0].zerr, (sb4) (sizeof(mag_model[0].zerr)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineByPos(sthp, &dfnhp[12], errhp, (ub4) 13,
        (dvoid *) &mag_model[0].yerr, (sb4) (sizeof(mag_model[0].yerr)),
        SQLT_FLT, (dvoid *) 0, (ub2 *)0,
        (ub2 *)0, (ub4) OCI_DEFAULT);
  checkDBerr(errhp, status);

  status = OCIDefineArrayOfStruct(dfnhp[0], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[1], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[2], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[3], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[4], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[5], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[6], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[7], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[8], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[9], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[10], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[11], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);
  status = OCIDefineArrayOfStruct(dfnhp[12], errhp, sizeof(mag_model[0]),0,0,0);
  checkDBerr(errhp, status);

  status = OCIStmtExecute(svchp, sthp, errhp, (ub4) numCoaddObj, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  checkDBerr(errhp, status);

  return (mag_model);

}



