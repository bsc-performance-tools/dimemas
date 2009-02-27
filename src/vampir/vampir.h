#ifndef __vampir_h
#define __vampir_h
/**
 * External routines defined in file vampir.c
 **/
extern int vampir_binary;  /* Write binary tracefile? */
extern int vampir_details; /* Log detailed simulator information? */

extern t_boolean vampir_start, vampir_stop;
/** Start/stop time for Vampir tracefile defined? **/
extern dimemas_timer vampir_start_timer, vampir_stop_timer;
/** Start/stop time for Vampir tracefile **/

extern char *vampir_filename; /* Vampir tracefile name */

void
Vampir_Send(
  int sender,
  int receiver,
  int communicator,
  int type,
  int length
);

void
Vampir_Receive(
  int receiver,
  int sender,
  int communicator,
  int type,
  int length
);

void
Activity_Enter( int from_user, struct t_thread *thread, int activityid);

void
Activity_Exit(int from_user,  struct t_thread *thread, int activityid);

void
Vampir_Start (char *filename);

void
Vampir_new_application_symbol (char *name, int num, char *symname);

void
Vampir_Stop(void);

#endif
