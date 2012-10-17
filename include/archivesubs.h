/* archivesubs.c */
void filename_resolve(char filename[], char imageclass[], char runiddesc[], char nite[], char tilename[], char imagetype[], char imagename[], char band[], int *ccdnumber);
void filename_construct(char filename[], char imageclass[], char runiddesc[], char nite[], char tilename[], char imagetype[], char imagename[], char band[], int ccd_number);
void splitstring(char line[], char separator[], int num_fields, int len_field, char results[]);
int mkpath(const char *filename, int flag_verbose);
char *striparchiveroot(char filename[]);
char *strip_path(const char *path);
