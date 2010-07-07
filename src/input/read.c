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

#include "define.h"
#include "types.h"

#include "file_data_access.h"

#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "random.h"
#include "read.h"
#include "schedule.h"
#include "subr.h"
#include "task.h"
#include "yacc.h"

#if defined(OS_MACOSX) || defined(OS_CYGWIN)
#include "macosx_limits.h"
#else
#include <values.h> /* Ho necessito per la constant MAXDOUBLE */
#endif
#include <math.h>   /* Per l'arrel quadrada */

t_boolean              configuration_file;
extern struct t_queue *Registers;
static struct t_queue  Register_queue;
struct t_Ptask        *Ptask_current;
char                  *current_file;
extern t_boolean       NEW_MODIFICATIONS;

int             greatest_cpuid = 0;

dimemas_timer execution_end_time=0; /* Temps final de l'execucio */


/* For reading old trace format */
struct t_action*
get_next_action_from_file_binary(
  FILE           *file,
  int            *tid,
  int            *th_id,
  struct t_node  *node
)
{
  struct t_action *action;
  struct t_disk_action da;
  double relative;

  action = (struct t_action *) mallocame (sizeof (struct t_action));

  if (fread (&da, sizeof (struct t_disk_action), 1, file) != 1)
    return (AC_NIL);

  *tid = da.taskid;
  *th_id = da.threadid;
  *action = da.action;

  if (action->action == WORK)
  {
    relative = node->relative;
    /*
    if (randomness.processor_ratio.distribution!=NO_DISTRIBUTION)
      relative += random_dist(&randomness.processor_ratio);
    */
    if (relative<=0)
      (action->desc).compute.cpu_time = 0.0;
    else
    {
      (action->desc).compute.cpu_time =
        (t_micro) (((action->desc).compute.cpu_time) / relative);
      (action->desc).compute.cpu_time += 
        random_dist(&randomness.processor_ratio);
      
      if ((action->desc).compute.cpu_time<0)
        (action->desc).compute.cpu_time = 0.0;
    }
    /*
    if ((action->desc).compute.cpu_time == 0)
     (action->desc).compute.cpu_time = 1;
    */
   }
   action->next = AC_NIL;
   return (action);
}

static char BIGPOINTER[BUFSIZE];
static char BIGPOINTER2[BUFSIZE];

/* For reading old trace format */
struct t_action*
get_next_action_from_file_sddf(
  FILE           *file,
  int            *tid,
  int            *th_id,
  struct t_node  *node,
  struct t_Ptask *Ptask
)
{
  char   buf[BUFSIZE], new[BUFSIZE];
  struct t_action *tmp_action, *aux_action;
  int    i, j;
  int    taskid, thid;
  t_micro         cpu_time;
  int    dest, size, tag, communic_id;
  int    ori;
  int    rendez_vous;
  
  long long ev_ty, ev_va;
  
  int    fd, is, om, re;
  char  *bn, *an;
  int    block_id, burst_category_id, src_file, src_line;
  int    commid, Oop, fh, request;
  char   block_name[256], activity_name[256];
  double relative, burst_category_mean, burst_category_std_dev;
  int    glop_id, comm_id, root_rank, root_thid;
  int    bytes_send, bytes_recvd, recv_type;
  char  *c;
  int    win_id, target_rank, mode, post_size, post_rank;

  tmp_action = (struct t_action *) mallocame (sizeof (struct t_action));
  tmp_action->next = AC_NIL;

next_one:

  i = fscanf (file, "%[^\n]\n", BIGPOINTER);
  
  if (strstr(BIGPOINTER, "/* SEEK")!= (char *)0)
  {
    return (AC_NIL);
  }
  if (i == -1)
  {
    return (AC_NIL);
  }

  { /* CPU burst */
  i = sscanf ( BIGPOINTER,
               "\"CPU burst\" { %d, %d, %le };;\n",
               &taskid, &thid, &cpu_time);
  if (i == 3)
  {
    if (synthetic_bursts)
    {
      i = sscanf ( BIGPOINTER,
                   "\"CPU burst\" { %d, %d, %d };;\n",
                   &taskid, &thid, &burst_category_id );
      
      if ( (cpu_time = SYNT_BURST_get_burst_value(burst_category_id)) < 0)
        cpu_time = 0;

      /*
      fprintf(stdout,
              "Synt Burst (%d): %lf\n",
              burst_category_id,
              cpu_time);
      
      
      if (cpu_time < 0)
        cpu_time = fabs(cpu_time);
      */
      
      /* To generate Paraver trace with cluster information */
      
      /* First, the end event */
      aux_action = (struct t_action *) mallocame (sizeof (struct t_action));
      aux_action->action          = EVEN;
      aux_action->desc.even.type  = 90000001;
      aux_action->desc.even.value = 0;
      aux_action->next            = AC_NIL;

      /* Link with main (Burst) action */
      tmp_action->next = aux_action;

      /* The cluster ID event */
      aux_action = (struct t_action *) mallocame (sizeof (struct t_action));
      aux_action->action     = EVEN;
      aux_action->desc.even.type  = 90000001;
      aux_action->desc.even.value = burst_category_id+1;
      aux_action->next            = tmp_action;

    }
    
    cpu_time = cpu_time * 1e6;
    relative = node->relative;
    /*
    if (randomness.processor_ratio.distribution!=NO_DISTRIBUTION)
    {
      relative += random_dist(&randomness.processor_ratio);
    }
    */
    if (relative<=0)
    {
      cpu_time = (t_micro) 0.0;
    }
    else
    {
      cpu_time = (t_micro) (cpu_time / relative);
      /* cpu_time += random_dist(&randomness.processor_ratio); */
      if (cpu_time<=0)
      {
        cpu_time = (t_micro) 0.0;
      }
    }
    /*
    if (cpu_time < 1.0)
    {
      cpu_time = 1.0;
    }
    */
    (tmp_action->desc).compute.cpu_time = (t_micro) cpu_time;

    tmp_action->action = WORK;
    *tid = taskid + 1;
    *th_id = thid + 1;

    if (synthetic_bursts)
      return aux_action;
    else
      return tmp_action;
  }
  }
  { /* NX send (7 parameters) */
  i = sscanf (BIGPOINTER,
              "\"NX send\" { %d, %d, %d, %d, %d, %d, %d };;\n",
              &taskid, &thid, &dest, &size, &tag, &communic_id, &rendez_vous );
  if (i == 7)
  {
    /* Send trace */
    (tmp_action->desc).send.dest        = dest + 1;
    (tmp_action->desc).send.mess_size   = size;
    (tmp_action->desc).send.mess_tag    = tag;
    (tmp_action->desc).send.communic_id = communic_id;
     /* OJOJOJOJOJO */
    (tmp_action->desc).send.immediate   = 1;
    /* El bit mes baix del rendez_vous indica si el send es sincron o no. */
    (tmp_action->desc).send.rendez_vous = 
      USE_RENDEZ_VOUS((rendez_vous & ((int)1)),size);
    /*
    fprintf(
      stderr,
      "Un SEND 7 amb rendez_vous=%d\n",
      (tmp_action->desc).send.rendez_vous
    );
    */
    /* El segon bit mes baix del rendez_vous indica si es un immediate send o no */
    if (rendez_vous & ((int)2))
    {
      (tmp_action->desc).send.immediate = TRUE;
    }
    else
    {
      (tmp_action->desc).send.immediate = FALSE;
    }
    tmp_action->action = SEND;
    *tid               = taskid + 1;
    *th_id             = thid + 1;
    return (tmp_action);
  }
  }
  { /* NX send (6 parameters) */
  i = sscanf( BIGPOINTER,
              "\"NX send\" { %d, %d, %d, %d, %d, %d };;\n",
              &taskid, &thid, &dest, &size, &tag, &communic_id );
  if (i == 6)
  {
    /* Send trace */
    (tmp_action->desc).send.dest        = dest + 1;
    (tmp_action->desc).send.mess_size   = size;
    (tmp_action->desc).send.mess_tag    = tag;
    (tmp_action->desc).send.communic_id = communic_id;
    /* OJOJOJOJOJO */
    (tmp_action->desc).send.immediate   = 1;
    (tmp_action->desc).send.rendez_vous = USE_RENDEZ_VOUS(0,size);
    /*
    fprintf(
      stderr,
      "Un SEND 6 amb rendez_vous=%d\n",
      (tmp_action->desc).send.rendez_vous
    );
    */
    tmp_action->action = SEND;
    *tid   = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* NX send (5 parameters) */
  i = sscanf( BIGPOINTER,
            "\"NX send\" { %d, %d, %d, %d, %d };;\n",
            &taskid, &thid, &dest, &size, &tag );
  
  if (i == 5)
  {
    /* Send trace */
    (tmp_action->desc).send.dest        = dest + 1;
    (tmp_action->desc).send.mess_size   = size;
    (tmp_action->desc).send.mess_tag    = tag;
    (tmp_action->desc).send.communic_id = -1;
    /* OJOJOJOJOJO */
    (tmp_action->desc).send.immediate   = 1;
    (tmp_action->desc).send.rendez_vous = USE_RENDEZ_VOUS(0,size);
    tmp_action->action = SEND;
    *tid   = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* NX recv (7 parameters) */
  /* FEC: Ara hi pot haver un camp que indica el tipus de recv */
  i = sscanf( BIGPOINTER,
              "\"NX recv\" { %d, %d, %d, %d, %d, %d, %d};;\n",
              &taskid, &thid, &ori, &size, &tag, &communic_id, &recv_type );
  
  if (i == 7)
  {
    /* Send trace */
    (tmp_action->desc).recv.ori = ori + 1;
    (tmp_action->desc).recv.mess_size = size;
    (tmp_action->desc).recv.mess_tag = tag;
    (tmp_action->desc).recv.communic_id = communic_id;
    if (recv_type==1)
    {
      tmp_action->action = IRECV;
    }
    else if (recv_type==2)
    {
      tmp_action->action = WAIT;
    }
    else
    {
      tmp_action->action = RECV;
    }
    *tid   = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* NX recv (6 parameters) */
  i = sscanf( BIGPOINTER,
              "\"NX recv\" { %d, %d, %d, %d, %d, %d };;\n",
              &taskid, &thid, &ori, &size, &tag, &communic_id );
  if (i == 6)
  {
    /* Send trace */
    (tmp_action->desc).recv.ori         = ori + 1;
    (tmp_action->desc).recv.mess_size   = size;
    (tmp_action->desc).recv.mess_tag    = tag;
    (tmp_action->desc).recv.communic_id = communic_id;
    tmp_action->action = RECV;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* NX recv (5 parameters) */
  i = sscanf( BIGPOINTER, 
              "\"NX recv\" { %d, %d, %d, %d, %d };;\n",
              &taskid, &thid, &ori, &size, &tag );
  
  if (i == 5)
  {
    /* Send trace */
    (tmp_action->desc).recv.ori = ori + 1;
    (tmp_action->desc).recv.mess_size = size;
    (tmp_action->desc).recv.mess_tag = tag;
    (tmp_action->desc).recv.communic_id = -1;
    tmp_action->action = RECV;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* block begin */
  i = sscanf( BIGPOINTER,
              "\"block begin\" { %d, %d, %lld};;\n",
              &taskid, &thid, &ev_ty );
  
  if (i == 3)
  {
    (tmp_action->desc).even.type = BLOCK_BEGIN;
    (tmp_action->desc).even.value = ev_ty;
    tmp_action->action = EVEN;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* block end */
  i = sscanf( BIGPOINTER,
              "\"block end\" { %d, %d, %lld};;\n",
              &taskid, &thid, &ev_ty );
  
  if (i == 3)
  {
    (tmp_action->desc).even.type = BLOCK_END;
    (tmp_action->desc).even.value = ev_ty;
    tmp_action->action = EVEN;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* user event */
  i = sscanf ( BIGPOINTER,
              "\"user event\" { %d, %d, %lld, %lld};;\n",
              &taskid, &thid, &ev_ty, &ev_va);
  if (i == 4)
  {
    (tmp_action->desc).even.type  = ev_ty;
    (tmp_action->desc).even.value = ev_va;
    tmp_action->action = EVEN;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* global OP */
  i = sscanf ( BIGPOINTER,
               "\"global OP\" { %d, %d, %d, %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &glop_id,
               &comm_id, &root_rank, &root_thid,
               &bytes_send, &bytes_recvd );
  if (i == 8)
  {
    tmp_action->action = GLOBAL_OP;
    tmp_action->desc.global_op.glop_id = glop_id;
    tmp_action->desc.global_op.comm_id = comm_id;
    tmp_action->desc.global_op.root_rank = root_rank;
    tmp_action->desc.global_op.root_thid = root_thid;
    tmp_action->desc.global_op.bytes_send = bytes_send;
    tmp_action->desc.global_op.bytes_recvd = bytes_recvd;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* block definition */
  i = sscanf( BIGPOINTER,
              "\"block definition\" { %d, \"%[^\"]\", \"%[^\"]\", %d, %d};;\n",
              &block_id, block_name, activity_name,
              &src_file, &src_line );
  /* DEBUG
  fprintf(stdout, "Trying block definition\n");
  */
  if (i == 5)
  {
    bn = (char *)mallocame (strlen(block_name)+1);
    strcpy (bn, block_name);
    an = (char *)mallocame (strlen(activity_name)+1);
    strcpy (an, activity_name);
    module_name(Ptask, block_id, bn, an, src_file, src_line);
    goto next_one;
  }
  }
  { /* user event type definition */
  i = sscanf( BIGPOINTER, 
              "\"user event type definition\" { %d, \"%[^\"]\", %d};;\n",
              &block_id,   /* user event type */
              block_name,  /* user event type name */
              &src_line);  /* user event type flag color */
  if (i == 3)
  {
    bn = (char *)mallocame (strlen(block_name)+1);
    strcpy (bn, block_name);
    user_event_type_name(Ptask, block_id, bn, src_line);
    goto next_one;
  }
  }
  { /* user event value definition */
  i = sscanf( BIGPOINTER, 
              "\"user event value definition\" { %d, %d, \"%[^\"]\"};;\n",
              &block_id,    /* user event type */
              &src_line,    /* user event value */
              block_name);  /* user event value name */

  if (i == 3)
  {
    bn = (char *)mallocame (strlen(block_name)+1);
    strcpy (bn, block_name);
    user_event_value_name(Ptask, block_id, src_line, bn);
    goto next_one;
  }
  }
  { /* file definition */
  i = sscanf ( BIGPOINTER,
               "\"file definition\" { %d, \"%[^\"]\"};;\n",
               &block_id, block_name );
  if (i == 2)
  {
    bn = (char *)mallocame (strlen(block_name)+1);
    strcpy (bn, block_name);
    file_name(Ptask, block_id, bn);
    goto next_one;
  }
  }  
  { /* global OP definition */
  i = sscanf ( BIGPOINTER,
               "\"global OP definition\" { %d, \"%[^\"]\"};;\n",
               &block_id, block_name );

  if (i == 2)
  {
    bn = (char *)mallocame (strlen(block_name)+1);
    strcpy (bn, block_name);
    new_global_op(block_id, bn);
    goto next_one;
  }
  }
  { /* communicator definition */
  i = sscanf ( BIGPOINTER,
               "\"communicator definition\" { %d, %d, [%*d] {%[^}]}};;\n",
               &comm_id, &root_rank, BIGPOINTER2 );
  if (i == 3)
  {
    new_communicator_definition(Ptask, comm_id);
    c = strtok (BIGPOINTER2, ",}");
    for (i=0;i<root_rank; i++)
    {
      j = atoi(c);
      add_identificator_to_communicator(Ptask, comm_id, j);
      c = strtok (NULL, ",}");
    }
    no_more_identificator_to_communicator (Ptask, comm_id);
    goto next_one;
  }
  } 
  { /* OS window definition */
  i = sscanf ( BIGPOINTER,
               "\"OS window definition\" { %d, %d, [%*d] {%[^}]}};;\n",
               &win_id, &root_rank, BIGPOINTER2 );
  if (i == 3)
  {
    new_window_definition(Ptask, win_id);
    c = strtok (BIGPOINTER2, ",}");
    for (i=0;i<root_rank; i++)
    {
      j = atoi(c);
      add_identificator_to_window(Ptask, win_id, j);
      c = strtok (NULL, ",}");
    }
    no_more_identificator_to_window (Ptask, win_id);
    goto next_one;
  }
  }
  { /* IO OP definition */
  i = sscanf ( BIGPOINTER,
               "\"IO OP definition\" { %d, \"%[^\"]\"};;\n",
               &block_id, block_name );
  if (i == 2)
  {
    new_io_operation (block_id, bn);
    goto next_one;
  }
  }
  { /* IO Collective Metadata */
  i = sscanf ( BIGPOINTER,
               "\"IO Collective Metadata\" { %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &commid, &fh, &Oop);
  if (i == 5)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_Metadata;
    tmp_action->desc.mpi_io.commid = commid;
    tmp_action->desc.mpi_io.fh = fh;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO Blocking NonCollective */
  i = sscanf ( BIGPOINTER,
               "\"IO Blocking NonCollective\" { %d, %d, %d, %d};;\n",
               &taskid, &thid, &size, &Oop);
  if (i == 4)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_Block_NonCollective;
    tmp_action->desc.mpi_io.size = size;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO Blocking Collective */
  i = sscanf ( BIGPOINTER,
               "\"IO Blocking Collective\" { %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &fh, &size, &Oop);
  if (i == 5)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_Block_Collective;
    tmp_action->desc.mpi_io.fh = fh;
    tmp_action->desc.mpi_io.size = size;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO NonBlocking NonCollective begin */
  i = sscanf ( BIGPOINTER,
             "\"IO NonBlocking NonCollective begin\" { %d, %d, %d, %d, %d};;\n",
             &taskid, &thid, &size, &request, &Oop);
  if (i == 5)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_NonBlock_NonCollective_Begin;
    tmp_action->desc.mpi_io.size = size;
    tmp_action->desc.mpi_io.request = request;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO NonBlocking NonCollective end */
  i = sscanf ( BIGPOINTER,
               "\"IO NonBlocking NonCollective end\" { %d, %d, %d, %d};;\n",
               &taskid, &thid, &request, &Oop);
  if (i == 4)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_NonBlock_NonCollective_End;
    tmp_action->desc.mpi_io.request = request;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO NonBlocking Collective begin */
  i = sscanf ( BIGPOINTER,
               "\"IO NonBlocking Collective begin\" { %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &fh, &size, &Oop );
  if (i == 5)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_NonBlock_Collective_Begin;
    tmp_action->desc.mpi_io.fh = fh;
    tmp_action->desc.mpi_io.size = size;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* IO NonBlocking Collective end */
  i = sscanf ( BIGPOINTER,
               "\"IO NonBlocking Collective end\" { %d, %d, %d, %d};;\n",
               &taskid, &thid, &fh, &Oop );
  if (i == 4)
  {
    tmp_action->action = MPI_IO;
    tmp_action->desc.mpi_io.which_io = MPI_IO_NonBlock_Collective_End;
    tmp_action->desc.mpi_io.fh = fh;
    tmp_action->desc.mpi_io.Oop = Oop;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* OS operation */
  i = sscanf ( BIGPOINTER,\
               "\"OS operation\" { %d, %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &Oop, &target_rank, &win_id, &size );
  if (i == 6)
  {
    tmp_action->action = MPI_OS;
    tmp_action->desc.mpi_os.which_os = MPI_OS_GETPUT;
    tmp_action->desc.mpi_os.Oop = Oop;
    tmp_action->desc.mpi_os.target_rank = target_rank;
    tmp_action->desc.mpi_os.window_id = win_id;
    tmp_action->desc.mpi_os.size = size;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* OS fence */
  i = sscanf ( BIGPOINTER,
               "\"OS fence\" { %d, %d, %d };;\n",
               &taskid, &thid, &win_id );
  if (i == 3)
  {
    tmp_action->action = MPI_OS;
    tmp_action->desc.mpi_os.which_os = MPI_OS_FENCE;
    tmp_action->desc.mpi_os.window_id = win_id;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* OS lock */
  i = sscanf ( BIGPOINTER,
               "\"OS lock\" { %d, %d, %d, %d, %d };;\n",
               &taskid, &thid, &win_id, &Oop, &mode );
  if (i == 5)
  {
    tmp_action->action = MPI_OS;
    tmp_action->desc.mpi_os.which_os = MPI_OS_LOCK;
    tmp_action->desc.mpi_os.window_id = win_id;
    tmp_action->desc.mpi_os.Oop = Oop;
    tmp_action->desc.mpi_os.mode = mode;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* OS post */
  i = sscanf ( BIGPOINTER,
               "\"OS post\" { %d, %d, %d, %d, %d, %d };;\n",
               &taskid, &thid, &win_id, &Oop, &post_size, &post_rank );
  if (i == 6)
  {
    tmp_action->action = MPI_OS;
    tmp_action->desc.mpi_os.which_os = MPI_OS_POST;
    tmp_action->desc.mpi_os.window_id = win_id;
    tmp_action->desc.mpi_os.Oop = Oop;
    tmp_action->desc.mpi_os.post_size = post_size;
    /* tmp_action->desc.mpi_os.post_ranks = post_rank;*/
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* FS open */
  i = sscanf ( BIGPOINTER,
               "\"FS open\" { %d, %d, %d, %d, %d, \"%[^\"]\"};;\n",
               &taskid, &thid, &fd, &is, &om, new );
  if (i == 6)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop                = OPEN;
    (tmp_action->desc).fs_op.fs_o.fs_open.fd           = fd;
    (tmp_action->desc).fs_op.fs_o.fs_open.initial_size = is;
    (tmp_action->desc).fs_op.fs_o.fs_open.flags        = om;
    strncpy ((tmp_action->desc).fs_op.fs_o.fs_open.filename, new, FILEMAX);
    (tmp_action->desc).fs_op.fs_o.fs_open.filename[FILEMAX] = (char) 0;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS read */
  i = sscanf ( BIGPOINTER,
               "\"FS read\" { %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &fd, &is, &om );
  if (i == 5)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop                  = READ;
    (tmp_action->desc).fs_op.fs_o.fs_read.fd             = fd;
    (tmp_action->desc).fs_op.fs_o.fs_read.requested_size = is;
    (tmp_action->desc).fs_op.fs_o.fs_read.delivered_size = om;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS write */
  i = sscanf ( BIGPOINTER,
               "\"FS write\" { %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &fd, &is, &om );
  if (i == 5)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop                   = WRITE;
    (tmp_action->desc).fs_op.fs_o.fs_write.fd             = fd;
    (tmp_action->desc).fs_op.fs_o.fs_write.requested_size = is;
    (tmp_action->desc).fs_op.fs_o.fs_write.delivered_size = om;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS seek */
  i = sscanf ( BIGPOINTER,
               "\"FS seek\" { %d, %d, %d, %d, %d, %d};;\n",
               &taskid, &thid, &fd, &is, &om, &re );
  if (i == 6)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop          = SEEK;
    (tmp_action->desc).fs_op.fs_o.fs_seek.fd     = fd;
    (tmp_action->desc).fs_op.fs_o.fs_seek.offset = is;
    (tmp_action->desc).fs_op.fs_o.fs_seek.whence = om;
    (tmp_action->desc).fs_op.fs_o.fs_seek.result = re;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS close */
  i = sscanf ( BIGPOINTER,
               "\"FS close\" { %d, %d, %d};;\n",
               &taskid, &thid, &fd);
  if (i == 3)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop       = CLOSE;
    (tmp_action->desc).fs_op.fs_o.fs_close.fd = fd;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS dup */
  i = sscanf ( BIGPOINTER,
               "\"FS dup\" { %d, %d, %d, %d};;\n",
               &taskid, &thid, &fd, &re );
  if (i == 4)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop         = DUP;
    (tmp_action->desc).fs_op.fs_o.fs_dup.old_fd = fd;
    (tmp_action->desc).fs_op.fs_o.fs_dup.new_fd = re;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }

  { /* FS unlink */
  i = sscanf ( BIGPOINTER,
               "\"FS unlink\" { %d, %d, \"%[^\"]\"};;\n",
               &taskid, &thid, new);
  if (i == 3)
  {
    tmp_action->action = FS;
    (tmp_action->desc).fs_op.which_fsop = UNLINK;
    strncpy ((tmp_action->desc).fs_op.fs_o.fs_unlink.filename, new, FILEMAX);
    (tmp_action->desc).fs_op.fs_o.fs_unlink.filename[FILEMAX] = (char) 0;
    *tid = taskid + 1;
    *th_id = thid + 1;
    return (tmp_action);
  }
  }
  { /* Burst category definition */
    i = sscanf ( BIGPOINTER,
                 "\"burst category definition\" { %d, %lf, %lf };;\n",
                 &burst_category_id,
                 &burst_category_mean,
                 &burst_category_std_dev);
    
    if (i == 3)
    {

      SYNT_BURST_add_new_burst_category(burst_category_id,
                                        burst_category_mean,
                                        burst_category_std_dev);
      
      goto next_one;
    }
  }
  { /* Comments */
  if (strstr(BIGPOINTER, "/*")!= (char *)0)
    goto next_one;
  }
  panic ("Unkown trace file format (%s)\n", BIGPOINTER);
  
  return (AC_NIL);
}

static t_boolean
it_is_ascii_file ()
{
  t_boolean       re = FALSE;
  char            b[BUFSIZE];

  /* First, we must rewind std input, because GZIP test function has 
   * advanced file pointer */ 
  rewind(stdin);
 
  scanf ("%5s", b);
  if (strcmp ("SDDFA", b) == 0)
  {
    re = TRUE;
    rewind (stdin);
  }
  else 
  {
   if (strcmp("SDDFB",b) !=0)
     panic ("File %s is not a Dimemas trace file format\n",current_file);
  }
  return(re);
}

/* Function it_is_gzip_file
 * This function detects if current tracefile is compressed with GZIP
 *
 * @return TRUE value if current tracefile is compressed with GZIP
 */

static int const gz_magic[2] = {0x1f, 0x8b}; /* Needed to detect GZIP files */

static t_boolean
it_is_gzip_file()
{
  t_boolean result = FALSE;
  int       magic[2];
  
  scanf ("%c%c", &magic[0], &magic[1]);
  
  if (magic[0] == gz_magic[0] && magic[1] == gz_magic[1])
  {
    result = TRUE;
  }
  else
  {
    result = FALSE;
  }
  return result;
}


/* This function detects if current tracefile is in new format or not */
t_boolean 
is_it_new_format()
{
  FILE *tracefile;
  char buffer[32];
  t_boolean new_format;


  if((tracefile = fopen(((struct t_Ptask *) head_queue (&Ptask_queue))->tracefile, "r")) == NULL)
  {
    panic ("Tracefile %s can't be opened\n", ((struct t_Ptask *) head_queue (&Ptask_queue))->tracefile);
  }

  while(strlen(fgets(buffer, 32, tracefile)) == 1);

  if(strstr(buffer, "#DIMEMAS:") != NULL)
	new_format = TRUE;
  else
	new_format = FALSE;


  fclose(tracefile);
  return new_format;
}




static void
read_trace_file_for_Ptask (
  struct t_Ptask *Ptask,
  char           *TraceFile
)
{
  current_file = TraceFile;

  if (MYFREOPEN (TraceFile, "r", stdin) == NULL)
  {
    panic ("Tracefile %s can't be opened\n", TraceFile);
  }

  /* JGG Aqui hay que anyadir el acceso a trazas GZIP */ 
  Ptask_current = Ptask;
  
  if (it_is_gzip_file())
  {
    panic("Compressed tracefiles support is under development!\n");
    binary_files     = FALSE;
    gzip_files       = TRUE;
    sorted_files     = TRUE;
    load_interactive = TRUE;
  }
  
  if (it_is_ascii_file())
  {
    gzip_files = FALSE;
    if (load_interactive)
    {
      binary_files = FALSE;
      sorted_files = TRUE;
    }
    else
    {
      if (yyparse ())
        panic ("Error parsing tracefile %s\n", TraceFile);
    }
  }
  else
  {
    binary_files     = TRUE;
    load_interactive = TRUE;
    sorted_files     = TRUE;
  }

  if (load_interactive && !gzip_files)
  {
    sddf_load_initial_work_to_threads (Ptask);
  }
  else if (gzip_files)/* gzip_files */
  {
    /* GZIP load inital work */
    panic("Compressed tracefiles not supported yet!\n");
  }
}

static void
Ptask_reload (
  struct t_Ptask *Ptask,
  char           *TraceFile
)
{
  struct t_queue        *comms_queue;
  struct t_communicator *comm;
  
  struct t_task   *task;
  struct t_thread *thread;
  struct t_action *new_action;
  
  int main_app_findex = -1;
  
    if (debug)
    {
      printf ("* RELOADing Ptask %02d\n", Ptask->Ptaskid);
    }

    main_app_findex = DATA_ACCESS_ptask_id_end(Ptask->Ptaskid); // End current task (closes the file, frees memory, etc.)
    if (main_app_findex < 0) {
      panic ("While reloading - DATA_ACCESS_end did not find the Ptask %02d\n", Ptask->Ptaskid);
    }
    
    if (DATA_ACCESS_init_index(Ptask->Ptaskid, TraceFile, main_app_findex) == FALSE)
      panic(DATA_ACCESS_get_error());
    
    if (DATA_ACCESS_get_communicators(Ptask->Ptaskid, &comms_queue) == FALSE)
      panic(DATA_ACCESS_get_error());
    
    
    for (comm  = (struct t_communicator*) head_queue (comms_queue);
         comm != COM_NIL;
         comm  = (struct t_communicator*) next_queue(comms_queue))
    {
      insert_queue(&Ptask->Communicator,
                   (char*) comm,
                   (t_priority)comm->communicator_id);
    }
    /* This comm_queue is not freed? */
    
    /* Load initial actions to each thread */
    for ( task  = (struct t_task *) head_queue (&(Ptask->tasks));
          task != T_NIL;
          task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      for (thread  = (struct t_thread *) head_queue (&(task->threads));
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&(task->threads)))
      {
        get_next_action(thread);
      }
    }
}

void
reload_new_Ptask (struct t_Ptask *Ptask)
{
  struct t_Ptask *P;
  int             i = 0;
  int             sin = 0;
  struct t_task   *task;
  struct t_thread *thread;
  struct t_cpu    *cpu;


  
  for (P  = (struct t_Ptask *) head_queue (&Ptask_queue);
       P != P_NIL;
       P  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    if (P->synthetic_application)
    {
      sin++;
      continue;
    }
    
    if (  (P->Ptaskid == Ptask->Ptaskid)  /* this solves a bug */
       && (P->n_rerun < reload_limit)  )
    {
      break;
    }
    i++;
  }
  
  if (i == count_queue (&(Ptask_queue)) - sin)
  {
    if (EQ_0_TIMER(final_statistical_time))
      ASS_ALL_TIMER (final_statistical_time, current_time);
    return;
  }

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": RELOAD  P%d iteration %d out of %d\n",
            Ptask->Ptaskid, Ptask->n_rerun + 1, reload_limit);
    PRINT_TIMER (current_time);
    printf (": RELOAD  i = %d, count_queue = %d, sin = %d\n", i, count_queue (&(Ptask_queue)), sin);
  }
  
  Ptask->n_rerun++;
  clear_last_actions (Ptask);

  /* Old or new trace format */
  if(!new_trace_format)
	read_trace_file_for_Ptask (Ptask, Ptask->tracefile);
  else
	Ptask_reload (Ptask, Ptask->tracefile); /* new format */


  SCHEDULER_reload (Ptask);
  reload_done = TRUE;


  /* Putting and initial event in every Ptask simulation */

  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {

    thread = (struct t_thread *) head_queue (&(task->threads));

    cpu = get_cpu_of_thread(thread);

    Paraver_event (cpu->unique_number,
                  Ptask->Ptaskid,
                  task->taskid,
                  thread->threadid,
                  current_time,
                  (long long int)40000000,
                  (long long int)Ptask->n_rerun);

  }



}

void
init_simul_char()
{

  /* Assign default values */
  sim_char.number_machines               = 1;
  sim_char.general_net.name              = (char *)0;
  sim_char.general_net.traffic_function  = 0;
  sim_char.general_net.max_traffic_value = 0;
  sim_char.general_net.bandwidth         = 0;
  sim_char.general_net.global_operation  = GOP_MODEL_CTE;
  
  create_queue (&(sim_char.general_net.threads_on_network));
  sim_char.dedicated_connections.number_connections=0;
  sim_char.general_net.flight_times = NULL;
  create_queue(&sim_char.general_net.global_ops_info);
}

static void
print_statistics(
  char             *buf,
  struct t_account *account, 
  dimemas_timer     total_time
)
{
  
  t_micro  cpu_time, latency_time, block_due_resources, block_time, ready_time,
           tot_time;

  TIMER_TO_FLOAT (account->cpu_time, cpu_time);
  TIMER_TO_FLOAT (account->latency_time, latency_time);
  TIMER_TO_FLOAT (account->block_due_resources, block_due_resources);
  TIMER_TO_FLOAT (account->time_waiting_for_message, block_time);
  TIMER_TO_FLOAT (account->time_ready_without_cpu, ready_time);
  TIMER_TO_FLOAT (total_time, tot_time);

  fprintf (salida_datos, "%s\t", buf);
  FPRINT_TIMER (salida_datos, account->cpu_time);
  
  fprintf (salida_datos, "\t%03.02f\t", cpu_time * 100 / tot_time);
  FPRINT_TIMER (salida_datos, account->time_waiting_for_message);
  
  fprintf (salida_datos, "\t%03.02f\t", block_time * 100 / tot_time);
  
  if (NEW_MODIFICATIONS)
  {
    FPRINT_TIMER (salida_datos, account->latency_time);
    fprintf (salida_datos, "\t%03.02f\t", latency_time * 100 / tot_time);
    
    FPRINT_TIMER (salida_datos, account->block_due_resources);
    fprintf (salida_datos, "\t%03.02f\t", block_due_resources * 100 / tot_time);
  }

  if ( full_out_info )
  {
    FPRINT_TIMER (salida_datos, account->time_ready_without_cpu);
    fprintf (salida_datos, "\t%03.02f\t", ready_time * 100 / tot_time);
  }

  fprintf (salida_datos, "%le\t%le\t", account->n_sends, account->n_bytes_send);
  
  fprintf (salida_datos,
           "%le\t%le\t%le", 
           account->n_recvs_on_processor,
           account->n_recvs_must_wait,
           account->n_bytes_recv);
   
  fprintf (salida_datos, "%le\t", account->n_group_operations);
  FPRINT_TIMER (salida_datos, account->block_due_group_operations);
  
  fprintf (salida_datos, "\t");
  FPRINT_TIMER (salida_datos, account->group_operations_time);
   
  if (full_out_info)
  {
    fprintf (salida_datos, "\t%.0f",account->n_th_in_run);
  }
  fprintf (salida_datos,"\n");
}

static void
show_individual_statistics (struct t_Ptask *Ptask)
{

  struct t_task  *task;
  struct t_thread *thread;
  struct t_account *acc_Ptask_sum, *acc_Ptask_min, *acc_Ptask_max, *account,
                   *acc_th;
  dimemas_timer   total_time;
  int             num_task;
  struct t_node  *node;
  char            buf[100];
  t_micro         tmp_micro;

  fprintf (salida_datos, "\nInformation for application %d\n", Ptask->Ptaskid);

  acc_Ptask_sum = new_accounter ();
  acc_Ptask_min = new_accounter ();
  acc_Ptask_max = new_accounter ();
  
  acc_Ptask_min->cpu_time = current_time;
  acc_Ptask_min->latency_time             = current_time;
  acc_Ptask_min->block_due_resources      = current_time;
  acc_Ptask_min->n_th_in_run              = INT_MAX;
  acc_Ptask_min->n_sends                  = INT_MAX;
  acc_Ptask_min->n_bytes_send             = INT_MAX;
  acc_Ptask_min->n_recvs                  = INT_MAX;
  acc_Ptask_min->n_bytes_recv             = INT_MAX;
  acc_Ptask_min->n_recvs_on_processor     = INT_MAX;
  acc_Ptask_min->n_recvs_must_wait        = INT_MAX;
  acc_Ptask_min->time_waiting_for_message = current_time;
  acc_Ptask_min->time_ready_without_cpu   = current_time;

  num_task = count_queue (&(Ptask->tasks));

  ASS_TIMER (total_time, 0);
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));

    if (reload_Ptasks==FALSE)
    {
      account = current_account (thread);
    }
    else
    {
      for (account  = (struct t_account*)head_queue (&(thread->account));
           account != ACC_NIL;
           account  = (struct t_account*)next_queue(&(thread->account)))
      {
        if (account->iteration==Ptask->n_rerun-1)
          break;
      }
    }
    MAX_TIMER (account->final_time, total_time, total_time);
  }

  if (EQ_0_TIMER(total_time))
    ASS_ALL_TIMER (total_time, current_time);
     
  if ((full_out_info) && (reload_Ptasks))
  {
    fprintf (salida_datos,
             "Total rerun %d and application time: ",
             Ptask->n_rerun-1);
    
    FPRINT_TIMER (salida_datos, total_time);
    TIMER_TO_FLOAT (total_time, tmp_micro);
    tmp_micro = tmp_micro / (Ptask->n_rerun-1);
    fprintf (salida_datos, "\tAverage time: %.6f\n",tmp_micro/1e6);
  }

  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));

    if (reload_Ptasks == FALSE)
    {
      account = current_account (thread);
    }
    else
    {
      acc_th = new_accounter();
      for (account  = (struct t_account *)head_queue (&(thread->account));
           account != ACC_NIL;
           account  = (struct t_account *)next_queue (&(thread->account)))
      {
        if (account->iteration<Ptask->n_rerun)
          add_account (acc_th, account);
      }
      account = acc_th;
    }

    node = get_node_of_thread (thread);
    add_account (acc_Ptask_sum, account);
    min_account (acc_Ptask_min, account);
    max_account (acc_Ptask_max, account);

    if (full_out_info)
      sprintf (buf, "%d (%d)", task->taskid, node->nodeid );
    else
      sprintf (buf, "%d", task->taskid);

    print_statistics (buf, account, total_time);
  }

  fprintf (salida_datos, "\n");
  print_statistics ("Sum", acc_Ptask_sum, total_time);

  print_statistics ("Min", acc_Ptask_min, total_time);

  print_statistics ("Max", acc_Ptask_max, total_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->cpu_time, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->cpu_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->latency_time, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->latency_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->block_due_resources, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->block_due_resources);

  TIMER_TO_FLOAT (acc_Ptask_sum->time_waiting_for_message, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->time_waiting_for_message);

  TIMER_TO_FLOAT (acc_Ptask_sum->time_ready_without_cpu, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->time_ready_without_cpu);

  acc_Ptask_sum->n_sends = 
    acc_Ptask_sum->n_sends / num_task;

  acc_Ptask_sum->n_bytes_send = 
    acc_Ptask_sum->n_bytes_send / num_task;

  acc_Ptask_sum->n_recvs_on_processor =
    acc_Ptask_sum->n_recvs_on_processor / num_task;

  acc_Ptask_sum->n_recvs_must_wait =
    acc_Ptask_sum->n_recvs_must_wait / num_task;

  acc_Ptask_sum->n_bytes_recv =
    acc_Ptask_sum->n_bytes_recv / num_task;

  print_statistics ("Aver.", acc_Ptask_sum, total_time);
}

void calculate_execution_end_time()
{
  struct t_Ptask   *Ptask;
  struct t_task    *task;
  struct t_thread  *thread;
  struct t_account *account;

  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    ASS_TIMER (execution_end_time, 0);
    for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      thread = (struct t_thread *) head_queue (&(task->threads));
      account = current_account (thread);
      MAX_TIMER (account->final_time, execution_end_time, execution_end_time);
    }
  }

  if (EQ_0_TIMER(execution_end_time))
    ASS_ALL_TIMER (execution_end_time, current_time);
}

/*******************************************************************************
 * MACROS PARA LA FUNCION 'show_individual_statistics_pallas'
 ******************************************************************************/
#define SP_TOTAL_INDEX 0   
#define SP_AVG_INDEX   1
#define SP_MAX_INDEX   2
#define SP_MIN_INDEX   3
#define SP_STDEV_INDEX 4

/* Macro que inicialitza totes les estadistiques totals */
#define SP_INIT_ESTADISTIQUES() \
{ \
  int i; \
  \
  bzero(totals,sizeof(double)*NUM_COLS_EST*NUM_ESTADISTICS); \
  for(i=0;i<NUM_COLS_EST;i++) \
  { \
    totals[i][SP_MIN_INDEX]=MAXDOUBLE; \
    totals[i][SP_MAX_INDEX]=-MAXDOUBLE; \
  } \
}

/* Macro que calcula totes les estadistiques d'una columna */
#define SP_ESTADISTIQUES(index,valor) \
{ \
  totals[index][SP_TOTAL_INDEX]+=(valor); \
  totals[index][SP_AVG_INDEX]++; /* De moment conta el numero de files */ \
  if ((valor)<totals[index][SP_MIN_INDEX]) \
    totals[index][SP_MIN_INDEX]=(valor); \
  if ((valor)>totals[index][SP_MAX_INDEX]) \
    totals[index][SP_MAX_INDEX]=(valor); \
  /* Acumulem els valors al quadrat */ \
  totals[index][SP_STDEV_INDEX] += ((valor)*(valor)); \
}

/* Macro que fa els ultims tractaments a totes les estadistiques totals */
#define SP_FINI_ESTADISTIQUES() \
{ \
  int i; \
  double num_files, aux; \
  \
  for(i=0;i<NUM_COLS_EST;i++) \
  { \
    num_files=totals[i][SP_AVG_INDEX]; \
    /* Es calcula la mitjana */ \
    totals[i][SP_AVG_INDEX]=totals[i][SP_TOTAL_INDEX] / num_files; \
    /* Es calcula la variança */ \
    totals[i][SP_STDEV_INDEX]/=num_files; \
    aux=(totals[i][SP_AVG_INDEX] * totals[i][SP_AVG_INDEX]); \
    totals[i][SP_STDEV_INDEX]-=aux; \
    /* Si el valor es -0 es passa a positiu */ \
    if (totals[i][SP_STDEV_INDEX]==0.0) \
      totals[i][SP_STDEV_INDEX]=0.0; \
    totals[i][SP_STDEV_INDEX]=sqrt(totals[i][SP_STDEV_INDEX]); \
  } \
}

static void
show_individual_statistics_pallas (struct t_Ptask *Ptask)
{
  struct t_task  *task;
  struct t_thread *thread;
  struct t_account *acc_Ptask_sum, *acc_Ptask_min, *acc_Ptask_max, *account,
                   *acc_th, **acc_Ptask_rerun;
  dimemas_timer   total_time, comm_time;
  int             num_task;
  struct t_node  *node;
  t_micro         tmp_micro;
  float x,y,z;
  int i, j;
  dimemas_timer  t_io;

  /* FEC: Afegeixo aixo per tenir linies de totals de les estadistiques/task */
  #define NUM_COLS_EST 11
  #define NUM_ESTADISTICS 5

  char *totals_names[NUM_ESTADISTICS]={
    "Total",
    "Average",
    "Maximum",
    "Minimum",
    "Stdev"
  };

  /* Les linies de totals de les estadistiques */
  double totals[NUM_COLS_EST][NUM_ESTADISTICS];

  acc_Ptask_sum = new_accounter ();

  num_task = count_queue (&(Ptask->tasks));

  ASS_TIMER (total_time, 0);

  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));
    account = current_account (thread);
    MAX_TIMER (account->final_time, total_time, total_time);
  }

  if (EQ_0_TIMER(total_time))
    ASS_ALL_TIMER (total_time, current_time);

  /* JGG: Show an small App information */
  fprintf(salida_datos,
          "**** Application %d (%s) ****\n",
          Ptask->Ptaskid,
          Ptask->tracefile);

  /* JGG: For short out info, only 'total_time' is needed */
  if (short_out_info)
  {
    FPRINT_TIMER(salida_datos, total_time);
    fprintf(salida_datos, "\n");
    return;
  }
  
  /* FEC: Inicialitzo les estadistiques totals */
  SP_INIT_ESTADISTIQUES();
    
  /* Initialization of accounters per reload */
  /* We have (n_rerun + 1) iterations */
  acc_Ptask_rerun = (struct t_account **)malloc(sizeof(struct t_account *) * (Ptask->n_rerun + 1));
  for(i = 0; i <= Ptask->n_rerun; i++)
        acc_Ptask_rerun[i] = new_accounter();

  /* Calcul Estadistiques per iteracio i totals */
  int n_rerun;
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));
 
    n_rerun = 0;
    for (account  = (struct t_account*)head_queue (&(thread->account));
        account != ACC_NIL;
        account  = (struct t_account*)next_queue(&(thread->account)))
    {

        add_account(acc_Ptask_rerun[n_rerun], account);
        n_rerun++;
        add_account(acc_Ptask_sum, account);

     }
  }



  /* Estadistiques per iteracio */
  if(Ptask->n_rerun > 0)
  {
     fprintf(salida_datos, "\n**** Statistics by iteration ****\n\n");
     for(n_rerun = 0; n_rerun <= Ptask->n_rerun; n_rerun++)
     {
             fprintf(salida_datos,"Execution time Iteration %d:\t", n_rerun);

             FPRINT_TIMER (salida_datos, (acc_Ptask_rerun[n_rerun]->final_time - acc_Ptask_rerun[n_rerun]->initial_time));

             TIMER_TO_FLOAT(acc_Ptask_rerun[n_rerun]->cpu_time, x);
             TIMER_TO_FLOAT((acc_Ptask_rerun[n_rerun]->final_time - acc_Ptask_rerun[n_rerun]->initial_time), y);
             fprintf (salida_datos,"\nSpeedup Iteration %d:\t\t%.2f \n",n_rerun, x/y);

             fprintf (salida_datos,"CPU Time Iteration %d:\t\t", n_rerun);
             FPRINT_TIMER (salida_datos, acc_Ptask_rerun[n_rerun]->cpu_time);
             fprintf (salida_datos,"\n\n");
     }
  }



  /* Estadistiques totals */
  fprintf(salida_datos, "\n**** Total Statistics ****\n\n");
  fprintf (salida_datos,"Execution time:\t");
  FPRINT_TIMER (salida_datos, total_time);

  TIMER_TO_FLOAT(acc_Ptask_sum->cpu_time, x);
  TIMER_TO_FLOAT(total_time, y);
  fprintf (salida_datos,"\nSpeedup:\t%.2f \n",x/y);

  /* FEC: Mostro el temps de cpu total */
  fprintf (salida_datos,"CPU Time:\t");
  FPRINT_TIMER (salida_datos, acc_Ptask_sum->cpu_time);
  fprintf (salida_datos,"\n\n");

   
  fprintf (salida_datos," Id.\tComputation\t%%time\tCommunication\n");
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));
    account = current_account (thread);
    node = get_node_of_thread (thread);
    fprintf (salida_datos, "%3d\t",task->taskid);
    FPRINT_TIMER (salida_datos, account->cpu_time);
    TIMER_TO_FLOAT(account->cpu_time, x);
    TIMER_TO_FLOAT(account->latency_time, y);
    TIMER_TO_FLOAT(account->time_waiting_for_message,z);
    y = y+z;
    TIMER_TO_FLOAT(account->block_due_group_operations, z);
    y = y+z;
    TIMER_TO_FLOAT(account->group_operations_time, z);
    y = y+z;
    FLOAT_TO_TIMER(y, comm_time);
    fprintf (salida_datos, "\t%03.2f\t", (float)x*100/(x+y));
    FPRINT_TIMER (salida_datos, comm_time);
    fprintf (salida_datos,"\n");
  }

  fprintf (salida_datos,"\n");

  /* FEC: Afegeixo mes informacio */
  /* 
  fprintf (salida_datos," Id.\tMess.sent\tBytes sent\tImmediate recv\tWaiting recv\tBytes recv\tColl.op.\tBlock time\tComm. time\n");*/
  
  fprintf (salida_datos,
           " Id.\tMess.sent\tBytes sent\tImmediate recv\tWaiting recv");
  fprintf (salida_datos,
           "\tBytes recv\tColl.op.\tBlock time\tComm. time\tWait link time");
  fprintf (salida_datos,
           "\tWait buses time\tI/O time\n");

  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    thread = (struct t_thread *) head_queue (&(task->threads));
    account = current_account (thread);
    node = get_node_of_thread (thread);
    fprintf (salida_datos, "%3d\t%le\t%le\t",
             task->taskid,
             account->n_sends, account->n_bytes_send);

    fprintf (salida_datos, "%le\t%le\t%le",
             account->n_recvs_on_processor, 
             account->n_recvs_must_wait,
             account->n_bytes_recv);

    fprintf (salida_datos, "\t%le\t",
             account->n_group_operations);

    FPRINT_TIMER (salida_datos, account->block_due_group_operations);
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->group_operations_time);

    /* FEC: Afegeixo mes informacio */
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->block_due_link);
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->block_due_buses);

    fprintf (salida_datos, "\t");
    /* Si s'han comptat junts a la trac,a� trf els reads i writes, no s'han de 
     * sumar perque ja s'han comptat junts tamb�al simular. */
    /* ADD_TIMER(account->io_read_time,account->io_write_time,t_io); */
    ASS_ALL_TIMER(t_io,account->io_call_time);
    FPRINT_TIMER (salida_datos, t_io);

    fprintf (salida_datos, "\n"); 

    /* FEC: Calculo les estadistiques totals d'aquesta fila */
    SP_ESTADISTIQUES(0,account->n_sends);
    SP_ESTADISTIQUES(1,account->n_bytes_send);
    SP_ESTADISTIQUES(2,account->n_recvs_on_processor);
    SP_ESTADISTIQUES(3,account->n_recvs_must_wait);
    SP_ESTADISTIQUES(4,account->n_bytes_recv);
    SP_ESTADISTIQUES(5,account->n_group_operations);
    TIMER_TO_FLOAT(account->block_due_group_operations,z);
    SP_ESTADISTIQUES(6,z);
    TIMER_TO_FLOAT(account->group_operations_time,z);
    SP_ESTADISTIQUES(7,z);
    TIMER_TO_FLOAT(account->block_due_link,z);
    SP_ESTADISTIQUES(8,z);
    TIMER_TO_FLOAT(account->block_due_buses,z);
    SP_ESTADISTIQUES(9,z);
    TIMER_TO_FLOAT(t_io,z);
    SP_ESTADISTIQUES(10,z);

  }

  /* FEC: Acabo les estadistiques totals i les mostro */
  SP_FINI_ESTADISTIQUES();
  fprintf(salida_datos,"----");

  for (j=0; j<NUM_COLS_EST; j++)
    fprintf(salida_datos,"----------------");

  fprintf(salida_datos,"\n");

  for (i=0; i<NUM_ESTADISTICS; i++)
  {
    fprintf(salida_datos,"%s\t",totals_names[i]);

    for (j=0; j<NUM_COLS_EST; j++)
    {
      fprintf(salida_datos,"%le\t",totals[j][i]);
    }
    fprintf(salida_datos,"\n");
  }

}

void
show_statistics (int pallas_output)
{
 struct t_Ptask *Ptask;
 t_micro         total_time;

 if (pallas_output)
 {
   for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
     show_individual_statistics_pallas(Ptask);
   }
 }
 else
 {
   TIMER_TO_FLOAT (current_time, total_time);

   fprintf (salida_datos, "Final time: ");
   FPRINT_TIMER (salida_datos, current_time);
   fprintf (salida_datos, "\n");
   for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
     show_individual_statistics (Ptask);
   }
 }
}

void
load_configuration_file (char *filename)
{
  configuration_file = TRUE;
  current_file = filename;
   
  if (MYFREOPEN (filename, "r", stdin) == NULL)
  {
    panic ("Can't open configuration file %s\n", filename);
  }
  Registers = &Register_queue;
  create_queue (Registers);
  
  if (yyparse ())
  {
    panic ("Error parsing configuration file %s\n", filename);
  }
}

void
Read_Ptasks()
{
  char           *TraceFile;
  struct t_Ptask *Ptask;
  
  configuration_file = FALSE;
  
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    TraceFile = (char *) mallocame (strlen (Ptask->tracefile)+1);
    strcpy (TraceFile, Ptask->tracefile);
    Ptask->tracefile = TraceFile;
    
    if (debug)
    {
      printf ("* Loading Ptask %02d\n", Ptask->Ptaskid);
    }
  
    read_trace_file_for_Ptask (Ptask, TraceFile);
  }
}

void
add_global_ops(void)
{
  int i;
  
  for (i = 0; i < GLOBAL_OPS_COUNT; i++)
  {
    new_global_op(i, Global_Ops_Labels[i]);
  }
}

void
Ptasks_init()
{
  char                  *TraceFile;
  struct t_Ptask        *Ptask;
  struct t_queue        *comms_queue;
  struct t_communicator *comm;
  
  struct t_task   *task;
  struct t_thread *thread;
  struct t_action *new_action;
  
  /* Add predefined global ops identificators */
  add_global_ops();
  
  /* Used on in-core parsing */
  configuration_file = FALSE;
  
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    TraceFile = (char *) mallocame (strlen (Ptask->tracefile)+1);
    strcpy (TraceFile, Ptask->tracefile);
    Ptask->tracefile = TraceFile;
    
    if (debug)
    {
      printf ("* Loading Ptask %02d\n", Ptask->Ptaskid);
    }

    if (DATA_ACCESS_init(Ptask->Ptaskid, TraceFile) == FALSE)
      panic(DATA_ACCESS_get_error());

    if (DATA_ACCESS_get_communicators(Ptask->Ptaskid, &comms_queue) == FALSE)
      panic(DATA_ACCESS_get_error());
    
    
    for (comm  = (struct t_communicator*) head_queue (comms_queue);
         comm != COM_NIL;
         comm  = (struct t_communicator*) next_queue(comms_queue))
    {
      insert_queue(&Ptask->Communicator,
                   (char*) comm,
                   (t_priority)comm->communicator_id);
    }

    /* Load initial actions to each thread */
    for ( task  = (struct t_task *) head_queue (&(Ptask->tasks));
          task != T_NIL;
          task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      for (thread  = (struct t_thread *) head_queue (&(task->threads));
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&(task->threads)))
      {
        get_next_action(thread);
      }
    }
  }
}

void MAIN_TEST_print_communicator(struct t_communicator* comm) {
  char* item = NULL;
  int* task = NULL;
  struct t_queue* rank = NULL;
  
  rank = &comm->global_ranks;

  printf("Comunicator [%02d]: ", comm->communicator_id);
  
  // Print prcedure of global ranks
  item = head_queue(rank);
  while(item != NULL) {
    task = (int*)item;
    
    printf("%02d ", *task);
    
    item = next_queue(rank);
  }
  printf("\n");
}


void
get_next_action(struct t_thread *thread)
{
  int Ptask_id, task_id, thread_id;
  struct t_action *new_action, *new_action_aux;
  
  Ptask_id  = thread->task->Ptask->Ptaskid;
  task_id   = thread->task->taskid;
  thread_id = thread->threadid;
  
  if (DATA_ACCESS_get_next_action(Ptask_id,
                                  task_id,
                                  thread_id,
                                  &new_action) == FALSE)
  {
    panic(DATA_ACCESS_get_error());
  }
  
  if (new_action == AC_NIL)
  {
    /* DEBUG 
    fprintf(stdout,"NULL ACTION\n"); */
    thread->action = AC_NIL;
    return;
  }
  
  if (new_action->action == WORK)
  {
    new_action->desc.compute.cpu_time *= 1e6; 
    if (PREEMP_enabled)
      new_action->desc.compute.cpu_time += PREEMP_overhead(thread->task);
      {
	  struct t_module* mod;
	  t_priority identificator = 0;

	  mod = (struct t_module*) query_prio_queue(&Ptask_current->Modules,
						    (t_priority)identificator);
	  if (mod==M_NIL)
	  {
	    mod = (struct t_module *)mallocame (sizeof(struct t_module));
	    mod->identificator = identificator;
	    mod->ratio = 1.0;
	    mod->block_name = (char *)0;
	    mod->activity_name = (char *)0;
	    mod->src_file = -1;
	    mod->src_line = -1;
	    insert_queue (&Ptask_current->Modules, (char *)mod, (t_priority) identificator);
	  }
  	  inLIFO_queue (&(thread->modules), (char *)mod);
    }
    recompute_work_upon_modules(thread, new_action);
  }
  
  thread->action = new_action;
  
  /* Ensure last action is not a MPI IO action */
  new_action = thread->action;
  if (new_action == AC_NIL)
  {
    return;
  }
  
  while (new_action->next != AC_NIL)
    new_action = new_action->next;
  
  if ((new_action->action == MPI_IO))
  {
    new_action_aux = thread->action;
    
    if (DATA_ACCESS_get_next_action(Ptask_id,
                                    task_id,
                                    thread_id,
                                    &new_action) == FALSE)
    {
      panic(DATA_ACCESS_get_error());
    }
    
    new_action->next = thread->action;
    thread->action = new_action_aux;
  }
  
  return;
}


/* Function to mask if we read old or new traces */
void 
get_next_action_wrapper(struct t_thread *th)
{
    if(new_trace_format)
	get_next_action(th);
    else
	sddf_seek_next_action_to_thread (th);
}
