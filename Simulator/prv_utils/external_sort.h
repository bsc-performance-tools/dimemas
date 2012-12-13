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

#ifndef _EXTERNAL_MERGE_SORT_H_
#define _EXTERNAL_MERGE_SORT_H_

#include "paraver_records.h"

#include <vector>
using std::vector;


/*
 * That could be changed for actual memory usage
 */
#define MAX_IN_FLIGHT_RECORDS 10000

class ExternalSort
{
  private:
    static string         TmpDir;
    static string         TmpFilesPrefix;
    static string         EventsFileName;
    static string         StatesAndCommsFileName;

    FILE                       *EventsFile;
    FILE                       *StatesAndCommsFile;

    size_t                      TotalRecords;

    size_t                      CurrentRecord;
    vector<SimpleParaverRecord> Records;

    size_t                      TotalEvents;
    SimpleParaverRecord         CurrentEvent;

    size_t                      NumFiles;

    ostringstream               TemporalFileNames;

  public:
    ExternalSort() {};

    void Init();

    void End();

    void NewRecord(int        TYPE,
                   int        CPU,
                   int        Ptask,
                   int        Task,
                   int        Thread,
                   prv_time_t timestamp,
                   string     ascii_record);

    void GenerateFinalTrace(FILE* DefinitiveTrace);

  private:

    void FlushRecords(void);



    /*
    void SelectMinRecord(vector<size_t>&      RemainingRecords,
                         size_t&              RemainingEvents,
                         SimpleParaverRecord& SelectedRecord);
    */
    void SortStatesAndComms(void);

    void Merge(string InFileName1,
               string InFileName2,
               string OutFileName);

    void Merge(string InFileName1,
               string InFileName2,
               FILE* OutFile);

    void Merge(FILE *InFile1,
               FILE *InFile2,
               FILE *OutFile,
               bool   DefinitiveTrace = false);

    size_t FillBuffer(FILE* InFile,
                      vector<SimpleParaverRecord>& Buffer,
                      bool& MoreData);


    bool   LoadRecord(FILE                *InFile,
                      SimpleParaverRecord& Record);


    void Copy(SimpleParaverRecord&         Record,
              vector<SimpleParaverRecord>& Buffer,
              size_t&                      Index,
              FILE                        *OutFile,
              bool                         DefinitiveTrace = false);
};

#endif
