#include <oci.h>

#define MAXIMGS 1000

//
// Global Oracle DB handles
//
OCIEnv *envhp;
OCIError *errhp;
OCISvcCtx *svchp;
OCIStmt *sthp;

//
// Various data structures
//

double position[2]; /* position[0] = ra, position[1] = dec */

typedef struct imageIdStruct{
    int imageId;
    double magZP;
} imageIdStruct;

typedef struct parentStruct{
    int parentId;
    char *parentType[20];
} parentStruct;

typedef struct tileStruct{
    int tileId;
    double ra;
    double dec;
    float equinox;
    float pixelsize;
    int npixra;
    int npixdec;
} tileStruct;

typedef struct imageInfo{
    int imageId;
    int naxis1;
    int naxis2;
    char band[20];
    float ra;
    float dec;
    float exptime;
    int ccd;
    float gaina;
    float gainb;
    float rdnoisea;
    float rdnoiseb;
    float equinox;
    int wcsdim;
    char *ctype1[10];
    char *ctype2[10];
    char *cunit1[20];
    char *cunit2[20];
    double crval1;
    double crval2;
    double crpix1;
    double crpix2;
    double cd1_1;
    double cd1_2;
    double cd2_1;
    double cd2_2;
    double pv1_0;
    double pv1_1;
    double pv1_2;
    double pv1_3;
    double pv1_4;
    double pv1_5;
    double pv1_6;
    double pv1_7;
    double pv1_8;
    double pv1_9;
    double pv1_10;
    double pv2_0;
    double pv2_1;
    double pv2_2;
    double pv2_3;
    double pv2_4;
    double pv2_5;
    double pv2_6;
    double pv2_7;
    double pv2_8;
    double pv2_9;
    double pv2_10;
    float skybrite;
    float skysigma;
    char *imageType[20];
    float psfscale;
    float fwhm;
} imageInfo;

typedef struct mag_modelStruct{
    int objId;
    double ra;
    double dec;
    float g;
    float r;
    float i;
    float z;
    float y;
    float gerr;
    float rerr;
    float ierr;
    float zerr;
    float yerr;
} mag_modelStruct;

typedef struct spread_modelStruct{
	int objId;
	float g;
	float r;
	float i;
	float z;
	float y;
} spread_modelStruct;

typedef struct fit_modelStruct{
    int objId;
    double ra_g;
    double dec_g;
    float spheroid_reff_world_g;
    float disk_scale_world_g;
    float spheroid_aspect_world_g;
    float disk_aspect_world_g;
    float spheroid_theta_j2000_g;
    float disk_theta_j2000_g;
    float flux_spheroid_g;
    float flux_model_g;
    double ra_r;
    double dec_r;
    float spheroid_reff_world_r;
    float disk_scale_world_r;
    float spheroid_aspect_world_r;
    float disk_aspect_world_r;
    float spheroid_theta_j2000_r;
    float disk_theta_j2000_r;
    float flux_spheroid_r;
    float flux_model_r;
    double ra_i;
    double dec_i;
    float spheroid_reff_world_i;
    float disk_scale_world_i;
    float spheroid_aspect_world_i;
    float disk_aspect_world_i;
    float spheroid_theta_j2000_i;
    float disk_theta_j2000_i;
    float flux_spheroid_i;
    float flux_model_i;
    double ra_z;
    double dec_z;
    float spheroid_reff_world_z;
    float disk_scale_world_z;
    float spheroid_aspect_world_z;
    float disk_aspect_world_z;
    float spheroid_theta_j2000_z;
    float disk_theta_j2000_z;
    float flux_spheroid_z;
    float flux_model_z;
    double ra_y;
    double dec_y;
    float spheroid_reff_world_y;
    float disk_scale_world_y;
    float spheroid_aspect_world_y;
    float disk_aspect_world_y;
    float spheroid_theta_j2000_y;
    float disk_theta_j2000_y;
    float flux_spheroid_y;
    float flux_model_y;
    float zeropoint_g;
    float zeropoint_r;
    float zeropoint_i;
    float zeropoint_z;
    float zeropoint_y;
}fit_modelStruct;


//
// Prototypes
//
int dbconnect();
int dbdisconnect();
void checkDBerr(OCIError*, sword);
struct imageIdStruct *getCoaddIdsForTile(char*,char*);

int getNumCoaddObj();
mag_modelStruct* getMagModel(mag_modelStruct*, int);
fit_modelStruct* getFitModel(fit_modelStruct*, int);
spread_modelStruct* getClassStar(spread_modelStruct*, int);


