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

#include "TranslatorRecord.hpp"
#include <iostream>
using std::cout;
using std::endl;

/*****************************************************************************
 * class PartialCommunication
 ****************************************************************************/
/*****************************************************************************
 * Public functions
 ****************************************************************************/

PartialCommunication::PartialCommunication(INT32 Type,
                                           UINT64 Timestamp,
                                           INT32 SrcCPU,    INT32 SrcAppId,
                                           INT32 SrcTaskId, INT32 SrcThreadId,
                                           INT32 DstCPU,    INT32 DstAppId,
                                           INT32 DstTaskId, INT32 DstThreadId,
                                           INT32 Size,      INT32 Tag,
                                           INT32 CommId)
{
  this->Type      = Type;
  this->Timestamp = Timestamp;
  
  InitFields(SrcCPU, SrcAppId, SrcTaskId, SrcThreadId,
             DstCPU, DstAppId, DstTaskId, DstThreadId);
  
  this->Size   = Size;
  this->Tag    = Tag;
  this->CommId = CommId;
}

PartialCommunication::PartialCommunication(INT32           Type,
                                           Communication_t Comm,
                                           INT32           CommId)
{
  UINT64 InterestingTime;
  INT32  SrcCPU, SrcAppId, SrcTaskId, SrcThreadId;
  INT32  DstCPU, DstAppId, DstTaskId, DstThreadId;
  INT32  ReadedSize, ReadedTag;
  
  SrcCPU      = Comm->GetSrcCPU();
  SrcAppId    = Comm->GetSrcAppId();
  SrcTaskId   = Comm->GetSrcTaskId();
  SrcThreadId = Comm->GetSrcThreadId();
  
  DstCPU      = Comm->GetDstCPU();
  DstAppId    = Comm->GetDstAppId();
  DstTaskId   = Comm->GetDstTaskId();
  DstThreadId = Comm->GetDstThreadId();
  
  ReadedSize  = Comm->GetSize();
  ReadedTag   = Comm->GetTag();
  
  this->Type = Type;
  
  switch(Type)
  {
    case LOGICAL_SEND:
      InterestingTime = Comm->GetLogicalSend();
      break;
    case PHYSICAL_SEND:
      InterestingTime = Comm->GetPhysicalSend();
      break;
    case LOGICAL_RECV:
      InterestingTime = Comm->GetLogicalRecv();
      break;
    case PHYSICAL_RECV:
      InterestingTime = Comm->GetPhysicalRecv();
      break;
  }

  /*
  cout << "Creating Partial Communication" << endl;
  cout << "Type:" << InputType << " Timestamp: " << InterestingTime << endl;

  cout << "Main:    " << SrcCPU << ":" << SrcAppId << ":" << SrcTaskId;
  cout << ":" << SrcThreadId << endl;
  
  cout << "Partner: " << DstCPU << ":" << DstAppId << ":" << DstTaskId;
  cout << ":" << DstThreadId << endl;
  
  cout << "Size: " << ReadedSize << " Tag: " << ReadedTag << endl;
  */
  Timestamp = InterestingTime;
  
  InitFields(SrcCPU, SrcAppId, SrcTaskId, SrcThreadId,
             DstCPU, DstAppId, DstTaskId, DstThreadId);
  
  this->Size   = ReadedSize;
  this->Tag    = ReadedTag;
  this->CommId = CommId;
  
}

void PartialCommunication::Write ( ostream & os ) const
{
  switch (Type)
  {
    case LOGICAL_RECV:
      os << "LOG_RECV";
      break;
    case LOGICAL_SEND:
      os << "LOG_SEND";
      break;
    case PHYSICAL_RECV:
      os << "PHY_RECV";
      break;
    case PHYSICAL_SEND:
      os << "PHY_SEND";
      break;
    default:
      os << "UNKNOW!";
      break;
  }

  os << " [";
  
  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "] ";
  
  os << " T: " << Timestamp;
  
  os << " Partner: ";
  
  os << "[";
  
  os.width(3);
  os.fill('0');
  os << PartnerTaskId << ":";

  os.width(2);
  os.fill('0');
  os << PartnerThreadId << "] ";
  
  os << "Size: " << Size << " Tag: " << Tag << " CommId: " << CommId << endl;
}

void PartialCommunication::ToFile(FILE* OutputFile)
{
  fprintf(OutputFile,
          "%d:%llu:%d,%d:%d,%d:%d:%d\n",
          Type,
          Timestamp,
          TaskId,
          ThreadId,
          PartnerTaskId,
          PartnerThreadId,
          Size,
          Tag);
  return;
}

ostream& operator<< (ostream& os, const PartialCommunication& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void PartialCommunication::InitFields(INT32 SrcCPU,    INT32 SrcAppId,
                                      INT32 SrcTaskId, INT32 SrcThreadId,
                                      INT32 DstCPU,    INT32 DstAppId,
                                      INT32 DstTaskId, INT32 DstThreadId)
{
  switch(this->Type)
  {
    case LOGICAL_SEND:
    case PHYSICAL_SEND:
      CPU             = SrcCPU;
      AppId           = SrcAppId;
      TaskId          = SrcTaskId;
      ThreadId        = SrcThreadId;
      PartnerCPU      = DstCPU;
      PartnerAppId    = DstAppId;
      PartnerTaskId   = DstTaskId;
      PartnerThreadId = DstThreadId;
      break;
    case LOGICAL_RECV:
    case PHYSICAL_RECV:
      CPU             = DstCPU;
      AppId           = DstAppId;
      TaskId          = DstTaskId;
      ThreadId        = DstThreadId;
      PartnerCPU      = SrcCPU;
      PartnerAppId    = SrcAppId;
      PartnerTaskId   = SrcTaskId;
      PartnerThreadId = SrcThreadId;
  }
}

/*****************************************************************************
 * class TranslationCommunicator
 ****************************************************************************/
/*****************************************************************************
 * Public functions
 ****************************************************************************/

bool
TranslationCommunicator::AddGlobalOp(GlobalOp_t NewGlobalOp)
{
  if (!PendingGlobalOp)
  {
    FinishedGlobalOp = false;
    PendingGlobalOp  = true;
  }
  
  if (CommunicatorTasks.count(NewGlobalOp->GetTaskId()) != 1)
  {
    char CurrentError[128]; 
    
    sprintf(CurrentError,
      "Adding operation from task %02d not associated with communicator %02d",
      NewGlobalOp->GetTaskId(),
      CommunicatorId);

    LastError = CurrentError;
    return false;
  }
  
  /*
  if (TaskIdArrived.count(NewGlobalOp->GetTaskId()) == 1)
  {
    char CurrentError[128]; 
    
    sprintf(CurrentError,
      "Arrived two global op operations from task %02d on communicator %02d",
      NewGlobalOp->GetTaskId(),
      CommunicatorId);

    LastError = CurrentError;
    return false;
  }
  */
  
  TaskIdArrived.insert(NewGlobalOp->GetTaskId());
  GlobalOpsArrived.push_back(NewGlobalOp);

  if (NewGlobalOp->GetIsRoot())
  {
    RootTaskId = NewGlobalOp->GetTaskId();
  }
  
  if (TaskIdArrived.size() == CommunicatorTasks.size())
  { /* Global Op finished!! */
    for (UINT32 i = 0; i < GlobalOpsArrived.size(); i++)
    { /* Root TaskId distribution */
      GlobalOpsArrived[i]->SetRootTaskId(RootTaskId);
    }
    
    /* Clear all containers */
    GlobalOpsArrived.clear();
    TaskIdArrived.clear();
    
    PendingGlobalOp  = false;
    FinishedGlobalOp = true;
  }
  
  return true;
}
