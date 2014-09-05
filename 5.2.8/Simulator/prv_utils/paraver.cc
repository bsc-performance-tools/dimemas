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

  $URL:: https://svn.bsc.es/repos/DIMEMAS/trunk/s#$:  File
  $Rev:: 35                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-01-11 19:45:04 +0100 (Wed, 11 Jan #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
extern "C" {
#endif

#include <EventEncoding.h>

#include <define.h>
#include <types.h>

/*
#include "cpu.h"
#include "extern.h"
#include "fs.h"
#include "list.h"

#include "subr.h"

*/
#include "paraver.h"
#include "paraver_pcf.h"

#include "dimemas_io.h"
#include "list.h"
#include "subr.h"
#include "task.h"
#include "node.h"
#include "simulator.h"

#ifdef __cplusplus
}
#endif

#include <climits>
#include <cfloat>
#include <cstring>
#include <cerrno>

#include <vector>
using std::vector;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include <algorithm>
using std::sort;

#include <map>
using std::map;

#include <utility>
using std::pair;
using std::make_pair;

#include <iostream>
#include <fstream>
using std::ofstream;
using std::cout;
using std::endl;

#include "paraver_records.h"
#include "external_sort.h"

t_boolean      paraver_priorities      = FALSE;
t_boolean      paraver_different_waits = TRUE;
dimemas_timer  start_paraver;
dimemas_timer  stop_paraver;
t_boolean      paraver_initial_timer = FALSE;
t_boolean      paraver_final_timer   = FALSE;

static string    paraver_trace_filename;
static string    pcf_insert_filename;         // PCF portion to be added to output PCF

int       PARAVER_cpu;
int       paraver_line_number    = 1;

t_boolean generar_paraver = FALSE;

#define VERIFICA_GENERACIO_PARAVER {if (!generar_paraver) return;}

/*
 * Private functions
 */
void NewState(int cpu, int ptask, int task, int thread,
              dimemas_timer init,
              dimemas_timer end,
              int           state);

void NewEvent(int cpu, int ptask, int task, int thread,
              prv_time_t        time,
              unsigned long long type,
              unsigned long long value);

void NewCommunication(int cpu_s, int ptask_s, int task_s, int thread_s,
                      prv_time_t log_s,
                      prv_time_t phy_s,
                      int cpu_r, int ptask_r, int task_r, int thread_r,
                      prv_time_t log_r,
                      prv_time_t phy_r,
                      long int   size,
                      long int   tag);

void GenerateParaverHeader(FILE* ParaverTraceFile);

bool CopyParaverRow(const char* output_trace_name);


/*
 * Class to collapse the Paraver events generated at the same timestamp
 */

class _EventCollapser
{
  private:
    int                                                   CPU;
    int                                                   Ptask;
    int                                                   Task;
    int                                                   Thread;
    prv_time_t                                            Timestamp;
    vector<pair<unsigned long long, unsigned long long> > Events;

  public:

    _EventCollapser(void)
    {
      CPU       = -1;
      Ptask     = -1;
      Task      = -1,
      Thread    = -1;
      Timestamp = 0;
      Events.clear();
    }

    int        cpu(void)       { return CPU;       }
    int        ptask(void)     { return Ptask;     }
    int        task(void)      { return Task;      }
    int        thread(void)    { return Thread;    }
    prv_time_t timestamp(void) { return Timestamp; }

    bool isLastEvent(int        CPU,
                     int        Ptask,
                     int        Task,
                     int        Thread,
                     prv_time_t Timestamp)
    {
      /* DEBUG
      cout << "CPU stored = " << this->CPU << " CPU query = " << CPU << endl;
      cout << "Ptask stored = " << this->Ptask << " Ptask query = " << Ptask << endl;
      cout << "Task stored = " << this->Task << " Task query = " << Task << endl;
      cout << "Thread stored = " << this->Thread << " Thread query = " << Thread << endl;
      cout << "Timestap stored = " << this->Timestamp << " Timestamp query = " << Timestamp << endl;
      */

      if (this->CPU       == CPU    &&
          this->Ptask     == Ptask  &&
          this->Task      == Task   &&
          this->Thread    == Thread &&
          this->Timestamp == Timestamp)
      {
        // cout << "Comparison TRUE!" << endl;
        return true;
      }

      // cout << "Comparison FALSE!" << endl;
      return false;
    }

    bool voidEvent(void) { return (Events.size() == 0); }

    void addEvent(unsigned long long type, unsigned long long value)
    {
      Events.push_back(make_pair(type, value));
    }

    string toString(void)
    {
      ostringstream Result;

      for (size_t i = 0; i < Events.size(); i++)
      {
        Result << Events[i].first << ":" << Events[i].second;

        if (i+1 != Events.size())
        {
          Result << ":";
        }
      }

      return Result.str();
    }

    void reset(int        CPU,
               int        Ptask,
               int        Task,
               int        Thread,
               prv_time_t Timestamp)
    {
      this->CPU       = CPU;
      this->Ptask     = Ptask;
      this->Task      = Task;
      this->Thread    = Thread;
      this->Timestamp = Timestamp;
      this->Events.clear();
    }
};

static _EventCollapser EventCollapser;
static ExternalSort TraceMerger;
static ostringstream to_ascii;

/**
 * Initialization of Paraver trace generation structures
 *
 */
void PARAVER_Init(const char   *output_trace,
                  const char   *pcf_insert,
                  dimemas_timer start_time,
                  dimemas_timer end_time,
                  t_boolean     priorities)
{
  if (output_trace != NULL)
  {
    generar_paraver = TRUE;

    paraver_trace_filename = string(output_trace);
  }

  VERIFICA_GENERACIO_PARAVER;

  if (pcf_insert != NULL)
  {
    pcf_insert_filename = string(pcf_insert);
  }

  if (start_time != -DBL_MAX && start_time > 0)
  {
    paraver_initial_timer = TRUE;
    start_paraver         = start_time;
  }
  else
  {
    start_paraver = 0;
  }

  if (end_time != -DBL_MAX && end_time > 0)
  {
    paraver_final_timer = TRUE;
    stop_paraver        = end_time;
  }

  if (paraver_initial_timer && paraver_final_timer)
  {

    if LE_TIMER (stop_paraver, start_paraver)
    {
      die("Final paraver time must be greater than initial\n");
      // fprintf (stderr, USAGE, argv[0]);
      // exit (EXIT_FAILURE);
    }
  }
  paraver_priorities = priorities;

  /* Initialize the trace manager */
  TraceMerger.Init();

}

void PARAVER_End(void)
{
  bool end;

  char* pcf_insert_c_str;

  FILE *ParaverTraceFile;

  VERIFICA_GENERACIO_PARAVER;

  /* Open file for final trace */
  if ( (ParaverTraceFile = IO_fopen(paraver_trace_filename.c_str(), "w")) == NULL )
  {
    // TODO: TraceMerger.CleanTemporalFiles();
    die("Unable to open output paraver trace %s: %s\n",
        paraver_trace_filename.c_str(),
        IO_get_error());
  }

  /* Flush last collapsed events */
  if (!EventCollapser.voidEvent())
  {
    /* Flush previous event */
    to_ascii.str("");
    to_ascii << EventCollapser.toString();

    TraceMerger.NewRecord(PRV_EVENT,
                          EventCollapser.cpu(),
                          EventCollapser.ptask(),
                          EventCollapser.task(),
                          EventCollapser.thread(),
                          EventCollapser.timestamp(),
                          to_ascii.str());
  }

  /* Generate trace header */
  GenerateParaverHeader(ParaverTraceFile);

  /* Proceed to merge the trace */
  TraceMerger.GenerateFinalTrace(ParaverTraceFile);

  printf("\n");
  printf("Output Paraver trace \"%s\" generated\n",
         paraver_trace_filename.c_str());
  printf("\n");


  IO_fclose(ParaverTraceFile);

  /* Generate the output trace PCF */
  /* BAD TRICK! */
  if (pcf_insert_filename.compare("") == 0)
  {
    pcf_insert_c_str = NULL;
  }
  else
  {
    pcf_insert_c_str = (char*) pcf_insert_filename.c_str();
  }

  MakeParaverPCFandROW(paraver_trace_filename.c_str(), pcf_insert_c_str);
}

void PARAVER_Start_Op (int cpu, int ptask, int task, int thread,
                       dimemas_timer time)
{
  prv_time_t time_prv;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_final_timer)
  {
    if (GT_TIMER (time, stop_paraver))
    {
      return;
    }
  }

  if (paraver_initial_timer)
  {
    if (GT_TIMER (start_paraver, time))
    {
      return;
    }

    SUB_TIMER (time, start_paraver, time);
  }

  TIMER_TO_PRV_TIME_T (time, time_prv);

  /* Check event collapser to avoid the case of 'zero duration' communications
   * that produce a 'large' record with start and end (0) events */

  if (EventCollapser.isLastEvent(cpu, ptask, task, thread, time_prv))
  {
    if (!EventCollapser.voidEvent()) // To guarantee first usage
    {
      /* Flush previous event */
      to_ascii.str("");
      to_ascii << EventCollapser.toString();

      TraceMerger.NewRecord(PRV_EVENT,
                            EventCollapser.cpu(),
                            EventCollapser.ptask(),
                            EventCollapser.task(),
                            EventCollapser.thread(),
                            EventCollapser.timestamp(),
                            to_ascii.str());
    }

    /* Reset the event collapser */
    EventCollapser.reset(cpu, ptask, task, thread, time_prv);
  }
}

void PARAVER_Idle (int cpu, int ptask, int task, int thread,
                   dimemas_timer init_time,
                   dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Idle State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_IDLE_ST);
}

void PARAVER_Running (int cpu, int ptask, int task, int thread,
                      dimemas_timer init_time,
                      dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Running State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_RUNNING_ST);
}

void PARAVER_Startup (int cpu, int ptask, int task, int thread,
                      dimemas_timer init_time,
                      dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Startup Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_STARTUP_ST);
}

void PARAVER_Wait (int cpu, int ptask, int task, int thread,
                   dimemas_timer init_time,
                   dimemas_timer end_time,
                   int type)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Wait State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           type);
}

void PARAVER_Busy_Wait (int cpu, int ptask, int task, int thread,
                        dimemas_timer init_time,
                        dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Busy Wait Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task, thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_BLOCKED_ST);
}

void PARAVER_Waiting_Links (int cpu, int ptask, int task, int thread,
                            dimemas_timer init_time,
                            dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Wait Links Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task, thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_WAIT_LINKS_ST);
}

/* TODO: This operation prints TEST/PROBE state... */
void PARAVER_Ctx_Switch (int cpu, int ptask, int task, int thread,
                         dimemas_timer init_time,
                         dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Ctx, Switch (Test/Probe) Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task, thread);
  }

  NewState(cpu, ptask, task, thread,
          init_time,
          end_time,
          PRV_TEST_PROBE_ST);
}

void PARAVER_Data_Copy (int cpu, int ptask, int task, int thread,
                        dimemas_timer init_time,
                        dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Data Copy Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_DATA_COPY_ST);
}

void PARAVER_RTT (int cpu, int ptask, int task, int thread,
                  dimemas_timer init_time,
                  dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Startup Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }


  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_RTT_ST);
}

void PARAVER_IO_Op (int cpu, int ptask, int task, int thread,
                    dimemas_timer init_time,
                    dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver IO State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_IO_ST);
}

void PARAVER_IO_Blocked (int cpu, int ptask, int task, int thread,
                         dimemas_timer init_time,
                         dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver IO Block Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_IO_ST);
}

void PARAVER_OS_Blocked (int cpu, int ptask, int task, int thread,
                         dimemas_timer init_time,
                         dimemas_timer end_time)
{
  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER (init_time, end_time)
    return;

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver OS Block Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           init_time,
           end_time,
           PRV_BLOCKED_ST);
}

void PARAVER_Dead (int cpu, int ptask, int task, int thread,
                   dimemas_timer time)
{
  dimemas_timer end_time;

  VERIFICA_GENERACIO_PARAVER;

  ADD_MIN_INCR_TO_TIMER(time, end_time);

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver thread dead Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
            cpu, ptask, task,thread);
  }

  NewState(cpu, ptask, task, thread,
           time,
           end_time,
           PRV_NOT_CREATED_ST);
}

void PARAVER_P2P_Comm (int cpu_s,  int ptask_s, int task_s, int thread_s,
                       dimemas_timer log_s, dimemas_timer phy_s,
                       int cpu_r, int ptask_r, int task_r, int thread_r,
                       dimemas_timer log_r, dimemas_timer phy_r,
                       int size, int tag)
{
  prv_time_t log_s_prv, phy_s_prv, log_r_prv, phy_r_prv;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_final_timer)
  {
    if (GT_TIMER (log_s, stop_paraver))
    {
      return;
    }

    if (GT_TIMER (log_r, stop_paraver))
    {
      ASS_ALL_TIMER (log_r, stop_paraver);
    }
  }

  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, log_s)) && (GT_TIMER (start_paraver, log_r)))
    {
      return;
    }

    if (GT_TIMER (start_paraver, log_s))
    {
      ASS_ALL_TIMER (log_s, start_paraver);
    }

    if (GT_TIMER (start_paraver, log_r))
    {
      ASS_ALL_TIMER (log_r, start_paraver);
    }

    SUB_TIMER (log_s, start_paraver, log_s);
    SUB_TIMER (log_r, start_paraver, log_r);
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (phy_s, stop_paraver))
    {
      return;
    }

    if (GT_TIMER (phy_r, stop_paraver))
    {
      ASS_ALL_TIMER (phy_r, stop_paraver);
    }
  }

  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, phy_s)) && (GT_TIMER (start_paraver, phy_r)))
    {
      return;
    }

    if (GT_TIMER (start_paraver, phy_s))
    {
      ASS_ALL_TIMER (phy_s, start_paraver);
    }

    if (GT_TIMER (start_paraver, phy_r))
    {
      ASS_ALL_TIMER (phy_r, start_paraver);
    }

    SUB_TIMER (phy_s, start_paraver, phy_s);
    SUB_TIMER (phy_r, start_paraver, phy_r);
  }

  TIMER_TO_PRV_TIME_T (log_s, log_s_prv);
  TIMER_TO_PRV_TIME_T (phy_s, phy_s_prv);
  TIMER_TO_PRV_TIME_T (log_r, log_r_prv);
  TIMER_TO_PRV_TIME_T (phy_r, phy_r_prv);


  /* JGG: DEBUG DE LA GENERACIÓN DE LAS COMUNICACIONES PARAVER */
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf( ": Paraver Communication Printed\n\tSENDER   CPU %d P%02d T%02d (t%02d)\n",
            cpu_s, ptask_s, task_s,thread_s);

    printf( "\tLgcSend: %llu PhySend: %llu\n",
           log_s_prv,
           phy_s_prv);

    printf( "\tRECEIVER CPU %d P%02d T%02d (t%02d)\n",
           cpu_r, ptask_r, task_r, thread_r);

    printf("\tLgcRecv: %llu PhyRecv: %llu\n",
           log_r_prv,
           phy_r_prv);
  }

  NewCommunication(cpu_s, ptask_s, task_s,thread_s,
                   log_s_prv, phy_s_prv,
                   cpu_r, ptask_r, task_r, thread_r,
                   log_r_prv, phy_r_prv,
                   size, tag);
}

void PARAVER_Event (int cpu, int ptask, int task, int thread,
                    dimemas_timer time,
                    unsigned long long type,
                    unsigned long long value)
{
  prv_time_t time_prv;
  unsigned long long   tipus_final, valor_final;

  VERIFICA_GENERACIO_PARAVER;

  /* Special case to avoid producing Dimemas internal IDLE events */
  if (type == IDLE_EVENT_TYPE    ||
      type == PRIORITY_SET_EVENT ||
      type == PREEMPTION_SET_EVENT)
  {
    return;
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (time, stop_paraver))
    {
      return;
    }
  }

  if (paraver_initial_timer)
  {
    if (GT_TIMER (start_paraver, time))
    {
      return;
    }

    SUB_TIMER (time, start_paraver, time);
  }

  TIMER_TO_PRV_TIME_T (time, time_prv);

  /* fprintf (paraver, PA_EVENT_STRING, cpu, ptask, task, thread,
           temps_final, tipus_final, valor_final); */

  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (": Paraver Event Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
           cpu, ptask, task, thread);

    printf("\tTime: %llu Final Type: %llu Final Value: %llu\n",
           time_prv,
           type,
           value);
  }

  NewEvent(cpu, ptask, task, thread,
           time_prv,
           type,
           value);
}


/*****************************************************************************
 * Private functions implementation
 ****************************************************************************/
void NewState(int cpu, int ptask, int task, int thread,
              dimemas_timer init_time,
              dimemas_timer end_time,
              int           state)
{
  prv_time_t init_prv, end_prv;

  int        StateType; // Just to distinguish running or not running

  if (paraver_final_timer)
  {
    if (GT_TIMER (init_time, stop_paraver))
    {
      return;
    }

    if (GT_TIMER (end_time, stop_paraver))
    {
      ASS_ALL_TIMER (end_time, stop_paraver);
    }
  }

  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, init_time)) && (GT_TIMER (start_paraver, end_time)))
    {
      return;
    }
    if (GT_TIMER (start_paraver, init_time))
    {
      ASS_ALL_TIMER (init_time, start_paraver);
    }

    SUB_TIMER (init_time, start_paraver, init_time);
    SUB_TIMER (end_time,  start_paraver, end_time);
  }

  TIMER_TO_PRV_TIME_T (init_time, init_prv);
  TIMER_TO_PRV_TIME_T (end_time,  end_prv);

  if (debug&D_PRV)
  {
    printf("\tInitial Time: %llu End Time: %llu\n",
          init_prv,
          end_prv);
  }

  to_ascii.str("");;
  // to_ascii << PRV_STATE << ":"; // STATE!
  // to_ascii << cpu << ":" << ptask+1 << ":" << task+1 << ":" << thread+1 << ":"; // Object
  to_ascii << end_prv << ":";                // Interval
  to_ascii << state;

  if (state == PRV_RUNNING_ST)
  {
    StateType = PRV_STATE_RUN;
  }
  else
  {
    StateType = PRV_STATE;
  }

  TraceMerger.NewRecord(StateType,
                        cpu, ptask, task, thread,
                        init_prv,
                        to_ascii.str());
}

void NewEvent(int        cpu, int ptask, int task, int thread,
              prv_time_t time,
              unsigned long long type, unsigned long long value)
{
  if (!EventCollapser.isLastEvent(cpu, ptask, task, thread, time))
  {
    if (!EventCollapser.voidEvent()) // To guarantee first usage
    {
      /* Flush previous event */
      to_ascii.str("");
      to_ascii << EventCollapser.toString();

      TraceMerger.NewRecord(PRV_EVENT,
                            EventCollapser.cpu(),
                            EventCollapser.ptask(),
                            EventCollapser.task(),
                            EventCollapser.thread(),
                            EventCollapser.timestamp(),
                            to_ascii.str());
    }

    /* Reset the event collapser */
    EventCollapser.reset(cpu, ptask, task, thread, time);
  }

  EventCollapser.addEvent(type, value);
}

void NewCommunication(int cpu_s, int ptask_s, int task_s, int thread_s,
                      prv_time_t log_s, prv_time_t phy_s,
                      int cpu_r, int ptask_r, int task_r, int thread_r,
                      prv_time_t log_r, prv_time_t phy_r,
                      long int size, long int tag)
{
  prv_time_t    timestamp = log_s;

  to_ascii.str("");
  to_ascii << phy_s << ":"; // logical send (log_s) is the timestamp of the record!
  to_ascii << cpu_r << ":" << ptask_r+1 << ":" << task_r+1 << ":" << thread_r+1 << ":"; // Sender object
  to_ascii << log_r << ":" << phy_r << ":";
  to_ascii << size << ":" << tag;

  TraceMerger.NewRecord(PRV_COMM,
                        cpu_s, ptask_s, task_s, thread_s,
                        timestamp,
                        to_ascii.str());

}

void GenerateParaverHeader(FILE* ParaverTraceFile)
{
  time_t     h;
  char       Date[80];
  struct tm *tm_str;

  dimemas_timer final_paraver_timer;
  prv_time_t    final_time;

  struct t_node*         node;
  struct t_Ptask*        ptask;
  struct t_task*         task;
  struct t_communicator* communicator;

  unsigned int total_communicators = 0;

  ostringstream Header;

  /* Cal calcular el temps final */
  if ((paraver_initial_timer) || (paraver_final_timer))
  {
    if (paraver_final_timer)
    {
      final_paraver_timer = stop_paraver;
    }
    else
    {
      final_paraver_timer = execution_end_time; // defined in 'read.c' ??
    }
    SUB_TIMER (final_paraver_timer, start_paraver, final_paraver_timer);
  }
  else
  {
    final_paraver_timer = execution_end_time;
  }

  //TIMER_TO_DOUBLE (final_paraver_timer, final_time);
  TIMER_TO_PRV_TIME_T(final_paraver_timer, final_time);

  time( &h );
  tm_str = localtime( &h );
  strftime( Date, 80, "(%d/%m/%y at %H:%M)", tm_str );

  Header << "#Paraver " << Date << ":" << (prv_time_t) final_time << "_ns" << ":";

  /*
   * Iterate through nodes to print the number of CPUs
   */
#ifdef USE_EQUEUE
  Header << (unsigned int) count_Equeue(&Node_queue) << "("; // Number of nodes
#else
  Header << (unsigned int) count_queue(&Node_queue) << "("; // Number of nodes
#endif

#ifdef USE_EQUEUE
  node = (struct t_node*) head_Equeue(&Node_queue);
#else
  node = (struct t_node*) head_queue (&Node_queue);
#endif
  while (node != NULL)
  {
    Header << (unsigned int) count_queue(&(node->Cpus)); // Number of CPUs

#ifdef USE_EQUEUE
    node  = (struct t_node *) next_Equeue (&Node_queue);
#else
    node  = (struct t_node *) next_queue (&Node_queue);
#endif

    if (node != NULL)
    {
      Header << ",";
    }
  }
  Header << "):";

  /*
   * Iterate through applications (ptasks) to print the tasks/threads
   */
  Header << (unsigned int) count_queue(&Ptask_queue) << ":"; // Number of applications

  for(ptask  = (struct t_Ptask*) head_queue (&(Ptask_queue));
      ptask != P_NIL;
      ptask  = (struct t_Ptask*) next_queue (&(Ptask_queue)))
  {
    Header << ptask->tasks_count << "("; // Number of tasks

    for (size_t i = 0; i < ptask->tasks_count; i++)
    {
      task = &(ptask->tasks[i]);
      Header << (unsigned int) task->threads_count << ":"; // Number of threads
      Header << task->nodeid+1;                            // Node

      if (i+1 < ptask->tasks_count)
      {
        Header << ",";
      }
    }

    Header << "),";

    total_communicators += (unsigned int) count_queue(&(ptask->Communicator));
  }

  Header << total_communicators << std::endl;

  /*
   * Iterate through applications (ptasks) to print the tasks/threads
   */
  for(ptask  = (struct t_Ptask*) head_queue (&(Ptask_queue));
      ptask != P_NIL;
      ptask  = (struct t_Ptask*) next_queue (&(Ptask_queue)))
  {
    for(communicator  = (struct t_communicator*) head_queue(&ptask->Communicator);
        communicator != NULL;
        communicator  = (struct t_communicator*) next_queue(&ptask->Communicator))
    {
      int i;

      Header << "c:" << ptask->Ptaskid+1 << ":";                       // Ptask
      Header << (unsigned int) communicator->communicator_id << ":";   // Communicator ID, +1 not required!
      Header << (unsigned int) communicator->size;

      for (i = 0; i < communicator->size; i++)
      {
        Header << ":" << communicator->global_ranks[i]+1; // Tasks ID (Note the ":" before each ID)
      }

      /*
      Header << (unsigned int) count_queue(&communicator->global_ranks); // Total tasks

      for(global_ranks  = (int*) head_queue(&communicator->global_ranks);
          global_ranks != NULL;
          global_ranks  = (int*) next_queue(&communicator->global_ranks))
      {
        Header << ":" << (*global_ranks)+1; // Tasks ID (Note the ":" before each ID)
      }
      */
      Header << std::endl;
    }
  }

  fprintf(ParaverTraceFile, "%s", Header.str().c_str());
}

bool CopyParaverRow(const char* output_trace_name)
{
  struct t_Ptask* ptask;

  FILE *input_row_file, *output_row_file;
  char *input_row_name, *output_row_name;

  char*     line  = NULL;
  size_t    current_line_length = 0;
  ssize_t   bytes_read;

  /* The ROW would be a copy of the first trace ROW, if exists! */
  ptask  = (struct t_Ptask *) head_queue(&Ptask_queue);

  input_row_name = strdup(ptask->tracefile);

  if (input_row_name == NULL)
  {
    return true;
  }
  else
  {
    strncpy(&input_row_name[strlen(input_row_name)-3],"row", 3);

    /*
    printf("PCF to be copied = %s\n",
           input_pcf_name);
    */

    if ( ( input_row_file = IO_fopen(input_row_name, "r")) == NULL)
    {
      warning ("Unable to open input ROW file (%s): %s\n",
               input_row_name,
               IO_get_error());

      return false;
    }
  }

  /* Generate the output PCF name */
  output_row_name = strdup(output_trace_name);

  if (strcmp(&output_row_name[strlen(output_trace_name)-4],".prv") != 0)
  {
    warning ("Wrong extension in output Paraver trace. ROW file would be named wrong\n");
  }

  strcpy(&output_row_name[strlen(output_trace_name)-4],".row");

  if ( (output_row_file = IO_fopen(output_row_name, "w")) == NULL)
  {
    warning ("Unable to open output ROW file (%s): %s\n",
             output_row_name,
             IO_get_error());

    return false;
  }

  while ( (bytes_read = getline(&line, &current_line_length, input_row_file)) != -1)
  {
    fprintf(output_row_file, "%s", line);
    free(line);
    line = NULL;
  }

  if (!feof(input_row_file))
  {
    die ("Error reading input ROW: %s\n",
         strerror(errno));
  }

  fclose(input_row_file);
  fclose(output_row_file);

  return true;
}
