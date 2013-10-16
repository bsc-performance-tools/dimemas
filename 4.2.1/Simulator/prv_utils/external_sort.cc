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

#include <unistd.h>

#include "dimemas_io.h"
#include "subr.h"
#include "paraver.h"

#ifdef __cplusplus
}
#endif

#include <cstring>
#include <cerrno>
#include <cstdio>

#include <iostream>
using std::cout;
using std::endl;

#include <algorithm>
using std::stable_sort;

#include "external_sort.h"

string ExternalSort::TmpDir                 = "/tmp";
string ExternalSort::TmpFilesPrefix         = "prv_states_comms_tmp";
string ExternalSort::EventsFileName         = "";
string ExternalSort::StatesAndCommsFileName = "";

void ExternalSort::Init()
{
  char* TmpDirChar;

  if ( (TmpDirChar = getenv("TMPDIR")) != NULL)
  {
    ExternalSort::TmpDir = string(TmpDirChar);
  }

  ExternalSort::EventsFileName = ExternalSort::TmpDir + "/prv_events_tmp";

  if ( (EventsFile = IO_fopen(ExternalSort::EventsFileName.c_str(), "w")) == NULL)
  {
    die("Unable to open a temporal Paraver events file: %s\n", IO_get_error());
  }

  ExternalSort::StatesAndCommsFileName = ExternalSort::TmpDir + "/prv_states_comms_tmp";

  /* This open is to guarantee the availability of file pointers */
  NumFiles = 0;

  ExternalSort::TmpFilesPrefix = ExternalSort::TmpDir+"/prv_states_comms_tmp";



  TemporalFileNames.str("");
  TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << NumFiles;

  if ( (StatesAndCommsFile = IO_fopen(TemporalFileNames.str().c_str(), "w")) == NULL)
  {
    die ("Unable to open temporal file %s: %s\n",
         TemporalFileNames.str().c_str(),
         IO_get_error());
  }

  // unlink("prv_events_tmp");

  TotalRecords  = 0;
  CurrentRecord = 0;
  Records       = vector<SimpleParaverRecord> (MAX_IN_FLIGHT_RECORDS);
  TotalEvents   = 0;

}

void ExternalSort::End()
{
  Records.clear();
}

void ExternalSort::NewRecord(int        TYPE,
                             int        CPU,
                             int        Ptask,
                             int        Task,
                             int        Thread,
                             prv_time_t timestamp,
                             string     ascii_record)
{
  if (TYPE == PRV_EVENT)
  {
    CurrentEvent.FillRecord(TYPE,
                            CPU,
                            Ptask,
                            Task,
                            Thread,
                            timestamp,
                            ascii_record);

    CurrentEvent.serialize(EventsFile);
    TotalEvents++;
  }
  else
  {
    if (CurrentRecord == MAX_IN_FLIGHT_RECORDS)
    {
      FlushRecords();
    }

    /* Event records should be flush directly to a secondary file */
    Records[CurrentRecord].FillRecord(TYPE,
                                      CPU,
                                      Ptask,
                                      Task,
                                      Thread,
                                      timestamp,
                                      ascii_record);
    CurrentRecord++;
  }

  TotalRecords++;
}

void ExternalSort::GenerateFinalTrace(FILE* DefinitiveTrace)
{
  // First, close current open files
  IO_fclose(EventsFile);

  // First, sort states and communications file
  SortStatesAndComms();

  // Finally, merge this file with the sorted events file
  Merge(ExternalSort::StatesAndCommsFileName,
        ExternalSort::EventsFileName,
        DefinitiveTrace);

  // Delete temporal files
  unlink(ExternalSort::StatesAndCommsFileName.c_str());
  unlink(ExternalSort::EventsFileName.c_str());

  return;
}

void ExternalSort::SortStatesAndComms(void)
{
  string InFileName1, InFileName2, OutFileName;

  bool   Odd;
  size_t k, NumPairs, Count;

  /* Flush last records on the buffer */
  if (CurrentRecord != 0)
  {
    FlushRecords();
  }

  /* Here we can close the last 'StatesAndCommsFile' */
  IO_fclose(StatesAndCommsFile);

  /* And erase it, because is empty! */
  TemporalFileNames.str("");
  TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << NumFiles;
  unlink(TemporalFileNames.str().c_str());

  if (NumFiles == 1)
  {
    /* No need to merge! Rename temporal file to definitive states&comms file */

    if (rename((ExternalSort::TmpFilesPrefix+".0").c_str(),
               (ExternalSort::TmpFilesPrefix).c_str()) == -1)
    {
      die ("Unable to rename temporal sort files (%s to %s): %s\n",
           (ExternalSort::TmpFilesPrefix+".0").c_str(),
           (ExternalSort::TmpFilesPrefix).c_str(),
           strerror(errno));
    }

  }
  else if (NumFiles == 2)
  {
    /* Directly perform a single merge step! */
    Merge(ExternalSort::TmpFilesPrefix+".0",
          ExternalSort::TmpFilesPrefix+".1",
          ExternalSort::StatesAndCommsFileName);

    unlink((ExternalSort::TmpFilesPrefix+".0").c_str());
    unlink((ExternalSort::TmpFilesPrefix+".1").c_str());

    return;
  }
  else
  {
    // Merge temp files, 2 at a time, until only 2 remain, then proceed as in last case.
    while (NumFiles > 2)
    {
      // Merge ExtSortTemp.0 and ExtSortTemp.1 into ExtSortTempA.0,
      // merge ExtSortTemp.2 and ExtSortTemp.3 into ExtSortTempA.1, etc.
      NumPairs = NumFiles / 2;
      for (k = 0, Count = 0; k < NumPairs; k++)
      {
        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << Count++;
        InFileName1 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << Count++;
        InFileName2 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << k;
        OutFileName = TemporalFileNames.str();

        Merge(InFileName1, InFileName2, OutFileName);
      }
      // If the number of files is odd, just rename the last one.
      if (2 * NumPairs < NumFiles)
      {
        Odd = true;

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << Count;
        InFileName1 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << NumPairs;
        OutFileName = TemporalFileNames.str();

        if (rename(InFileName1.c_str(), OutFileName.c_str()) == -1)
        {
          die ("Unable to rename temporal sort files (%s to %s): %s\n",
               InFileName1.c_str(),
               OutFileName.c_str(),
               strerror(errno));
        }

        #ifdef DEBUG
           cout << "Renaming " << InFileName1 << " as " << OutFileName << endl;
        #endif
      }
      else
      {
        Odd = false;
      }

      // Remove the temp files from before the merge.
      for (k = 0; k < NumFiles; k++)
      {
        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << k;
        InFileName1 = TemporalFileNames.str();

        unlink(InFileName1.c_str());
      }

      // Get the new number of sorted files.
      NumFiles = NumPairs;

      if (Odd)
      {
        NumFiles++;
      }

      // If number of ExtSortTempA files is now 2, merge them with output to original file and return.
      if (NumFiles == 2)
      {
        Merge(ExternalSort::TmpFilesPrefix+"A.0",
              ExternalSort::TmpFilesPrefix+"A.1",
              ExternalSort::StatesAndCommsFileName);

        unlink((ExternalSort::TmpFilesPrefix+"A.0").c_str());
        unlink((ExternalSort::TmpFilesPrefix+"A.1").c_str());
        return;   // The sort is finished.
      }

      // Otherwise, merge pairs of files as above, but start at the top end so as to
      // incorporate any small remainder file.  Note that we now merge from the
      // ExtSortTempA files into ExtSortTemp files.
      NumPairs = NumFiles / 2;
      for (k = 0, Count = NumFiles - 1; k < NumPairs; k++)
      {
        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << Count--;
        InFileName1 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << Count--;
        InFileName2 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << k;
        OutFileName = TemporalFileNames.str();

        Merge(InFileName1, InFileName2, OutFileName);
      }

      // If the number of files is odd, just rename the last one.
      if (2 * NumPairs < NumFiles)
      {
        Odd = true;

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << 0;
        InFileName1 = TemporalFileNames.str();

        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << NumPairs;
        OutFileName = TemporalFileNames.str();

        if (rename(InFileName1.c_str(), OutFileName.c_str()) == -1)
        {
          die ("Unable to rename temporal sort files (%s to %s): %s\n",
               InFileName1.c_str(),
               OutFileName.c_str(),
               strerror(errno));
        }

        #ifdef DEBUG
           cout << "Renaming " << InFileName1 << " as " << OutFileName << endl;
        #endif
      }
      else
      {
        Odd = false;
      }

      // exit(EXIT_SUCCESS);

      // Remove the temp files from before the merge.
      for (k = 0; k < NumFiles; k++)
      {
        TemporalFileNames.str("");
        TemporalFileNames << ExternalSort::TmpFilesPrefix << "A." << k;
        InFileName1 = TemporalFileNames.str();

        unlink(InFileName1.c_str());
      }

      // Get the new number of sorted files.
      NumFiles = NumPairs;
      if (Odd)
      {
        NumFiles++;
      }

      // If number of ExtSortTemp files is now 2, merge them with output to original file and return.
      if (NumFiles == 2)
      {
        Merge(ExternalSort::TmpFilesPrefix+".0",
              ExternalSort::TmpFilesPrefix+".1",
              ExternalSort::StatesAndCommsFileName);

        unlink((ExternalSort::TmpFilesPrefix+".0").c_str());
        unlink((ExternalSort::TmpFilesPrefix+".1").c_str());
        return;   // The sort is finished.
      }
    }
  }

  return;
}

void ExternalSort::FlushRecords(void)
{
  // sort(Records.begin(), Records.end());
  stable_sort(Records.begin(), Records.begin()+CurrentRecord);

  for (size_t i = 0; i < CurrentRecord; i++)
  {
    Records[i].serialize(StatesAndCommsFile);
  }

  NumFiles++;

  /* Close current file and open the next one */

  IO_fclose(StatesAndCommsFile);

  TemporalFileNames.str("");
  TemporalFileNames << ExternalSort::TmpFilesPrefix << "." << NumFiles;

  if ( (StatesAndCommsFile = IO_fopen(TemporalFileNames.str().c_str(), "w")) == NULL)
  {
    die ("Unable to open temporal file for states and comms. '%s': %s\n",
         TemporalFileNames.str().c_str(),
         IO_get_error());
  }

  CurrentRecord = 0;
}

void ExternalSort::Merge(string InFileName1,
                         string InFileName2,
                         string OutFileName)
{
  FILE *InFile1, *InFile2, *OutFile;

#ifdef DEBUG
  cout << "Merging \"" << InFileName1 << " and \"" << InFileName2 << " to ";
  cout << "\"" << OutFileName << "\"" << endl;
#endif

  // Open each file
  if ( (InFile1 = IO_fopen(InFileName1.c_str(), "r")) == NULL)
  {
    die("Unable to open temporal input file (%s) during Paraver external sort: %s\n",
        InFileName1.c_str(),
        IO_get_error());
  }

  if ( (InFile2 = IO_fopen(InFileName2.c_str(), "r")) == NULL)
  {
    die("Unable to open temporal input file (%s) during Paraver external sort: %s\n",
        InFileName1.c_str(),
        IO_get_error());
  }

  if ( (OutFile = IO_fopen(OutFileName.c_str(), "w")) == NULL)
  {
    die("Unable to open temporal output file (%s) during Paraver external sort: %s\n",
        InFileName1.c_str(),
        IO_get_error());
  }

  Merge(InFile1, InFile2, OutFile);

  IO_fclose(InFile1);
  IO_fclose(InFile2);
  IO_fclose(OutFile);

  return;
}

void ExternalSort::Merge(string InFileName1,
                         string InFileName2,
                         FILE  *OutFile)
{
#ifdef DEBUG
  cout << "Merging \"" << InFileName1 << " and \"" << InFileName2 << " to ";
  cout << "externally open file" << endl;
#endif
  FILE *InFile1, *InFile2;

  // Open each file
  if ( (InFile1 = IO_fopen(InFileName1.c_str(), "r")) == NULL)
  {
    die("Unable to open temporal input file (%s) during Paraver external sort: %s\n",
        InFileName1.c_str(),
        IO_get_error());
  }

  if ( (InFile2 = IO_fopen(InFileName2.c_str(), "r")) == NULL)
  {
    die("Unable to open temporal input file (%s) during Paraver external sort: %s\n",
        InFileName2.c_str(),
        IO_get_error());
  }

  Merge(InFile1,
        InFile2,
        OutFile,
        true); // true to generate definitive events

  IO_fclose(InFile1);
  IO_fclose(InFile2);
  // IO_fclose(OutFile);

  return;
}

void ExternalSort::Merge(FILE *InFile1,
                         FILE *InFile2,
                         FILE *OutFile,
                         bool  DefinitiveTrace)
{
  vector<SimpleParaverRecord> Buffer1 = vector<SimpleParaverRecord> (MAX_IN_FLIGHT_RECORDS);
  vector<SimpleParaverRecord> Buffer2 = vector<SimpleParaverRecord> (MAX_IN_FLIGHT_RECORDS);
  vector<SimpleParaverRecord> Buffer3 = vector<SimpleParaverRecord> (MAX_IN_FLIGHT_RECORDS);

  bool   HaveData1, HaveData2, MoreData1, MoreData2;
  size_t NumRecords1, NumRecords2;
  size_t Index1, Index2, Index3;

  // Try to fill a buffer from InFile1.
  MoreData1   = true;
  NumRecords1 = FillBuffer(InFile1, Buffer1, MoreData1);

  // Try to fill a buffer from InFile2.
  MoreData2   = true;
  NumRecords2 = FillBuffer(InFile2, Buffer2, MoreData2);

  Index1 = 0;
  Index2 = 0;
  Index3 = 0;

  // Each HaveData boolean indicates if we have more unmerged data.  This data could be in
  // the corresponding NextWord variable, the corresponding Buffer, or both.
  HaveData1 = MoreData1 || (Index1 < NumRecords1);
  HaveData2 = MoreData2 || (Index2 < NumRecords2);

  while (HaveData1 && HaveData2)
  {
    if (Index1 == NumRecords1)
    {
      NumRecords1 = FillBuffer(InFile1, Buffer1, MoreData1);
      if (NumRecords1 == 0)
      {
        break;   // We are out of data from InFile1.
      }
      else
      {
        Index1 = 0;
      }
    }

    if (Index2 == NumRecords2)
    {
      NumRecords2 = FillBuffer(InFile2, Buffer2, MoreData2);

      if (NumRecords2 == 0)
      {
        break;   // We are out of data from InFile2.
      }
      else
      {
        Index2 = 0;
      }
    }

    if ( Buffer1[Index1] < Buffer2[Index2])
    {
      Copy(Buffer1[Index1], Buffer3, Index3, OutFile, DefinitiveTrace);
      Index1++;
    }
    else
    {
      Copy(Buffer2[Index2], Buffer3, Index3, OutFile, DefinitiveTrace);
      Index2++;
    }

    HaveData1 = MoreData1 || (Index1 < NumRecords1);
    HaveData2 = MoreData2 || (Index2 < NumRecords2);
  }

  HaveData1 = MoreData1 || (Index1 < NumRecords1);

  // Handle remaining data in either file.
  while (HaveData1)
  {
    if (Index1 == NumRecords1)
    {
      NumRecords1 = FillBuffer(InFile1, Buffer1, MoreData1);

      if (NumRecords1 == 0)
      {
        break;   // We are out of data from InFile1.
      }
      else
      {
        Index1 = 0;
      }
    }

    Copy(Buffer1[Index1], Buffer3, Index3, OutFile, DefinitiveTrace);
    Index1++;
    HaveData1 = MoreData1 || (Index1 < NumRecords1);
  }

  HaveData2 = MoreData2 || (Index2 < NumRecords2);
  while (HaveData2)
  {
    if (Index2 == NumRecords2)
    {
      NumRecords2 = FillBuffer(InFile2, Buffer2, MoreData2);

      if (NumRecords2 == 0)
      {

        break;   // We are out of data from InFile2.
      }
      else
      {
        Index2 = 0;
      }
    }

    Copy(Buffer2[Index2], Buffer3, Index3, OutFile, DefinitiveTrace);
    Index2++;

    HaveData2 = MoreData2 || (Index2 < NumRecords2);
  }

  // Flush any remaining data from the output buffer.
  for (size_t k = 0; k < Index3; k++)
  {
    if (DefinitiveTrace)
    {
      Buffer3[k].toTracefile(OutFile);
    }
    else
    {
      Buffer3[k].serialize(OutFile);
    }
  }
}


size_t ExternalSort::FillBuffer(FILE                        *InFile,
                                vector<SimpleParaverRecord>& Buffer,
                                bool&                        MoreData)
{
  size_t k = 0;

  while (MoreData && (k < MAX_IN_FLIGHT_RECORDS))
  {
    if (!LoadRecord(InFile, Buffer[k]))
    {
      MoreData = false;
      k--;
    }
    else
    {
      MoreData = true;
    }

    k++;
  }
  return k;
}

bool ExternalSort::LoadRecord(FILE*                InFile,
                              SimpleParaverRecord& Record)
{
  char*   current_ascii_record  = NULL;
  size_t  current_record_length = 0;
  ssize_t bytes_read;

  if ( (bytes_read = getline(&current_ascii_record, &current_record_length, InFile)) == -1)
  {
    if (feof(InFile))
    {
      return false;
    }

    die("Error reading temporal Paraver file: %s\n",
        strerror(errno));
  }

  if (!Record.deserialize(current_ascii_record))
  {
    die("Error loading record from temporal Paraver file: %s\nLine: %s",
        strerror(errno),
        current_ascii_record);
  }

  free(current_ascii_record);

  return true;
}

void ExternalSort::Copy(SimpleParaverRecord&         Record,
                        vector<SimpleParaverRecord>& Buffer,
                        size_t&                      Index,
                        FILE                        *OutFile,
                        bool                         DefinitiveTrace)
{

  if (Index == MAX_IN_FLIGHT_RECORDS)
  {
    for (size_t i = 0; i < MAX_IN_FLIGHT_RECORDS; i++)
    {
      if (DefinitiveTrace)
      {
        Buffer[i].toTracefile(OutFile);
      }
      else
      {
        Buffer[i].serialize(OutFile);
      }
    }

    Index = 0;
  }

  Buffer[Index] = Record;
  Index++;
}
