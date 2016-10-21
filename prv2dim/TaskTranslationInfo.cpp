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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <Macros.h>

#include "TaskTranslationInfo.hpp"
#include "Dimemas2Prv.h"
#include "EventEncoding.h"
#include "Dimemas_Generation.h"

#include <algorithm>
using std::sort;

#include <iostream>
using std::cout;

#include <iomanip>
#include <sstream>
using std::ostringstream;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

TaskTranslationInfo::TaskTranslationInfo(INT32 TaskId,
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
					 vector < vector <
					 TaskTranslationInfo *
					 > >*AllTranslationInfo,
					 INT32 AcceleratorThread,
					 char *TemporaryFileName,
					 FILE * TemporaryFile)
{
	this->AllTranslationInfo = AllTranslationInfo;

	const char *tmp_dir_default = "/tmp";
	char *tmp_dir;

	this->TaskId = TaskId;
	this->ThreadId = ThreadId;

	/* Burst Counter initialization */
	this->BurstCounterGeneration = BurstCounterGeneration;
	this->BurstCounterType = BurstCounterType;
	this->BurstCounterFactor = BurstCounterFactor;

	this->GenerateMPIInitBarrier = GenerateMPIInitBarrier;
	MPIInitBarrierWritten = false;

	this->PreviouslySimulatedTrace = PreviouslySimulatedTrace;

	if (TemporaryFile == NULL) {
		FilePointerAvailable = false;
		this->TemporaryFileName = TemporaryFileName;
		this->TemporaryFile = NULL;
	} else {
		FilePointerAvailable = true;
		this->TemporaryFile = TemporaryFile;
		this->TemporaryFileName = TemporaryFileName;
	}

	PendingGlobalOp = false;
	this->TimeFactor = TimeFactor;
	LastBlockEnd = 0;
	FirstPrint = false;
	FlushClusterStack = false;
	FirstClusterRead = false;
	FirstCUDARead = false;
	FirstOCLRead = false;
	this->AcceleratorThread = AcceleratorThread;

	if (!FilePointerAvailable) {
		if ((TemporaryFile = fopen(TemporaryFileName, "a")) == NULL) {
			SetError(true);
			SetErrorMessage("unable to open output temporary file",
					strerror(errno));
		}
	}
#ifdef DEBUG
	cout << "Printing Initial CPU Burst [";

	cout.width(3);
	cout.fill('0');
	cout << TaskId << ":";

	cout.width(2);
	cout.fill('0');
	cout << ThreadId << "]";
	cout << endl;
#endif

	if (ThreadId == 1) {	/* First CPU Burst only appears in the Host (main) thread */
		if (Dimemas_CPU_Burst(TemporaryFile,
				      TaskId - 1, ThreadId - 1, 0.0) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
		}
	}

	else if (AcceleratorThread == ACCELERATOR_KERNEL) {	/*GPU thread case, needs CPU burst in first record */
		if (Dimemas_CPU_Burst(TemporaryFile,
				      TaskId - 1, ThreadId - 1, 0.0) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
		}
	}

	this->GenerateFirstIdle = GenerateFirstIdle;

	if (!EmptyTask && GenerateFirstIdle && ThreadId == 1) {

#ifdef DEBUG
		cout << "Printing Initial Idle Block Begin [";
		cout.width(3);
		cout.fill('0');
		cout << TaskId << ":";

		cout.width(2);
		cout.fill('0');
		cout << ThreadId << "]";
		cout << endl;
#endif

		if (Dimemas_Block_Begin(TemporaryFile,
					TaskId - 1, ThreadId - 1,
					(INT64) 0, (INT64) 1) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
		}
#ifdef DEBUG
		cout << "Printing Initial CPU burst [";
		cout.width(3);
		cout.fill('0');
		cout << TaskId << ":";

		cout.width(2);
		cout.fill('0');
		cout << ThreadId << "]";
		cout << endl;
#endif

		if (Dimemas_CPU_Burst(TemporaryFile,
				      TaskId - 1, ThreadId - 1,
				      1.0 * InitialTime * TimeFactor) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
		}
#ifdef DEBUG
		cout << "Printing Initial CPU Block End [";
		cout.width(3);
		cout.fill('0');
		cout << TaskId << ":";

		cout.width(2);
		cout.fill('0');
		cout << ThreadId << "]";
		cout << endl;
#endif

		if (Dimemas_Block_End(TemporaryFile,
				      TaskId - 1, ThreadId - 1, (INT64) 0) < 0)
		{
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
		}
		LastBlockEnd = InitialTime;
	}

	if (!FilePointerAvailable) {
		if (fclose(TemporaryFile) != 0) {
			SetError(true);
			SetErrorMessage
			    ("Error closing Dimemas trace file to share descriptors",
			     strerror(errno));
			return;
		}
	}

	/* DEBUG
	   cout << "Initial Time = " << InitialTime << endl; */

	if (!EmptyTask && !GenerateFirstIdle) {
		LastBlockEnd = InitialTime;
	}

	this->IprobeMissesThreshold = IprobeMissesThreshold;
	this->TestMissesThreshold = TestMissesThreshold;

	OngoingIprobe = false;
	OngoingTest = false;
	IprobeBurstFlushed = false;
	TestBurstFlushed = false;
	OutsideComms = false;
	WrongComms = false;
	NonDeterministicComms = false;
	DisorderedRecords = false;
	// RecordStack.empty();

	send_counter = 0;
	isend_counter = 0;
	recv_counter = 0;
	irecv_counter = 0;
	wait_counter = 0;
	glop_counter = 0;

	pendent_i_Send_counter = 0;
	pendent_i_Recv_counter = 0;
	pendent_Glop_counter = 0;

	OngoingDeviceSync = false;
	StreamIdToSync = -1;
	LastGPUBurstBlock = InitialTime;
	OCLFinishComm = false;
}

TaskTranslationInfo::~TaskTranslationInfo(void)
{
	{
		unlink(TemporaryFileName);
		free(TemporaryFileName);
	}
	return;
}

bool TaskTranslationInfo::PushRecord(ParaverRecord_t Record)
{
	ParaverRecord_t LastRecord;
	Event_t NewEvent;

	if (RecordStack.size() > 0) {
		LastRecord = RecordStack.back();

		/* Where we need to flush */
		if (LastRecord->operator<(*Record)) {
			if (!ReorderAndFlush())
				return false;

			RecordStack.clear();
			RecordStack.push_back(Record);
		} else if (LastRecord->operator==(*Record)) {
			RecordStack.push_back(Record);

			if ((NewEvent = dynamic_cast < Event_t > (Record)) != NULL) {	/* Check if there is block end */
				if (NewEvent->IsMPIBlockEnd()) {
					if (!ReorderAndFlush())
						return false;

					RecordStack.clear();
				}
			}
		} else if (LastRecord->operator>(*Record)) {
			DisorderedRecords = true;
		}
	} else {
		RecordStack.push_back(Record);
	}
	return true;
}

bool TaskTranslationInfo::LastFlush(void)
{
	bool result;

	result = ReorderAndFlush();

	if (!result)
		return false;
	else {
		if (FilePointerAvailable) {
			if (fclose(TemporaryFile) != 0) {
				SetError(true);
				SetErrorMessage
				    ("Error closing Dimemas trace file to share descriptors",
				     strerror(errno));
				return false;
			}
		}
	}

	return true;
}

bool TaskTranslationInfo::Merge(FILE * DimemasFile)
{
	struct stat FileStat;
	char MergeMessage[128];
	off_t TemporaryFileSize = 0, CurrentPosition;

	bool SizeAvailable;
	INT32 CurrentPercentage = 0;
	INT32 PercentageRead = 0;

	char Buffer[1048576];
	size_t EffectiveBytes;

	if ((TemporaryFile = fopen(TemporaryFileName, "r")) == NULL) {
		SetError(true);
		SetErrorMessage("unable to open output temporary file",
				strerror(errno));
		return false;
	}

	if (fstat(fileno(TemporaryFile), &FileStat) < 0) {
		SizeAvailable = false;
	} else {
		SizeAvailable = true;
		TemporaryFileSize = FileStat.st_size;
		sprintf(MergeMessage, "   * Merging %04d:%03d ", this->TaskId,
			this->ThreadId);
	}

	fseek(TemporaryFile, 0, SEEK_SET);

	while (!feof(TemporaryFile)) {
		EffectiveBytes = fread(Buffer,
				       sizeof(char), sizeof(Buffer),
				       TemporaryFile);

		if (EffectiveBytes != sizeof(Buffer) && ferror(TemporaryFile)) {
			LastError = strerror(errno);
			if (!FilePointerAvailable) {
				fclose(TemporaryFile);
			}
			return false;
		}

		if (fwrite(Buffer,
			   sizeof(char),
			   EffectiveBytes, DimemasFile) != EffectiveBytes) {
			LastError = strerror(errno);
			if (!FilePointerAvailable)
				fclose(TemporaryFile);
			return false;
		}

		if (SizeAvailable) {
			CurrentPosition = ftello(TemporaryFile);
			PercentageRead =
			    lround(100.0 * CurrentPosition / TemporaryFileSize);

			if (PercentageRead > CurrentPercentage) {
				CurrentPercentage = PercentageRead;
				SHOW_PERCENTAGE_PROGRESS(stdout, MergeMessage,
							 CurrentPercentage);
			}
		}
	}

	fclose(TemporaryFile);

	return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/
bool TaskTranslationInfo::ReorderAndFlush(void)
{
	UINT32 i;

	if (RecordStack.size() == 0)
		return true;

#ifdef DEBUG
	fprintf(stdout, "[%03d:%02d] BEFORE sort stack\n", TaskId, 1);
	for (i = 0; i < RecordStack.size(); i++)
		cout << (*RecordStack[i]);
#endif

	/* There is a communication on the stack, we can flush it! */
	if (RecordStack.size() > 1) {
		if (MPIBlockIdStack.size() > 0 || CUDABlockIdStack.size() > 0
		    || OCLBlockIdStack.size() > 0) {
#ifdef DEBUG
			fprintf(stdout, "[%03d:%02d] InBlockComparison\n",
				TaskId, 1);
#endif
			sort(RecordStack.begin(), RecordStack.end(),
			     InBlockComparison());
		} else {
#ifdef DEBUG
			fprintf(stdout, "[%03d:%02d] OutBlockComparison\n",
				TaskId, 1);
#endif
			sort(RecordStack.begin(), RecordStack.end(),
			     OutBlockComparison());
		}
	}
#ifdef DEBUG
	fprintf(stdout, "[%03d:%02d] AFTER sort stack\n", TaskId, 1);
	for (i = 0; i < RecordStack.size(); i++)
		cout << (*RecordStack[i]);
#endif

	if (!FilePointerAvailable) {
		if ((TemporaryFile = fopen(TemporaryFileName, "a")) == NULL) {
			char CurrentError[128];
			sprintf(CurrentError,
				"Error opening temporary file to task %02d",
				TaskId);
			SetErrorMessage(CurrentError, strerror(errno));
		}
	}

	for (i = 0; i < RecordStack.size(); i++) {
		ParaverRecord_t CurrentRecord;
		CurrentRecord = RecordStack[i];
		if (!ToDimemas(CurrentRecord)) {
			if (!FilePointerAvailable)
				fclose(TemporaryFile);

			return false;
		}

		delete CurrentRecord;
	}

	/* TEST */
	if (PendingGlobalOp)
		FinalizeGlobalOp();

	if (!FilePointerAvailable)
		fclose(TemporaryFile);

	RecordStack.clear();

	return true;
}

/* ToDimemas (GENERIC) ********************************************************/
bool TaskTranslationInfo::ToDimemas(ParaverRecord_t Record)
{
	Event_t CurrentEvent;
	PartialCommunication_t CurrentComm;
	GlobalOp_t CurrentGlobOp;
	bool InnerResult;

	if ((CurrentEvent = dynamic_cast < Event_t > (Record)) != NULL) {
		return ToDimemas(CurrentEvent);
	} else
	    if ((CurrentComm =
		 dynamic_cast < PartialCommunication_t > (Record)) != NULL) {
		return ToDimemas(CurrentComm);
	} else if ((CurrentGlobOp = dynamic_cast < GlobalOp_t > (Record)) !=
		   NULL) {
		return ToDimemas(CurrentGlobOp);
	} else {
		return false;
	}

	return true;
}

/* ToDimemas (EVENT) **********************************************************/
bool TaskTranslationInfo::ToDimemas(Event_t CurrentEvent)
{
	Block_t CurrentBlock;

	INT32 TaskId, ThreadId;
	INT32 Type;
	INT64 Value;
	UINT64 Timestamp;
	double OnTraceTime;

#ifdef DEBUG
	cout << "Processing User Event: " << *CurrentEvent;
#endif

	/* We must ensure that current event only has one type/value pair */
	if (CurrentEvent->GetTypeValueCount() != 1)
		return false;

	/* Pseudo logical receive events must be erased during the translation */
	if (CurrentEvent->GetFirstType() == 9 ||
	    CurrentEvent->GetFirstType() == 10 ||
	    CurrentEvent->GetFirstType() == 11 ||
	    CurrentEvent->GetFirstType() == 12 ||
	    CurrentEvent->GetFirstType() == 13 ||
	    CurrentEvent->GetFirstType() == 14 ||
	    CurrentEvent->GetFirstType() == 15 ||
	    CurrentEvent->GetFirstType() == 16) {
		return true;
	}

	TaskId = CurrentEvent->GetTaskId() - 1;
	ThreadId = CurrentEvent->GetThreadId() - 1;
	Type = CurrentEvent->GetFirstType();
	Value = CurrentEvent->GetFirstValue();
	Timestamp = CurrentEvent->GetTimestamp();

	if (PendingGlobalOp) {
		Event2GlobalOp(CurrentEvent);
	}

	if (Type == MPITYPE_COLLECTIVE && Value != 0) {

		DimBlock CurrentBlock;
		DimCollectiveOp GlobalOpId;

		PendingGlobalOp = true;

		CurrentBlock =
		    MPIEventEncoding_DimemasBlockId((MPI_Event_Values) Value);
		GlobalOpId = MPIEventEncoding_GlobalOpId(CurrentBlock);

		PartialGlobalOp = new GlobalOp(Timestamp, CurrentEvent->GetCPU(), CurrentEvent->GetAppId(), CurrentEvent->GetTaskId(), CurrentEvent->GetThreadId(), 0, 0, 0,	/* CommunicatorId, SendSize, RecvSize */
					       GlobalOpId, false);

		GlobalOpFields = 1;
	}

	if (!CheckIprobeCounters(CurrentEvent)) {
		return false;
	}

	if (!CheckTestCounters(CurrentEvent)) {
		return false;
	}

	if (!MPIEventEncoding_Is_MPIBlock((INT64) Type) && !MPIEventEncoding_Is_UserBlock((INT64) Type) && !ClusterEventEncoding_Is_ClusterBlock((INT64) Type) && !CUDAEventEncoding_Is_CUDABlock((INT64) Type) && !OCLEventEncoding_Is_OCLBlock((INT64) Type)) {	/* It's a generic event! */

#ifdef DEBUG
		cout << "Printing User Event: " << *CurrentEvent;
#endif

		/* Test: if event appears in the middle of a CPU burst it breaks the
		 * burst */
		if (LastBlockEnd != Timestamp &&
		    MPIBlockIdStack.size() == 0 &&
		    CUDABlockIdStack.size() == 0 && OCLBlockIdStack.size() == 0)
		{
			if (Type != FLUSHING_EV &&
			    Type != MPITYPE_PROBE_SOFTCOUNTER &&
			    Type != MPITYPE_PROBE_TIMECOUNTER) {
				if (!GenerateBurst(TaskId, ThreadId, Timestamp)) {
					return false;
				}

				LastBlockEnd = Timestamp;
			}
		}

		if (OngoingDeviceSync && AcceleratorThread != ACCELERATOR_NULL) {
			if (Type == CUDA_SYNCH_STREAM_EV)	// || Type == OCL_SYNCH_STREAM_EV)
			{
				StreamIdToSync = (INT32) Value;
			}
		}
#ifdef DEBUG
		cout << "Printing User Event: " << *CurrentEvent;
#endif

		if (Dimemas_User_Event(TemporaryFile,
				       TaskId, ThreadId, (INT64) Type,
				       Value) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
			return false;
		}
	}

	if (CUDAEventEncoding_Is_CUDABlock(Type) &&
	    AcceleratorThread != ACCELERATOR_NULL) {
		/* It's a CUDA event */
		if (CUDAEventEncoding_Is_BlockBegin(Value)) {
			if (Timestamp > LastBlockEnd) {
				if (AcceleratorThread == ACCELERATOR_HOST) {	/* Burst are only generated in the Host thread, not in the device
										   threads */
					if (!GenerateBurst
					    (TaskId, ThreadId, Timestamp)) {
						return false;
					}
				}
			}

			if (Value == CUDA_THREADSYNCHRONIZE_VAL ||
			    Value == CUDA_STREAMSYNCHRONIZE_VAL) {
				OngoingDeviceSync = true;
			}
#ifdef DEBUG
			cout << "Printing CUDA Opening Event: " <<
			    *CurrentEvent;
#endif
			if (AcceleratorThread == ACCELERATOR_KERNEL && Value != CUDA_THREADSYNCHRONIZE_VAL && Value != CUDA_STREAMSYNCHRONIZE_VAL) {	/* Device threads must include a synchronization before they start

																			   /* Idle block start */
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId, ThreadId,
							(INT64) 0,
							(INT64) 1) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
				}

				/* Synchronization before the actual call */
				if (Dimemas_NX_Recv(TemporaryFile,
						    TaskId, ThreadId,
						    TaskId, 0, 0, 0,
						    (INT64) CUDA_TAG) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				/* Idle block end */
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId, ThreadId,
							(INT64) 0,
							(INT64) 0) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
				}
			}

			/* CUDA block begin */
			if (Dimemas_Block_Begin(TemporaryFile,
						TaskId,
						ThreadId, (INT64) Type,
						(INT64) Value) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			FirstCUDARead = true;

			/* Block management */
			CurrentBlock.first = Type;
			CurrentBlock.second = Value;

			CUDABlockIdStack.push_back(CurrentBlock);

			LastBlockEnd = Timestamp;
		} else {
			if (CUDABlockIdStack.size() == 0
			    && AcceleratorThread != ACCELERATOR_NULL) {
				if (FirstCUDARead) {
					// There is a event closing without is opening in the middle of the
					// trace
					cout <<
					    "WARNING: unbalanced CUDA blocks on original trace"
					    << endl;
					cout << "Task " << TaskId +
					    1 << " Thread: " << ThreadId +
					    1 << " ";
					cout << "Time " << Timestamp << endl;
				}
			} else {
				CurrentBlock = CUDABlockIdStack.back();
				CUDABlockIdStack.pop_back();

				/* A kernel execution in the device, characterized as a GPU burst */
				if ((CurrentBlock.second == CUDA_LAUNCH_VAL ||
				     CurrentBlock.second ==
				     CUDA_CONFIGURECALL_VAL)
				    && AcceleratorThread == ACCELERATOR_KERNEL) {
					if (!GenerateGPUBurst
					    (TaskId, ThreadId, Timestamp,
					     LastBlockEnd)) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}

				/* Generation of pseudo-collectives to simulate the thread/stream
				   synchronizations */
				if (CurrentBlock.second ==
				    CUDA_THREADSYNCHRONIZE_VAL
				    || CurrentBlock.second ==
				    CUDA_STREAMSYNCHRONIZE_VAL) {
					if (AcceleratorThread == ACCELERATOR_HOST) {	/* Host thread */
						if (StreamIdToSync != -1) {	/* Synchronization to a given stream */
							if (Dimemas_Global_OP(TemporaryFile, TaskId, ThreadId, 0,	/* MPI_BARRIER  */
									      (StreamIdToSync * (-1)) - 1, TaskId, 0, 0, 0) < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						} else {	/* Synchronization to all threads */
							if (Dimemas_Global_OP(TemporaryFile, TaskId, ThreadId, 0,	/* MPI_BARRIER  */
									      -1,	/* All threads! */
									      TaskId,
									      0,
									      0,
									      0)
							    < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
					} else if (AcceleratorThread == ACCELERATOR_KERNEL) {	/* Device threads */
						if (Dimemas_Global_OP(TemporaryFile, TaskId, ThreadId, 0,	/* MPI_BARRIER  */
								      -1,	/* All threads! */
								      TaskId, 0,
								      0,
								      0) < 0) {
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
					}

					OngoingDeviceSync = false;
					StreamIdToSync = -1;	//reset value to All threads
				}
			}

#ifdef DEBUG
			cout << "Printing CUDA Closing Event: " <<
			    *CurrentEvent;
#endif
			if (Dimemas_Block_End(TemporaryFile,
					      TaskId, ThreadId,
					      (INT64) Type) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			LastBlockEnd = Timestamp;
		}
	}

	if (OCLEventEncoding_Is_OCLBlock(Type) &&
	    AcceleratorThread != ACCELERATOR_NULL) {
		/* It's a OCL event */
		if (OCLEventEncoding_Is_BlockBegin(Value)) {
			if (Timestamp > LastBlockEnd) {
				if (AcceleratorThread == ACCELERATOR_HOST) {	/* Burst are only generated in the Host thread, not in the device
										   threads (except kernel executions) */
					if (!GenerateBurst
					    (TaskId, ThreadId, Timestamp)) {
						return false;
					}
				}
			}

			if (Value == OCL_FINISH_VAL) {
				OngoingDeviceSync = true;
			}
#ifdef DEBUG
			cout << "Printing OCL Opening Event: " << *CurrentEvent;
#endif
			if (AcceleratorThread == ACCELERATOR_KERNEL && Value != OCL_FINISH_VAL && Type != OCL_KERNEL_NAME_EV) {	/* Device threads must include a synchronization before they start
																   /* Idle block start */
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId, ThreadId,
							(INT64) 0,
							(INT64) 1) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
				}

				/* Synchronization before the actual call */
				if (Dimemas_NX_Recv(TemporaryFile,
						    TaskId, ThreadId,
						    TaskId, 0, 0, 0,
						    (INT64) OCL_TAG) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				/* Idle block end */
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId, ThreadId,
							(INT64) 0,
							(INT64) 0) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
				}
			}

			/* OpenCL block begin */
			if (Dimemas_Block_Begin(TemporaryFile,
						TaskId,
						ThreadId, (INT64) Type,
						(INT64) Value) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			if (AcceleratorThread == ACCELERATOR_KERNEL &&
			    Type == OCL_ACCELERATOR_CALL_EV &&
			    Value == OCL_ENQUEUE_NDRANGE_KERNEL_VAL)
				LastGPUBurstBlock = Timestamp;

			FirstOCLRead = true;

			/* Block management */
			CurrentBlock.first = Type;
			CurrentBlock.second = Value;

			OCLBlockIdStack.push_back(CurrentBlock);

			LastBlockEnd = Timestamp;
		} else {
			if (OCLBlockIdStack.size() == 0
			    && AcceleratorThread != ACCELERATOR_NULL) {
				if (FirstOCLRead) {
					// There is a event closing without is opening in the middle of the
					// trace
					cout <<
					    "WARNING: unbalanced OCL blocks on original trace"
					    << endl;
					cout << "Task " << TaskId +
					    1 << " Thread: " << ThreadId +
					    1 << " ";
					cout << "Time " << Timestamp << endl;
				}
			} else {
				CurrentBlock = OCLBlockIdStack.back();
				OCLBlockIdStack.pop_back();

				if (CurrentBlock.first == OCL_ACCELERATOR_CALL_EV && CurrentBlock.second == OCL_ENQUEUE_NDRANGE_KERNEL_ACC_VAL && AcceleratorThread == ACCELERATOR_KERNEL) {	/* Kernel side launch finish */
					if (!GenerateGPUBurst
					    (TaskId, ThreadId, Timestamp,
					     LastGPUBurstBlock)) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}

					/* To avoid having the send before the GPU burst */
					if (OCLFinishComm) {
#ifdef DEBUG
						cout <<
						    "Printing OCL Launch (Kernel sends): "
						    << *CurrentEvent;
#endif
						if (Dimemas_NX_Generic_Send
						    (TemporaryFile, TaskId,
						     ThreadId, TaskId, 0, 0, 0,
						     (INT64) OCL_TAG, 3) < 0) {
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
						OCLFinishComm = false;
					}
				}

				if (OCLEventEncoding_Is_OCLSchedblock
				    (CurrentBlock.first, CurrentBlock.second)
				    && AcceleratorThread == ACCELERATOR_HOST) {
					if (!GenerateGPUBurst
					    (TaskId, ThreadId, Timestamp,
					     LastBlockEnd)) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}
			}

#ifdef DEBUG
			cout << "Printing OCL Closing Event: " << *CurrentEvent;
#endif
			if (Dimemas_Block_End(TemporaryFile,
					      TaskId, ThreadId,
					      (INT64) Type) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			LastBlockEnd = Timestamp;
		}
	}

	if (ClusterEventEncoding_Is_ClusterBlock(Type)) {
		/* It's a cluster block */
		if (ClusterEventEncoding_Is_BlockBegin(Value)) {
			/* It's a cluster block begin */
			if (MPIBlockIdStack.size() == 0) {	/* No MPI call in proces */

#ifdef DEBUG
				cout << "Printing Cluster Opening Event: " <<
				    *CurrentEvent;
#endif
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId,
							ThreadId,
							(INT64) Type,
							(INT64) Value) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				FlushClusterStack = false;
			} else {
				/* we can not put the record on trace yet */
				FlushClusterStack = true;
			}

			/* Block management */
			CurrentBlock.first = Type;
			CurrentBlock.second = Value;

			ClusterBlockIdStack.
			    push_back(std::make_pair(Type, Value));

			FirstClusterRead = true;
			/* In order to generate burst at block end */
			LastBlockEnd = Timestamp;
		} else {
			/* It's a cluster block end */
			if (ClusterBlockIdStack.size() == 0) {
				if (FirstClusterRead) {
					// There is a event closing without is opening in the middle of the
					// trace
					cout <<
					    "WARNING: unbalanced cluster blocks on original trace"
					    << endl;
					cout << "Task " << TaskId +
					    1 << " Thread: " << ThreadId +
					    1 << " ";
					cout << "Time " << Timestamp << endl;
				}
			} else {
				CurrentBlock = ClusterBlockIdStack.back();
				ClusterBlockIdStack.pop_back();

				/* Generate burst associated with this cluster */
				if (Timestamp > LastBlockEnd) {
					if (!GenerateBurst
					    (TaskId, ThreadId, Timestamp)) {
						return false;
					}

					LastBlockEnd = Timestamp;
				}

				if (Dimemas_Block_End(TemporaryFile,
						      TaskId, ThreadId,
						      (INT64) Type) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
			}
		}
	}

	if (MPIEventEncoding_Is_MPIBlock(Type)) {
		/* It's a MPI function */
		if (MPIEventEncoding_Is_BlockBegin(Value)) {
			/* It's a Dimemas MPI block begin */

			/* Iprobe checks! */
			if (OngoingIprobe) {
				OngoingIprobe = false;
			}

			if (OngoingTest) {
				OngoingTest = false;
			}

			MPI_Event_Values MPIValue = (MPI_Event_Values) Value;
			CurrentBlock.first = Type;
			CurrentBlock.second = Value;

#ifdef DEBUG
			fprintf(stdout,
				"[%03d:%02d %lld] Opening Block %03lld (%s)\n",
				CurrentEvent->GetTaskId(),
				CurrentEvent->GetThreadId(),
				CurrentEvent->GetTimestamp(),
				(INT64) CurrentBlock.second,
				MPIEventEncoding_GetBlockLabel((MPI_Event_Values)
							       (CurrentBlock.
								second)));
#endif

			/* CPU Burst */
			if (     !(Value == MPI_IPROBE_VAL && IprobeBurstFlushed) 
			     &&  !(Value == MPI_TEST_VAL && TestBurstFlushed)) {
				if (Timestamp > LastBlockEnd) {
					if (!GenerateBurst
					    (TaskId, ThreadId, Timestamp)) {
						return false;
					}
				}
			}

			IprobeBurstFlushed = false;
			TestBurstFlushed = false;

#ifdef DEBUG
			cout << "Printing Block Begin " << *CurrentEvent;
#endif

			if (Dimemas_Block_Begin(TemporaryFile,
						TaskId,
						ThreadId, (INT64) Type,
						(INT64) Value) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			/* Block management */
			MPIBlockIdStack.push_back(CurrentBlock);
			CommunicationPrimitivePrinted = false;

		} else {
			/* It's a Dimemas MPI block end */

			if (MPIBlockIdStack.size() == 0) {
				/* Relax this constraint. Unbalanced MPI blocks are not common
				   MPI_Event_Values MPIValue = (MPI_Event_Values) Value;
				   CurrentBlock    = MPIEventEncoding_DimemasBlockId(MPI_Event_Values);

				   char CurrentError[128];

				   PrintStack();

				   sprintf(CurrentError,
				   "unbalanced MPI event on original trace (task %02d, block %02d) ",
				   TaskId,
				   (long int) Value);

				   SetError(true);
				   // SetErrorMessage(CurrentError, "");
				   this->LastError = CurrentError;

				   return false;
				 */
				return true;
			}
			// CurrentBlock = MPIEventEncoding_DimemasBlockId(BlockIdStack.back());
			CurrentBlock = MPIBlockIdStack.back();
			MPIBlockIdStack.pop_back();

			if ((MPI_Event_Values) CurrentBlock.second ==
			    MPI_INIT_VAL && GenerateMPIInitBarrier) {
				if (Dimemas_Global_OP(TemporaryFile, CurrentEvent->GetTaskId() - 1, CurrentEvent->GetThreadId() - 1, GLOP_ID_MPI_Barrier,	// BARRIER ID?,
						      1,	// Check this communicator!!
						      0, 0,	// RootRank | RootThread = 0
						      0, 0) < 0)	// No send/recv sizes
				{
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				MPIInitBarrierWritten = true;
				CommunicationPrimitivePrinted = true;
			}

#ifdef DEBUG
			fprintf(stdout,
				"[%03d:%02d %lld] Closing Block %03lld (%s)\n",
				CurrentEvent->GetTaskId(),
				CurrentEvent->GetThreadId(),
				CurrentEvent->GetTimestamp(),
				(INT64) CurrentBlock.second,
				MPIEventEncoding_GetBlockLabel((MPI_Event_Values)
							       CurrentBlock.
							       second));
#endif

			/* CurrentBlock = MPIEventEncoding_DimemasBlockId(MPI_Event_Values); */

			LastBlockEnd = Timestamp;

#ifdef DEBUG
			cout << "Printing Block End " << *CurrentEvent;
#endif

			if (!CommunicationPrimitivePrinted) {
				if (Dimemas_NOOP
				    (TemporaryFile, TaskId, ThreadId) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
			}

			if (Dimemas_Block_End(TemporaryFile,
					      TaskId, ThreadId,
					      (INT64) Type) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			/* If there are some block not flushed in the stack */

			if (FlushClusterStack) {
				if (Dimemas_Block_Begin(TemporaryFile,
							TaskId,
							ThreadId,
							(INT64)
							ClusterBlockIdStack.back
							().first,
							(INT64)
							ClusterBlockIdStack.back
							().second) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				FlushClusterStack = false;
			}

		}
	}

	if (MPIEventEncoding_Is_UserBlock(Type)) {	/* It's a User function */
		if (MPIEventEncoding_Is_BlockBegin(Value)) {	/* It's a Dimemas User block begin */
			/* Iprobe checks! */
			if (OngoingIprobe) {
				OngoingIprobe = false;
			}
			if (OngoingTest) {
				OngoingTest = false;
			}


			/* Generate the CPU burst previous to this user-function */
			if (Timestamp > LastBlockEnd) {
				if (!GenerateBurst(TaskId, ThreadId, Timestamp)) {
					return false;
				}
			}

			/* This 'trick' is done to ensure the generation of the next CPU burst */
			LastBlockEnd = Timestamp;

#ifdef DEBUG
			cout << "Printing Block Begin " << *CurrentEvent;
#endif

			/* Print the Block Begin */
			if (Dimemas_Block_Begin(TemporaryFile,
						TaskId,
						ThreadId, (INT64) Type,
						(INT64) Value) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}

			/* Block management */
			UserBlockIdStack.push_back(std::make_pair(Type, Value));
		} else {	/* It's a Dimemas User block end */
			if (UserBlockIdStack.size() == 0) {
				/* Relax this constraint. Unbalanced blocks are not common */
				return true;
			}

			/* Get last block */
			CurrentBlock = UserBlockIdStack.back();
			UserBlockIdStack.pop_back();

			/* Generate the CPU burst up to this user-function end */
			if (Timestamp > LastBlockEnd) {
				if (!GenerateBurst(TaskId, ThreadId, Timestamp)) {
					return false;
				}
			}

			/* This 'trick' is done to ensure the generation of the next CPU burst */
			LastBlockEnd = Timestamp;

#ifdef DEBUG
			cout << "Printing Block End " << *CurrentEvent;
#endif

			/* Print the Block Begin */
			if (Dimemas_Block_End
			    (TemporaryFile, TaskId, ThreadId, Type) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",
						strerror(errno));
				return false;
			}
		}
	}

	return true;
}

/* ToDimemas (COMMUNICATION) **************************************************/
bool TaskTranslationInfo::ToDimemas(PartialCommunication_t CurrentComm)
{
	Block_t CurrentBlock;
	INT64 CurrentBlockValue;
	INT32
	    TaskId, ThreadId, PartnerTaskId, PartnerThreadId, CommId, Size, Tag;
	UINT64 Timestamp;

	if (MPIBlockIdStack.size() == 0 && CUDABlockIdStack.size() == 0 && OCLBlockIdStack.size() == 0) {	/* Error! Communications must be inside a block! */

		/* Communications outside a block only emit a Warning

		   cout << "WARNING: Communication outside a block on Task ";
		   cout << CurrentComm->GetTaskId() << "(TimeStamp = " << CurrentComm->GetTimestamp() << ")";
		   cout << ". The simulation of this trace could be inconsistent" << endl;

		   LastError = "Communication outside a block";
		   return false;
		 */

#ifdef DEBUG
		fprintf(stdout,
			"Comm: [%03d:%02d] T:%lld ",
			CurrentComm->GetTaskId(),
			CurrentComm->GetThreadId(),
			CurrentComm->GetTimestamp());

		switch (CurrentComm->GetType()) {
		case LOGICAL_SEND:
			fprintf(stdout, "LOGICAL_SEND ");
			break;
		case PHYSICAL_SEND:
			fprintf(stdout, "PHYSICAL_SEND ");
			break;
		case LOGICAL_RECV:
			fprintf(stdout, "LOGICAL_RECV ");
			break;
		case PHYSICAL_RECV:
			fprintf(stdout, "PHYSICAL_RECV ");
			break;
		}

		fprintf(stdout, "outside a block!");

#endif				// DEBUG

		OutsideComms = true;
		return true;
	}

	TaskId = CurrentComm->GetTaskId() - 1;
	ThreadId = CurrentComm->GetThreadId() - 1;
	PartnerThreadId = CurrentComm->GetPartnerThreadId() - 1;
	CommId = CurrentComm->GetCommId();
	Size = CurrentComm->GetSize();
	Tag = CurrentComm->GetTag();
	Timestamp = CurrentComm->GetTimestamp();

	if (PreviouslySimulatedTrace) {
		PartnerTaskId = CurrentComm->GetPartnerTaskId();
	} else {
		PartnerTaskId = CurrentComm->GetPartnerTaskId() - 1;
	}

	if (MPIBlockIdStack.size() > 0) {
		CurrentBlock = MPIBlockIdStack.back();
		CurrentBlockValue =
		    MPIEventEncoding_DimemasBlockId((MPI_Event_Values)
						    CurrentBlock.second);
		switch (CurrentBlockValue) {
		case BLOCK_ID_MPI_Recv:
			/* DEBUG
			   fprintf(stdout, "MPI_RECV\n");
			 */
			if (CurrentComm->GetType() == LOGICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX Recv " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Recv(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->recv_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Send_counter == 0)
					this->pendent_i_Recv_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Send_counter--;
			} else if (CurrentComm->GetType() != PHYSICAL_RECV) {
				WrongComms = true;
			}

			break;
		case BLOCK_ID_MPI_Rsend:
		case BLOCK_ID_MPI_Send:
		case BLOCK_ID_MPI_Ssend:
			/* DEBUG
			   fprintf(stdout, "MPI_SSEND\n");
			 */
			if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
				cout << "Printing NX BSend " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_SEND, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
							    CommId, Size,
							    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				this->send_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Recv_counter == 0)
					this->pendent_i_Send_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Recv_counter--;
			} else if (CurrentComm->GetType() == PHYSICAL_RECV) {	/* Not common case, when translation comes from a simulated trace
										   Wait synchronizations may appear inside a reception operation */

				if (!PrintPseudoCommunicationEndpoint
				    (PHYSICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Wait(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->wait_counter++;
			} else {	/* if (CurrentComm->GetType() != PHYSICAL_SEND) */

				WrongComms = true;
			}
			break;
		case BLOCK_ID_MPI_Bsend:
		case BLOCK_ID_MPI_Isend:
		case BLOCK_ID_MPI_Issend:
			/* DEBUG
			   fprintf(stdout, "MPI_ISEND\n");
			 */
			if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
				cout << "Printing NX ISend " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_SEND, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_ImmediateSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
							     CommId, Size,
							     (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->isend_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Recv_counter == 0)
					this->pendent_i_Send_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Recv_counter--;
			} else if (CurrentComm->GetType() == PHYSICAL_RECV) {	/* Not common case, when translation comes from a simulated trace
										   Wait synchronizations may appear inside a reception operation */

				if (!PrintPseudoCommunicationEndpoint
				    (PHYSICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Wait(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->wait_counter++;
			} else {	/* (CurrentComm->GetType() != PHYSICAL_SEND) */

				WrongComms = true;
			}
			break;
		case BLOCK_ID_MPI_Irecv:
			/* DEBUG
			   fprintf(stdout, "MPI_IRECV\n");
			 */
			if (CurrentComm->GetType() == LOGICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX IRecv " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Irecv(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						     CommId, Size,
						     (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->irecv_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Send_counter == 0)
					this->pendent_i_Recv_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Send_counter--;
			} else if (CurrentComm->GetType() == PHYSICAL_RECV) {
#ifdef DEBUG
				cout << "PHYSICAL_RECV inside an Irecv" <<
				    *CurrentComm;
#endif
			} else if (CurrentComm->GetType() != PHYSICAL_RECV) {
				WrongComms = true;
			}
			break;
		case BLOCK_ID_MPI_Wait:
			/* DEBUG
			   fprintf(stdout, "MPI_WAIT\n");
			 */
			if (CurrentComm->GetType() == PHYSICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX Wait " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (PHYSICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Wait(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->wait_counter++;
			} else if (CurrentComm->GetType() != LOGICAL_RECV) {
				WrongComms = true;
			}
			break;
		case BLOCK_ID_MPI_Waitany:
		case BLOCK_ID_MPI_Waitall:
		case BLOCK_ID_MPI_Waitsome:
		case BLOCK_ID_MPI_Test:
		case BLOCK_ID_MPI_Testany:
		case BLOCK_ID_MPI_Testall:

			/* DEBUG
			   fprintf(stdout, "MPI_WAIT\n");
			 */
			if (CurrentComm->GetType() == PHYSICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX Wait " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (PHYSICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Wait(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->wait_counter++;

				NonDeterministicComms = true;
			} else if (CurrentComm->GetType() != LOGICAL_RECV) {
				WrongComms = true;
			}
			break;
		case BLOCK_ID_MPI_Sendrecv:
		case BLOCK_ID_MPI_Sendrecv_replace:
			/* DEBUG
			   fprintf(stdout, "MPI_SENDRECV\n");
			 */
			if (CurrentComm->GetType() == LOGICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX IRecv " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Irecv(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						     CommId, Size,
						     (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->irecv_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Send_counter == 0)
					this->pendent_i_Recv_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Send_counter--;
			} else if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
				cout << "Printing NX Send " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_SEND, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Send(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->send_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Recv_counter == 0)
					this->pendent_i_Send_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Recv_counter--;
			} else if (CurrentComm->GetType() == PHYSICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX Wait " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (PHYSICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Wait(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						    CommId, Size,
						    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace", "");
					return false;
				}
				this->wait_counter++;
			}
			break;
		case BLOCK_ID_MPI_Start:
		case BLOCK_ID_MPI_Startall:
			if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
				cout << "Printing NX BSend " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_SEND, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
							    CommId, Size,
							    (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->send_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Recv_counter == 0)
					this->pendent_i_Send_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Recv_counter--;
			} else if (CurrentComm->GetType() == LOGICAL_RECV) {
#ifdef DEBUG
				cout << "Printing NX Recv " << *CurrentComm;
#endif
				CommunicationPrimitivePrinted = true;

				if (!PrintPseudoCommunicationEndpoint
				    (LOGICAL_RECV, TaskId, ThreadId,
				     PartnerTaskId, -1, Size, Tag, CommId)) {
					return false;
				}

				if (Dimemas_NX_Irecv(TemporaryFile, TaskId, ThreadId, PartnerTaskId, -1,	/* That should be corrected eventually */
						     CommId, Size,
						     (INT64) Tag) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}
				this->irecv_counter++;

				if ((*AllTranslationInfo)[PartnerTaskId]
				    [0]->pendent_i_Send_counter == 0)
					this->pendent_i_Recv_counter++;
				else
					(*AllTranslationInfo)[PartnerTaskId]
					    [0]->pendent_i_Send_counter--;

			}
			break;
		default:

			MPI_Event_Values CurrentBlockMPIVal =
			    (MPI_Event_Values) CurrentBlockValue;

			fprintf(stdout,
				"[%03d:%02d %lu] Communication wrapped with unknown communication call (%d:%s)\n",
				CurrentComm->GetTaskId(),
				CurrentComm->GetThreadId(),
				CurrentComm->GetTimestamp(),
				(INT32) CurrentBlockValue,
				MPIEventEncoding_GetBlockLabel
				(CurrentBlockMPIVal));
			break;
		}
	}			//end MPIBlockIdStack if.
	else if (CUDABlockIdStack.size() > 0
		 && AcceleratorThread != ACCELERATOR_NULL) {
		CurrentBlock = CUDABlockIdStack.back();

		switch (CurrentBlock.second) {
		case CUDA_MEMCPY_VAL:
			{
				if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
					cout <<
					    "Printing CUDA Memory Transfer (Sync): "
					    << *CurrentComm;
#endif
					if (AcceleratorThread == ACCELERATOR_HOST) {	/* In the Host thread, first a synchronization */
						if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag) < 0)	// CommId and Size are set to 0
						{
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
					}

					if (Dimemas_NX_BlockingSend
					    (TemporaryFile, TaskId, ThreadId,
					     PartnerTaskId, PartnerThreadId,
					     CommId, Size, (INT64) Tag) < 0) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				} else if (CurrentComm->GetType() ==
					   LOGICAL_RECV) {
#ifdef DEBUG
					cout <<
					    "Printing CUDA Memory Transfer (Sync): "
					    << *CurrentComm;
#endif
					CommunicationPrimitivePrinted = true;
					if (AcceleratorThread == ACCELERATOR_HOST) {	/* In the Host thread, first a synchronization */
						if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag) < 0)	// CommId and Size are set to 0
						{
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
					}

					if (Dimemas_NX_Recv(TemporaryFile,
							    TaskId, ThreadId,
							    PartnerTaskId,
							    PartnerThreadId,
							    CommId, Size,
							    (INT64) Tag) < 0) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}

				break;
			}
		case CUDA_MEMCPY_ASYNC_VAL:
			{
				if (CurrentComm->GetType() == LOGICAL_SEND) {
#ifdef DEBUG
					cout <<
					    "Printing CUDA Memory Transfer (Async): "
					    << *CurrentComm;
#endif
					if (AcceleratorThread == ACCELERATOR_HOST) {	/* In the Host thread, first a synchronization */
						if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0,	// CommId and Size are set to 0
									    (INT64) Tag) < 0) {
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
					}

					/* The actual transfer */
					if (Dimemas_NX_ImmediateSend
					    (TemporaryFile, TaskId, ThreadId,
					     PartnerTaskId, PartnerThreadId,
					     CommId, Size, (INT64) Tag) < 0) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				} else if (CurrentComm->GetType() ==
					   LOGICAL_RECV) {
#ifdef DEBUG
					cout <<
					    "Printing CUDA Memory Transfer (Async): "
					    << *CurrentComm;
#endif
					CommunicationPrimitivePrinted = true;

					if (AcceleratorThread == ACCELERATOR_HOST) {	/* In the Host thread, first a synchronization */
						if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0,	// CommId and Size are set to 0
									    (INT64) Tag) < 0) {
							SetError(true);
							SetErrorMessage
							    ("error writing output trace",
							     strerror(errno));
							return false;
						}
					}

					if (Dimemas_NX_Irecv(TemporaryFile,
							     TaskId, ThreadId,
							     PartnerTaskId,
							     PartnerThreadId,
							     CommId, Size,
							     (INT64) Tag) < 0) {
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}

				break;
			}

		case CUDA_CONFIGURECALL_VAL:
			{
				if (CurrentComm->GetType() == LOGICAL_SEND) {	/* Host side cudeConfigureCall synchronization (SEND) */
#ifdef DEBUG
					cout <<
					    "Printing CUDA Host configureCall sync: "
					    << *CurrentComm;
#endif
					if (Dimemas_NX_Generic_Send(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, Size, (INT64) Tag, 3) < 0)	// CommId and Size are set to 0)
					{
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}
				break;
			}
		case CUDA_LAUNCH_VAL:
			{
				if (CurrentComm->GetType() == LOGICAL_SEND) {	/* Host side cudaLaunch synchronization */
#ifdef DEBUG
					cout <<
					    "Printing CUDA Host Launch sync: "
					    << *CurrentComm;
#endif
					if (Dimemas_NX_Generic_Send(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, Size, (INT64) Tag, 3) < 0)	// CommId and Size are set to 0)
					{
						SetError(true);
						SetErrorMessage
						    ("error writing output trace",
						     strerror(errno));
						return false;
					}
				}
				break;
			}
		default:
			break;
		}
	}			//end CUDABlockIdStack if
	else if (OCLBlockIdStack.size() > 0
		 && AcceleratorThread != ACCELERATOR_NULL) {
		CurrentBlock = OCLBlockIdStack.back();

		switch (CurrentBlock.first) {
		case OCL_HOST_CALL_EV:
			{
				switch (CurrentBlock.second) {
				case OCL_ENQUEUE_WRITE_BUFF_VAL:
				case OCL_ENQUEUE_WRITE_BUFF_RECT_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_SEND) {	/* Host side mem_transfer synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Memory Transfer (Host sends): "
							    << *CurrentComm;
#endif
							if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag) < 0)	// CommId and Size are set to 0
							{
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}

							if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, CommId, Size, (INT64) Tag) < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				case OCL_FINISH_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_RECV) {	/* Host side synch synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Synchronization (Host receives): "
							    << *CurrentComm;
#endif
							if (Dimemas_NX_Recv(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag) < 0)	// CommId and Size are set to 0)
							{
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				case OCL_ENQUEUE_READ_BUFF_VAL:
				case OCL_ENQUEUE_READ_BUFF_RECT_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_RECV) {	/* Host side synch synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Memory transfer (Host receives): "
							    << *CurrentComm;
#endif
							/* In host side first a synchronization */
							if (Dimemas_NX_BlockingSend(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag) < 0)	// CommId and Size are set to 0
							{
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
							if (Dimemas_NX_Recv
							    (TemporaryFile,
							     TaskId, ThreadId,
							     PartnerTaskId,
							     PartnerThreadId,
							     CommId, Size,
							     (INT64) Tag) < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				case OCL_ENQUEUE_NDRANGE_KERNEL_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_SEND) {	/* Host side synch synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Launch (Host sends): "
							    << *CurrentComm;
#endif
							/* In host side first a synchronization */
							if (Dimemas_NX_Generic_Send(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, 0, 0, (INT64) Tag, 3) < 0)	// CommId and Size are set to 0
							{
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				default:
					break;
				}	/* end switch CurrentBlock.second */
				break;
			}	/* end case OCL_HOST_CALL_EV */

		case OCL_ACCELERATOR_CALL_EV:
			{
				switch (CurrentBlock.second) {
				case OCL_ENQUEUE_WRITE_BUFF_ACC_VAL:
				case OCL_ENQUEUE_WRITE_BUFF_RECT_ACC_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_RECV) {	/* Kernel side mem_transf synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Memory Transfer (Kernel receives): "
							    << *CurrentComm;
#endif
							if (Dimemas_NX_Recv
							    (TemporaryFile,
							     TaskId, ThreadId,
							     PartnerTaskId,
							     PartnerThreadId,
							     CommId, Size,
							     (INT64) Tag) < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				case OCL_ENQUEUE_READ_BUFF_ACC_VAL:
				case OCL_ENQUEUE_READ_BUFF_RECT_ACC_VAL:
					{
						if (CurrentComm->GetType() == LOGICAL_SEND) {	/* Kernel side mem_transfer synchronization */
#ifdef DEBUG
							cout <<
							    "Printing OCL Memory Transfer (Kernel sends): "
							    << *CurrentComm;
#endif
							if (Dimemas_NX_Generic_Send(TemporaryFile, TaskId, ThreadId, PartnerTaskId, PartnerThreadId, CommId, Size, (INT64) Tag, 3) < 0) {
								SetError(true);
								SetErrorMessage
								    ("error writing output trace",
								     strerror
								     (errno));
								return false;
							}
						}
						break;
					}

				case OCL_ENQUEUE_NDRANGE_KERNEL_ACC_VAL:
					{
						/* To avoid having the comunication before the GPUBurst */
						OCLFinishComm = true;
						break;
					}

				default:
					break;
				}	/* end switch CurrentBlock.second */

				break;
			}	/* end case OCL_ACCELERATOR_CALL_EV */

		default:
			cout << "Printing OCL non-treated communication: " <<
			    *CurrentComm;
			cout << " first:" << CurrentBlock.first;
			cout << ", second:" << CurrentBlock.second << endl;
			break;
		}		/* end switch CurrentBlock.first */
	}			//end OCLBlockIdStack if

	return true;
}

/* ToDimemas (GLOBAL_OP) ******************************************************/
bool TaskTranslationInfo::ToDimemas(GlobalOp_t CurrentGlobOp)
{
	INT32 RootTaskId;

	/*
	   if (CurrentGlobOp->GetRootTaksId() == -1)
	   RootTaskId = 0;
	   else
	   RootTaskId = CurrentGlobOp->GetTaskId()-1;
	 */

	if (CurrentGlobOp->GetIsRoot()) {
		RootTaskId = 1;
	} else {
		RootTaskId = 0;
	}

#ifdef DEBUG
	cout << "Printing GlobalOP " << *CurrentGlobOp;
#endif
	if (Dimemas_Global_OP(TemporaryFile,
			      CurrentGlobOp->GetTaskId() - 1,
			      CurrentGlobOp->GetThreadId() - 1,
			      CurrentGlobOp->GetGlobalOpId(),
			      CurrentGlobOp->GetCommunicatorId(),
			      RootTaskId, 0,
			      CurrentGlobOp->GetSendSize(),
			      CurrentGlobOp->GetRecvSize()) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	this->glop_counter++;

	/* DEBUG
	   fprintf(stdout, "GlobalOP printed!!\n");
	 */
	return true;
}

/* Event2GlobalOp *************************************************************/
void TaskTranslationInfo::Event2GlobalOp(Event_t CurrentEvent)
{
	bool NoMoreEvents = false;

#ifdef DEBUG
	fprintf(stdout,
		"[%03d:%02d %lld] Event to global translation: ",
		CurrentEvent->GetTaskId(),
		CurrentEvent->GetThreadId(), CurrentEvent->GetTimestamp());
#endif

	switch (CurrentEvent->GetFirstType()) {
	case MPI_GLOBAL_OP_SENDSIZE:
		PartialGlobalOp->SetSendSize((INT32) CurrentEvent->
					     GetFirstValue());
		GlobalOpFields++;

#ifdef DEBUG
		fprintf(stdout, "SEND SIZE = %lld\n",
			CurrentEvent->GetFirstValue());
#endif
		break;
	case MPI_GLOBAL_OP_RECVSIZE:
		PartialGlobalOp->SetRecvSize((INT32) CurrentEvent->
					     GetFirstValue());
		GlobalOpFields++;

#ifdef DEBUG
		fprintf(stdout, "RECEIVE SIZE = %lld\n",
			CurrentEvent->GetFirstValue());
#endif
		break;
	case MPI_GLOBAL_OP_ROOT:
		if (CurrentEvent->GetFirstValue() == 1) {
			PartialGlobalOp->SetIsRoot(true);
			// PartialGlobalOp->SetRootTaskId(CurrentEvent->GetTaskId());
		}

		GlobalOpFields++;
#ifdef DEBUG
		fprintf(stdout, "ROOT\n", CurrentEvent->GetFirstValue());
#endif
		break;
	case MPI_GLOBAL_OP_COMM:
		// DEBUG
		// cout << "Communicator in Global OP = " << CurrentEvent->GetFirstValue() << endl;
		PartialGlobalOp->SetCommunicatorId((INT32) CurrentEvent->
						   GetFirstValue());
		GlobalOpFields++;

#ifdef DEBUG
		fprintf(stdout, "COMMUNICATOR = %lld\n",
			CurrentEvent->GetFirstValue());
#endif
		break;
	default:
		/* TEST */

#ifdef DEBUG
		fprintf(stdout, "NOT A GLOBAL OP EVENT (%d)\n",
			CurrentEvent->GetFirstType());
#endif

		return;
		//NoMoreEvents = true;
		break;
	}

	if (NoMoreEvents) {
		/* DEBUG */
		printf("FINALIZING GlobalOp Fields = %d\n", GlobalOpFields);

		/* if (GlobalOpFields == 4 || GlobalOpFields == 5) */
		if (GlobalOpFields >= 2) {
			if (CurrentEvent->GetFirstType() == MPITYPE_COLLECTIVE) {
				ToDimemas(PartialGlobalOp);
			} else {
				RecordStack.push_back(PartialGlobalOp);
			}

			/* PushRecord(PartialGlobalOp); */
			/* ToDimemas(PartialGlobalOp); */
			GlobalOpFields = 0;
			PendingGlobalOp = false;
		} else {
			delete PartialGlobalOp;
			PendingGlobalOp = false;
		}
	}
}

void TaskTranslationInfo::FinalizeGlobalOp(void)
{
	/* if (GlobalOpFields == 4 || GlobalOpFields == 5) */

#ifdef DEBUG
	fprintf(stdout,
		"[%03d:%02d %lld] Finalizing global \n",
		PartialGlobalOp->GetTaskId(),
		PartialGlobalOp->GetThreadId(),
		PartialGlobalOp->GetTimestamp());
#endif

	if (GlobalOpFields >= 2) {
		ToDimemas(PartialGlobalOp);

		/* PushRecord(PartialGlobalOp); */
		/* ToDimemas(PartialGlobalOp); */
		GlobalOpFields = 0;
		PendingGlobalOp = false;
	} else {
		delete PartialGlobalOp;
		PendingGlobalOp = false;
	}
}

/* CheckTestCounters ********************************************************/
bool TaskTranslationInfo::CheckTestCounters(Event_t CurrentEvent)
{

	INT32 TaskId, ThreadId;
	INT32 Type;
	INT64 Value;
	UINT64 Timestamp;
	double OnTraceTime;

	TaskId = CurrentEvent->GetTaskId() - 1;
	ThreadId = CurrentEvent->GetThreadId() - 1;
	Type = CurrentEvent->GetFirstType();
	Value = CurrentEvent->GetFirstValue();
	Timestamp = CurrentEvent->GetTimestamp();

	if (Type == MPITYPE_TEST_SOFTCOUNTER && Value == 0) {
		/* CPU Burst */
		if (!GenerateBurst(TaskId, ThreadId, Timestamp))
			return false;

		LastBlockEnd = Timestamp;
		OngoingTest = true;
	}

	if (Type == MPITYPE_TEST_SOFTCOUNTER && Value != 0) {
		if (OngoingTest) {
			/* CPU Burst */
			double
			    TestAreaDuration =
			    Timestamp - LastBlockEnd;
			double ActualTestRate =
			    (Value / TestAreaDuration);

			if (TimeFactor == 1e-9) {
				ActualTestRate *= 1e6;
			} else {
				ActualTestRate *= 1e3;
			}

			//printf("ACTUAL TEST RATE = %f\n", ActualTestRate);
			if (TestMissesThreshold == 0.0
			    || ActualTestRate < TestMissesThreshold)
			{
				OnTraceTime =
				    (double)1.0 *(Timestamp -
						  LastBlockEnd) *
				    TimeFactor;
			} else {
				//printf("ERASED BURST!!!");
				/* Ereased burst! */
				OnTraceTime = 0.0;
			}

#ifdef DEBUG
			cout << "Printing CPU Burst [";
			cout.width(3);
			cout.fill('0');
			cout << TaskId + 1 << ":";

			cout.width(2);
			cout.fill('0');
			cout << ThreadId + 1 << "]: ";
			cout << std::setprecision(9);
			cout << std::fixed;
			cout << OnTraceTime;
			cout << endl;
#endif
			if (Dimemas_CPU_Burst(TemporaryFile,
					      TaskId, ThreadId,
					      OnTraceTime) < 0) {
				SetError(true);
				SetErrorMessage("error writing output trace",strerror(errno));
				return false;
			}

			OngoingTest = false;
			TestBurstFlushed = true;
			LastBlockEnd = Timestamp;
		}
	}
	return true;
}

/* CheckIprobeCounters ********************************************************/
bool TaskTranslationInfo::CheckIprobeCounters(Event_t CurrentEvent)
{

	INT32 TaskId, ThreadId;
	INT32 Type;
	INT64 Value;
	UINT64 Timestamp;
	double OnTraceTime;

	TaskId = CurrentEvent->GetTaskId() - 1;
	ThreadId = CurrentEvent->GetThreadId() - 1;
	Type = CurrentEvent->GetFirstType();
	Value = CurrentEvent->GetFirstValue();
	Timestamp = CurrentEvent->GetTimestamp();

	if (Type == MPITYPE_PROBE_SOFTCOUNTER && Value == 0) {
		/* CPU Burst */
		if (!GenerateBurst(TaskId, ThreadId, Timestamp))
			return false;
		/*
		   OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;

		   if (Dimemas_CPU_Burst(TemporaryFile,
		   TaskId, ThreadId,
		   OnTraceTime) < 0)
		   {
		   SetError(true);
		   SetErrorMessage("error writing output trace", strerror(errno));
		   return false;
		   }
		 */

		LastBlockEnd = Timestamp;
		OngoingIprobe = true;
	}

	if ((Type == MPITYPE_PROBE_SOFTCOUNTER
	     || Type == MPITYPE_PROBE_TIMECOUNTER)
	    && Value != 0) {
		if (OngoingIprobe) {
			if (Type == MPITYPE_PROBE_TIMECOUNTER
			    && IprobeMissesThreshold == 0.0) {
				/* if (Value < TimeCounterThreshold) */
#ifdef DEBUG
				cout << "Printing CPU Burst [";
				cout.width(3);
				cout.fill('0');
				cout << TaskId + 1 << ":";

				cout.width(2);
				cout.fill('0');
				cout << ThreadId + 1 << "]: ";
				cout << std::setprecision(9);
				cout << std::fixed;
				cout << Value / TimeFactor;
				cout << " (IProbe timecounter) ";
				cout << endl;
#endif
				if (Dimemas_CPU_Burst(TemporaryFile,
						      TaskId, ThreadId,
						      Value / TimeFactor) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				OngoingIprobe = false;
				IprobeBurstFlushed = true;
				LastBlockEnd = Timestamp;
			} else if (Type == MPITYPE_PROBE_SOFTCOUNTER) {
				/* CPU Burst */
				double
				    IprobeAreaDuration =
				    Timestamp - LastBlockEnd;
				double ActualIprobeRate =
				    (Value / IprobeAreaDuration);

				if (TimeFactor == 1e-9) {
					ActualIprobeRate *= 1e6;
				} else {
					ActualIprobeRate *= 1e3;
				}

				if (IprobeMissesThreshold == 0.0
				    || ActualIprobeRate < IprobeMissesThreshold)
				{
					OnTraceTime =
					    (double)1.0 *(Timestamp -
							  LastBlockEnd) *
					    TimeFactor;
				} else {
					/* Ereased burst! */
					OnTraceTime = 0.0;
				}

#ifdef DEBUG
				cout << "Printing CPU Burst [";
				cout.width(3);
				cout.fill('0');
				cout << TaskId + 1 << ":";

				cout.width(2);
				cout.fill('0');
				cout << ThreadId + 1 << "]: ";
				cout << std::setprecision(9);
				cout << std::fixed;
				cout << OnTraceTime;
				cout << endl;
#endif
				if (Dimemas_CPU_Burst(TemporaryFile,
						      TaskId, ThreadId,
						      OnTraceTime) < 0) {
					SetError(true);
					SetErrorMessage
					    ("error writing output trace",
					     strerror(errno));
					return false;
				}

				OngoingIprobe = false;
				IprobeBurstFlushed = true;
				LastBlockEnd = Timestamp;
			}
		}
	}
	return true;
}

/* GenerateBurst **************************************************************/
bool TaskTranslationInfo::GenerateBurst(INT32 TaskId,
					INT32 ThreadId, UINT64 Timestamp)
{
	double OnTraceTime = 0.0;
	bool found = false;

	if (FirstPrint || !BurstCounterGeneration) {
		if (FirstPrint) {
			FirstPrint = false;
		}

		OnTraceTime =
		    (double)1.0 *(Timestamp - LastBlockEnd) * TimeFactor;
	} else {		/* Burst duration computation based on counter value and factor */
		UINT32 i;
		Event_t CurrentEvent = NULL;

		for (i = 0; i < RecordStack.size(); i++) {
			CurrentEvent =
			    dynamic_cast < Event_t > (RecordStack[i]);

			if (CurrentEvent == NULL) {
				continue;
			}

			if (CurrentEvent->GetFirstType() == BurstCounterType) {
				OnTraceTime =
				    1.0 * CurrentEvent->GetFirstValue() *
				    BurstCounterFactor;

				/*
				   printf("Factor: %.9f Evt. Val: %ld Time: %.6f\n",
				   BurstCounterFactor,
				   (long int) CurrentEvent->GetFirstValue(),
				   OnTraceTime );
				 */

				found = true;
				break;
			}
		}

		if (!found) {	/* If no counter found, burst duration is computed with the 'classical
				   method' */
			OnTraceTime =
			    (double)1.0 *(Timestamp -
					  LastBlockEnd) * TimeFactor;
		}
	}

#ifdef DEBUG
	cout << "Printing CPU Burst [";
	cout.width(3);
	cout.fill('0');
	cout << TaskId + 1 << ":";

	cout.width(2);
	cout.fill('0');
	cout << ThreadId + 1 << "]: ";
	cout << std::setprecision(9);
	cout << std::fixed;
	cout << OnTraceTime;
	cout << endl;
#endif

	if (Dimemas_CPU_Burst(TemporaryFile, TaskId, ThreadId, OnTraceTime) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	return true;
}

bool TaskTranslationInfo::PrintPseudoCommunicationEndpoint(INT32 CommType,
							   INT32 TaskId,
							   INT32 ThreadId,
							   INT32 PartnerTaskId,
							   INT32
							   PartnerThreadId,
							   INT32 Size,
							   INT32 Tag,
							   INT32 CommId)
{
	if (Dimemas_User_Event(TemporaryFile,
			       TaskId, ThreadId, (INT64) 9, CommType) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	if (Dimemas_User_Event(TemporaryFile,
			       TaskId, ThreadId, (INT64) 10, PartnerTaskId) < 0)
	{
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	if (PartnerThreadId != -1) {
		if (Dimemas_User_Event(TemporaryFile,
				       TaskId,
				       ThreadId, (INT64) 11,
				       PartnerThreadId) < 0) {
			SetError(true);
			SetErrorMessage("error writing output trace",
					strerror(errno));
			return false;
		}
	}

	if (Dimemas_User_Event(TemporaryFile,
			       TaskId, ThreadId, (INT64) 12, Size) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	if (Dimemas_User_Event(TemporaryFile,
			       TaskId, ThreadId, (INT64) 13, Tag) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	if (Dimemas_User_Event(TemporaryFile,
			       TaskId, ThreadId, (INT64) 14, CommId) < 0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	return true;
}

void TaskTranslationInfo::PrintStack(void)
{
	int i;
	cout << "Record Stack for Task " << TaskId << endl;
	for (i = 0; i < RecordStack.size(); i++) {
		cout << *(RecordStack[i]);
	}
	return;
}

/* GenerateBurst **************************************************************/
bool TaskTranslationInfo::GenerateGPUBurst(INT32 TaskId,
					   INT32 ThreadId,
					   UINT64 Timestamp, UINT64 LastBlock)
{
	double OnTraceTime = 0.0;
	bool found = false;

	if (FirstPrint || !BurstCounterGeneration) {
		if (FirstPrint)
			FirstPrint = false;

		OnTraceTime = (double)1.0 *(Timestamp - LastBlock) * TimeFactor;
	} else {		/* Burst duration computation based on counter value and factor */
		UINT32 i;
		Event_t CurrentEvent = NULL;

		for (i = 0; i < RecordStack.size(); i++) {
			CurrentEvent =
			    dynamic_cast < Event_t > (RecordStack[i]);

			if (CurrentEvent == NULL)
				continue;

			if (CurrentEvent->GetFirstType() == BurstCounterType) {
				OnTraceTime =
				    1.0 * CurrentEvent->GetFirstValue() *
				    BurstCounterFactor;

				found = true;
				break;
			}
		}

		if (!found) {	/* If no counter found, burst duration is computed with the 'classical
				   method' */
			OnTraceTime =
			    (double)1.0 *(Timestamp - LastBlock) * TimeFactor;
		}
	}

#ifdef DEBUG
	cout << "Printing CPU Burst [";
	cout.width(3);
	cout.fill('0');
	cout << TaskId + 1 << ":";

	cout.width(2);
	cout.fill('0');
	cout << ThreadId + 1 << "]:" << OnTraceTime;
	cout << endl;
#endif

	if (!Dimemas_GPU_Burst(TemporaryFile, TaskId, ThreadId, OnTraceTime) <
	    0) {
		SetError(true);
		SetErrorMessage("error writing output trace", strerror(errno));
		return false;
	}

	return true;
}
