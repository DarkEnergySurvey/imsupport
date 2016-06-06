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

The shared library access to this file is what necessitates the double
definition of each bit.
*/

/*********************************************/
/* Define BADPIX bit mappings (for MASK HDU) */
/*********************************************/

// set in BPM (hot/dead pixel/column)
#define BADPIX_BPM          1  
const int badpix_bpm = BADPIX_BPM;
// saturated pixel
#define BADPIX_SATURATE     2  
const int badpix_saturate = BADPIX_SATURATE;
// interpolated pixel
#define BADPIX_INTERP       4
const int badpix_interp = BADPIX_INTERP;
// pixel on non-functional amplifier 
#define BADPIX_BADAMP       8  
const int badpix_badamp = BADPIX_BADAMP;
// cosmic-ray pixel
#define BADPIX_CRAY        16
const int badpix_cray = BADPIX_CRAY;
// bright star pixel
#define BADPIX_STAR        32  
const int badpix_star = BADPIX_STAR;
// bleed trail pixel
#define BADPIX_TRAIL       64  
const int badpix_trail = BADPIX_TRAIL;
// edge bleed pixel
#define BADPIX_EDGEBLEED  128  
const int badpix_edgebleed = BADPIX_EDGEBLEED;
// pixel potentially affected by xtalk from a super-saturated source
#define BADPIX_SSXTALK    256  
const int badpix_ssxtalk = BADPIX_SSXTALK;
// pixel flag to exclude CCD glowing edges
#define BADPIX_EDGE       512  
const int badpix_edge = BADPIX_EDGE;
// pixel associated with streak from a satellite, meteor, ufo...
#define BADPIX_STREAK    1024  
const int badpix_streak = BADPIX_STREAK;
// nominally useful pixel but not perfect
#define BADPIX_SUSPECT   2048 
const int badpix_suspect = BADPIX_SUSPECT;
// corrected by pixcorrect
#define BADPIX_FIXED     4096  
const int badpix_fixed = BADPIX_FIXED;
// suspect due to edge proximity
#define BADPIX_NEAREDGE  8192  
const int badpix_nearedge = BADPIX_NEAREDGE;
// suspect due to known tape bump
#define BADPIX_TAPEBUMP 16384  
const int badpix_tapebump = BADPIX_TAPEBUMP;

/* ADW 2015-06-01: Can these be removed? */

/* RAG thinks that this file definition is on the cusp of being deletable  */
// pixels less than this fraction of sky are filtered.
// helps remove failed reads
#define BADPIX_THRESHOLD 0.10
//const float badpix_threshold = BADPIX_THRESHOLD;

// a bad pixel that was fixed - DEPRECATED
// ADW: Still used in mask_utils.c
//#define BADPIX_FIX  (BADPIX_SUSPECT);
//const int badpix_fix = BADPIX_FIX;

// too little signal - DEPRICATED
//#define BADPIX_LOW (BADPIX_BADAMP) 
//const int badpix_low = BADPIX_LOW;


/****************************************/
/* Define bits for bad pixel mask (BPM) */
/****************************************/

//Pixels that are dull in the flats.
#define BPMDEF_FLAT_MIN    1  
const int bpmdef_flat_min = BPMDEF_FLAT_MIN;
//Pixels that are hot in the flats.
#define BPMDEF_FLAT_MAX    2  
const int bpmdef_flat_max = BPMDEF_FLAT_MAX;
//Pixels that are in the BPM for the flats.
#define BPMDEF_FLAT_MASK   4  
const int bpmdef_flat_mask = BPMDEF_FLAT_MASK;
// Pixels that are hot in the biases.
#define BPMDEF_BIAS_HOT    8  
const int bpmdef_bias_hot = BPMDEF_BIAS_HOT;
// Pixels that are warm in the biases.
#define BPMDEF_BIAS_WARM  16  
const int bpmdef_bias_warm = BPMDEF_BIAS_WARM;
// Pixels that are in the BPM for the biases
#define BPMDEF_BIAS_MASK  32  
const int bpmdef_bias_mask = BPMDEF_BIAS_MASK;
// Pixels that are downstream of a hot pixel in the bias.
#define BPMDEF_BIAS_COL   64  
const int bpmdef_bias_col = BPMDEF_BIAS_COL;
// Pixels on the glowing edges of the CCD.
#define BPMDEF_EDGE      128  
const int bpmdef_edge = BPMDEF_EDGE;
// Correctable pixels (usually downstream of hot pixels).
#define BPMDEF_CORR      256  
const int bpmdef_corr = BPMDEF_CORR;
// Imperfect calibration or excess noise, such as tape bumps.  
// Ignore for highest-precision work.
#define BPMDEF_SUSPECT   512  
const int bpmdef_suspect = BPMDEF_SUSPECT;
// Columns with charge redistribution in sky exposures.
#define BPMDEF_FUNKY_COL 1024 
const int bpmdef_funky_col = BPMDEF_FUNKY_COL;
// Outliers in stacked sky exposures.
#define BPMDEF_WACKY_PIX 2048
const int bpmdef_wacky_pix = BPMDEF_WACKY_PIX;
// Pixel on non-functional amplifier
#define BPMDEF_BADAMP    4096 
const int bpmdef_badamp = BPMDEF_BADAMP;
// suspect due to edge proximity
#define BPMDEF_NEAREDGE  8192
const int bpmdef_nearedge = BPMDEF_NEAREDGE;
// suspect due to known tape bump
#define BPMDEF_TAPEBUMP 16384  
const int bpmdef_tapebump = BPMDEF_TAPEBUMP;

/* ADW 2015-06-01: Can this be removed? */
// Generic bad pixel flag - DEPRECATED
//#define BPMDEF_GENERIC (BPMDEF_BADAMP);
//const int bpmdef_generic = BPMDEF_GENERIC; 
