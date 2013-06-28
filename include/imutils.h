
#include "longnam.h"
#include "fitsio.h"
#include "eventsubs.h"
#include "archivesubs.h"
#include "imreadsubs.h"
#include "imarithsubs.h"

/* define event types */
#define STATUS 33
#define QA 34

/* constants for old .dedmfiles */
#define DB_READONLY 1
#define DB_READWRITE 2
#define DB_FNAL 3

#define Squ(x) ((x)*(x))
#define Cube(x) ((x)*(x)*(x))
#define Quad(x) (Squ(x)*Squ(x))
#define FIT 1
#define CONSTANT 0
#define FAST 1
#define NO 0
#define YES 1
#define SLOW 0
#define RAD2DEG 57.29578
#define MEM(st) printf("%s\n",st);system("ps aux|grep sim_ensemble");fflush(stdout);

/* define BADPIX bit mappings */
#define BADPIX_BPM 1          /* set in bpm (hot/dead pixel/column)        */
#define BADPIX_SATURATE 2     /* saturated pixel                           */
#define BADPIX_INTERP 4       /* interpolated pixel                        */
#define BADPIX_THRESHOLD 0.10 /* pixels less than this fraction of sky     */
                              /* are filtered -- helps remove failed reads */
#define BADPIX_LOW     8      /* too little signal- i.e. poor read         */
#define BADPIX_CRAY   16      /* cosmic ray pixel                          */
#define BADPIX_STAR   32      /* bright star pixel                         */
#define BADPIX_TRAIL  64      /* bleed trail pixel                         */
#define BADPIX_FIX    2048    /* a bad pixel that was fixed                */

/* define BPM bit mappings */
#define BPMDEF_FLAT_MIN 1
#define BPMDEF_FLAT_MAX 2
#define BPMDEF_FLAT_MASK 4
#define BPMDEF_BIAS_HOT 8
#define BPMDEF_BIAS_WARM 16
#define BPMDEF_BIAS_MASK 32
#define BPMDEF_BIAS_COL 64
#define BPMDEF_EDGE 128
#define BPMDEF_CORR 256

/* define Image FLAVOR Check modes */
#define CHECK_FITS 1
#define REQUIRE_FITS 2
#define FLAVOR_FITS 1
#define FLAVOR_FZ 2
#define FLAVOR_GZ 3

/* image constants */
#define GBAND 0
#define RBAND 1
#define IBAND 2
#define ZBAND 3
#define YBAND 4

#define DES_IMAGE 1     /* "IMAGE" */
#define DES_VARIANCE 2  /* "VARIANCE" now deprecated */
#define DES_MASK 3      /* "BPM" also "MASK" but that is deprecated */
#define DES_SIGMA 4     /* "SIGMA" */
#define DES_WEIGHT 5    /* "WEIGHT" */
#define DES_SKYONLY 6   /* noisemodel uses only sky noise */
#define DES_FULL 7      /* noisemodel uses both sky and object noise */


#define DES_IMAGE 1	/* "IMAGE" */
#define DES_VARIANCE 2  /* "VARIANCE" now deprecated */
#define DES_MASK 3	/* "BPM" also "MASK" but that is deprecated */
#define DES_SIGMA 4	/* "SIGMA" */
#define DES_WEIGHT 5	/* "WEIGHT" */
#define DES_SKYONLY 6	/* noisemodel uses only sky noise */
#define DES_FULL 7	/* noisemodel uses both sky and object noise */


/* TODO - need so say what these mean  */
#define AVERAGE 0
#define MEDIAN 1
#define AVSIGCLIP 2
#define CLIPPEDMEDIAN 3
#define CLIPPEDAVERAGE 4
#define VARIANCE_DELTAPIXEL 1  /* use 3X3 square centered on each pixel               */
			       /* for variance                                        */
#define VARIANCE_DIRECT 1      /* calculate variance directly                         */
#define VARIANCE_CCD 2	       /* assume CCD noise is dominating variance             */


