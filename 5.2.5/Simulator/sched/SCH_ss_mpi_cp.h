#ifndef __SCH_ss_mpi_cp_h
#define __SCH_ss_mpi_cp_h
static char SCH_ss_mpi_cp_h_rcsid[]="$Id: SCH_ss_mpi_cp.h,v 1.0 2010/12/21 10:56:47 paraver Exp $";
static char *__a_SCH_ss_mpi_cp_h=SCH_ss_mpi_cp_h_rcsid;
/*
 * Scheduling for MPI/SMPSs execution
 * It is fifo with priorities, with preempting for specific threads
 *
 * Vladimir Subotic
 *
 * (c) BSC-UPC 2010-2011
 *
 */
#ifdef BASE_PRIORITY
#undef BASE_PRIORITY
#endif
#define BASE_PRIORITY (t_priority)0
struct t_SCH_ss_mpi_cp
{
   t_priority      priority;
   t_priority      forces_preemption;
};

/**
 * External routines defined in file SCH_ss_mpi_cp.c
 **/
extern void SS_MPI_CP_thread_to_ready(struct t_thread *thread);
extern t_nano SS_MPI_CP_get_execution_time(struct t_thread *thread);
extern struct t_thread *SS_MPI_CP_next_thread_to_run(struct t_node *node);
extern void SS_MPI_CP_init_scheduler_parameters(struct t_thread *thread);
extern void SS_MPI_CP_clear_parameters(struct t_thread *thread);
extern int SS_MPI_CP_info(int info);
extern void SS_MPI_CP_init(char *filename, struct t_machine *machine);
extern void SS_MPI_CP_copy_parameters(struct t_thread *th_o,
				      struct t_thread *th_d);
extern void SS_MPI_CP_free_parameters(struct t_thread *thread);
extern void SS_MPI_CP_modify_priority(struct t_thread *thread, t_priority priority);
extern void SS_MPI_CP_modify_preemption(struct t_thread *thread, t_priority preemption);
#endif
