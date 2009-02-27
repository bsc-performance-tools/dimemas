
FILE *VT_fopen (const char *filename, const char *mode);
int VT_fclose (FILE *stream);
size_t VT_fwrite (const void *ptr, size_t size, size_t nobj, FILE *stream);
size_t VT_fread (void *ptr, size_t size, size_t nobj, FILE *stream);
