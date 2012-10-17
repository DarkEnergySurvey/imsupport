/* Overscan configuration */
typedef struct {
  int choice;
  int sample;
  int function;
  int order;
  int trim;
  int debug;
} overscan_config;

#ifdef __cplusplus 
extern "C" {
/* imarithsubs.c */
#endif
  int os_sum_array(int *a, int num_elements);
  void overscan(desimage *data, desimage *output, int flag_verbose, int overscantype);
  void getxyz(double ra, double dec, double *x, double *y, double *z);
  void retrievescale(desimage *image, int *scaleregionn, float *scalesort, int flag_verbose, float *scalefactor, float *mode, float *sigma);
  void image_compare(desimage *image, desimage *tmplat, float *offset, float *rms, float *maxdev, int *npixels, int flag_verbose);
  void desimstat(desimage *image, int *scaleregion, float *meanval, float *sigma);
  void OverScan(desimage *input_image, desimage *output_image, overscan_config osconfig, int flag_verbose);
  void orient_section(int *section);
  int column_in_section(int col,int *sec);
#ifdef __cplusplus 
}
#endif
