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

#ifndef _TASKTRANSLATIONINFO_H
#define _TASKTRANSLATIONINFO_H

#include <Error.hpp>
using cepba_tools::Error;

#include "ParaverRecord.hpp"
#include "TranslatorRecord.hpp"

using std::pair;

#ifdef NEW_DIMEMAS_TRACE
typedef pair<INT32, INT64> Block_t;
#else
typedef DimBlock Block_t;
#endif

class TaskTranslationInfo : public Error
{
 private:
  INT32 TaskId;
  INT32 ThreadId;
  double TimeFactor; /* To adjust CPU Burst values */
  vector<ParaverRecord_t> RecordStack;
  vector<Block_t> MPIBlockIdStack;
  vector<Block_t> UserBlockIdStack;
  vector<Block_t> ClusterBlockIdStack;
  UINT64 LastBlockEnd;

  // For matching non-block collectives with its MPI_Wait
  vector<GlobalOp_t> NonBlockingGlopsInFlight;
  bool MPIWaitWithCommunication;

  /* For CUDA / OpenCL version */
  UINT64 LastGPUBurstBlock;
  bool FirstCUDARead;
  bool FirstOCLRead;
  bool OngoingDeviceSync;
  INT32 StreamIdToSync;
  INT32 AcceleratorThread;
  vector<Block_t> CUDABlockIdStack;
  vector<Block_t> OCLBlockIdStack;
  bool OCLFinishComm;

  /* For OMP version */
  INT32 OpenMP_thread;
  UINT32 OpenMP_nesting_level;

  /* MPI_Event_Values type is defined in 'EventEncoding.h', on common-files */
  GlobalOp_t PartialGlobalOp;
  INT32 GlobalOpFields;
  bool PendingGlobalOp;
  bool OutsideComms;
  bool WrongComms;
  bool NonDeterministicComms;
  bool DisorderedRecords;
  bool FlushClusterStack;

  bool CommunicationPrimitivePrinted;

  bool GenerateFirstIdle;
  bool FirstClusterRead;

  FILE* TemporaryFile;
  char* TemporaryFileName;

  double IprobeMissesThreshold;
  double TestMissesThreshold;

  bool GenerateMPIInitBarrier;
  bool MPIInitBarrierWritten;

  bool PreviouslySimulatedTrace;

  bool BurstCounterGeneration;
  INT32 BurstCounterType;
  double BurstCounterFactor;

  bool OngoingIprobe;
  bool OngoingTest;
  bool IprobeBurstFlushed;
  bool TestBurstFlushed;
  bool FilePointerAvailable;
  bool FirstPrint;

  bool InCUDAHostToHost;

  /* Needed to access counters from others TaskTranslators */
  vector<vector<TaskTranslationInfo*> >* AllTranslationInfo;


 public:
  TaskTranslationInfo( INT32 TaskId,
                       INT32 ThreadId,
                       double TimeFactor,
                       UINT64 InitialTime,
                       bool GenerateFirstIdle,
                       bool EmptyTask,
                       double IprobeMissesThreshold,
                       double TestMissesThreshold,
                       bool BurstCounterGeneration,
                       INT32 BurstCounterType,
                       double BurstCounterFactor,
                       bool GenerateMPIInitBarrier,
                       bool PreviouslySimulatedTrace,
                       vector<vector<TaskTranslationInfo*> >* AllTranslationInfo,
                       INT32 AcceleratorThread,
                       INT32 OpenMP_thread,
                       char* TemporaryFileName = NULL,
                       FILE* TemporaryFile     = NULL );

  ~TaskTranslationInfo();

  INT32 GetTaskId( void )
  {
    return TaskId;
  };

  bool PushRecord( ParaverRecord_t Record );

  void SetPendingGlobalOp( bool PendingGlobalOp )
  {
    this->PendingGlobalOp = PendingGlobalOp;
  }
  bool LastFlush( void );

  bool Merge( FILE* DimemasFile );

  bool GetOutsideComms( void )
  {
    return OutsideComms;
  };

  bool GetWrongComms( void )
  {
    return WrongComms;
  };

  bool GetNonDeterministicComms( void )
  {
    return NonDeterministicComms;
  };

  bool GetDisorderedRecords( void )
  {
    return DisorderedRecords;
  };

  bool GetMPIInitBarrierWritten( void )
  {
    return MPIInitBarrierWritten;
  };

  /* Some other statistics */
  unsigned int send_counter;
  unsigned int isend_counter;
  unsigned int recv_counter;
  unsigned int irecv_counter;
  unsigned int wait_counter;
  unsigned int glop_counter;

  unsigned int pendent_i_Send_counter;
  unsigned int pendent_i_Recv_counter;
  unsigned int pendent_Glop_counter;
  int pseudo_logic_recv_events;

 private:
  bool ReorderAndFlush( void );

  bool ToDimemas( ParaverRecord_t Record );
  bool ToDimemas( Event_t CurrentEvent );
  bool ToDimemas( PartialCommunication_t CurrentComm );
  bool ToDimemas( GlobalOp_t CurrentGlobOp );

  void Event2GlobalOp( Event_t CurrentEvent );
  void FinalizeGlobalOp( void );
  bool CheckIprobeCounters( Event_t CurrentEvent );
  bool CheckTestCounters( Event_t CurrentEvent );
  bool GenerateBurst( INT32 TaskId, INT32 ThreadId, UINT64 Timestamp );

  bool PrintPseudoCommunicationEndpoint( INT32 CommType,
                                         INT32 TaskId,
                                         INT32 ThreadId,
                                         INT32 PartnerTaskId,
                                         INT32 PartnerThreadId,
                                         INT32 Size,
                                         INT32 Tag,
                                         INT32 CommId );

  bool GenerateGPUBurst( INT32 TaskId, INT32 ThreadId, UINT64 Timestamp, UINT64 LastBlock );

  void PrintStack( void );
};

typedef TaskTranslationInfo* TaskTranslationInfo_t;

#endif /* _TASKTRANSLATIONINFO_H */
