/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <define.h>
#include <types.h>

#include "aleatorias.h"
#include "communic.h"
#include "cpu.h"
#include "node.h"
#include "events.h"
#include "extern.h"
#include "fs.h"
#include "list.h"
#ifdef USE_EQUEUE
#include "listE.h"
#endif
#include "mallocame.h"
#include "paraver.h"
#include "schedule.h"
#include "subr.h"
#include "task.h"

static char *Operation_Name[] = {
  "MPI_File_open",
  "MPI_File_preallocate",
  "MPI_File_read",
  "MPI_File_read_at",
  "MPI_File_read_shared",
  "MPI_File_write",
  "MPI_File_write_at",
  "MPI_File_write_shared",
  "MPI_File_read_all",
  "MPI_File_read_at_all",
  "MPI_File_read_ordered",
  "MPI_File_write_all",
  "MPI_File_write_at_all",
  "MPI_File_write_ordered",
  "MPI_File_iread",
  "MPI_File_iread_at",
  "MPI_File_iread_shared",
  "MPI_File_iwrite",
  "MPI_File_iwrite_at",
  "MPI_File_iwrite_shared",
  "MPI_Wait",
  "MPI_File_read_all_begin",
  "MPI_File_read_at_all_begin",
  "MPI_File_read_ordered_begin",
  "MPI_File_write_all_begin",
  "MPI_File_write_at_all_begin",
  "MPI_File_write_ordered_begin",
  "MPI_File_read_all_end",
  "MPI_File_read_at_all_end",
  "MPI_File_read_ordered_end",
  "MPI_File_write_all_end",
  "MPI_File_write_at_all_end",
  "MPI_File_write_ordered_end"
};

#define MAX_IO_OPERATIONS 32

static struct t_file_system_parameters file_system_parameters =
  {0.0, 0.0, 8.0, 0, 1.0};

static struct t_fh_commid *get_fh_commid (struct t_thread *thread, int fh)
{
  struct t_fh_commid *fh_commid;
  struct t_Ptask *Ptask = thread->task->Ptask;

  fh_commid = (struct t_fh_commid *) query_prio_queue (&Ptask->MPI_IO_fh_to_commid,
              (t_priority) fh);
  if (fh_commid == (struct t_fh_commid *) 0)
  {
    panic ("Unable to locate communicator for given fh %d\n", fh);
  }
  return (fh_commid);
}

static struct t_action *create_barrier (struct t_thread *thread)
{
  struct t_action *action;
  struct t_action *cur = thread->action;

  action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );

  action->action = GLOBAL_OP;
  action->desc.global_op.glop_id = get_global_op_id_by_name ("MPI_Barrier");
  if (action->desc.global_op.glop_id == -1)
    panic ("Unable to locate MPI_Barrier in Global Operation definition\n");
  /* The communicator identificator is specified in the MPI IO operation */
  action->desc.global_op.comm_id = cur->desc.mpi_io.commid;
  /* There is no information who is acting as root processor, we decide it is rank 0*/
  action->desc.global_op.root_rank = 0;
  action->desc.global_op.root_thid = 1;
  /* We have no information about the size, but we fix it to  4 bytes */
  action->desc.global_op.bytes_send = 4;
  action->desc.global_op.bytes_recvd = 4;
  action->next = AC_NIL;
  return (action);
}

static struct t_action *create_scatter (struct t_thread *thread, t_boolean twin)
{
  struct t_action *action;
  struct t_action *cur;
  struct t_fh_commid *fh_commid;

  if (twin)
    cur = thread->twin_thread->action;
  else
    cur = thread->action;

  action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );

  action->action = GLOBAL_OP;
  action->desc.global_op.glop_id = get_global_op_id_by_name ("MPI_Scatter");
  if (action->desc.global_op.glop_id == -1)
    panic ("Unable to locate MPI_Scatter in Global Operation definition\n");
  /* The communicator identificator is specified in the MPI IO operation */
  fh_commid = get_fh_commid (thread, cur->desc.mpi_io.fh);
  action->desc.global_op.comm_id = fh_commid->communicator->communicator_id;
  /* There is no information who is acting as root processor, we decide it is rank 0*/
  action->desc.global_op.root_rank = 0;
  action->desc.global_op.root_thid = 1;
  action->desc.global_op.bytes_send = cur->desc.mpi_io.size;
  action->desc.global_op.bytes_recvd = cur->desc.mpi_io.size;
  action->next = AC_NIL;
  return (action);
}

static struct t_action *create_return_to_fs (int id)
{
  struct t_action *ac;

  ac = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
  ac->next = AC_NIL;
  ac->action = FS;
  ac->desc.fs_op.which_fsop            = FS_RETURN;
  ac->desc.fs_op.fs_o.fs_user_event.id = id;

  return (ac);
}

static void create_fh_to_communicator (struct t_thread *thread)
{
  struct t_communicator *communicator;
  struct t_action *action;
  struct t_Ptask *Ptask;
  struct t_fh_commid *fh_commid;

  Ptask = thread->task->Ptask;
  action = thread->action;

  communicator = locate_communicator (&Ptask->Communicator,
                                      action->desc.mpi_io.commid);
  if (communicator == (struct t_communicator *) 0)
    panic ("MPI IO communication trough an invalid communicator %d to P%d T%d t%d\n",
           action->desc.mpi_io.commid, IDENTIFIERS (thread) );

  fh_commid = (struct t_fh_commid *) query_prio_queue (&Ptask->MPI_IO_fh_to_commid,
              (t_priority) action->desc.mpi_io.fh);
  if (fh_commid == (struct t_fh_commid *) 0)
  {
    /* Create a new one */
    fh_commid = (struct t_fh_commid *) MALLOC_get_memory (sizeof (struct t_fh_commid) );

    fh_commid->fh = action->desc.mpi_io.fh;
    fh_commid->communicator = communicator;
    create_queue (&fh_commid->threads);
    create_queue (&fh_commid->copy);
    insert_queue (&Ptask->MPI_IO_fh_to_commid, (char *) fh_commid,
                  (t_priority) action->desc.mpi_io.fh);
  }
  else
  {
    fh_commid->communicator = communicator;
    if (debug & D_FS)
      printf ("Warning: reutilization of fh %d to P%d T%d t%d\n",
              action->desc.mpi_io.fh, IDENTIFIERS (thread) );
  }
}

static t_nano compute_IO_collective_time (struct t_queue *threads)
{
  register t_nano ti = 0;
  int size, num_blocks;
  struct t_thread *th;

  /* Random number to get hit or miss */
  if (randomine() > file_system_parameters.hit_ratio)
    ti = 0;
  else
  {
    ti = file_system_parameters.disk_latency * 1e9;
    if (file_system_parameters.disk_bandwidth != 0)
    {
      size = 0;
      for (th = (struct t_thread *) head_queue (threads);
           th != TH_NIL;
           th = (struct t_thread *) next_queue (threads) )
      {
        size += th->action->desc.mpi_io.size;
      }
      num_blocks = size / (file_system_parameters.block_size * 1024);
      if (num_blocks*file_system_parameters.block_size * 1024 != size)
        num_blocks++;
      ti = ti + (file_system_parameters.block_size / (1024 * file_system_parameters.disk_bandwidth) ) *
           num_blocks;
    }
  }
  return (ti);
}

static t_nano compute_IO_time (struct t_thread *thread)
{
  register t_nano ti = 0;
  register struct t_action *action = thread->action;
  int num_blocks;

  /* Random number to get hit or miss */
  if (randomine() > file_system_parameters.hit_ratio)
    ti = 0;
  else
  {
    ti = file_system_parameters.disk_latency * 1e9;
    if (file_system_parameters.disk_bandwidth != 0)
    {
      num_blocks = action->desc.mpi_io.size / (file_system_parameters.block_size * 1024);
      if (num_blocks*file_system_parameters.block_size * 1024 != action->desc.mpi_io.size)
        num_blocks++;
      ti = ti + (file_system_parameters.block_size / (1024 * file_system_parameters.disk_bandwidth) ) *
           num_blocks;
    }
  }
  return (ti);
}

static struct t_request_thread*
thread_already_waiting (struct t_queue *queue, struct t_thread *thread, int request)
{
  struct t_request_thread *request_thread;

  for (request_thread = (struct t_request_thread *) head_queue (queue);
       request_thread != (struct t_request_thread *) 0;
       request_thread = (struct t_request_thread *) next_queue (queue) )
  {
    if ( (request_thread->thread == thread) && (request == request_thread->request) )
      break;
  }
  return (request_thread);
}

void Next_Action_to_thread (struct t_thread *thread)
{
  struct t_action *action;

  action = thread->action;
  thread->action = action->next;
  MALLOC_free_memory ( (char*) action);
  if (more_actions (thread) )
  {
    thread->loose_cpu = FALSE;
    SCHEDULER_thread_to_ready (thread);
  }
}

void FS_general (int value, struct t_thread *thread)
{
  struct t_mpi_io  *mpi_io;
  struct t_action  *action;
  struct t_thread  *copy, *copy2;
  dimemas_timer tmp_timer, new_time;
  struct t_node   *node;
  struct t_cpu *cpu;
  struct t_fh_commid *fh_commid;
  float ti;
  struct t_request_thread *request_thread;
  int    *token;

  action = thread->action;
  node = get_node_of_thread (thread);
  switch (value)
  {
  case FS_OPERATION:
    /* This is a new operation call */
    mpi_io = &action->desc.mpi_io;
    switch (mpi_io->which_io)
    {
    case MPI_IO_Metadata:
      /* Given the communicator id, and the fh, create a pair
        *  usefull for block operations, because those specifies the
        * fh instead of communicator id
        */
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO Metadata P%d T%d th%d\n", IDENTIFIERS (thread) );
      }
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                     current_time,
                     PARAVER_IO,
                     PARAVER_IO_METADATA_BEGIN);

      create_fh_to_communicator (thread);
      copy = duplicate_thread_fs (thread);
      action = create_barrier (thread);
      copy->action = action;
      copy->action->next = create_return_to_fs (IO_Metadata);
      GLOBAL_operation (copy,
                        action->desc.global_op.glop_id,
                        action->desc.global_op.comm_id,
                        action->desc.global_op.root_rank,
                        action->desc.global_op.root_thid,
                        action->desc.global_op.bytes_send,
                        action->desc.global_op.bytes_recvd);

      break;
    case MPI_IO_Block_NonCollective:
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO Block NonCollective P%d T%d th%d\n", IDENTIFIERS (thread) );
      }
      switch (mpi_io->Oop)
      {
      case 2:
      case 3:
      case 4:
        /* This is a read operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number,
                       IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO,
                       PARAVER_IO_BLOCK_NONCOLLECTIVE_READ_BEGIN);
        copy = duplicate_thread_fs (thread);
        if (file_system_parameters.concurrent_requests != 0)
        {
          token = (int *) outFIFO_queue (&node->IO_disks);
          if (token == (int *) 0)
          {
            inFIFO_queue (&node->IO_disks_threads, (char *) copy);
            copy->last_paraver = current_time;
            copy->IO_blocking_point = IO_Block_NonCollective;
            break;
          }
          MALLOC_free_memory ( (char*) token);
        }
A:
        FLOAT_TO_TIMER (compute_IO_time (thread), tmp_timer);
        ADD_TIMER (current_time, tmp_timer, new_time);
        copy->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, copy, IO_Block_NonCollective);
        copy->last_paraver = current_time;
        break;
      case 5:
      case 6:
      case 7:
        /* This is a write operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_NONCOLLECTIVE_WRITE_BEGIN);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_NONCOLLECTIVE_WRITE_END);
        Next_Action_to_thread (thread);
        break;
      default:
        panic ("Non valid operation %d for Blocking Non Collective P%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (thread) );
      }
      break;
    case MPI_IO_Block_Collective:
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO Block Collective P%d T%d th%d Oop %d fh %d\n",
                IDENTIFIERS (thread), mpi_io->Oop, mpi_io->fh);
      }
      switch (mpi_io->Oop)
      {
      case 8:
      case 9:
      case 10:
        /* This is a read operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_COLLECTIVE_READ_BEGIN);
        break;
      case 11:
      case 12:
      case 13:
        /* This is a write operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_COLLECTIVE_WRITE_BEGIN);
        break;
      default:
        panic ("Non valid operation %d for Blocking Collective P%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (thread) );
      }
      copy = duplicate_thread_fs (thread);
      action = create_barrier (thread);
      fh_commid = get_fh_commid (thread, mpi_io->fh);
      fh_commid->stage  = IN_BARRIER;
      if (empty_queue (&fh_commid->threads) )
        fh_commid->counter = 1;
      else
        fh_commid->counter++;
      inFIFO_queue (&fh_commid->threads, (char *) thread);
      action->desc.global_op.comm_id = fh_commid->communicator->communicator_id;
      copy->action = action;
      copy->action->next = create_return_to_fs (IO_Block_Collective);
      GLOBAL_operation (copy,
                        action->desc.global_op.glop_id,
                        action->desc.global_op.comm_id,
                        action->desc.global_op.root_rank,
                        action->desc.global_op.root_thid,
                        action->desc.global_op.bytes_send,
                        action->desc.global_op.bytes_recvd);

      break;
    case MPI_IO_NonBlock_NonCollective_Begin:
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO NonBlock NonCollective P%d T%d th%d\n", IDENTIFIERS (thread) );
      }
      switch (mpi_io->Oop)
      {
      case 14:
      case 15:
      case 16:
        copy = duplicate_thread_fs (thread);
        copy->last_paraver = current_time;
        copy->action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
        memcpy (copy->action, thread->action, sizeof (struct t_action) );
        copy->action->next = AC_NIL;
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_NONCOLLECTIVE_READ_BEGIN);
        break;
      case 17:
      case 18:
      case 19:
        request_thread = (struct t_request_thread *) MALLOC_get_memory (sizeof (struct t_request_thread) );
        request_thread->thread = thread;
        request_thread->request = mpi_io->request;
        inFIFO_queue (&thread->task->Ptask->MPI_IO_request_thread,
                      (char *) request_thread);
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_NONCOLLECTIVE_WRITE_BEGIN);
        break;
      default:
        panic ("Non valid operation %d for NonBlocking NonCollective beginP%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (thread) );
      }

      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);

      if ( (mpi_io->Oop >= 14) && (mpi_io->Oop <= 16) )
      {
        if (file_system_parameters.concurrent_requests != 0)
        {
          token = (int *) outFIFO_queue (&node->IO_disks);
          if (token == (int *) 0)
          {
            inFIFO_queue (&node->IO_disks_threads, (char *) copy);
            copy->last_paraver = current_time;
            copy->IO_blocking_point = IO_NonBlock_NonCollective;
            break;
          }
          MALLOC_free_memory ( (char*) token);
        }
B:
        FLOAT_TO_TIMER (compute_IO_time (copy), tmp_timer);
        ADD_TIMER (current_time, tmp_timer, new_time);
        copy->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, copy, IO_NonBlock_NonCollective);
        cpu = get_cpu_of_thread (copy);

        PARAVER_IO_Op (cpu->unique_number,
                       copy->task->Ptask->Ptaskid,
                       copy->task->taskid,
                       copy->task->threads_count + 1, // why +1
                       current_time, new_time);

        FLOAT_TO_TIMER (1.0, tmp_timer);
        ADD_TIMER (tmp_timer, new_time, tmp_timer);

        PARAVER_Idle (0,
                      copy->task->Ptask->Ptaskid,
                      copy->task->taskid,
                      copy->task->threads_count + 1, // why +1
                      new_time, tmp_timer);
        copy->last_paraver = tmp_timer;
        copy->task->io_thread = TRUE;
      }
      break;
    case MPI_IO_NonBlock_NonCollective_End:
      if ( (request_thread = thread_already_waiting (&thread->task->Ptask->MPI_IO_request_thread,
                             thread, mpi_io->request) ) != (struct t_request_thread *) 0)
      {
        extract_from_queue (&thread->task->Ptask->MPI_IO_request_thread, (char *) request_thread);
        MALLOC_free_memory ( (char*) request_thread);
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_NONCOLLECTIVE_END);
      }
      else
      {
        request_thread = (struct t_request_thread *) MALLOC_get_memory (sizeof (struct t_request_thread) );
        request_thread->thread = thread;
        request_thread->request = mpi_io->request;
        inFIFO_queue (&thread->task->Ptask->MPI_IO_request_thread,
                      (char *) request_thread);
        thread->last_paraver = current_time;
        return;
      }
      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);
      break;
    case MPI_IO_NonBlock_Collective_Begin:
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO NonBlock Collective P%d T%d th%d Oop %d fh %d\n",
                IDENTIFIERS (thread), mpi_io->Oop, mpi_io->fh);
      }
      switch (mpi_io->Oop)
      {
      case 21:
      case 22:
      case 23:
        /* This is a read operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_COLLECTIVE_READ_BEGIN);
        break;
      case 24:
      case 25:
      case 26:
        /* This is a write operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_COLLECTIVE_WRITE_BEGIN);
      default:
        panic ("Non valid operation %d for NonBlocking Collective P%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (thread) );
      }
      copy = duplicate_thread_fs (thread);
      copy->task->io_thread = TRUE;
      copy->threadid = copy->task->threads_count + 1;
      action = create_barrier (thread);
      fh_commid = get_fh_commid (thread, mpi_io->fh);
      action->desc.global_op.comm_id = fh_commid->communicator->communicator_id;
      fh_commid->stage  = IN_BARRIER;
      copy->action = action;
      copy->action->next = create_return_to_fs (IO_NonBlock_Collective);
      action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
      memcpy (action, thread->action, sizeof (struct t_action) );
      action->next = AC_NIL;
      copy->action->next->next = action;

      copy2 = duplicate_thread_fs (thread);
      copy2->action = action;
      if (empty_queue (&fh_commid->threads) )
        fh_commid->counter = 1;
      else
        fh_commid->counter++;
      inFIFO_queue (&fh_commid->threads, (char *) copy2);

      action = copy->action;
      GLOBAL_operation (copy,
                        action->desc.global_op.glop_id,
                        action->desc.global_op.comm_id,
                        action->desc.global_op.root_rank,
                        action->desc.global_op.root_thid,
                        action->desc.global_op.bytes_send,
                        action->desc.global_op.bytes_recvd);

      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);
      break;
    case MPI_IO_NonBlock_Collective_End:
      if ( (request_thread = thread_already_waiting (&thread->task->Ptask->MPI_IO_request_thread,
                             thread, mpi_io->request) ) != (struct t_request_thread *) 0)
      {
        extract_from_queue (&thread->task->Ptask->MPI_IO_request_thread, (char *) request_thread);
        MALLOC_free_memory ( (char*) request_thread);
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_COLLECTIVE_END);
      }
      else
      {
        request_thread = (struct t_request_thread *) MALLOC_get_memory (sizeof (struct t_request_thread) );
        request_thread->thread = thread;
        request_thread->request = mpi_io->request;
        inFIFO_queue (&thread->task->Ptask->MPI_IO_request_thread,
                      (char *) request_thread);
        thread->last_paraver = current_time;
        return;
      }
      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);
      break;
    default:
      panic ("Invalid operation %d in FS module for P%d T%d th%d\n",
             mpi_io->which_io, IDENTIFIERS (thread) );
      break;
    }
    break;

  case IO_Block_Collective:
    copy = thread;
    thread = copy->twin_thread;
    action = thread->action;
    mpi_io = &action->desc.mpi_io;
    fh_commid = get_fh_commid (thread, mpi_io->fh);
    switch (fh_commid->stage)
    {
    case IN_BARRIER:
      inFIFO_queue (&fh_commid->copy, (char *) copy);
      if (count_queue (&fh_commid->copy) == count_queue (&fh_commid->threads) )
      {
        for (thread = (struct t_thread *) head_queue (&fh_commid->copy);
             thread != TH_NIL;
             thread = (struct t_thread *) next_queue (&fh_commid->copy) )
        {
          node = get_node_of_thread (thread);
          if ( (mpi_io->Oop >= 8) && (mpi_io->Oop <= 10) )
          {
            if (file_system_parameters.concurrent_requests != 0)
            {
              token = (int *) outFIFO_queue (&node->IO_disks);
              if (token == (int *) 0)
              {
                inFIFO_queue (&node->IO_disks_threads, (char *) thread);
                thread->last_paraver = current_time;
                thread->IO_blocking_point = IO_Block_Collective;
                continue;
              }
              MALLOC_free_memory ( (char*) token);
            }

            ti = compute_IO_collective_time (&fh_commid->threads);
          }
          else
            ti = 0.0;
          FLOAT_TO_TIMER (ti, tmp_timer);
          ADD_TIMER (current_time, tmp_timer, new_time);
          thread->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, thread, IO_Block_Collective);
          node = get_node_of_thread (thread);
          if ( (mpi_io->Oop >= 8) && (mpi_io->Oop <= 10) )
          {
            cpu = get_cpu_of_thread (thread);
            PARAVER_IO_Op (cpu->unique_number,
                           IDENTIFIERS (thread),
                           current_time, new_time);
            thread->last_paraver = new_time;
          }
        }
        fh_commid->stage = IN_IO;
      }
      return;
    case IN_IO:
      extract_from_queue (&fh_commid->copy, (char *) copy);
      action = create_scatter (copy, TRUE);
      copy->action = action;
      copy->action->next = create_return_to_fs (IO_Block_Collective);
      GLOBAL_operation (copy,
                        action->desc.global_op.glop_id,
                        action->desc.global_op.comm_id,
                        action->desc.global_op.root_rank,
                        action->desc.global_op.root_thid,
                        action->desc.global_op.bytes_send,
                        action->desc.global_op.bytes_recvd);
      fh_commid->counter--;
      if (fh_commid->counter == 0)
        fh_commid->stage = IN_SCATTER;
      if ( (mpi_io->Oop >= 8) && (mpi_io->Oop <= 10) )
        goto free_disk_token;
      else
        break;
    case IN_SCATTER:
      extract_from_queue (&fh_commid->threads, (char *) thread);
      thread->last_paraver = current_time;
      switch (mpi_io->Oop)
      {
      case 8:
      case 9:
      case 10:
        /* This is a read operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_COLLECTIVE_READ_END);
        break;
      case 11:
      case 12:
      case 13:
        /* This is a write operation */
        cpu = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_BLOCK_COLLECTIVE_WRITE_END);
        break;
      default:
        panic ("Non valid operation %d for Blocking Collective P%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (thread) );
      }
      delete_duplicate_thread_fs (copy);
      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);
      if (debug & D_FS)
      {
        PRINT_TIMER (current_time);
        printf (": IO Block Collective P%d T%d th%d End\n", IDENTIFIERS (thread) );
      }
      break;
    default:
      panic ("Unknow stage %d for blocking and collective IO\n",
             fh_commid->stage);
    }
    break;

  case IO_Metadata:
    copy = thread;
    thread = copy->twin_thread;
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                   current_time,
                   PARAVER_IO, PARAVER_IO_METADATA_END);
    delete_duplicate_thread_fs (copy);
    thread->last_paraver = current_time;
    Next_Action_to_thread (thread);
    break;

  case IO_Block_NonCollective:
    copy = thread;
    thread = copy->twin_thread;
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                   current_time,
                   PARAVER_IO, PARAVER_IO_BLOCK_NONCOLLECTIVE_READ_END);
    cpu = get_cpu_of_thread (copy);
    PARAVER_IO_Op (cpu->unique_number, IDENTIFIERS (copy),
                   copy->last_paraver, current_time);
    thread->last_paraver = current_time;
    delete_duplicate_thread_fs (copy);
    Next_Action_to_thread (thread);
    goto free_disk_token;

  case IO_NonBlock_NonCollective:
    copy = thread;
    thread = copy->twin_thread;
    action = copy->action;
    mpi_io = &action->desc.mpi_io;
    if ( (request_thread = thread_already_waiting (&thread->task->Ptask->MPI_IO_request_thread,
                           thread, mpi_io->request) ) != (struct t_request_thread *) 0)
    {
      extract_from_queue (&thread->task->Ptask->MPI_IO_request_thread,
                          (char *) request_thread);
      MALLOC_free_memory ( (char*) request_thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                     current_time,
                     PARAVER_IO, PARAVER_IO_NONBLOCK_NONCOLLECTIVE_END);
      MALLOC_free_memory ( (char*) copy->action);
      PARAVER_IO_Blocked (0, IDENTIFIERS (thread),
                          thread->last_paraver, current_time);
      thread->last_paraver = current_time;
      action = thread->action;

      delete_duplicate_thread_fs (copy);
      thread->last_paraver = current_time;
      Next_Action_to_thread (thread);
    }
    else
    {
      MALLOC_free_memory ( (char*) copy->action);
      delete_duplicate_thread_fs (copy);
      request_thread = (struct t_request_thread *) MALLOC_get_memory (sizeof (struct t_request_thread) );
      request_thread->thread = thread;
      request_thread->request = mpi_io->request;
      inFIFO_queue (&thread->task->Ptask->MPI_IO_request_thread,
                    (char *) request_thread);
    }
    goto free_disk_token;

  case IO_NonBlock_Collective:
    copy = thread;
    thread = copy->twin_thread;
    if (copy->action->action == FS)
    {
      action = copy->action->next;
      MALLOC_free_memory ( (char*) copy->action);
      copy->action = action;
    }
    else
    {
      action = copy->action;
    }
    mpi_io = &action->desc.mpi_io;
    fh_commid = get_fh_commid (copy, mpi_io->fh);
    switch (fh_commid->stage)
    {
    case IN_BARRIER:
      inFIFO_queue (&fh_commid->copy, (char *) copy);
      if ( (mpi_io->Oop >= 21) && (mpi_io->Oop <= 23) )
      {
        if (count_queue (&fh_commid->copy) == count_queue (&fh_commid->threads) )
        {
          for (thread = (struct t_thread *) head_queue (&fh_commid->copy);
               thread != TH_NIL;
               thread = (struct t_thread *) next_queue (&fh_commid->copy) )
          {
            node = get_node_of_thread (thread);
            if (file_system_parameters.concurrent_requests != 0)
            {
              token = (int *) outFIFO_queue (&node->IO_disks);
              if (token == (int *) 0)
              {
                inFIFO_queue (&node->IO_disks_threads, (char *) thread);
                thread->last_paraver = current_time;
                thread->IO_blocking_point = IO_NonBlock_Collective;
                continue;
              }
              MALLOC_free_memory ( (char*) token);
            }

            ti = compute_IO_collective_time (&fh_commid->threads);
            FLOAT_TO_TIMER (ti, tmp_timer);
            ADD_TIMER (current_time, tmp_timer, new_time);
            thread->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, thread, IO_NonBlock_Collective);
            node = get_node_of_thread (thread);
            cpu = get_cpu_of_thread (thread);
            PARAVER_IO_Op (cpu->unique_number,
                           thread->task->Ptask->Ptaskid,
                           thread->task->taskid,
                           thread->task->threads_count + 1,
                           current_time, new_time);
            thread->last_paraver = new_time;
          }
          fh_commid->stage = IN_IO;
        }
        return;
      }
    case IN_IO:
      extract_from_queue (&fh_commid->copy, (char *) copy);
      action = copy->action;
      copy->action = create_scatter (copy, FALSE);
      copy->action->next = create_return_to_fs (IO_NonBlock_Collective);
      copy->action->next->next = action;
      copy->action->desc.global_op.comm_id = fh_commid->communicator->communicator_id;
      action = copy->action;
      GLOBAL_operation (copy,
                        action->desc.global_op.glop_id,
                        action->desc.global_op.comm_id,
                        action->desc.global_op.root_rank,
                        action->desc.global_op.root_thid,
                        action->desc.global_op.bytes_send,
                        action->desc.global_op.bytes_recvd);

      fh_commid->counter--;
      if (fh_commid->counter == 0)
        fh_commid->stage = IN_SCATTER;
      if ( (mpi_io->Oop >= 21) && (mpi_io->Oop <= 23) )
        goto free_disk_token;
      else
        return;
    case IN_SCATTER:
      /* it is not important which one, but we must extract one */
      (void) outFIFO_queue (&fh_commid->threads);
      switch (mpi_io->Oop)
      {
      case 21:
      case 22:
      case 23:
        /* This is a read operation */
      case 24:
      case 25:
      case 26:
        /* This is a write operation */
        node = get_node_of_thread (copy);
        cpu = get_cpu_of_thread (copy);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (copy),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_COLLECTIVE_END);
        break;
      default:
        panic ("Non valid operation %d for NonBlocking Collective P%d T%d th%d\n",
               mpi_io->Oop, IDENTIFIERS (copy) );
      }
      FLOAT_TO_TIMER (1.0, tmp_timer);
      ADD_TIMER (tmp_timer, current_time, tmp_timer);
      PARAVER_Idle (0,
                    IDENTIFIERS (copy),
                    current_time,  tmp_timer);

      if ( (request_thread = thread_already_waiting (&copy->task->Ptask->MPI_IO_request_thread,
                             thread, mpi_io->request) ) != (struct t_request_thread *) 0)
      {
        extract_from_queue (&thread->task->Ptask->MPI_IO_request_thread,
                            (char *) request_thread);
        MALLOC_free_memory ( (char*) request_thread);
        node = get_node_of_thread (thread);
        cpu  = get_cpu_of_thread (thread);
        PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                       current_time,
                       PARAVER_IO, PARAVER_IO_NONBLOCK_COLLECTIVE_END);
        MALLOC_free_memory ( (char*) copy->action);
        PARAVER_IO_Blocked (0, IDENTIFIERS (thread),
                            thread->last_paraver, current_time);
        thread->last_paraver = current_time;

        delete_duplicate_thread_fs (copy);
        Next_Action_to_thread (thread);
      }
      else
      {
        MALLOC_free_memory ( (char*) copy->action);
        delete_duplicate_thread_fs (copy);
        request_thread = (struct t_request_thread *) MALLOC_get_memory (sizeof (struct t_request_thread) );
        request_thread->thread = thread;
        request_thread->request = mpi_io->request;
        inFIFO_queue (&thread->task->Ptask->MPI_IO_request_thread,
                      (char *) request_thread);
        return;
      }

      break;
    default:
      printf ("Unknown status\n");
    }
    break;
  case FS_PORT_SEND_END:
    break;

  case FS_PORT_RECV_END:
    break;

  default:
    panic ("Unexpected operation %d in FS module for P%d T%d th%d\n",
           value, IDENTIFIERS (thread) );
    break;
  }
  return;

free_disk_token:
  if (file_system_parameters.concurrent_requests != 0)
  {
    if (empty_queue (&node->IO_disks_threads) )
    {
      token = (int *) MALLOC_get_memory (sizeof (int) );
      *token = IO_Block_NonCollective;
      inFIFO_queue (&node->IO_disks, (char *) token);
    }
    else
    {
      copy = (struct t_thread *) outFIFO_queue (&node->IO_disks_threads);
      switch (copy->IO_blocking_point)
      {
      case IO_Block_NonCollective:
        thread = copy->twin_thread;
        PARAVER_IO_Blocked (0, IDENTIFIERS (thread),
                            thread->last_paraver, current_time);
        thread->last_paraver = current_time;
        goto A;
      case IO_NonBlock_NonCollective:
        thread = copy->twin_thread;
        PARAVER_IO_Blocked (0,
                            copy->task->Ptask->Ptaskid,
                            copy->task->taskid,
                            copy->task->threads_count + 1,
                            copy->last_paraver, current_time);
        FLOAT_TO_TIMER (1, tmp_timer);
        ADD_TIMER (tmp_timer, new_time, tmp_timer);
        PARAVER_Idle (0,
                      copy->task->Ptask->Ptaskid,
                      copy->task->taskid,
                      copy->task->threads_count + 1,
                      new_time, tmp_timer);
        copy->last_paraver = current_time;
        goto B;
      case IO_NonBlock_Collective:

        thread = copy->twin_thread;
        PARAVER_IO_Blocked (0,
                            copy->task->Ptask->Ptaskid,
                            copy->task->taskid,
                            copy->task->threads_count + 1,
                            copy->last_paraver, current_time);
        copy->last_paraver = current_time;

        action = copy->action;
        mpi_io = &action->desc.mpi_io;
        fh_commid = get_fh_commid (copy, mpi_io->fh);

        ti = compute_IO_collective_time (&fh_commid->threads);
        FLOAT_TO_TIMER (ti, tmp_timer);
        ADD_TIMER (current_time, tmp_timer, new_time);
        copy->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, copy, IO_NonBlock_Collective);
        node = get_node_of_thread (copy);
        cpu = get_cpu_of_thread (copy);
        PARAVER_IO_Op (cpu->unique_number,
                       copy->task->Ptask->Ptaskid,
                       copy->task->taskid,
                       copy->task->threads_count + 1,
                       current_time, new_time);
        FLOAT_TO_TIMER (1, tmp_timer);
        ADD_TIMER (tmp_timer, new_time, tmp_timer);
        PARAVER_Idle (0,
                      copy->task->Ptask->Ptaskid,
                      copy->task->taskid,
                      copy->task->threads_count + 1,
                      new_time, tmp_timer);
        copy->last_paraver = new_time;
        break;
      case IO_Block_Collective:

        thread = copy->twin_thread;
        PARAVER_IO_Blocked (0, IDENTIFIERS (copy),
                            copy->last_paraver, current_time);
        copy->last_paraver = current_time;

        action = thread->action;
        mpi_io = &action->desc.mpi_io;
        fh_commid = get_fh_commid (copy, mpi_io->fh);

        ti = compute_IO_collective_time (&fh_commid->threads);
        FLOAT_TO_TIMER (ti, tmp_timer);
        ADD_TIMER (current_time, tmp_timer, new_time);
        copy->event = EVENT_timer (new_time, NOT_DAEMON, M_FS, copy, IO_Block_Collective);
        node = get_node_of_thread (copy);
        cpu = get_cpu_of_thread (copy);
        PARAVER_IO_Op (cpu->unique_number, IDENTIFIERS (thread),
                       current_time, new_time);
        copy->last_paraver = new_time;
        break;
      default:
        panic ("Unknown blocking IO point %d\n", copy->IO_blocking_point);
      }
    }
  }

}

void FS_Init (void)
{
  struct t_cpu   *cpu;
  struct t_node  *node;
  int i;
  int *token;

  CONFIGURATION_Load_FS_Configuration();

#ifdef EQUEUE
  for (node = (struct t_node *) head_Equeue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_Equeue (&Node_queue) )
#else
  for (node = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_queue (&Node_queue) )
#endif
  {
    for (cpu = (struct t_cpu *) head_queue (& (node->Cpus) );
         cpu != C_NIL;
         cpu = (struct t_cpu *) next_queue (& (node->Cpus) ) )
    {
      cpu->io = (struct t_queue *) MALLOC_get_memory (sizeof (struct t_queue) );
      create_queue (cpu->io);
    }

    if (file_system_parameters.concurrent_requests != 0)
    {
      for (i = 0; i < file_system_parameters.concurrent_requests; i++)
      {
        token = (int *) MALLOC_get_memory (sizeof (int) );
        *token = i;
        inFIFO_queue (&node->IO_disks, (char *) token);
      }
    }
  }
}

void FS_End()
{
  struct t_cpu   *cpu;
  struct t_node  *node;
  struct t_thread *thread;
  struct t_Ptask *Ptask;

  if (debug & D_FS)
  {
    PRINT_TIMER (current_time);
    printf (": FS end routine called\n");
  }
#ifdef USE_EQUEUE
  for (node = (struct t_node *) head_Equeue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_Equeue (&Node_queue) )
#else
  for (node = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_queue (&Node_queue) )
#endif
  {
    for (cpu = (struct t_cpu *) head_queue (& (node->Cpus) );
         cpu != C_NIL;
         cpu = (struct t_cpu *) next_queue (& (node->Cpus) ) )
    {
      if (count_queue (cpu->io) )
      {
        for (thread  = (struct t_thread *) head_queue (cpu->io);
             thread != TH_NIL;
             thread  = (struct t_thread *) next_queue (cpu->io) )
        {
          PARAVER_Idle (cpu->cpuid,
                        IDENTIFIERS (thread),
                        thread->last_paraver,
                        current_time);
        }
      }
    }

    if (count_queue (&node->IO_disks_threads) )
    {
      PRINT_TIMER (current_time);
      printf (": Warning FS end with %d threads waiting IO disk on node %d\n",
              count_queue (& (node->th_for_out) ),
              node->nodeid);

      for (thread  = (struct t_thread *) head_queue (&node->IO_disks_threads);
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&node->IO_disks_threads) )
      {
        printf (" P%d T%d th%d", IDENTIFIERS (thread) );
      }
      printf ("\n");
    }
  }

  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue) )
  {
    if (count_queue (&Ptask->MPI_IO_request_thread) )
    {
      PRINT_TIMER (current_time);
      printf (": Warning FS end with %d threads waiting nonblock IO access in Ptask  %d\n",
              count_queue (& (Ptask->MPI_IO_request_thread) ),
              Ptask->Ptaskid);

      for (thread  = (struct t_thread *) head_queue (&Ptask->MPI_IO_request_thread);
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&Ptask->MPI_IO_request_thread) )
      {
        printf (" P%d T%d th%d", IDENTIFIERS (thread) );
      }
      printf ("\n");
    }
  }

  return;
}

void FS_Parameters (double disk_latency,
                    double disk_bandwidth,
                    double block_size,
                    int    concurrent_requests,
                    double hit_ratio)
{
  file_system_parameters.disk_latency =  disk_latency;
  file_system_parameters.disk_bandwidth =  disk_bandwidth;
  file_system_parameters.block_size =  block_size;
  file_system_parameters.concurrent_requests =  concurrent_requests;
  file_system_parameters.hit_ratio =  hit_ratio;
}

void FS_show_version (void)
{
  printf ("Implemented File Server policies:\n");
  printf ("    NONE\n");
}

void FS_new_io_operation (int operation_id, char *operation_name)
{
  if ( (operation_id >= 0) && (operation_id <= MAX_IO_OPERATIONS) )
  {
    if (strcmp (Operation_Name[operation_id], operation_name) != 0)
    {
      die ("Incorrect IO operation name %s for operation %d\nIt must be %s",
           operation_name,
           operation_id,
           Operation_Name[operation_id]);
    }
  }
  else
  {
    die ("Invalid operation identicator %d for %s\n",
         operation_id, operation_name);
  }
}


