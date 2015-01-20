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
  $Rev:: 478                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 (jue, 28 oct #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ParaverRecord.hpp"
using std::endl;

#include "EventEncoding.h"
#include "Dimemas_Generation.h"


/*****************************************************************************
 * class ParaverRecord
 ****************************************************************************/

UINT64 ParaverRecord::CurrentRecordCount = 0;

ParaverRecord::ParaverRecord()
{
  this->RecordCount = NewRecordCount();
}

ParaverRecord::ParaverRecord(UINT64 Timestamp,
                             INT32  CPU,
                             INT32  AppId,
                             INT32  TaskId,
                             INT32  ThreadId)
{
  this->Timestamp   = Timestamp;
  this->CPU         = CPU;
  this->AppId       = AppId;
  this->TaskId      = TaskId;
  this->ThreadId    = ThreadId;
  this->RecordCount = NewRecordCount();
}

UINT64 ParaverRecord::NewRecordCount(void)
{
  return CurrentRecordCount++;
}

ostream& operator<< (ostream& os, const ParaverRecord& Rec)
{
  Rec.Write(os);
  return os;
}

/*****************************************************************************
 * class State
 ****************************************************************************/
State::State(INT32  CPU, INT32  AppId, INT32  TaskId, INT32  ThreadId,
             UINT64 BeginTime,
             UINT64 EndTime,
             INT32  StateValue)
:ParaverRecord(BeginTime, CPU, AppId, TaskId, ThreadId)
{
  this->TimestampEnd = EndTime;
  this->StateValue   = StateValue;
}

void State::Write(ostream& os) const
{
  os << "State: " << " [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "]";

  os << Timestamp << " T:" << TimestampEnd << " State: " << StateValue;
  os << endl;
}

ostream& operator<< (ostream& os, const State& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
 * class EventTypeValue
 ****************************************************************************/

INT64 EventTypeValue::CurrentTraceOrder = 0;

bool
EventTypeValue::IsUserBlockBegin(void)
{

  return MPIEventEncoding_Is_UserBlock( (INT64) Type ) &&
         MPIEventEncoding_Is_BlockBegin( Value );
}

bool
EventTypeValue::IsMPIBlockBegin(void)
{
  return MPIEventEncoding_Is_MPIBlock( (INT64)  Type ) &&
         MPIEventEncoding_Is_BlockBegin( Value );
}

bool
EventTypeValue::IsUserBlockEnd(void)
{
  return MPIEventEncoding_Is_UserBlock( (INT64) Type ) &&
         !MPIEventEncoding_Is_BlockBegin( Value );
}

bool
EventTypeValue::IsMPIBlockEnd(void)
{
  return MPIEventEncoding_Is_MPIBlock( (INT64)  Type ) &&
         !MPIEventEncoding_Is_BlockBegin( Value );
}

bool
EventTypeValue::IsCaller(void)
{
  if (Type >= MPI_CALLER_EV && Type <= MPI_CALLER_EV_END)
    return true;

  return false;
}

bool
EventTypeValue::IsCallerLine(void)
{
  if (Type >= MPI_CALLER_LINE_EV && Type <= MPI_CALLER_LINE_EV_END)
    return true;

  return false;
}

INT64 EventTypeValue::NewTraceOrder(void)
{
  return CurrentTraceOrder++;
}

/*****************************************************************************
 * class Event
 ****************************************************************************/

Event::Event(UINT64 Timestamp,
             INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId)
:ParaverRecord(Timestamp, CPU, AppId, TaskId, ThreadId)
{
  ContentPresent = false;
}

Event::~Event(void)
{
  if (ContentPresent)
  {
    for (unsigned int i = 0; i < Content.size(); i++)
      delete Content[i];
  }
}

void Event::AddTypeValue(INT32 Type, INT64 Value)
{
  EventTypeValue_t newTypeValue = new EventTypeValue(Type, Value);
  Content.push_back(newTypeValue);
  ContentPresent = true;
};

UINT32
Event::GetTypeValueCount(void)
{
  return (UINT32) Content.size();
}

INT32
Event::GetFirstType(void)
{
  return GetType(0);
}

INT32
Event::GetType(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetType();
  else
    return -1;
}

INT64
Event::GetFirstValue(void)
{
  return GetValue(0);
}

INT64
Event::GetValue(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetValue();
  else
    return -1;
}

INT64
Event::GetFirstTraceOrder(void)
{
  return GetTraceOrder(0);
}

INT64
Event::GetTraceOrder(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetTraceOrder();
  else
    return -1;
}

bool Event::IsUserBlockBegin(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsUserBlockBegin();
}

bool Event::IsMPIBlockBegin(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsMPIBlockBegin();
}

bool Event::IsUserBlockEnd (void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsUserBlockEnd();
}

bool Event::IsMPIBlockEnd (void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsMPIBlockEnd();
}

bool
Event::IsCaller(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsCaller();
}

bool
Event::IsCallerLine(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsCallerLine();
}

void Event::Write(ostream& os) const
{
  os << "Evt: [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "] ";

  os << "T:" << Timestamp;

  for (UINT32 i = 0; i < Content.size(); i++)
  {
    os << " [" << Content[i]->GetTraceOrder() << "]: ";
    os << Content[i]->GetType() << ":" << Content[i]->GetValue();
  }

  os << endl;
}

ostream& operator<< (ostream& os, const Event& Evt)
{
  Evt.Write(os);
  return os;
}

/*****************************************************************************
 * class Communication
 ****************************************************************************/

UINT64 Communication::CurrentTraceOrder = 0;

Communication::Communication(UINT64 LogSend, UINT64 PhySend,
                             UINT64 LogRecv, UINT64 PhyRecv,
                             INT32  SrcCPU,    INT32 SrcAppId,
                             INT32  SrcTaskId, INT32 SrcThreadId,
                             INT32  DstCPU,    INT32 DstAppId,
                             INT32  DstTaskId, INT32 DstThreadId,
                             INT32  Size,
                             INT32  Tag)
{

  CPU          = SrcCPU;
  AppId        = SrcAppId;
  TaskId       = SrcTaskId;
  ThreadId     = SrcThreadId;
  DestCPU      = DstCPU;
  DestAppId    = DstAppId;
  DestTaskId   = DstTaskId;
  DestThreadId = DstThreadId;

  Timestamp    = LogSend; /* Timestamp attribute corresponds to Logical Send*/
  PhysicalSend = PhySend;
  LogicalRecv  = LogRecv;
  PhysicalRecv = PhyRecv;

  this->Size = Size;
  this->Tag  = Tag;
  TraceOrder = Communication::NewTraceOrder();

}

void Communication::Write(ostream& os) const
{
  os << "Communication Record" << endl;

  os << "Sender: [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "]";

  os << " LogSend: " << Timestamp << " PhySend: " << PhysicalSend << endl;

  os << "Recvr: [";

  os.width(3);
  os.fill('0');
  os << DestTaskId << ":";

  os.width(2);
  os.fill('0');
  os << DestThreadId << "]";

  os << " LogRecv: " << LogicalRecv << " PhyRecv: " << PhysicalRecv << endl;

  os << "Size: " << Size << " Tag: " << Tag << endl;
}

UINT64 Communication::NewTraceOrder(void)
{
  return CurrentTraceOrder++;
}

ostream& operator<< (ostream& os, const Communication& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
 * class GlobalOp
 ****************************************************************************/

GlobalOp::GlobalOp(UINT64 Timestamp,
                   INT32  CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId,
                   INT32  CommunicatorId,
                   INT32  SendSize, INT32 RecvSize,
                   INT32  GlobalOpId,
                   INT32  RootTaskId)
:ParaverRecord(Timestamp, CPU, AppId, TaskId, ThreadId)
{
  this->CommunicatorId = CommunicatorId;
  this->SendSize       = SendSize;
  this->RecvSize       = RecvSize;
  this->GlobalOpId     = GlobalOpId;
  this->RootTaskId     = RootTaskId+1; /* RootTaskIds are in range 0..(n-1) */

  if (this->RootTaskId == this->TaskId)
    this->Root = true;
  else
    this->Root = false;
}

GlobalOp::GlobalOp(UINT64 Timestamp,
                   INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId,
                   INT32 CommunicatorId,
                   INT32 SendSize, INT32 RecvSize,
                   INT32 GlobalOpId,
                   bool  Root)
:ParaverRecord(Timestamp, CPU, AppId, TaskId, ThreadId)
{
  this->CommunicatorId = CommunicatorId;
  this->SendSize       = SendSize;
  this->RecvSize       = RecvSize;
  this->GlobalOpId     = GlobalOpId;
  this->Root           = Root;

  if (this->Root)
    this->RootTaskId = 1;
  else
    this->RootTaskId = UNKNOWN_ROOT;
}

void
GlobalOp::Write( ostream& os) const
{
  os << "GlobalOP [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "] ";

  os << "T: " << Timestamp << " CommId " << CommunicatorId;

  os << " GlobOpId: " << GlobalOpId << " RootTask: " << RootTaskId << endl;
}

ostream& operator<< (ostream& os, const GlobalOp& GlobOp)
{
  GlobOp.Write(os);
  return os;
}
