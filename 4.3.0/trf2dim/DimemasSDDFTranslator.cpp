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

  $URL:: https://svn.bsc.es/repos/ptools/trf2dim/trunk/src/DimemasSDDFTran#$:

  $Rev:: 483                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-29 11:54:45 +0200#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "DimemasSDDFTranslator.hpp"

#include <EventEncoding.h>
#include <Dimemas_Generation.h>
#include <Macros.h>

#include <cmath>
#include <cstring>

#include <iostream>
using std::cout;
using std::endl;

#include <algorithm>
using std::sort;

// This must be changed
extern bool VerboseMode;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

DimemasSDDFTranslator::DimemasSDDFTranslator(string SDDFTraceName,
                                             string OutputTraceName)
{
  if ( (SDDFTraceFile = fopen(SDDFTraceName.c_str(), "r")) == NULL)
  {
    char CurrentError[128];

    sprintf(CurrentError,
            "Unable to open SDDF trace (%s)",
            SDDFTraceName.c_str());

    SetError(true);
    SetErrorMessage(CurrentError, strerror(errno));
    return;
  }

  Parser = new DimemasSDDFTraceParser(SDDFTraceName, SDDFTraceFile);

  if (Parser->GetError())
  {
    SetError(true);
    SetErrorMessage("Unable to load SDDF parser",
                    Parser->GetLastError().c_str());
    return;
  }

  this->SDDFTraceName = SDDFTraceName;

  if ((OutputTraceFile = fopen(OutputTraceName.c_str(), "w")) == NULL)
  {
    char CurrentError[128];

    sprintf(CurrentError,
            "Unable to create output trace (%s)",
            OutputTraceName.c_str());

    SetError(true);
    SetErrorMessage(CurrentError, strerror(errno));
    return;
  }

  this->OutputTraceName = OutputTraceName;
}

bool
DimemasSDDFTranslator::InitTranslator(void)
{
  if (!Parser->InitTraceParsing())
  {
    SetErrorMessage("Unable to initilize parsing",
                    Parser->GetLastError().c_str());
    return false;
  }

  return true;
}

bool
DimemasSDDFTranslator::EndTranslator(void)
{
  return true;
}

bool
DimemasSDDFTranslator::Translate(void)
{
  ApplicationDescription_t ApplicationStructure;
  DimemasRecord_t          CurrentRecord;
  vector<off_t>            OutputOffsets;
  off_t                    OffsetsOffset;
  INT32                    CurrentPercentage, PercentageRead;

  if (!Parser->Reload())
  {
    SetErrorMessage("Unable to reload SDDF trace",
                    Parser->GetLastError().c_str());
    return false;
  }

  ApplicationStructure = Parser->GetApplicationsDescription();

  if (!WriteHeader(ApplicationStructure))
    return false;

  if (!WriteCommunicators(ApplicationStructure))
    return false;

  if (Parser->GetError())
  {
    return false;
  }

  if (VerboseMode)
  {
    CurrentPercentage = 0;
    SHOW_PROGRESS(stdout, "-> Translating records", CurrentPercentage);
    fflush(stdout);
  }

  for (INT32 Task = 0; Task < ApplicationStructure->GetTaskCount(); Task++)
  {
    OutputOffsets.push_back(ftello(OutputTraceFile));

    if (OutputOffsets[Task] == -1)
    {
      char CurrentError[128];

      sprintf(CurrentError,
              "Error getting output file position for task %02d",
              Task);

      SetError(true);
      SetErrorMessage(CurrentError, strerror(errno));
      return false;
    }

    CurrentRecord = Parser->GetNextRecord(Task, 0); /* Only thread 0 is needed*/

    while (CurrentRecord != NULL)
    {
      CurrentRecord->ToDimemas(OutputTraceFile);

      /* DEBUG
      cout << (*CurrentRecord) << endl; */

      delete CurrentRecord;
      CurrentRecord = Parser->GetNextRecord(Task, 0);

      if (VerboseMode)
      {
        PercentageRead =
          (INT32) lround (100.0 * Parser->GetCurrentOffset(Task)/
                                  Parser->GetTraceSize());
        if (PercentageRead > CurrentPercentage)
        {
          CurrentPercentage = PercentageRead;
          SHOW_PROGRESS(stdout, "-> Translating records", CurrentPercentage);
          fflush(stdout);
        }
      }
    }

    if (Parser->GetError())
    {
      char CurrentError[128];

      sprintf(CurrentError,
              "Error reading records for task %02d",
              Task);

      SetError(true);
      SetErrorMessage(CurrentError, Parser->GetLastError().c_str());
      return false;
    }
  }

  if (VerboseMode)
  {
    SHOW_PROGRESS_END(stdout, "-> Translating records");
    fprintf(stdout, "\n");
  }

  OffsetsOffset = ftello(OutputTraceFile);
  if (OffsetsOffset == -1)
  {
    SetError(true);
    SetErrorMessage("Error getting offsets offset on output trace file",
                    strerror(errno));
    return false;
  }

  /* Write offset lines */
  for (INT32 Task = 0; Task < ApplicationStructure->GetTaskCount(); Task++)
  {
    if (fprintf(OutputTraceFile, "s:%d:%lld\n", Task, OutputOffsets[Task]) < 0)
    {
      SetErrorMessage("Error printing offsets line",
                      strerror(errno));
      return false;
    }
  }

  if (!WriteHeader(ApplicationStructure, false, OffsetsOffset))
    return false;

  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool
DimemasSDDFTranslator::WriteHeader(ApplicationDescription_t AppDescription,
                                   bool                     InitialHeader,
                                   off_t                    OffsetsOffset
)
{
#define OFFSETS_OFFSET_RESERVE 15

  INT32 OffsetsLength;

  char   OffsetsLengthStr[10];

  if (InitialHeader)
  {
    if (VerboseMode)
    {
      cout << "-> Writing initial header... ";
      cout.flush();
    }
  }
  else
  {
    if (VerboseMode)
    {
      cout << "-> Writing definitive header... ";
      cout.flush();
    }
    if (fseeko(OutputTraceFile, 0, SEEK_SET) != 0)
    {
      SetError(true);
      SetErrorMessage("error seting position to write definitive header",
                      strerror(errno));
      if (VerboseMode)
        cout << "Error!" << endl;
      return false;
    }
  }

  if (fprintf (OutputTraceFile,
                 "#DIMEMAS:\"%s\":1,",
                 OutputTraceName.c_str()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));
    if (VerboseMode)
      cout << "Error!" << endl;
    return false;
  }

  if (InitialHeader)
  {
    /* Reserve characters for future information */
    if (fprintf (OutputTraceFile, "XXXXXXXXXXXXXXX") < 0)
    {
      SetErrorMessage("error writing header", strerror(errno));
      if (VerboseMode)
        cout << "Error!" << endl;
      return false;
    }
  }
  else
  {
    if ((OffsetsLength = fprintf(OutputTraceFile, "%lld", OffsetsOffset)) <= 0)
    {
      SetErrorMessage("error writing header", strerror(errno));
      if (VerboseMode)
        cout << "Error!" << endl;
      return false;
    }
  }

  if (fprintf (OutputTraceFile, ":%d(", AppDescription->GetTaskCount()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));

    if (VerboseMode)
      cout << "Error!" << endl;
    return false;
  }

  for (INT32 i = 0; i < AppDescription->GetTaskCount()-1; i++)
  {
    if (fprintf (OutputTraceFile, "1,") < 0)
    {
      SetErrorMessage("error writing header", strerror(errno));

      if (VerboseMode)
        cout << "Error!" << endl;
      return false;
    }
  }

  if (fprintf(OutputTraceFile,
              "1),%d",
              AppDescription->GetCommunicatorCount()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));

    if (VerboseMode)
      cout << "Error!" << endl;
    return false;
  }

  if (!InitialHeader)
  {
    for (INT32 i = 0; i < OFFSETS_OFFSET_RESERVE - OffsetsLength; i++)
    {
      if (fprintf(OutputTraceFile," ") < 0)
      {
        SetErrorMessage("error writing header", strerror(errno));
        if (VerboseMode)
          cout << "Error!" << endl;
        return false;
      }
    }
  }

  if (fprintf(OutputTraceFile,"\n") < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));
    cout << "Error!" << endl;
    return false;
  }

  if (VerboseMode)
    cout << "Done!" << endl;

  return true;
}

bool
DimemasSDDFTranslator::WriteCommunicators(
  ApplicationDescription_t AppDescription
)
{
  vector<Communicator_t> Communicators;
  Communicator_t         CurrentCommunicator;
  set<INT32>             CommunicatorTasks;
  INT32*                 TaskIdList;
  UINT32                 Index;

  Communicators = AppDescription->GetCommunicators();


  if (VerboseMode)
  {
    cout << "-> Translating communicators 0/0";
    fprintf(stdout,
            "\r-> Translating communicators 0/%d",
            Communicators.size());
  }

  for (Index = 0;
       Index < Communicators.size();
       Index++)
  {
    CurrentCommunicator = Communicators[Index];
    TaskIdList = (int*) calloc (CurrentCommunicator->GetCommunicatorSize(),
                                sizeof(int));

    CommunicatorTasks = CurrentCommunicator->GetCommunicatorTasks();

    set<INT32>::iterator it = CommunicatorTasks.begin();
    INT32 i = 0;
    while(it != CommunicatorTasks.end())
    {
      /* TaskIdList[i] = (int) (*it)+1; */
      TaskIdList[i] = (int) (*it);

      ++it; ++i;
    }

    /*
    for (INT32 i = 0;
         i < CommunicatorTasks.size();
         i++)
    {
      TaskIdList = (int) CurrentCommunicator->GetTaskVector();
    }
    */
    if (Dimemas_Communicator_Definition(
          OutputTraceFile,
          CurrentCommunicator->GetCommunicatorId(),
          CurrentCommunicator->GetCommunicatorSize(),
          TaskIdList) < 0)
    {
      char CurrentError[128];
      sprintf(CurrentError,
              "error writing communicator %02d",
              CurrentCommunicator->GetCommunicatorId());
      SetErrorMessage(CurrentError, strerror(errno));

      free (TaskIdList);
      cout << endl << "Error!" << endl;
      return false;
    }

    free(TaskIdList);

    if (VerboseMode)
      fprintf(stdout,
              "\r-> Translating communicators %d/%d",
              Index+1,
              Communicators.size());

  }

  if (VerboseMode)
    fprintf(stdout, "\n");

  return true;
}
