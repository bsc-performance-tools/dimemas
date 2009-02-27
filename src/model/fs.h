#ifndef __fs_h
#define __fs_h
/**
 * External routines defined in file fs.c
 **/
extern void FS_general(int value, struct t_thread *thread);
extern void FS_init(char *filename, t_boolean hot);
extern void FS_end(void);
extern void show_fs_version(void);
extern int FS_paraver(void);
extern void new_io_operation (int , char *);
extern void new_io_operation (int , char *);
extern void FS_parameters (double, double, double, int , double);
#endif
