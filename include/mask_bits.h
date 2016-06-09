/* 
$Id$

The *OFFICIAL* definition of DES masking bits. The masked bits are
divided into two categories:

BPMDEF -- Bits that are definited in the bad pixel mask (BPM)
file. These bits are partially flattened when creating the mask plane
of the reduced image.

BADPIX -- Bits that are set in the mask plane of the reduced
image. They are set from both BPM file and various image processing
routines.

Bits defined in this file can be accessed in C code with:

#include "mask_bits.h"
...
int bit = BADPIX_BPM;

Python code can interface with these bits through a shared library
that is compiled as part of despyfits and loaded with ctypes:

from despyfits import maskbits
...
bit = maskbits.BADPIX_BPM

The shared library access to this file necessitates the external
constant declaration of each bit. The bits are declared here but must
be defined in:

despyfits/src/libmaskbits.c
*/

/***********************************************/
/* NOTE: Bit definitions must be propagated to */
/* despyfits/src/libmaskbits.c                 */
/***********************************************/

/*********************************************/
/* Define BADPIX bit mappings (for MASK HDU) */
/*********************************************/

// set in BPM (hot/dead pixel/column)
#define BADPIX_BPM          1  
extern const int badpix_bpm;
// saturated pixel
#define BADPIX_SATURATE     2  
extern const int badpix_saturate;
// interpolated pixel
#define BADPIX_INTERP       4
extern const int badpix_interp;
// pixel on non-functional amplifier 
#define BADPIX_BADAMP       8  
extern const int badpix_badamp;
// cosmic-ray pixel
#define BADPIX_CRAY        16
extern const int badpix_cray;
// bright star pixel
#define BADPIX_STAR        32  
extern const int badpix_star;
// bleed trail pixel
#define BADPIX_TRAIL       64  
extern const int badpix_trail;
// edge bleed pixel
#define BADPIX_EDGEBLEED  128  
extern const int badpix_edgebleed;
// pixel potentially affected by xtalk from a super-saturated source
#define BADPIX_SSXTALK    256  
extern const int badpix_ssxtalk;
// pixel flag to exclude CCD glowing edges
#define BADPIX_EDGE       512  
extern const int badpix_edge;
// pixel associated with streak from a satellite, meteor, ufo...
#define BADPIX_STREAK    1024  
extern const int badpix_streak;
// nominally useful pixel but not perfect
#define BADPIX_SUSPECT   2048 
extern const int badpix_suspect;
// corrected by pixcorrect
#define BADPIX_FIXED     4096  
extern const int badpix_fixed;
// suspect due to edge proximity
#define BADPIX_NEAREDGE  8192  
extern const int badpix_nearedge;
// suspect due to known tape bump
#define BADPIX_TAPEBUMP 16384  
extern const int badpix_tapebump;

/* ADW 2015-06-01: Can these be removed? */

/* RAG thinks that this file definition is on the cusp of being deletable  */
// pixels less than this fraction of sky are filtered.
// helps remove failed reads
//#define BADPIX_THRESHOLD 0.10

// a bad pixel that was fixed - DEPRECATED
//#define BADPIX_FIX  (BADPIX_SUSPECT)
//extern const int badpix_fix;

// too little signal - DEPRICATED
//#define BADPIX_LOW (BADPIX_BADAMP) 
//extern const int badpix_low;


/****************************************/
/* Define bits for bad pixel mask (BPM) */
/****************************************/

//Pixels that are dull in the flats.
#define BPMDEF_FLAT_MIN    1  
extern const int bpmdef_flat_min;
//Pixels that are hot in the flats.
#define BPMDEF_FLAT_MAX    2  
extern const int bpmdef_flat_max;
//Pixels that are in the BPM for the flats.
#define BPMDEF_FLAT_MASK   4  
extern const int bpmdef_flat_mask;
// Pixels that are hot in the biases.
#define BPMDEF_BIAS_HOT    8  
extern const int bpmdef_bias_hot;
// Pixels that are warm in the biases.
#define BPMDEF_BIAS_WARM  16  
extern const int bpmdef_bias_warm;
// Pixels that are in the BPM for the biases
#define BPMDEF_BIAS_MASK  32  
extern const int bpmdef_bias_mask;
// Pixels that are downstream of a hot pixel in the bias.
#define BPMDEF_BIAS_COL   64  
extern const int bpmdef_bias_col;
// Pixels on the glowing edges of the CCD.
#define BPMDEF_EDGE      128  
extern const int bpmdef_edge;
// Correctable pixels (usually downstream of hot pixels).
#define BPMDEF_CORR      256  
extern const int bpmdef_corr;
// Imperfect calibration or excess noise, such as tape bumps.  
// Ignore for highest-precision work.
#define BPMDEF_SUSPECT   512  
extern const int bpmdef_suspect;
// Columns with charge redistribution in sky exposures.
#define BPMDEF_FUNKY_COL 1024 
extern const int bpmdef_funky_col;
// Outliers in stacked sky exposures.
#define BPMDEF_WACKY_PIX 2048
extern const int bpmdef_wacky_pix;
// Pixel on non-functional amplifier
#define BPMDEF_BADAMP    4096 
extern const int bpmdef_badamp;
// suspect due to edge proximity
#define BPMDEF_NEAREDGE  8192
extern const int bpmdef_nearedge;
// suspect due to known tape bump
#define BPMDEF_TAPEBUMP 16384  
extern const int bpmdef_tapebump;

/* ADW 2015-06-01: Can this be removed? */
// Generic bad pixel flag - DEPRECATED
//#define BPMDEF_GENERIC (BPMDEF_BADAMP);
//extern const int bpmdef_generic = BPMDEF_GENERIC; 
