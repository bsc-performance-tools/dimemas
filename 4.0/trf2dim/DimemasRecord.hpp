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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _DIMEMASRECORD_H
#define _DIMEMASRECORD_H

#include <basic_types.h>
#include <EventEncoding.h>

#include <vector>
using std::vector;

#include <ostream>
using std::ostream;

#include <cstdio>

#define DIMEMAS_CPUBURST 1
#define DIMEMAS_SEND     2
#define DIMEMAS_RECEIVE  3
#define DIMEMAS_GLOBALOP 10
#define DIMEMAS_EVENT    20

/*****************************************************************************
 * class ParaverRecord
 ****************************************************************************/
class DimemasRecord
{
  protected:
    INT32 TaskId, ThreadId;

  public:
    DimemasRecord(void){};
    
    DimemasRecord(INT32 TaskId, INT32 ThreadId);
      
    virtual ~DimemasRecord(void){};
      
    virtual INT32 GetTaskId(void)   { return TaskId; };
    virtual INT32 GetThreadId(void) { return ThreadId; };
    
    virtual bool ToDimemas(FILE* OutputTraceFile) {};
    
    virtual void Write(ostream& os) const {};
};
typedef DimemasRecord* DimemasRecord_t;

ostream& operator<< (ostream& os, const DimemasRecord& Record);


/*****************************************************************************
 * class CPUBurst
 ****************************************************************************/
class CPUBurst: public virtual DimemasRecord
{
  private:
    double BurstDuration;
  
  public:
    CPUBurst() {};

    CPUBurst(INT32 TaskId, INT32 ThreadId, double BurstDuration);
      
    double GetBurstDuration(void) { return BurstDuration; };
    
    bool ToDimemas(FILE* OutputTraceFile);
    
    void Write(ostream& os) const;
};

ostream& operator<< (ostream& os, const CPUBurst& Comm);

typedef CPUBurst* CPUBurst_t;



/*****************************************************************************
 * class Send
 ****************************************************************************/
#define IMMEDIATE_MASK  0x00000002
#define RENDEZVOUS_MASK 0x00000001

class Send: public virtual DimemasRecord
{
  private:
    INT32 DestTaskId;
    INT64 Size;
    INT32 Tag;
    INT32 CommunicatorId;
    INT32 Synchronism;
  
  public:
    Send() {};
    
    Send(INT32 TaskId, INT32 ThreadId,
         INT32 DestTaskId,
         INT64 Size,
         INT32 Tag,
         INT32 CommunicatorId,
         INT32 Synchronism);

    INT32 GetDestTaskId(void)     { return DestTaskId; };
    INT64 GetSize(void)           { return Size; };
    INT32 GetTag(void)            { return Tag; };
    INT32 GetCommunicatorId(void) { return CommunicatorId; };
    INT32 GetSynchronism(void)    { return Synchronism; };
    
    bool  IsImmediate(void);
    bool  IsRendezVous(void);
    
    bool ToDimemas(FILE* OutputTraceFile);
    
    void Write(ostream& os) const;
};
typedef Send* Send_t;

ostream& operator<< (ostream& os, const Send& Comm);

/*****************************************************************************
 * class Receive
 ****************************************************************************/

#define RECV  0
#define WAIT  2
#define IRECV 1

class Receive: public virtual DimemasRecord
{
  private:
    INT32 SrcTaskId;
    INT64 Size;
    INT32 Tag;
    INT32 CommunicatorId;
    INT32 Type;
  
  public:
    Receive() {};
    
    Receive(INT32 TaskId, INT32 ThreadId,
            INT32 SrcTaskId,
            INT64 Size,
            INT32 Tag,
            INT32 CommunicatorId,
            INT32 Type);

    INT32 SrcDestTaskId(void)     { return SrcTaskId; };
    INT64 GetSize(void)           { return Size; };
    INT32 GetTag(void)            { return Tag; };
    INT32 GetCommunicatorId(void) { return CommunicatorId; };
    INT32 GetType(void)           { return Type; };
    
    bool  IsReceive(void);
    bool  IsImmediateReceive(void);
    bool  IsWait(void);
    
    bool ToDimemas(FILE* OutputTraceFile);
    
    void Write(ostream& os) const;
};
typedef Receive* Receive_t;

ostream& operator<< (ostream& os, const Receive& Comm);

/*****************************************************************************
 * class GlobalOp
 ****************************************************************************/

#define UNKNOWN_ROOT -1

class GlobalOp: public virtual DimemasRecord
{
  private:
    INT32 CommunicatorId;
    INT32 GlobalOpId;
    INT32 RootTaskId;
    INT32 RootThreadId;
    INT64 SendSize, RecvSize;
    bool  Root;
  public:
    GlobalOp(INT32 TaskId, INT32 ThreadId,
             INT32 GlobalOpId,
             INT32 CommunicatorId,
             INT32 RootTaskId,
             INT32 RootThreadId,
             INT64 SendSize, INT64 RecvSize);
  
    ~GlobalOp(void){};
  
    void  SetCommunicatorId(INT32 CommunicatorId)
    {
      this->CommunicatorId = CommunicatorId;
    };
    INT32 GetCommunicatorId(void) { return CommunicatorId; };

    void SetSendSize(INT32 SendSize)
    {
      this->SendSize = SendSize;
    };
    INT32 GetSendSize(void)       { return SendSize; };

    void SetRecvSize(INT32 RecvSize)
    {
      this->RecvSize = RecvSize;
    };
    INT32 GetRecvSize(void)       { return RecvSize; };

    void SetGlobaOpId(INT32 GlobaOpId)
    {
      this->GlobalOpId = GlobalOpId;
    }
    INT32 GetGlobalOpId(void)     { return GlobalOpId; };

    void SetRootTaskId(INT32 RootTaskId) 
    { 
      this->RootTaskId = RootTaskId;
    };
    INT32 GetRootTaksId(void)     { return RootTaskId; };

    void  SetIsRoot(bool Root)
    {
      this->Root = Root;
    };
    bool  GetIsRoot(void)         { return Root; };
    
    bool ToDimemas(FILE* OutputTraceFile);
  
    void Write(ostream& os) const;
};
typedef GlobalOp* GlobalOp_t;

ostream& operator<< (ostream& os, const GlobalOp& Comm);


/*****************************************************************************
 * class Event
 ****************************************************************************/

/* EventTypeValue is a contanier to store Paraver event Type/Value pairs */
class EventTypeValue
{
  private:
    INT32 Type;
    INT64 Value;
    INT64 TraceOrder;
  
    static INT64 CurrentTraceOrder;
  public:
    EventTypeValue(){};
    EventTypeValue(INT32 Type, INT64 Value)
    {
      this->Type       = Type;
      this->Value      = Value;
      this->TraceOrder = EventTypeValue::NewTraceOrder();
    }
    
    INT32 GetType(void)       { return Type; };
    INT64 GetValue(void)      { return Value; };
    INT64 GetTraceOrder(void) { return TraceOrder; };
    
    bool IsMPIEvent(void)
    {
      #define TRUE  1
      #define FALSE !TRUE
      if (MPIEventEncoding_Is_MPIBlock(this->Type) == TRUE)
        return true;
      else
        return false;
    }
    
    static INT64 NewTraceOrder(void);
};
typedef EventTypeValue* EventTypeValue_t;

class Event: public virtual DimemasRecord
{
  private:
    vector<EventTypeValue_t> Content;
    bool ContentPresent;

  public:
    Event(){ ContentPresent = false; };
    ~Event();

    Event(INT32 TaskId, INT32 ThreadId);
    
    Event(INT32 TaskId, INT32 ThreadId, INT32 BlockId, bool BlockBegin);
  
    void AddTypeValue(INT32 Type, INT64 Value);
    
    UINT32 GetTypeValueCount(void);
    
    INT32 GetFirstType(void);
    INT32 GetType(UINT32 Index);
    
    INT64 GetFirstValue(void);
    INT64 GetValue(UINT32 Index);
    
    INT64 GetFirstTraceOrder(void);
    INT64 GetTraceOrder(UINT32 Index);
    
    bool ToDimemas(FILE* OutputTraceFile);
    
    void Write(ostream& os) const;
};
typedef Event* Event_t;

ostream& operator<< (ostream& os, const Event& Comm);

#endif /* _DIMEMASRECORD_H */
