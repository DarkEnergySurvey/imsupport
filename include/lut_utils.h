/*  lut_utils.c  */
void read_linearity_lut(char lutfile[], int ccdnum, float **lutx, double **luta, double **lutb);
float lut_srch(const float value, const double lut[], const int flag_interp);
