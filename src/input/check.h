#ifndef __check_h
#define __check_h
/**
 * External routines defined in file check.c
 **/

#if defined(DIMEMAS_GUI) || defined(LIBLEXYACC)
extern int get_max_tasks_in_tracefile (char *filename);
extern void complete_list_of_modules(char *filename);
#endif

#ifndef DIMEMAS_GUI
extern void near_line(void);
extern void check_record_1(char *c, struct t_queue *q);
extern void check_record_2(char *c, struct t_queue *q);
extern void check_record_3(char *c, struct t_queue *q);
extern void check_record_40(char *c, struct t_queue *q);
extern void check_record_41(char *c, struct t_queue *q);
extern void check_record_42(char *c, struct t_queue *q);
extern void check_record_43(char *c, struct t_queue *q);
extern void check_record_50(char *c, struct t_queue *q);
extern void check_record_101(char *c, struct t_queue *q);
extern void check_record_102(char *c, struct t_queue *q);
extern void check_record_103(char *c, struct t_queue *q);
extern void check_record_104(char *c, struct t_queue *q);
extern void check_record_105(char *c, struct t_queue *q);
extern void check_record_106(char *c, struct t_queue *q);
extern void check_record_107(char *c, struct t_queue *q);
extern void check_record_100(char *c, struct t_queue *q);
extern void check_record_200(char *c, struct t_queue *q);
extern void check_record_201(char *c, struct t_queue *q);
extern void check_record_300(char *c, struct t_queue *q);
extern void check_record_301(char *c, struct t_queue *q);
extern void check_record_302(char *c, struct t_queue *q);
extern void check_record_303(char *c, struct t_queue *q);
extern void check_record_304(char *c, struct t_queue *q);
extern void check_record_305(char *c, struct t_queue *q);
extern void check_record_306(char *c, struct t_queue *q);
extern void check_record_307(char *c, struct t_queue *q);
extern void check_record_400(char *c, struct t_queue *q);
extern void check_record_401(char *c, struct t_queue *q);
extern void check_record_402(char *c, struct t_queue *q);
extern void check_record_403(char *c, struct t_queue *q);
extern void check_record_404(char *c, struct t_queue *q);
extern void check_record_1c(char *c, struct t_queue *q);
extern void check_record_2c(char *c, struct t_queue *q);
extern void check_record_3c(char *c, struct t_queue *q);
extern void check_record_4c(char *c, struct t_queue *q);
extern void check_record_5c(char *c, struct t_queue *q);
extern void check_record_6c(char *c, struct t_queue *q);
extern void check_fields_with_structure(struct t_queue *q, 
					struct t_entry *en);
extern void check_fields_for_configuration(struct t_queue *q, 
					   struct t_entry *en);
#endif
#endif

#define TYPE_INTEGER  0
#define TYPE_DOUBLE   1
#define TYPE_CHAR     2
