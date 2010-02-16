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

#include "extern.h"
#include "sddf.h"


/* ************************************ */
/* SDDF PARAGRAPH Format              */
/* ************************************ */

char            label_1[] = {"executable information"};
char            label_2[] = {"source information"};
char            label_10[] = {"global trace statistics"};
char            label_11[] = {"global msgp statistics"};
char            label_20[] = {"monitor start"};
char            label_21[] = {"monitor stop"};
char            label_40[] = {"NX send"};
char            label_41[] = {"NX recv"};
char            label_44[] = {"send overhead start"};
char            label_45[] = {"send overhead end"};
char            label_46[] = {"recv overhead start"};
char            label_47[] = {"recv overhead end"};
char            label_48[] = {"msgp overhead start"};
char            label_49[] = {"msgp overhead end"};
char            label_50[] = {"msgp idle start"};
char            label_51[] = {"msgp idle end"};
char            label_82[] = {"task begin"};
char            label_83[] = {"task end"};

char           *sddf_header[7] = {
   "SDDFA",
   "/*",
   " * \"DIMEMAS Trace Format\" \"Version 1.0\"",
   " * \"Run date\"",
   " * \"System\" \"noname\"",
   " * \"Partition\" \"noname\"",
   " */ ;;"
};

char           *type1[21] = {
   "#1:",
   "\"executable information\" {",
   "// \"System height\" \"System height in nodes\"",
   "int	\"system height\";",
   "// \"System width\" \"System width in nodes\"",
   "int	\"system width\";",
   "// \"physical\" \"List of physical node IDs\"",
   "int	\"physical\"[];",
   "// \"numnodes\" \"Number of nodes in application\"",
   "int	\"numnodes\";",
   "// \"Process Group\" \"Process Group ID\"",
   "int	\"pgroup\";",
   "// \"pids\" \"List of Process IDs in trace\"",
   "int	\"pids\"[];",
   "// \"nodes\" \"List of nodes in trace\"",
   "int	\"nodes\"[];",
   "// \"executables\" \"List of indexes into path array\"",
   "int	\"executables\"[];",
   "// \"path\" \"List of full pathnames to executables\"",
   "char	\"path\"[][];",
   "};;"
};

char           *type2[7] = {
   "#2:",
   "\"source information\" {",
   "// \"path\" \"Full pathname to executables\"",
   "char	\"path\"[];",
   "// \"directories\" \"List of source directories\"",
   "char	\"directories\"[][];",
   "};;"
};

char           *type10[9] = {
   "#10:",
   "\"global trace statistics\" {",
   "// \"Min time\" \"Minimum timestamp in trace\"",
   "double	\"min time stamp\";",
   "// \"Max time\" \"Maximum time stamp in trace\"",
   "double	\"max time stamp\";",
   "// \"Rollout sum\" \"Sum of Roll out times\"",
   "double	\"rollout sum\";",
   "};;"
};

char           *type11[27] = {
   "#11:",
   "\"global msgp statistics\" {",
   "// \"sends\" \"Number of send records\"",
   "int	\"num sends\";",
   "// \"recvs\" \"Number of recv records\"",
   "int	\"num recvs\";",
   "// \"Min msg length\" \"Minimum msg length\"",
   "int	\"min msg length\";",
   "// \"Max msg length\" \"Maximum msg length\"",
   "int	\"max msg length\";",
   "// \"Min msg type\" \"Minimum msg type\"",
   "int	\"min msg type\";",
   "// \"Max msg type\" \"Maximum msg type\"",
   "int	\"max msg type\";",
   "// \"Max msg queue\" \"Max msgs queued\"",
   "int	\"max msg queue\";",
   "// \"Max msglength queue\" \"Max bytes queued\"",
   "int	\"max msglength queue\";",
   "// \"Total msgs queued\" \"Total msgs queued\"",
   "int	\"total msgs queued\";",
   "// \"Total msglength queued\" \"Total bytes queued\"",
   "int	\"total msglength queued\";",
   "// \"Min task ID\" \"Min user defined task ID\"",
   "int	\"min task\";",
   "// \"Max task ID\" \"Max user defined task ID\"",
   "int	\"max task\";",
   "};;"
};

char           *type20[17] = {
   "#20:",
   "\"monitor start\" {",
   "// \"Time\" \"Performance monitor start time\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "// \"frequency\" \"Node's clock frequency\"",
   "int	\"clock frequency\";",
   "};;"
};

char           *type21[15] = {
   "#21:",
   "\"monitor stop\" {",
   "// \"Time\" \"Performance monitor stop time\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type40[25] = {
   "#40:",
   "\"NX send\" {",
   "// \"Time\" \"Time message was sent\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "// \"type\" \"Message type\"",
   "int	\"msg type\";",
   "// \"buffer\" \"Address of buffer\"",
   "int	\"buffer address\";",
   "// \"length\" \"Message length\"",
   "int	\"msg length\";",
   "// \"dest node\" \"Message destination node\"",
   "int	\"dest node\";",
   "// \"dest ptype\" \"Message destination ptype\"",
   "int	\"dest ptype\";",
   "};;"
};

char           *type41[25] = {
   "#41:",
   "\"NX recv\" {",
   "// \"Time\" \"Time message was received\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "// \"type\" \"Message type\"",
   "int	\"msg type\";",
   "// \"length\" \"Message length\"",
   "int	\"msg length\";",
   "// \"source node\" \"Source node of send\"",
   "int	\"source node\";",
   "// \"sending ptype\" \"Messages's sending ptype\"",
   "int	\"sending ptype\";",
   "// \"sending node\" \"Messages's sending node\"",
   "int	\"sending node\";",
   "};;"
};

char           *type44[15] = {
   "#44:",
   "\"send overhead start\" {",
   "// \"Time\" \"Start of send overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type45[15] = {
   "#45:",
   "\"send overhead end\" {",
   "// \"Time\" \"End of send overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type46[15] = {
   "#46:",
   "\"recv overhead start\" {",
   "// \"Time\" \"Start of recv overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type47[15] = {
   "#47:",
   "\"recv overhead end\" {",
   "// \"Time\" \"End of recv overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type48[15] = {
   "#48:",
   "\"msgp overhead start\" {",
   "// \"Time\" \"Start of messge passing overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type49[15] = {
   "#49:",
   "\"msgp overhead end\" {",
   "// \"Time\" \"End of message passing overhead\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type50[15] = {
   "#50:",
   "\"msgp idle start\" {",
   "// \"Time\" \"Start of message passing idle time\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type51[15] = {
   "#51:",
   "\"msgp idle end\" {",
   "// \"Time\" \"End of message passing idle time\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "};;"
};

char           *type82[17] = {
   "#82:",
   "\"task begin\" {",
   "// \"Time\" \"Time of function entry\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "// \"task id\" \"User defined task id\"",
   "int	\"task id\";",
   "};;"
};

char           *type83[17] = {
   "#83:",
   "\"task end\" {",
   "// \"Time\" \"Time of task end\"",
   "double	\"time stamp\";",
   "// \"node\" \"Process's node number\"",
   "int	\"node\";",
   "// \"pid\" \"Process ID\"",
   "int	\"pid\";",
   "// \"pthread\" \"Pthread ID\"",
   "int	\"pthread\";",
   "// \"ptype\" \"Process's ptype\"",
   "int	\"ptype\";",
   "// \"address\" \"Address of tracepoint\"",
   "int	\"address\";",
   "// \"task id\" \"User defined task id\"",
   "int	\"task id\";",
   "};;"
};


struct t_sddf_stats
{
   int             n_cpus;
   int             n_tasks;
   int             n_sends;
   int             n_recvs;
   int             min_mesg_len;
   int             max_mesg_len;
   int             min_tag;
   int             max_tag;
   int             total_mesg_queued;
   int             total_bytes_queued;
   int             max_mesg_queued;
   int             max_bytes_queued;
};

static char     date[256];
static struct t_sddf_stats sddf_stats = {0, 0, 0, 0, INT_MAX, 0, INT_MAX, 0, 0, 0, 0, 0};

char           *sddf_filename = (char *) 0;
FILE           *sddf_file = (FILE *) 0;

static void
SDDF_type (char *line[], int top)
{
   int             i;

   for (i = 0; i < 2; i++)
      fprintf (sddf_file, "%s\n", line[i]);
   for (i = 2; i < (top - 1); i++)
      fprintf (sddf_file, "\t%s\n", line[i]);
   fprintf (sddf_file, "%s\n", line[top - 1]);
   fprintf (sddf_file, "\n");
}


static void
SDDF_Header()
{
   int             i;
   float           max_time;

   for (i = 0; i < 7; i++)
      if (i != 3)
	 fprintf (sddf_file, "%s\n", sddf_header[i]);
      else
	 fprintf (sddf_file, "%s \"%s\"\n", sddf_header[i], date);
   fprintf (sddf_file, "\n");

   SDDF_type (type1, 21);
   SDDF_type (type2, 7);
   SDDF_type (type10, 9);
   SDDF_type (type11, 27);
   SDDF_type (type20, 17);
   SDDF_type (type21, 15);
   SDDF_type (type40, 25);
   SDDF_type (type41, 25);
   SDDF_type (type44, 15);
   SDDF_type (type45, 15);
   SDDF_type (type46, 15);
   SDDF_type (type47, 15);
   SDDF_type (type48, 15);
   SDDF_type (type49, 15);
   SDDF_type (type50, 15);
   SDDF_type (type51, 15);
   SDDF_type (type82, 17);
   SDDF_type (type83, 17);


/* Executable information */
   fprintf (sddf_file, "\"%s\" { %d, 1,\n", label_1, sddf_stats.n_cpus);

/* list of physical nodes IDs */
   fprintf (sddf_file, "\t[%d] {\n", sddf_stats.n_cpus + 1);
   for (i = 0; i < sddf_stats.n_cpus; i++)
      fprintf (sddf_file, "\t%d,\n", i);
   fprintf (sddf_file, "\t9999\n\t}, %d, %d,\n", sddf_stats.n_cpus,
	    sddf_stats.n_tasks);

/* list of process Ids in trace */
   fprintf (sddf_file, "\t[%d] {\n", sddf_stats.n_tasks);
   for (i = 0; i < sddf_stats.n_tasks - 1; i++)
      fprintf (sddf_file, "\t%d,\n", i);
   fprintf (sddf_file, "\t%d\n", sddf_stats.n_tasks - 1);
   fprintf (sddf_file, "\t},\n");

/* list of nodes in trace */
   fprintf (sddf_file, "\t[%d] {\n", sddf_stats.n_cpus);
   for (i = 0; i < sddf_stats.n_cpus - 1; i++)
      fprintf (sddf_file, "\t%d,\n", i);
   fprintf (sddf_file, "\t%d\n", sddf_stats.n_cpus - 1);
   fprintf (sddf_file, "\t},\n");

/* list of indexes into path array */
   fprintf (sddf_file, "\t[%d] {\n", sddf_stats.n_cpus);
   for (i = 0; i < sddf_stats.n_cpus - 1; i++)
      fprintf (sddf_file, "\t0,\n");
   fprintf (sddf_file, "\t1\n");
   fprintf (sddf_file, "\t},\n");

/* list of full pathnames to executables */
   fprintf (sddf_file, "\t[%d][%d] {\n", 2, 20);
   fprintf (sddf_file, "\t\"./executable1\",\n");
   fprintf (sddf_file, "\t\"./executable2\",\n");
   fprintf (sddf_file, "\t} };;\n\n");

/* source information */
   fprintf (sddf_file, "\"%s\" {\n", label_2);
   fprintf (sddf_file, "\t[%d] {\n", 5);
   fprintf (sddf_file, "\t\"host\"\n");
   fprintf (sddf_file, "\t},\n");
   fprintf (sddf_file, "\t[%d][%d] {\n", 1, 2);
   fprintf (sddf_file, "\t\".\"\n");
   fprintf (sddf_file, "\t} };;\n\n");

/* global trace statistics */
   TIMER_TO_FLOAT (current_time, max_time);
   fprintf (sddf_file, "\"%s\" { 0.000000, %f, 0 };;\n\n",
	    label_10, max_time / 1000000);
/* Statistics */
   fprintf (sddf_file,
	    "\"%s\" { %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0, 0 };;\n\n",
	    label_11, sddf_stats.n_sends, sddf_stats.n_recvs,
	    sddf_stats.min_mesg_len, sddf_stats.max_mesg_len,
	    sddf_stats.min_tag, sddf_stats.max_tag,
	    sddf_stats.max_mesg_queued, sddf_stats.max_bytes_queued,
	    sddf_stats.total_mesg_queued, sddf_stats.total_bytes_queued);
}


void
SDDF_start(int cpuid, int taskid, dimemas_timer time)
{
   t_micro         ti;

   if (sddf_file == (FILE *) 0)
      return;

   sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid);
   sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid);

   TIMER_TO_FLOAT (time, ti);
   ti = ti / 1000000;

   cpuid--;
   taskid--;

/* monitor start (label 20) */
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0, -1 };;\n\n",
	    label_20, ti, taskid, taskid);
}


void
SDDF_stop(int cpuid, int taskid, dimemas_timer time)
{
   t_micro         ti;

   if (sddf_file == (FILE *) 0)
      return;

   sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid);
   sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid);

   TIMER_TO_FLOAT (time, ti);
   ti = ti / 1000000;

   cpuid--;
   taskid--;

/* monitor stop (label 21) */
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0, -1 };;\n\n",
	    label_21, ti, taskid, taskid);
}


void
SDDF_recv_start (int cpuid_r, int taskid_r, dimemas_timer logical_time)
{
   t_micro         ti;

   if (sddf_file == (FILE *) 0)
      return;

   sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_r);
   sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_r);


   TIMER_TO_FLOAT (logical_time, ti);
   ti = ti / 1000000;

   cpuid_r--;
   taskid_r--;

/* RECV + BEGIN */
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
	    label_46, ti, taskid_r, taskid_r);
}

void
SDDF_recv_block (int cpuid_r, int taskid_r, dimemas_timer logical_time)
{
  t_micro         ti;
  
  if (sddf_file == (FILE *) 0)
    return;
  
  sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_r);
  sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_r);
  
  
  TIMER_TO_FLOAT (logical_time, ti);
  ti = ti / 1000000;
  
  cpuid_r--;
  taskid_r--;
  
  fprintf (sddf_file,
           "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
           label_47,
           ti,
           taskid_r,
           taskid_r);
  
  fprintf (sddf_file,
           "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
           label_50,
           ti,
           taskid_r,
           taskid_r);
}


void
SDDF_recv_stop(int           cpuid_r,
               int           taskid_r,
               dimemas_timer tim,
               int           tag,
               int           size,
               int           cpuid_s,
               int           taskid_s)
{
  t_micro         time;
  
  if (sddf_file == (FILE *) 0)
    return;
  
  sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_r);
  sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_r);
  sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_s);
  sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_s);
  
  sddf_stats.n_recvs++;
  
  sddf_stats.min_mesg_len = MIN (sddf_stats.min_mesg_len, size);
  sddf_stats.max_mesg_len = MAX (sddf_stats.max_mesg_len, size);
  sddf_stats.min_tag = MIN (sddf_stats.min_tag, tag);
  sddf_stats.max_tag = MAX (sddf_stats.max_tag, tag);
  
  TIMER_TO_FLOAT (tim, time);
  time = time / 1000000;
  
  cpuid_r--;
  taskid_r--;
  cpuid_s--;
  taskid_s--;

  /* RECV + END */
  fprintf (sddf_file,
           "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
           label_51,
           time,
           taskid_r,
           taskid_r);

  fprintf (sddf_file,
           "\"%s\" { %f, %d, %d, 0, 0, 0, %d, %d, %d, 0, %d };;\n\n",
           label_41,
           time,
           taskid_r,
           taskid_r,
           tag,
           size,
           taskid_s,
           taskid_s);
}


void
SDDF_send_start(int cpuid_s, int taskid_s, dimemas_timer physical)
{
  t_micro         ti;
  
  if (sddf_file == (FILE *) 0)
    return;
  
  sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_s);
  sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_s);
  
  sddf_stats.n_sends++;
  
  cpuid_s--;
  taskid_s--;
  
  TIMER_TO_FLOAT (physical, ti);
  ti = ti / 1000000;
  
  fprintf (sddf_file,
           "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
           label_44,
           ti,
           taskid_s,
           taskid_s);
  }


void
SDDF_send_stop(int cpuid_s, int taskid_s, dimemas_timer physical, int tag,
	       int size, int cpuid_r, int taskid_r)
{
   t_micro         ti;

   if (sddf_file == (FILE *) 0)
      return;

   sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_s);
   sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_s);
   sddf_stats.n_cpus = MAX (sddf_stats.n_cpus, cpuid_r);
   sddf_stats.n_tasks = MAX (sddf_stats.n_tasks, taskid_r);

   cpuid_s--;
   taskid_s--;
   cpuid_r--;
   taskid_r--;

   TIMER_TO_FLOAT (physical, ti);
   ti = ti / 1000000;

   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0, %d, 0, %d, %d, 0 };;\n\n",
	    label_40, ti, taskid_s, taskid_s, tag, size, taskid_r);
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
	    label_50, ti, taskid_s, taskid_s);
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
	    label_51, ti, taskid_s, taskid_s);
   fprintf (sddf_file, "\"%s\" { %f, %d, %d, 0, 0, 0 };;\n\n",
	    label_45, ti, taskid_s, taskid_s);
}

static int      current_bytes_queued = 0;
static int      current_messg_queued = 0;

void
SDDF_in_message (int by)
{
  sddf_stats.total_mesg_queued++;
  sddf_stats.total_bytes_queued += by;

  current_bytes_queued += by;
  current_messg_queued++;

  sddf_stats.max_mesg_queued = MAX (sddf_stats.max_mesg_queued, 
                                    current_messg_queued);
   
  sddf_stats.max_bytes_queued = MAX (sddf_stats.max_bytes_queued,
                                     current_bytes_queued);
}

void
SDDF_out_message(int by)
{
  current_bytes_queued -= by;
  current_messg_queued--;
}

static char     nom1[1250];

static char     buf[BUFSIZE];

void
SDDF_do()
{
  int             i;
  char           *c;
  FILE           *sour;
  
  if (sddf_file == (FILE *) 0)
    return;
  
  fclose (sddf_file);
  c = tempnam (".", nom1);
  i = rename (sddf_filename, c);
  
  sour = MYFOPEN (c, "r");
  if (sour == (FILE *) 0)
  {
    printf ("Can't open source file %s\n", c);
    perror ((char *) 0);
    exit (2);
  }
  
  sddf_file = MYFOPEN (sddf_filename, "w");
  if (sddf_file == (FILE *) 0)
  {
    printf ("Can't open destination file %s\n", sddf_filename);
    perror ((char *) 0);
    exit (2);
  }
  
  SDDF_Header ();
  while (feof (sour) == FALSE)
  {
    i = fread (buf, 1, BUFSIZE, sour);
    fwrite (buf, 1, i, sddf_file);
  }
  fclose (sour);
  fclose (sddf_file);
  unlink (c);
}
