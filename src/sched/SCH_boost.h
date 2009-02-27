#ifndef __SCH_boost_h
#define __SCH_boost_h
static char SCH_boost_h_rcsid[]="$Id: SCH_boost.h,v 1.2 2005/05/25 10:56:47 paraver Exp $";
static char *__a_SCH_boost_h=SCH_boost_h_rcsid;

/*
 * Boost priority structures
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-95
 * 
 */
#ifdef BASE_PRIORITY
#undef BASE_PRIORITY
#endif

#define BASE_PRIORITY (t_priority)0


#define BOOST_RECV_FIRST -1
#define BOOST_RECV_LAST 1
#define BOOST_SENDER_LAST -1
#define BOOST_MAX_PRIO -5
#define BOOST_MIN_PRIO 5


struct t_SCH_boost
{
   t_priority      priority;
   t_micro         last_quantum;
};

/* Boost scheduler parameters */
struct t_boost
{
   int             best;
   int             worst;
   int             base_prio;
   int             recv_first;
   int             recv_last;
   int             sender_last;
};

#define PRV_PRIO 90

/**
 * External routines defined in file SCH_boost.c
 **/
extern void SCH_boost_thread_to_ready(struct t_thread *thread);
extern t_micro SCH_boost_get_execution_time(struct t_thread *thread);
extern struct t_thread *SCH_boost_next_thread_to_run(struct t_node *node);
extern void SCH_boost_init_scheduler_parameters(struct t_thread *thread);
extern void SCH_boost_clear_parameters(struct t_thread *thread);
extern int SCH_boost_info(int info, struct t_thread *thread_s, 
		     struct t_thread *thread_r);
extern void SCH_boost_init(char *filename, struct t_machine  *machine);
extern void SCH_boost_copy_parameters(struct t_thread *th_o, 
				      struct t_thread *th_d);
extern void SCH_boost_free_parameters (struct t_thread *thread);

#endif
