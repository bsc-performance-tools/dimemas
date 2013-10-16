/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  trf2trf                                  *
 *       Dimemas TRF to Dimemas DIM trace translator (old to new format)     *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1   *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This tool is distributed in hope that it will be         *
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

  $URL:: https://svn.bsc.es/repos/ptools/trf2dim/trunk/src/DimemasSDDFTrac#$:

  $Rev:: 483                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-29 11:54:45 +0200#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "DimemasSDDFTraceParser.hpp"

#include <Dimemas2Prv.h>

#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
using std::cout;
using std::endl;

// This must be changed
extern bool VerboseMode;

/******************************************************************************
 * Public functions
 ******************************************************************************/

DimemasSDDFTraceParser::DimemasSDDFTraceParser(string TraceName,
                                               FILE*  TraceFile)
{
  char Buffer[15];

  this->TraceName = TraceName;

  if (TraceFile != NULL)
  {
    this->TraceFile = TraceFile;
  }
  else if ( (this->TraceFile = fopen(TraceName.c_str(), "r")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to open Dimemas trace", strerror(errno));
    return;
  }

  fscanf (this->TraceFile, "%5s", Buffer);

  if (strcmp ("SDDFA", Buffer) != 0)
  {
    SetError(true);
    LastError = "wrong format (not SDDF)";
    return;
  }

  if (!GetTaskCount())
    return;

  if (VerboseMode)
    cout << "-> " << TraceName << " has " << TaskCount << " tasks" << endl;

  /* Default ApplicationId (PtaskId) is 0 */
  this->ApplicationStructure = new ApplicationDescription(0);

  if (VerboseMode)
  {
    fprintf(stdout, "\r-> Creating control structure for task 0/%d", TaskCount);
    fflush(stdout);
  }

  for (int i = 0; i < TaskCount; i++)
  {
    TaskDescription_t NewTaskDescription = new TaskDescription(i, 1);
    ApplicationStructure->AddTaskDescription(NewTaskDescription);

    TaskFile.push_back(NULL);
    OriginalOffset.push_back((off_t) 0);
    CurrentOffset.push_back((off_t) 0);
    SharingDescriptors.push_back(false);

    if (VerboseMode)
    {
      fprintf(stdout,
              "\r-> Creating control structure for task %d/%d",
              i+1,
              TaskCount);
      fflush(stdout);
    }
  }

  if (VerboseMode)
      fprintf(stdout, "\n");

  return;
}


bool
DimemasSDDFTraceParser::InitTraceParsing(void)
{
  struct stat FileStat;
  INT32 HeaderLength;
  vector<ApplicationDescription_t> AppsDescription;

  if (ParsingInitialized)
  {
    Reload();
    return true;
  }

  if (fstat(fileno(TraceFile), &FileStat) < 0)
  {
    SetErrorMessage("unable to read SDDF trace statistics",
                    strerror(errno));
    return false;
  }
  this->TraceSize = FileStat.st_size;

  /* PROGRESS MESSAGES */
  if (VerboseMode)
  {
    fprintf(stdout, "\r-> Locating initial offset for task 0/%d", TaskCount);
    fflush(stdout);
  }
  for (INT32 CurrentTaskId = 0; CurrentTaskId < TaskCount; CurrentTaskId++)
  {
    /* PROGRESS MESSAGES */
    if (VerboseMode)
    {
      fprintf(stdout,
              "\r-> Locating initial offset for task %d/%d",
              CurrentTaskId+1,
              TaskCount);
      fflush(stdout);
    }

    if (!FastLocateInitialOffset(CurrentTaskId))
      return false;
  }

  if (VerboseMode)
    fprintf(stdout, "\n");

  ParsingInitialized = true;

  return true;
}

ApplicationDescription_t
DimemasSDDFTraceParser::GetApplicationsDescription(void)
{
  if (!ParsingInitialized)
  {
    SetError(true);
    LastError = "Parsing not initialized";
    return NULL;
  }

  return ApplicationStructure;
}

DimemasRecord_t
DimemasSDDFTraceParser::GetNextRecord(INT32 TaskId, INT32 ThreadId)
{
  DimemasRecord_t Record = NULL;
  FILE           *EffectiveFile;
  char           *Buffer;
  INT32           CurrentLineLength;
  INT32           Matches;

  INT32           ReadTaskId, ReadThreadId, DstTaskId, SrcTaskId, Size, Tag;
  INT32           CommId, Synchronism, RecvType;
  INT64           BlockId;
  double          BurstDuration;

  INT32           GlobalOpId, RootTaskId, RootThreadId;
  INT64           SendSize, RecvSize;

  INT32           EvtType;
  INT64           EvtValue;

  /* Set correct offset */
  if (SharingDescriptors[TaskId])
  {
    if (fseeko (TraceFile,
                CurrentOffset[TaskId],
                SEEK_SET) == -1)
    {
      char CurrentError[128];

      sprintf(CurrentError,
              "error reading records for task %02d",
              TaskId);

      SetErrorMessage(CurrentError,
                      strerror(errno));
      return false;
    }

    EffectiveFile = TraceFile;
  }
  else
  {
    EffectiveFile = TaskFile[TaskId];
  }

  if ( (CurrentLineLength = GetLongLine(EffectiveFile, &Buffer)) < 0)
  {
    return NULL;
  }
  else if (CurrentLineLength == 0)
  {
    return NULL;
  }
  else
  {
    CurrentOffset[TaskId] += CurrentLineLength;
  }

  Matches = sscanf(Buffer,
                   "\"CPU burst\" { %d, %d, %le };;\n",
                   &ReadTaskId, &ReadThreadId, &BurstDuration);
  if (Matches == 3)
  {
    if (ReadTaskId > TaskId)
      return NULL;

    Record = new CPUBurst(ReadTaskId, ReadThreadId, BurstDuration);
  }

  /* NX send */
  Matches = sscanf(Buffer,
                   "\"NX send\" { %d, %d, %d, %d, %d, %d, %d };;\n",
                   &ReadTaskId, &ReadThreadId,
                   &DstTaskId, &Size, &Tag, &CommId, &Synchronism);
  if (Matches == 7)
  {
    if (ReadTaskId > TaskId)
      return NULL;

    Record = new Send(ReadTaskId, ReadThreadId,
                      DstTaskId, Size, Tag, CommId, Synchronism);
  }

  /* NX recv */
  Matches = sscanf(Buffer,
                   "\"NX recv\" { %d, %d, %d, %d, %d, %d, %d};;\n",
                   &ReadTaskId, &ReadThreadId,
                   &SrcTaskId, &Size, &Tag, &CommId, &RecvType );

  if (Matches == 7)
  {
    if (ReadTaskId < TaskId)
      return NULL;

    Record = new Receive(ReadTaskId, ReadThreadId,
                         SrcTaskId, Size, Tag, CommId, RecvType);
  }

  /* block begin */
  Matches = sscanf(Buffer,
                   "\"block begin\" { %d, %d, %lld};;\n",
                   &ReadTaskId, &ReadThreadId, &BlockId );

  if (Matches == 3)
  {
    INT64 ParaverType, ParaverValue;


    if (ReadTaskId < TaskId)
      return NULL;

    Block_Dimemas2Paraver_TypeAndValue ((long long)   BlockId,
                                        (long long*) &ParaverType,
                                        (long long*) &ParaverValue);

    PCFGeneration.SetUsedBlockUsed(ParaverType, ParaverValue);

    Record = new Event(ReadTaskId, ReadThreadId, BlockId, true);

  }

  /* block end */
  Matches = sscanf(Buffer,
                   "\"block end\" { %d, %d, %lld};;\n",
                   &ReadTaskId, &ReadThreadId, &BlockId );

  if (Matches == 3)
  {
    if (ReadTaskId < TaskId)
      return NULL;

    Record = new Event(ReadTaskId, ReadThreadId, BlockId, false);
  }

  /* user event */
  Matches = sscanf(Buffer,
                   "\"user event\" { %d, %d, %d, %lld};;\n",
                   &ReadTaskId, &ReadThreadId, &EvtType, &EvtValue);
  if (Matches == 4)
  {
    if (ReadTaskId < TaskId)
      return NULL;

      Event_t EventRecord = new Event(ReadTaskId, ReadThreadId);
      EventRecord->AddTypeValue(EvtType, EvtValue);
      Record = EventRecord;
    // dynamic_cast<Event_t>(Record)->AddTypeValue(EvtType, EvtValue);
  }

  /* global OP */
  Matches = sscanf(Buffer,
                  "\"global OP\" { %d, %d, %d, %d, %d, %d, %lld, %lld};;\n",
                  &ReadTaskId, &ReadThreadId,
                  &GlobalOpId,
                  &CommId, &RootTaskId, &RootThreadId,
                  &SendSize, &RecvSize );
  if (Matches == 8)
  {
    if (ReadTaskId < TaskId)
      return NULL;

    Record = new GlobalOp(ReadTaskId, ReadThreadId,
                          GlobalOpId,
                          CommId, RootTaskId, RootThreadId,
                          SendSize, RecvSize);
  }

  /* Free input buffer */
  free(Buffer);

  if (Record == NULL)
  {
    SetError(true);
    SetErrorMessage(Buffer," Unknown record");
    return NULL;
  }
  else
    return Record;
}

bool
DimemasSDDFTraceParser::Reload(void)
{
  if (!ParsingInitialized)
  {
    LastError = "parsing not initialized";
    return false;
  }

  for (INT32 CurrentTaskId = 0; CurrentTaskId < TaskCount; CurrentTaskId++)
  {
    CurrentOffset[CurrentTaskId] = OriginalOffset[CurrentTaskId];

    if (!SharingDescriptors[CurrentTaskId])
    {
      if (fseeko (TaskFile[CurrentTaskId],
                  CurrentOffset[CurrentTaskId],
                  SEEK_SET) == -1)
      {
        char CurrentError[128];
        sprintf(CurrentError, "error reloading task %02d", CurrentTaskId);
        SetErrorMessage(CurrentError,
                        strerror(errno));
        return false;
      }
    }
  }

  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool
DimemasSDDFTraceParser::GetTaskCount()
{
  INT32 LastTaskId;
  INT32 Matches, TaskId, ThreadId;
  char  Buffer[PARSER_BUFFER];

  if (fseek(TraceFile, -(2*PARSER_BUFFER), SEEK_END) == -1)
  {
    if (fseek(TraceFile, -(PARSER_BUFFER), SEEK_END) == -1)
    {
      SetError(true);
      SetErrorMessage("unable to seek file for task count",
                    strerror(errno));
      return false;
    }
  }

  while (fgets(Buffer, PARSER_BUFFER-1, TraceFile) != NULL)
  {
    Matches = sscanf (Buffer, "\"CPU burst\" { %d, %d,", &TaskId, &ThreadId);

    if (Matches == 2)
      LastTaskId = TaskId;
  }

  if (ferror(TraceFile))
  {
    SetError(true);
    SetErrorMessage("unable to compute task count",
                    strerror(errno));
    return false;
  }

  if (fseek(TraceFile, 0, SEEK_SET) == -1)
  {
    SetError(true);
    SetErrorMessage("unable to seek SDDF file for task count",
                    strerror(errno));
    return false;
  }

  TaskCount = LastTaskId+1;
  return true;
}


/* This function locates first CPU burst for the desired task. If is the first
   task it also load the communicators */
bool
DimemasSDDFTraceParser::LocateFirstCPUBurst(INT32 TaskId)
{
  char *Buffer = NULL, BufferCommunicator[PARSER_BUFFER], *BufferTaskId = NULL;
  INT32 ReadTaskId, ReadThreadId;
  INT32 CommunicatorId, CommunicatorSize;
  INT32 BlockId;
  INT32 CurrentLineLength, Matches;

  char *DefinitionName;

  while(true)
  {
    if ( (CurrentLineLength = GetLongLine(TraceFile, &Buffer)) < 0)
    {
      return false;
    }

    Matches = sscanf (Buffer,
                      "\"CPU burst\" { %d, %d,",
                      &ReadTaskId,
                      &ReadThreadId);

    if (Matches == 2)
    {
      if (ReadTaskId == TaskId)
      {
        off_t CurrentTaskOffset;

        if ( (CurrentTaskOffset = ftello(TraceFile)) == -1)
        {
          char CurrentError[128];

          sprintf(CurrentError,
                  "unable to get file position while looking for task %02d location",
                  TaskId);

          SetError(true);
          SetErrorMessage(CurrentError,
                          strerror(errno));
          return false;
        }

        OriginalOffset[TaskId] = CurrentTaskOffset - CurrentLineLength;
        CurrentOffset[TaskId]  = CurrentTaskOffset - CurrentLineLength;

        if ( (TaskFile[TaskId] = fopen(TraceName.c_str(), "r")) == NULL)
        {
          SharingDescriptors[TaskId] = true;
        }
        else
        {
          if ( fseeko(TaskFile[TaskId], CurrentOffset[TaskId], SEEK_CUR) == -1)
          {
            char CurrentError[128];

            sprintf(CurrentError,
                    "unable to set file position while looking for task %02d location",
                    TaskId);

            SetError(true);
            SetErrorMessage(CurrentError,
                            strerror(errno));
            return false;
          }
        }
        break;
      }
      else if (ReadTaskId > TaskId)
      {
        char CurrentError[128];
        SetError(true);

        sprintf(CurrentError,
                "trace not correctly sorted, found task %02d before task %02d",
                ReadTaskId,
                TaskId);

        this->LastError = CurrentError;
        return false;
      }
    }

    Matches = sscanf (Buffer,
                      "\"communicator definition\" { %d, %d,",
                      &CommunicatorId,
                      &CommunicatorSize);
    if (Matches == 2)
    {
      Communicator_t NewCommunicator = new Communicator(Buffer);
      ApplicationStructure->AddCommunicator(NewCommunicator);
    }

    /* Here, we should also deal the definitions to generate the PCF */
    DefinitionName = (char*) malloc (CurrentLineLength*sizeof(char));

    Matches = sscanf (Buffer,
                      "\"block definition\" { %d, \"%[^\"]\",",
                      &BlockId,
                      DefinitionName);

    if (Matches == 2)
    {
      INT64 ParaverType, ParaverValue;

      Block_Dimemas2Paraver_TypeAndValue ((long long)   BlockId,
                                          (long long*) &ParaverType,
                                          (long long*) &ParaverValue);

      PCFGeneration.AddBlockDefinition(ParaverType,
                                       ParaverValue,
                                       string(DefinitionName));

      // BLOCK_TRF2PRV (BlockId, ParaverType, ParaverValue);

      /* DEBUG
      printf ("Block definition: %d -> %lld:%lld (%s)\n",
              BlockId,
              ParaverType,
              ParaverValue,
              DefinitionName);
      */

    }

    /* Free Buffer */
    free(Buffer);
  }

  /* Free Buffer */
  free(Buffer);
  return true;
}

bool
DimemasSDDFTraceParser::FastLocateInitialOffset(INT32 TaskId)
{
  char  Buffer[PARSER_BUFFER];
  INT32 Matches, ReadTaskId, ReadThreadId;
  off_t LowBound, UppBound, CurrPosition, InitialPosition;
#define STEP_BY_STEP (off_t)(2*PARSER_BUFFER)

/*
#define DEBUGA_CERCA_PRIMER_CPUBURST 1 */

#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  fprintf(stderr,"\tBuscant cpu burst de la task %d\n",TaskId);
#endif

  if (TaskId == 0)
  {
    return LocateFirstCPUBurst(TaskId);
  }

  InitialPosition = OriginalOffset[TaskId-1]; /* Initial position */
  LowBound        = InitialPosition; /* Aquesta sera la cota inferior */
  CurrPosition    = InitialPosition;
  UppBound        = TraceSize;

  if ( fseeko(TraceFile, InitialPosition, SEEK_SET) == -1)
  {
    char CurrentError[128];

    sprintf(CurrentError,
            "unable to set file position while looking for task %02d location",
            TaskId);

    SetError(true);
    SetErrorMessage(CurrentError,
                    strerror(errno));
    return false;
  }

#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  fprintf(stderr,"\t\tp_ini %llu, p_sup %llu\n",InitialPosition, UppBound);
#endif

  while (true)
  {
    /*
    fprintf(stderr,"Abans de llegir, la posicio es %llu\n",FTELL(file));
     */
    Matches = fscanf (TraceFile, "%[^\n]\n", Buffer);

    if (Matches == -1)
    {
      char CurrentError[128];
      sprintf(CurrentError,
              "can't locate actions for task %02d",
              TaskId);
      SetError(true);
      this->LastError = CurrentError;
      return false;
    }

    if (Matches == 0)
    {
      if ( fseeko(TraceFile, 1, SEEK_CUR) == -1)
      {
        char CurrentError[128];

        sprintf(CurrentError,
                "unable to set file position while looking for task %02d location",
                TaskId);

        SetError(true);
        SetErrorMessage(CurrentError,
                        strerror(errno));
        return false;
      }

      CurrPosition++;
      continue;
    }
    /*
     fprintf(stderr,"Despres de llegir, la posicio es %llu\n",FTELL(file));
     */
    Matches = sscanf (Buffer,
                      "\"CPU burst\" { %d, %d,",
                      &ReadTaskId,
                      &ReadThreadId);
    if (Matches == 2)
    {
      if (TaskId <= ReadTaskId)
      {
        /* New upper bound found */
        UppBound = CurrPosition;
        if ((UppBound - LowBound) <= STEP_BY_STEP)
          break;
      }
      else /* TaskId > ReadTask */
      {
        /* New lower bound */
        LowBound = CurrPosition;
        if ((UppBound - LowBound) <= STEP_BY_STEP)
          break;
      }

      CurrPosition = LowBound + (UppBound - LowBound)/2;
      if ( fseeko(TraceFile, CurrPosition, SEEK_SET) == -1)
      {
        char CurrentError[128];

        sprintf(CurrentError,
                "unable to set file position while looking for task %02d location",
                TaskId);

        SetError(true);
        SetErrorMessage(CurrentError,
                        strerror(errno));
        return false;
      }
#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
      fprintf(stderr,"\t\t\tRecord: %s\n",Buffer);
      fprintf(stderr,
              "\t\tINF: %llu, Sup: %llu Current: %llu\n",
              LowBound,
              UppBound,
              CurrPosition);
#endif
    }
    else
    {
      if (feof(TraceFile))
        break;
    }
  }

#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  /*
   fprintf(stderr,"\t\tp_inf %llu, p_sup %llu\n",p_inf, p_sup);
   fprintf(stderr,"\t\t\tUltim Record: %s\n",buf);
   */
  fprintf(stderr, "\n\t\tLast record: %s\n", Buffer);
  fprintf(stderr,
          "\t\tSTEP BY STEP SEARCH ON [%llu - %llu]\n\n",
          LowBound,
          UppBound);
#endif
  /* Ens situem a la cota inferior i busquem d'un en un */
  if ( fseeko(TraceFile, LowBound, SEEK_SET) == -1)
  {
    char CurrentError[128];

    sprintf(CurrentError,
            "unable to set file position while looking for task %02d location",
            TaskId);

    SetError(true);
    SetErrorMessage(CurrentError,
                    strerror(errno));
    return false;
  }

  return LocateFirstCPUBurst(TaskId);
}

INT32 DimemasSDDFTraceParser::GetLongLine(FILE* File, char** Line)
{
  INT32 LineLength = 0;
  off_t InitialPosition;
  int   CharRead;

  InitialPosition = ftello(File);

  if (feof(File))
    return 0;

  /* To avoid initial position on CR */
  if (fgetc(File) == '\n')
    InitialPosition++;
  else
  {
    if ( fseeko(File, InitialPosition, SEEK_SET) < 0)
    {
      SetError(true);
      SetErrorMessage("unable to seek file reading line",
                      strerror(errno));
      return -1;
    }
  }

  if (feof(File))
    return 0;

  while ( (CharRead = fgetc(File)) != '\n' )
  {
    if (CharRead == EOF && LineLength < 1)
      return 0;
    LineLength++;
  }

  *Line = (char*) calloc(sizeof(char), LineLength+2);

  if ( fseeko(File, InitialPosition, SEEK_SET) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to seek file reading line",
                    strerror(errno));
    return -1;;
  }

  fgets(*Line, LineLength+1, File);

  return LineLength;
}

bool DimemasSDDFTraceParser::GetTraceLine(FILE* File, char* Line, int LineSize)
{
  if (feof(File))
    return false;

  if (fgets (Line, LineSize, File) == NULL)
  {
    SetError(true);
    SetErrorMessage("Error reading Paraver trace file", strerror(errno));
    return false;
  }

  return true;
}



CPUBurst_t
DimemasSDDFTraceParser::ParseSDDFCPUBurst(char* SDDFCPUBurst)
{
}

Send_t
DimemasSDDFTraceParser::ParseSDDFSend(char* SDDFSend)
{
}

Receive_t
DimemasSDDFTraceParser::ParseSDDFReceive(char* SDDFReceive)
{
}

Event_t
DimemasSDDFTraceParser::ParseEvent(char* SDDFEvent)
{
}

Event_t
DimemasSDDFTraceParser::AppendEvents(Event_t CurrentEvent, char* SDDFEvent)
{
}

GlobalOp_t
DimemasSDDFTraceParser::ParseSDDFGlobalOp(char* SDDFGlobalOp)
{
}

/*
State_t
DimemasSDDFTraceParser::ParseState(char* ASCIIState)
{
  State_t NewState;
  INT32   CPU, AppId, TaskId, ThreadId;
  UINT64  BeginTime, EndTime;
  INT32   StateValue;

  if (sscanf(ASCIIState,
             "%d:%d:%d:%d:%llu:%llu:%d",
             &CPU, &AppId, &TaskId, &ThreadId,
             &BeginTime, &EndTime,
             &StateValue) == 7)
  {
    NewState = new State(CPU, AppId, TaskId, ThreadId,
                         BeginTime, EndTime,
                         StateValue);
  }
  else
  {
    char CurrentError[128];

    SetError(true);
    sprintf(CurrentError,
            "Wrong state record on line %d",
            CurrentLine);
    LastError = CurrentError;
    NewState = NULL;
  }
  return NewState;
}
*/
