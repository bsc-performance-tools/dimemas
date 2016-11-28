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

  $URL:: https://svn.bsc.es/repos/ptools/trf2dim/trunk/src/DimemasRecord.c#$:

  $Rev:: 483                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-29 11:54:45 +0200#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "DimemasRecord.hpp"

#include <EventEncoding.h>
#include <Dimemas2Prv.h>
#include <Dimemas_Generation.h>

#include <iostream>
using std::endl;


/*****************************************************************************
 * class ParaverRecord
 ****************************************************************************/

DimemasRecord::DimemasRecord(INT32 TaskId, INT32 ThreadId)
{
  this->TaskId    = TaskId;
  this->ThreadId  = ThreadId;
}

ostream& operator<< (ostream& os, const DimemasRecord& Record)
{
  Record.Write(os);
  return os;
}

/*****************************************************************************
 * class CPUBurst
 ****************************************************************************/
CPUBurst::CPUBurst(INT32 TaskId, INT32 ThreadId, double BurstDuration)
:DimemasRecord(TaskId, ThreadId)
{
  this->BurstDuration = BurstDuration;
}

bool
CPUBurst::ToDimemas(FILE* OutputTraceFile)
{
  if (Dimemas_CPU_Burst(OutputTraceFile,TaskId,ThreadId,BurstDuration) < 0)
    return false;

  return true;
}

void CPUBurst::Write(ostream& os) const
{
  os << "CPU Burst: " << "[" << TaskId << "|" << ThreadId << "]";
  os << " Duration: " << BurstDuration << endl;
}

ostream& operator<< (ostream& os, const CPUBurst& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
* class Send
****************************************************************************/

Send::Send(INT32 TaskId, INT32 ThreadId,
           INT32 DestTaskId,
           INT32 DestThreadId,
           INT64 Size,
           INT32 Tag,
           INT32 CommunicatorId,
           INT32 Synchronism)
:DimemasRecord(TaskId, ThreadId)
{
  this->DestTaskId     = DestTaskId;
  this->DestThreadId   = DestThreadId;
  this->Size           = Size;
  this->Tag            = Tag;
  this->CommunicatorId = CommunicatorId;
  this->Synchronism    = Synchronism;
}

bool
Send::IsImmediate(void)
{
  if ( (Synchronism | IMMEDIATE_MASK) != 0)
    return true;
  else
    return false;
}

bool
Send::IsRendezVous(void)
{
  if ( (Synchronism | RENDEZVOUS_MASK) != 0)
    return true;
  else
    return false;
}

bool
Send::ToDimemas(FILE* OutputTraceFile)
{
  if (Dimemas_NX_Generic_Send(OutputTraceFile, TaskId, ThreadId,
                              DestTaskId, DestThreadId, CommunicatorId, (INT32) Size,
                              (INT64) Tag, Synchronism) < 0)
    return false;

  return true;
}

void
Send::Write(ostream& os) const
{
  os << "Send [" << TaskId << ":" << ThreadId << "] -> " << DestTaskId << ":" << DestThreadId;
  os << " Size: " << Size << " Tag: " << Tag;
  os << " CommId: " << CommunicatorId << " Sync: " << Synchronism;
  
  switch (Synchronism)
  {
    case 0:
      os << " (NO IMMD/NO RV)" << endl;
      break;
    case 1:
      os << " (NO IMMD/RV)" << endl;
      break;
    case 2:
      os << " (IMMD/NO RV)" << endl;
      break;
    case 3:
      os << " (IMMD/RV)" << endl;
      break;
    default:
      os << " (UNKNOWN!)" << endl;
      break;
  }
    
  return;
}

ostream& operator<< (ostream& os, const Send& SendRecord)
{
  SendRecord.Write(os);
  return os;
}

/*****************************************************************************
 * class Receive
 ****************************************************************************/

Receive::Receive(INT32 TaskId, INT32 ThreadId,
                 INT32 SrcTaskId,
                 INT32 SrcThreadId,
                 INT64 Size,
                 INT32 Tag,
                 INT32 CommunicatorId,
                 INT32 Type)
:DimemasRecord(TaskId, ThreadId)
{
  this->SrcTaskId      = SrcTaskId;
  this->SrcThreadId    = SrcThreadId;
  this->Size           = Size;
  this->Tag            = Tag;
  this->CommunicatorId = CommunicatorId;
  this->Type           = Type;
}

bool
Receive::IsReceive(void)
{
  if (Type == DIMEMAS_TRACE_RECV)
    return true;
  else
    return false;
}

bool
Receive::IsImmediateReceive(void)
{
  if (Type == DIMEMAS_TRACE_IRECV)
    return true;
  else
    return false;
}

bool
Receive::IsWait(void)
{
  if (Type == DIMEMAS_TRACE_WAIT)
    return true;
  else
    return false;
}

bool
Receive::ToDimemas(FILE* OutputTraceFile)
{
  if (Dimemas_NX_Generic_Recv(OutputTraceFile, TaskId, ThreadId,
                              SrcTaskId, SrcThreadId, CommunicatorId, (INT32) Size,
                              (INT64) Tag, Type) < 0)
    return false;
  
  return true;
}

void
Receive::Write(ostream& os) const
{
  os << "Recv [" << TaskId << ":" << ThreadId << "] <- " << SrcTaskId << ":" << SrcThreadId;
  os << " Size: " << Size << " Tag: " << Tag;
  os << " CommId: " << CommunicatorId << " Type: " << Type;
  
  switch (Type)
  {
    case DIMEMAS_TRACE_RECV:
      os << " (RECV)" << endl;
      break;
    case DIMEMAS_TRACE_IRECV:
      os << " (IRECV)" << endl;
      break;
    case DIMEMAS_TRACE_WAIT:
      os << " (WAIT)" << endl;
      break;
    default:
      os << " (UNKNOWN!)" << endl;
      break;
  }
    
    return;
}

ostream& operator<< (ostream& os, const Receive& ReceiveRecord)
{
  ReceiveRecord.Write(os);
  return os;
}

/*****************************************************************************
 * class GlobalOp
 ****************************************************************************/

GlobalOp::GlobalOp(INT32 TaskId, INT32 ThreadId,
                   INT32 GlobalOpId,
                   INT32 CommunicatorId,
                   INT32 RootTaskId,
                   INT32 RootThreadId,
                   INT64 SendSize, INT64 RecvSize)
:DimemasRecord(TaskId, ThreadId)
{
  this->GlobalOpId     = GlobalOpId;
  this->CommunicatorId = CommunicatorId;
  this->RootTaskId     = RootTaskId;
  this->RootThreadId   = RootThreadId;
  this->SendSize       = SendSize;
  this->RecvSize       = RecvSize;
  
  if (this->RootTaskId == this->TaskId && this->ThreadId == this->RootThreadId)
    this->Root = true;
  else
    this->Root = false;
}

bool
GlobalOp::ToDimemas(FILE* OutputTraceFile)
{
  if (Dimemas_Global_OP(OutputTraceFile, TaskId, ThreadId,
                        GlobalOpId, CommunicatorId,
                        RootTaskId, RootThreadId,
                        SendSize, RecvSize, 1) < 0)
    return false;
  
  return true;
}

void
GlobalOp::Write( ostream& os) const
{
  os << "GlobaOP [" << TaskId << "|" << ThreadId << "] (" << GlobalOpId << ") ";
  os << "CommId: " << CommunicatorId << " Root: [" << RootTaskId << "|";
  os << RootThreadId << "] Bytes send: " << SendSize;
  os << " Bytes recv: " << RecvSize << endl;
}

ostream& operator<< (ostream& os, const GlobalOp& GlobOp)
{
  GlobOp.Write(os);
  return os;
}

/*****************************************************************************
* class EventTypeValue
****************************************************************************/

INT64 EventTypeValue::CurrentTraceOrder = 0;

INT64 EventTypeValue::NewTraceOrder(void)
{
  return CurrentTraceOrder++;
}

/*****************************************************************************
* class Event
****************************************************************************/

Event::Event(INT32 TaskId, INT32 ThreadId)
:DimemasRecord(TaskId, ThreadId)
{
  ContentPresent = false;
}

Event::Event(INT32 TaskId, INT32 ThreadId, INT32 BlockId, bool BlockBegin)
:DimemasRecord(TaskId, ThreadId)
{
  EventTypeValue_t newTypeValue;
  INT64            FinalType;
  INT64            FinalValue;
  
  /*
  if (BlockId < 0 || BlockId > (BLOCK_ID_COUNT-1))
  {
    ContentPresent = false;
    return;
  }
  */
  
  /* Special case for Idle block! */
  if (BlockId == 0)
  {
    if (BlockBegin)
    {
      newTypeValue = new EventTypeValue(SPECIAL_EVENT_TYPE,
                                        1);
    }
    else
    {
      newTypeValue = new EventTypeValue(SPECIAL_EVENT_TYPE,
                                        0);
    }
  }
  else
  {
    Block_Dimemas2Paraver_TypeAndValue( (long long)  BlockId,
                                        (long long*) &FinalType,
                                        (long long*) &FinalValue);
      
    if (BlockBegin)
    {
      newTypeValue = new EventTypeValue(FinalType,
                                        FinalValue);
    }
    else
    {
      newTypeValue = new EventTypeValue(FinalType,
                                        0);
    }
  }
  
  /*
  if (BlockBegin)
  {
    newTypeValue = new EventTypeValue(Block2TypeValue[BlockId].Type,
                                      Block2TypeValue[BlockId].Value);
  }
  else
  {
    newTypeValue = new EventTypeValue(Block2TypeValue[BlockId].Type,
                                      0);
    
  }
  */
  
  Content.push_back(newTypeValue);
  ContentPresent = true;
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

bool
Event::ToDimemas(FILE* OutputTraceFile)
{
  if (Dimemas_User_Event(OutputTraceFile, TaskId, ThreadId,
                          GetFirstType(), GetFirstValue()) < 0)
    return false;
  
  return true;
}

void Event::Write(ostream& os) const
{
  os << "Event " << "[" << TaskId << "|" << ThreadId << "] Values: ";
  
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
