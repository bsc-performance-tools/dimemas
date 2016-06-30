/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  prv2dim                                  *
 *           Paraver to Dimemas trace translator (old and new format)        *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1    *
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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/#$:  File
  $Rev:: 1044                                     $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-03-27 17:58:59 +0200 (Tue, 27 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "ParaverTraceTranslator.hpp"

#include "EventEncoding.h"
#include "Dimemas_Generation.h"
// #include "UIParaverTraceConfig.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Macros.h>

#include <iostream>
using std::cout;
using std::endl;
using std::flush;

#include <algorithm>
using std::sort;

#include <sstream>
using std::ostringstream;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

ParaverTraceTranslator::ParaverTraceTranslator(string ParaverTraceName,
                                               string DimemasTraceName,
                                               string ExtraStatsName)
{
  if ( (ParaverTraceFile = fopen(ParaverTraceName.c_str(), "r")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to open Paraver trace", strerror(errno));
    return;
  }

  Parser = new ParaverTraceParser(ParaverTraceName, ParaverTraceFile);

  this->ParaverTraceName = ParaverTraceName;

  if (Parser->GetError())
  {
    SetError(true);
    LastError = Parser->GetLastError();
  }

  if ((DimemasTraceFile = fopen(DimemasTraceName.c_str(), "w")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to create Dimemas trace", strerror(errno));
    return;
  }

  this->DimemasTraceName = DimemasTraceName;

  DescriptorShared         = false;
  MultiThreadTrace         = false;
  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;

  this->ExtraStatsName = ExtraStatsName;
  this->withExtraStats = ExtraStatsName.size() > 0;
}

ParaverTraceTranslator::ParaverTraceTranslator(string ParaverTraceName,
                                               string DimemasTraceName)
{
  if ( (ParaverTraceFile = fopen(ParaverTraceName.c_str(), "r")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to open Paraver trace", strerror(errno));
    return;
  }

  Parser = new ParaverTraceParser(ParaverTraceName, ParaverTraceFile);

  this->ParaverTraceName = ParaverTraceName;

  if (Parser->GetError())
  {
    SetError(true);
    LastError = Parser->GetLastError();
  }

  if ((DimemasTraceFile = fopen(DimemasTraceName.c_str(), "w")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to create Dimemas trace", strerror(errno));
    return;
  }

  this->DimemasTraceName = DimemasTraceName;

  this->withExtraStats     = false;
  DescriptorShared         = false;
  MultiThreadTrace         = false;
  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;
}

ParaverTraceTranslator::ParaverTraceTranslator(FILE* ParaverTraceFile,
                                               FILE* DimemasTraceFile)
{
  this->ParaverTraceFile = ParaverTraceFile;
  this->DimemasTraceFile = DimemasTraceFile;

  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;
}

bool ParaverTraceTranslator::InitTranslator(void)
{
  cout << "INITIALIZING PARSER... ";
  if (!Parser->InitTraceParsing())
  {
    cout << "NOK" << endl;
    SetErrorMessage("Unable to initilize parsing",
                    Parser->GetLastError().c_str());
    return false;
  }

  cout << "OK!" << endl;

  return true;
}

bool ParaverTraceTranslator::EndTranslator(void)
{
  if (this->ParaverTraceFile != NULL)
  {
    if (fclose(this->ParaverTraceFile) != 0)
    {
      SetErrorMessage("Error closing Paraver trace file",
                      strerror(errno));
      return false;
    }
  }

  if (!DescriptorShared && this->DimemasTraceFile != NULL)
  {
    if (fclose(this->DimemasTraceFile) != 0)
    {
      SetErrorMessage("Error closing Dimemas trace file",
                      strerror(errno));
      return false;
    }
  }

  if (this->CommunicationsFile != NULL)
  {
    if (fclose(this->CommunicationsFile) != 0)
    {
      SetErrorMessage("Error closing communications temporal file",
                      strerror(errno));
      return false;
    }

    unlink(this->CommunicationsFileName);
  }

  for (unsigned int i = 0; i < TranslationInfo.size(); i++)
  {
    delete TranslationInfo[i];
  }

  return true;
}

bool
ParaverTraceTranslator::SplitCommunications(void)
{
  // vector<PartialCommunication_t> CommunicationVector;
  vector<ApplicationDescription_t> AppDescription;
  ParaverRecord_t                  CurrentRecord;
  Event_t                          CurrentEvent;
  Communication_t                  CurrentCommunication;
  PartialCommunication_t           SplittedCommunication;
  UINT64                           i;
  INT32                            COMM_WORLD_Id;
  INT32                            PseudoCommId;

  /* DEBUG */
  FILE *UnsortedFile;

  INT32 CurrentPercentage = 0;
  INT32 PercentageRead    = 0;

  const char* tmp_dir_default = "/tmp";
  char* tmp_dir;

  if ( (tmp_dir = getenv("TMPDIR")) == NULL)
  {
    tmp_dir = strdup(tmp_dir_default);
  }

  CommunicationsFileName = (char*) malloc (strlen(tmp_dir) + 1 + 20);

  if (CommunicationsFileName == NULL)
  {
    SetErrorMessage("unable to allocate memory to communications filename",
                    strerror(errno));
    return false;
  }

  srand(time(NULL));

  sprintf(CommunicationsFileName,
          "%s/ParaverComms_%06d",
          tmp_dir,
          rand()%999999);

  AppDescription = Parser->GetApplicationsDescription();

  COMM_WORLD_Id = AppDescription[0]->GetCOMM_WORLD_Id();

  CommunicationsFile = fopen(CommunicationsFileName, "w+");

  /* Uncomment this section when testing finishes
  CommunicationsFile = tmpfile();
  */

  if (CommunicationsFile == NULL)
  {
    ostringstream  ErrorMessage;

    ErrorMessage << "Unable to create temporal communications file : ";
    ErrorMessage << strerror(errno) << endl;
    ErrorMessage << "Please, check permissions on '/tmp' or update 'TMPDIR' environment variable";

    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (!Parser->Reload())
  {
    SetErrorMessage("Unable to reload Paraver trace",
                    Parser->GetLastError().c_str());
    return false;
  }

  i = 0;

  // printf("SPLITTING COMMUNICATIONS\n");
  /* In order to guarantee the cyclic simulations, when splitting the communications
   * we also need to process the 'pseudo-physical receive' events */

  SHOW_PERCENTAGE_PROGRESS(stdout, "SPLITTING COMMUNICATIONS", CurrentPercentage);
  PseudoCommId = 0;
  // CurrentCommunication = Parser->GetNextCommunication();
  CurrentRecord = Parser->GetNextRecord(EVENT_REC | COMM_REC);

  while (CurrentRecord != NULL)
  {
    PseudoCommId++;

    if ( (CurrentEvent = dynamic_cast<Event_t> (CurrentRecord)) != NULL)
    { /* Current Record is an event. We split it, if needed */

      INT32  Type;
      UINT64 Timestamp;

      INT32 PartnerTaskId, PartnerThreadId;
      INT32  Size, Tag, CommId;

      // SrcCPU = SrcAppId = SrcTaskId = SrcThreadId = -1;
      Type          = -1;
      PartnerTaskId = PartnerThreadId = -1;
      Size          = Tag             = CommId = -1;

      if (CurrentEvent->GetTypeValueCount() > 1)
      {
        /* Capture the 'pseudo-physical receive' events generated by Dimemas */

        for (unsigned int i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
        {
          switch (CurrentEvent->GetType(i))
          {
            case 9:
              Type            = CurrentEvent->GetValue(i);
              break;
            case 10:
              PartnerTaskId   = CurrentEvent->GetValue(i);
              break;
            case 11:
              PartnerThreadId = CurrentEvent->GetValue(i);
              break;
            case 12:
              Size            = CurrentEvent->GetValue(i);
              break;
            case 13:
              Tag             = CurrentEvent->GetValue(i);
              break;
            case 14:
              CommId          = CurrentEvent->GetValue(i);
              break;
          }
        }

        /* If all events are available, generate the the PartialCommunication record */
        //if (SrcCPU != -1 && SrcAppId != -1 && SrcTaskId != -1 && SrcThreadId != -1 &&
        if (Type != -1  && PartnerTaskId != -1 &&
            Size != -1  && Tag           != -1 && CommId    != -1)
        {
          INT32  SrcCPU, SrcAppId, SrcTaskId, SrcThreadId;
          INT32  DstCPU, DstAppId, DstTaskId, DstThreadId;

          if (Type == PHYSICAL_RECV || Type == LOGICAL_RECV)
          {
            SrcCPU      = 0;
            SrcAppId    = CurrentEvent->GetAppId();
            SrcTaskId   = PartnerTaskId;
            SrcThreadId = PartnerThreadId;
            DstCPU      = 0;
            DstAppId    = CurrentEvent->GetAppId();
            DstTaskId   = CurrentEvent->GetTaskId();
            DstThreadId = CurrentEvent->GetThreadId();

          }
          else if (Type == PHYSICAL_SEND || Type == LOGICAL_SEND)
          {
            SrcCPU      = 0;
            SrcAppId    = CurrentEvent->GetAppId();
            SrcTaskId   = CurrentEvent->GetTaskId();
            SrcThreadId = CurrentEvent->GetThreadId();
            DstCPU      = 0;
            DstAppId    = CurrentEvent->GetAppId();
            DstTaskId   = PartnerTaskId;
            DstThreadId = PartnerThreadId;
          }

          PreviouslySimulatedTrace = true;
          SplittedCommunication    = new PartialCommunication(Type,
                                                              CurrentEvent->GetTimestamp(),
                                                              SrcCPU, SrcAppId, SrcTaskId, SrcThreadId,
                                                              DstCPU, DstAppId, DstTaskId, DstThreadId,
                                                              Size, Tag, CommId, CurrentEvent->GetRecordCount());
          Communications.push_back(SplittedCommunication);
        }
      }
    }
    else
    {
      /**
       * JGG (2014/12/17): PseudoCommId is now the CommID of the original
       * execution!
       */

      CurrentCommunication = dynamic_cast<Communication_t> (CurrentRecord);

      SplittedCommunication = new PartialCommunication(LOGICAL_SEND,
                                                       CurrentCommunication,
                                                       PseudoCommId);
      Communications.push_back(SplittedCommunication);

      SplittedCommunication = new PartialCommunication(LOGICAL_RECV,
                                                       CurrentCommunication,
                                                       PseudoCommId);
      Communications.push_back(SplittedCommunication);

      SplittedCommunication = new PartialCommunication(PHYSICAL_RECV,
                                                       CurrentCommunication,
                                                       PseudoCommId);
      Communications.push_back(SplittedCommunication);
    }


    /*
    delete CurrentCommunication;

    CurrentCommunication = Parser->GetNextCommunication();
    */

    delete CurrentRecord;

    if (PreviouslySimulatedTrace)
    {
      CurrentRecord = Parser->GetNextRecord(EVENT_REC);
    }
    else
    {
      CurrentRecord = Parser->GetNextRecord(COMM_REC | EVENT_REC);
    }

    i++;

    PercentageRead = Parser->GetFilePercentage();

    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      SHOW_PERCENTAGE_PROGRESS(stdout, "SPLITTING COMMUNICATIONS", CurrentPercentage);
    }
  }

  cout << endl;
  cout << "-> Trace first pass finished (communications split)" << endl;
  cout << "   * Records parsed:          " << i << endl;;
  cout << "   * Splitted communications " << Communications.size();
  cout << endl;

  if (Parser->GetError())
  {
    SetError(true);
    SetErrorMessage("Error reading communication records",
                    Parser->GetLastError().c_str());
    return false;
  }

  /* DEBUG
  UnsortedFile = fopen("TEST", "w");
  for (i = 0; i < Communications.size(); i++)
    Communications[i]->ToFile(UnsortedFile);
  */

  cout << "SORTING PARTIAL COMMUNICATIONS" << endl;
  sort(Communications.begin(), Communications.end(), InBlockComparison());
  cout << "COMMUNICATIONS SORTED" << endl;

  /*
  for (i = 0; i < Communications.size(); i++)
    Communications[i]->ToFile(CommunicationsFile);
  */

  return true;

}

bool ParaverTraceTranslator::WriteNewFormatHeader(ApplicationDescription_t AppDescription,
                                                  off_t                    OffsetsOffset)
{
#define OFFSETS_OFFSET_RESERVE 15

  INT32 OffsetsLength;
  bool  InitialHeader = false;

  if (OffsetsOffset == 0)
  {
    InitialHeader = true;
  }

  if(!InitialHeader)
  {
    /* rewind(DimemasTraceFile); */
    if (fseek(DimemasTraceFile, 0, SEEK_SET) != 0)
    {
      SetError(true);
      SetErrorMessage("error seting position to write definitive header",
                      strerror(errno));
      return false;
    }
  }



  if (fprintf (DimemasTraceFile,
               "#DIMEMAS:\"%s\":1,",
               DimemasTraceName.c_str()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));
    return false;
  }

  if (InitialHeader)
  {
    /* Reserve characters for future information */
    /* !!! Ensure that the number of crosses is the same that
           OFFSETS_OFFSET_RESERVE value */
    if (fprintf (DimemasTraceFile, "XXXXXXXXXXXXXXX") < 0)
    {
      SetErrorMessage("error writing header", strerror(errno));
      return false;
    }
  }
  else
  {
    if ((OffsetsLength = fprintf(DimemasTraceFile, "%zu", OffsetsOffset)) <= 0)
    {
      SetErrorMessage("error writing header", strerror(errno));
      return false;
    }
  }



  if (fprintf (DimemasTraceFile, ":%d(", AppDescription->GetTaskCount()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));

    return false;
  }


  for (INT32 i = 0; i < AppDescription->GetTaskCount()-1; i++)
  {
    if (fprintf (DimemasTraceFile, "1,") < 0)
    {
      SetErrorMessage("error writing header", strerror(errno));

      return false;
    }
  }

  if (fprintf(DimemasTraceFile,
              "1),%d",
              AppDescription->GetCommunicatorCount()) < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));

    return false;
  }


  if (!InitialHeader)
  {
    for (INT32 i = 0; i < OFFSETS_OFFSET_RESERVE - OffsetsLength; i++)
    {
      if (fprintf(DimemasTraceFile," ") < 0)
      {
        SetErrorMessage("error writing header", strerror(errno));
        return false;
      }
    }
  }


  if (fprintf(DimemasTraceFile,"\n") < 0)
  {
    SetErrorMessage("error writing header", strerror(errno));
    cout << "Error!" << endl;
    return false;
  }

  return true;
}



bool
ParaverTraceTranslator::Translate(bool   GenerateFirstIdle,
                                  double IprobeMissesThreshold,
                                  INT32  BurstCounterType,
                                  double BurstCounterFactor,
                                  bool   GenerateMPIInitBarrier)
{
  /* unsigned int CurrentCommunication; */
  vector<ApplicationDescription_t> AppsDescription;
  ParaverRecord_t CurrentRecord;
  INT32  TimeUnits;
  double TimeFactor;

  INT32  CurrentPercentage = 0;
  INT32  PercentageRead    = 0;

  vector<INT32> TasksWithOutsideComms;
  bool          OutsideCommsPresent = false;

  vector<INT32> TasksWithWrongComms;
  bool          WrongCommsPresent = false;

  vector<INT32> TasksWithNonDeterministicComms;
  bool          NonDeterministicCommsPresent = false;

  vector<INT32> TasksWithDisorderedRecords;
  bool          DisorderedRecordsPresent = false;

  /* For writting new header format with offsets */
  off_t                    OffsetsOffset;
  vector<off_t>            OutputOffsets;

  /* For obtaining cluster labels from .pcf
  using namespace libparaver;
  UIParaverTraceConfig *pcfTrace;
  string               InputPCFName;

  vector<unsigned int> ClusteringValues;
  vector<unsigned int> UserFunctionValues;

  int                 *pcfValues;
  char               **pcfLabels;
  */

  cout << flush;

  cout << "INITIALIZING TRANSLATION... ";
  if (!Parser->Reload())
  {
    cout << " NOK!" << endl;

    SetErrorMessage("Unable to reload Paraver trace",
                    Parser->GetLastError().c_str());
    return false;
  }

  AppsDescription = Parser->GetApplicationsDescription();

  if (AppsDescription.size() == 0 && Parser->GetError())
  {
    cout << " NOK!" << endl;

    SetErrorMessage("Unable to read applications description",
                    Parser->GetLastError().c_str());
    return false;
  }
  else if (AppsDescription.size() == 0)
  {
    cout << " NOK!" << endl;

    LastError = "Trace without applications description";
    return false;
  }
  else if (AppsDescription.size() != 1)
  {
    cout << " NOK!" << endl;

    LastError =
      "This translator is only able to translate Paraver traces with 1 application";
    return false;
  }

  if ((TimeUnits = Parser->GetTimeUnits()) < 0)
  {
    cout << " NOK!" << endl;
    SetErrorMessage("Unable to get trace time units",
                    Parser->GetLastError().c_str());
    return false;
  }

  switch(TimeUnits)
  {
    case MICROSECONDS:
      TimeFactor = 1e-6;
      break;
    case NANOSECONDS:
      TimeFactor = 1e-9;
      break;
    default:
      cout << " NOK!" << endl;
      LastError = "Error reading trace, time units are unknown";
      return false;
      break;
  }
  cout << " OK" << endl;

  // cout << "CREATING CONTROL STRUCTURES..." << flush;
  if (!InitTranslationStructures(AppsDescription[0],
                                 TimeFactor,
                                 GenerateFirstIdle,
                                 IprobeMissesThreshold,
                                 BurstCounterType,
                                 BurstCounterFactor,
                                 GenerateMPIInitBarrier))
    return false;

  if (DescriptorShared)
  {
    DimemasTraceFile = fopen(DimemasTraceName.c_str(), "a");

    if (DimemasTraceFile == NULL)
    {
      SetErrorMessage("Error opening output tracefile", strerror(errno));
      return false;
    }
  }


  /*
  */

  if (MultiThreadTrace)
  {
    cout << " WARNING! Tasks with multiple threads. Just first thread will be translated" << endl;
  }
  else
  {
    cout << endl;
  }
  /*
  else
  {
    cout << " OK" << endl;
  }
  */

  cout << "WRITING HEADER...";

  /* Temporary header! */
  if(!WriteNewFormatHeader(AppsDescription[0]))
    return false;

  cout << " OK" << endl;

  cout << "TRANSLATING COMMUNICATORS...";
  if (!TranslateCommunicators(AppsDescription[0]))
    return false;

  if (DescriptorShared)
  {
    if (fclose(DimemasTraceFile) != 0)
    {
      SetError(true);
      SetErrorMessage("Error closing Dimemas trace file to share descriptors",
                      strerror(errno));
      return false;
    }
  }
  cout << " OK" << endl;

  printf("RECORD TRANSLATION\n");

  SHOW_PERCENTAGE_PROGRESS(stdout, "TRANSLATING RECORDS", CurrentPercentage);

  Parser->Reload();

  CurrentRecord = SelectNextRecord();

  while (CurrentRecord != NULL)
  {
    Event_t  CurrentEvent;
    INT32    CurrentTaskId = CurrentRecord->GetTaskId()-1;

    if (CurrentTaskId < 0 || CurrentTaskId >= TranslationInfo.size())
    {
      /* Wrong record on the trace! */
      WrongRecordsFound ++;
      delete CurrentRecord;
    }
    else
    {

#ifdef DEBUG
      cout << "SELECTED RECORD: "<< endl << *CurrentRecord;
#endif

      /* GlobalOp_t CurrentGlobalOp; */
      /* Record translation */

      /* DEBUG
      cout << *CurrentRecord;
      */

      if ( (CurrentEvent = dynamic_cast<Event_t> (CurrentRecord)) != NULL)
      { /* Current Record is an event. We split it, if needed */
        if (CurrentEvent->GetTypeValueCount() > 1)
        {
          /* DEBUG
          fprintf(stdout, "Adding an Event with %d type/values\n",
                  CurrentEvent->GetTypeValueCount());
          */

          for (unsigned int i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
          {
            Event_t SubEvent;

            SubEvent = new Event( CurrentEvent->GetTimestamp(),
                                  CurrentEvent->GetCPU(),
                                  CurrentEvent->GetAppId(),
                                  CurrentEvent->GetTaskId(),
                                  CurrentEvent->GetThreadId());

            SubEvent->AddTypeValue(CurrentEvent->GetType(i),
                                   CurrentEvent->GetValue(i));

  #ifdef DEBUG
            // cout << "Pushing SubEvent: " << *SubEvent;
  #endif

            if (!TranslationInfo[CurrentTaskId]->PushRecord(SubEvent))
            {
              SetError(true);
              this->LastError = TranslationInfo[CurrentTaskId]->GetLastError();

              return false;
            }
          }

          /*
          CurrentGlobalOp = Event2GlobalOp(CurrentEvent);
          if (CurrentGlobalOp != NULL)
          {
            CurrentRecord = static_cast<ParaverRecord_t> (CurrentGlobalOp);
            continue;
          }
          */

          delete CurrentRecord;
        }
        else
        {
          if (!TranslationInfo[CurrentTaskId]->PushRecord(CurrentRecord))
          {
            SetError(true);
            this->LastError = TranslationInfo[CurrentTaskId]->GetLastError();
            return false;
          }
        }
      }
      else
      {
        if (!TranslationInfo[CurrentTaskId]->PushRecord(CurrentRecord))
        {
          SetError(true);
          this->LastError = TranslationInfo[CurrentTaskId]->GetLastError();
          return false;
        }
      }
    }

    CurrentRecord = SelectNextRecord();

    PercentageRead = Parser->GetFilePercentage();

    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      SHOW_PERCENTAGE_PROGRESS(stdout, "TRANSLATING RECORDS", CurrentPercentage);
    }
  }

  if (this->withExtraStats)
  {
    /* Flush extra statistics on file */
    FILE * extra_statistics_file = fopen(this->ExtraStatsName.c_str(), "w");
    assert(extra_statistics_file != NULL);

    fprintf(extra_statistics_file, "// #send,#isend,#recv, #irecv, #wait, #glop\n");

    unsigned int min_glops = 10000000000;
    for (int i=0; i < TranslationInfo.size(); ++i)
    {
      if (TranslationInfo[i]->glop_counter < min_glops)
        min_glops = TranslationInfo[i]->glop_counter;
    }

    for (int i=0; i < TranslationInfo.size(); ++i)
    {
      fprintf(extra_statistics_file, "%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
              TranslationInfo[i]->send_counter,
              TranslationInfo[i]->isend_counter,
              TranslationInfo[i]->recv_counter,
              TranslationInfo[i]->irecv_counter,
              TranslationInfo[i]->wait_counter,
              TranslationInfo[i]->glop_counter,
              TranslationInfo[i]->pendent_i_Send_counter,
              TranslationInfo[i]->pendent_i_Recv_counter,
              TranslationInfo[i]->glop_counter - min_glops);
              //TranslationInfo[i]->pendent_Glop_counter);
    }

    fclose(extra_statistics_file);
  }

  if (Parser->GetError())
  {
    return false;
  }

  if (DescriptorShared)
  {
    if ((DimemasTraceFile = fopen(DimemasTraceName.c_str(), "a")) == NULL)
    {
      SetError(true);
      SetErrorMessage("Unable to re-open Dimemas trace", strerror(errno));
      return false;
    }
  }

  cout << endl;
  cout << "MERGING PARTIAL OUTPUT TRACES" << endl;
  int TotalMPIInitBarriersWritten = 0;
  for (UINT32 i = 0; i < TranslationInfo.size(); i++)
  {
    if (!(TranslationInfo[i]->LastFlush()))
    {
      char CurrentError[128];

      sprintf(CurrentError,
              "Error flushing last records from task %02d : ",
              i+1);
      SetErrorMessage(CurrentError,
                      TranslationInfo[i]->GetLastError().c_str());
      return false;
    }

    OutputOffsets.push_back(ftello(DimemasTraceFile));

    if (!(TranslationInfo[i]->Merge(DimemasTraceFile)))
    {
      char CurrentError[128];

      sprintf(CurrentError,
              "Error merging file from task %02d : ",
              i+1);
      SetErrorMessage(CurrentError,
                      TranslationInfo[i]->GetLastError().c_str());
      return false;
    }

    if (TranslationInfo[i]->GetOutsideComms())
    {
      OutsideCommsPresent = true;
      TasksWithOutsideComms.push_back(TranslationInfo[i]->GetTaskId());
    }

    if (TranslationInfo[i]->GetWrongComms())
    {
      WrongCommsPresent = true;
      TasksWithWrongComms.push_back(TranslationInfo[i]->GetTaskId());
    }

    if (TranslationInfo[i]->GetNonDeterministicComms())
    {
      NonDeterministicCommsPresent = true;
      TasksWithNonDeterministicComms.push_back(TranslationInfo[i]->GetTaskId());
    }

    if (TranslationInfo[i]->GetDisorderedRecords())
    {
      DisorderedRecordsPresent = true;
      TasksWithDisorderedRecords.push_back(TranslationInfo[i]->GetTaskId());
    }

    if (TranslationInfo[i]->GetMPIInitBarrierWritten())
    {
      TotalMPIInitBarriersWritten++;
    }
  }
  printf("   * All task merged!         \r\n");

  if (OutsideCommsPresent)
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithOutsideComms.size() << " ";
    /*
    cout << "Tasks ";
    cout << TasksWithOutsideComms[0];
    for (size_t i = 1; i < TasksWithOutsideComms.size(); i++)
      cout << ", " << TasksWithOutsideComms[i];
    cout << endl;
    */
    cout << "tasks have communications records outside a communication block" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: If the Paraver trace comes from a trace cut, check the cut limtis" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if (WrongCommsPresent)
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithWrongComms.size() << " ";
    /*
    cout << "Tasks ";
    cout << TasksWithWrongComms[0];
    for (size_t i = 1; i < TasksWithOutsideComms.size(); i++)
      cout << ", " << TasksWithOutsideComms[i];
    cout << endl; */
    cout << "tasks have communications records wrapped with not matching blocks" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Pleasy check the Paraver trace time resolution" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if (NonDeterministicCommsPresent)
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithNonDeterministicComms.size() << " ";
    /*
    cout << "Tasks ";
    cout << TasksWithWrongComms[0];
    for (size_t i = 1; i < TasksWithOutsideComms.size(); i++)
      cout << ", " << TasksWithOutsideComms[i];
    cout << endl; */
    cout << "tasks of this application execute 'non-deterministic' communications " << endl;
    cout << "primitives (MPI_Test[*] | MPI_Waitany | MPI_Waitall | MPI_Waitsome)" << endl;
    cout << "The simulation of this trace will not capture the possible indeterminism " << endl;
    // cout << "NOTE: Pleasy check the Paraver trace time resolution" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if (DisorderedRecordsPresent)
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithDisorderedRecords.size() << " ";
    /*
    cout << "Tasks ";
    cout << TasksWithDisorderedRecords[0];
    for (size_t i = 1; i < TasksWithDisorderedRecords.size(); i++)
      cout << ", " << TasksWithDisorderedRecords[i];
    cout << endl; */
    cout << "tasks have disordered records" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Contact tools@bsc.es to check how to solve this problem" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if (WrongRecordsFound > 0)
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << WrongRecordsFound << " wrong records found in the input trace.  " << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Contact tools@bsc.es to check how to solve this problem" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if (TotalMPIInitBarriersWritten != 0 &&
      TotalMPIInitBarriersWritten != TranslationInfo.size() &&
      GenerateMPIInitBarrier)
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               ERROR                                          *" << endl;
    cout << "********************************************************************************" << endl;
    cout << "ERROR: The 'MPI_Init' primitive does not appear at the beggining of the all" << endl;
    cout << "ERROR: tasks in this trace. Current translation will fail" << endl;
    cout << endl;
    cout << "ERROR: Please, re-run the translator using the '-s' flags" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }


#ifdef NEW_DIMEMAS_TRACE

  /* Writting offsets */

  OffsetsOffset = ftello(DimemasTraceFile);
  if (OffsetsOffset == -1)
  {
    SetErrorMessage("Error getting offsets offset on output trace file",
                    strerror(errno));
    return false;
  }

  /* Write offset line
   * JGG (12/2011) TEST FOR NEW TRACE
  if (fprintf(DimemasTraceFile, "s") < 0)
  {
    SetErrorMessage("Error printing offsets line",
                    strerror(errno));
    return false;
  }
  */

  for (INT32 Task = 0; Task < (AppsDescription[0])->GetTaskCount(); Task++)
  {
    // JGG (12/2011): TESTS NEW TRACE
    // if (fprintf(DimemasTraceFile, ":%d,%lld", Task, OutputOffsets[Task]) < 0, )
    if (fprintf(DimemasTraceFile, "s:%d:%zu\n", Task, OutputOffsets[Task]) < 0)
    {
      SetErrorMessage("Error printing offsets line",
                      strerror(errno));
      return false;
    }
  }

  if (DescriptorShared)
  {
    fclose(DimemasTraceFile);

    DimemasTraceFile = fopen(DimemasTraceName.c_str(), "r+");

    if (DimemasTraceFile == NULL)
    {
      SetErrorMessage("Error opening output tracefile", strerror(errno));
      return false;
    }
  }

  if(!WriteNewFormatHeader(AppsDescription[0], OffsetsOffset))
  {
    return false;
  }

#endif

  cout << "TRANSLATION FINISHED" << endl;

  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool
ParaverTraceTranslator::InitTranslationStructures(
  ApplicationDescription_t AppDescription,
  double                   TimeFactor,
  bool                     GenerateFirstIdle,
  double                   IprobeMissesThreshold,
  INT32                    BurstCounterType,
  double                   BurstCounterFactor,
  bool                     GenerateMPIInitBarrier)
{
  vector<TaskDescription_t> TaskInfo;
  vector<Communicator_t>    TraceCommunicators;
  UINT32                    CurrentCommunicator, CurrentTask;
  TaskTranslationInfo_t     NewTranslationInfo;
  FILE *TemporaryFile;

  bool   BurstCounterGeneration;

  if (BurstCounterType != -1)
    BurstCounterGeneration = true;
  else
    BurstCounterGeneration = false;

  /* Temporal directory management */
  char* TemporaryFileName;
  const char* tmp_dir_default = "/tmp";
  char* tmp_dir;

  if ( (tmp_dir = getenv("TMPDIR")) == NULL)
  {
    tmp_dir = strdup(tmp_dir_default);
  }

  /* Translation communicator initialization */
  TraceCommunicators = AppDescription->GetCommunicators();

  for (CurrentCommunicator = 0;
       CurrentCommunicator < TraceCommunicators.size();
       CurrentCommunicator++)
  {
    TranslationCommunicator_t NewTransalationComm;

    NewTransalationComm =
      new TranslationCommunicator(TraceCommunicators[CurrentCommunicator]);

    Communicators.push_back(NewTransalationComm);
  }

  TaskInfo = AppDescription->GetTaskInfo();

  for (CurrentTask = 0; CurrentTask < TaskInfo.size(); CurrentTask++)
  {
    ParaverRecord_t FirstRecord;
    bool            EmptyTask       = false;
    UINT64          FirstRecordTime = 0;

    TemporaryFileName = (char*) malloc (strlen(tmp_dir) + 1 + 20);

    SHOW_PROGRESS(stdout,
                  "CREATING TRANSLATION STRUCTURES ",
                  CurrentTask+1,
                  (int) TaskInfo.size());

    if (TemporaryFileName == NULL)
    {
      SetErrorMessage("unable to allocate memory to temporay filename",
                    strerror(errno));
      return false;
    }

    sprintf(TemporaryFileName,
            "%s/TmpTask_%04d_%06d",
            tmp_dir,
            CurrentTask,
            rand()%999999);

    if (!DescriptorShared)
    {
      TemporaryFile = fopen(TemporaryFileName, "a");
    }
    else
    {
      TemporaryFile = NULL;
    }

    if (!Parser->Reload())
    {
      SetErrorMessage("Unable to reload Paraver trace",
                      Parser->GetLastError().c_str());
      return false;
    }

    Parser->Reload();

    FirstRecord = Parser->GetNextTaskRecord((INT32)CurrentTask+1);

    if (FirstRecord == NULL)
    {
      EmptyTask = true;
    }
    else
    {
      FirstRecordTime = FirstRecord->GetTimestamp();
    }

    if ( DescriptorShared ||
        (TemporaryFile == NULL && (errno == EMFILE || errno == ENFILE)))
    { /* No file descriptors available */
      /*
      cout << "Task " << TaskInfo[CurrentTask]->GetTaskId();
      cout << " shares descriptor" << endl; */

      if (!DescriptorShared)
      {
        if (!ShareDescriptor())
        {
          return false;
        }
      }

      NewTranslationInfo =
        new TaskTranslationInfo(TaskInfo[CurrentTask]->GetTaskId(),
                                TimeFactor,
                                FirstRecordTime,
                                GenerateFirstIdle,
                                EmptyTask,
                                IprobeMissesThreshold,
                                BurstCounterGeneration,
                                BurstCounterType,
                                BurstCounterFactor,
                                GenerateMPIInitBarrier,
                                PreviouslySimulatedTrace,
                                &TranslationInfo,
                                TemporaryFileName );
    }
    else if (TemporaryFile == NULL)
    { /* Other error */
      char CurrentError[128];
      sprintf(CurrentError,
              "Error opening temporary file to task %02d",
              TaskInfo[CurrentTask]->GetTaskId());
      SetErrorMessage(CurrentError, strerror(errno));
      return false;
    }
    else
    {
      NewTranslationInfo =
        new TaskTranslationInfo(TaskInfo[CurrentTask]->GetTaskId(),
                                TimeFactor,
                                FirstRecordTime,
                                GenerateFirstIdle,
                                EmptyTask,
                                IprobeMissesThreshold,
                                BurstCounterGeneration,
                                BurstCounterType,
                                BurstCounterFactor,
                                GenerateMPIInitBarrier,
                                PreviouslySimulatedTrace,
                                &TranslationInfo,
                                TemporaryFileName,
                                TemporaryFile );
    }

    if (TaskInfo[CurrentTask]->GetThreadCount() > 1)
    {
      MultiThreadTrace = true;
    }

    TranslationInfo.push_back(NewTranslationInfo);

    delete FirstRecord;
  }
  SHOW_PROGRESS_END(stdout,
                    "CREATING TRANSLATION STRUCTURES ",
                    (int) TaskInfo.size());

  return true;
}

bool ParaverTraceTranslator::TranslateCommunicators(ApplicationDescription_t AppDescription)
{
  unsigned int           Index;
  vector<Communicator_t> Communicators;
  Communicator_t         CurrentCommunicator;
  set<INT32>             CommunicatorTasks;
  int* TaskIdList;


  Communicators = AppDescription->GetCommunicators();

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
      TaskIdList[i] = (int) (*it);

      if (TaskIdList[i] != -1)
        TaskIdList[i]--;

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
          DimemasTraceFile,
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
      return false;
    }

    free(TaskIdList);
  }

  return true;
}

ParaverRecord_t
ParaverTraceTranslator::SelectNextRecord(void)
{
  static ParaverRecord_t CurrentTraceRecord = NULL;
  static UINT32          CurrentCommunicationIndex = 0;
  static bool            LastRecordTrace = true;

  PartialCommunication_t CurrentCommunication;
  Event_t                PrvEventRecord;

  if (LastRecordTrace)
  {
    /* Only events or global ops are needed */
    CurrentTraceRecord = Parser->GetNextRecord(EVENT_REC | GLOBOP_REC);
  }

  if (CurrentTraceRecord == NULL && Parser->GetError())
  {
    SetError(true);
    SetErrorMessage("Error reading trace record",
                    Parser->GetLastError().c_str());
    return NULL;
  }

  if (CurrentTraceRecord == NULL)
  {
    if ((CurrentCommunicationIndex == Communications.size()))
      return NULL;
    else
    {
      LastRecordTrace = false;
      CurrentCommunication = Communications[CurrentCommunicationIndex];
      CurrentCommunicationIndex++;
      return CurrentCommunication;
    }
  }

  if (CurrentCommunicationIndex == Communications.size())
  {
    LastRecordTrace = true;
    return CurrentTraceRecord;
  }

  /* Here, both kind of records are available */
  CurrentCommunication = Communications[CurrentCommunicationIndex];

  if (CurrentTraceRecord->operator<(static_cast<ParaverRecord>(*CurrentCommunication)))
  /* if (CurrentTraceRecord < CurrentCommunication)*/
  {
    LastRecordTrace = true;
    return CurrentTraceRecord;
  }

  if (CurrentTraceRecord->operator>(static_cast<ParaverRecord>(*CurrentCommunication)))
  /* if (CurrentTraceRecord > CurrentCommunication) */
  {
    LastRecordTrace = false;
    CurrentCommunicationIndex++;
    return CurrentCommunication;
  }

  /* Both records are simultaneous */
  if (CurrentTraceRecord->GetTaskId() != CurrentCommunication->GetTaskId())
  { /* They come from different tasks. Communication is return */
    LastRecordTrace = false;
    CurrentCommunicationIndex++;
    return CurrentCommunication;
  }

  PrvEventRecord = dynamic_cast<Event_t> (CurrentTraceRecord);

  if (PrvEventRecord != NULL) /* Current trace record is an event */
  {
    if (PrvEventRecord->GetTypeValueCount() >= 1)
    {
      if (PrvEventRecord->IsMPIBlockBegin())
      {
        LastRecordTrace = true;
        return CurrentTraceRecord;
      }
      else if (PrvEventRecord->IsMPIBlockEnd())
      {
        LastRecordTrace = false;
        CurrentCommunicationIndex++;
        return CurrentCommunication;
      }
      else
      {
        LastRecordTrace = true;
        return CurrentTraceRecord;
      }
    }
    else
    { /* ¿Invalid? event record */
      LastRecordTrace = true;
      CurrentCommunicationIndex++;
      return CurrentCommunication;
    }
  }
  else /* Current trace record is a global op */
  {
    LastRecordTrace = false;
    return CurrentTraceRecord;
  }
}

TranslationCommunicator_t
ParaverTraceTranslator::GetCommunicator(INT32 CommId)
{
  if (Communicators.size() == 0)
    return NULL;

  for (UINT32 i = 0; i < Communicators.size(); i++)
  {
    if (Communicators[i]->GetCommunicatorId() == CommId)
      return Communicators[i];
  }

  return NULL;
}

bool
ParaverTraceTranslator::ShareDescriptor(void)
{
  if (DescriptorShared)
  {
    SetError(true);
    this->LastError = "Dimemas descriptor already share";
    return false;
  }
  else
  {
    if (fclose(DimemasTraceFile) != 0)
    {
      SetError(true);
      SetErrorMessage("Error closing Dimemas trace file to share descriptors",
                      strerror(errno));
      return false;
    }
    DescriptorShared = true;

    cout << " (SHARING DESCRIPTORS)" << flush;

    return true;
  }
}
