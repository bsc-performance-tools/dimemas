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

#ifndef _PARAVERRECORD_H
#define _PARAVERRECORD_H

#include <basic_types.h>
#include <vector>
using std::vector;

#include <ostream>
using std::ostream;

#include "EventEncoding.h"
#include "define.h"

#include <stdio.h>

#define PARAVER_STATE         1
#define PARAVER_EVENT         2
#define PARAVER_COMMUNICATION 3
#define PARAVER_GLOBALOP      4

/*****************************************************************************
 * class ParaverRecord
 ****************************************************************************/
class ParaverRecord
{
 protected:
  UINT64 RecordCount;
  UINT64 Timestamp;
  INT32 CPU, AppId, TaskId, ThreadId;

  static UINT64 CurrentRecordCount;

 public:
  ParaverRecord( void );

  ParaverRecord( UINT64 Timestamp, INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId );

  virtual ~ParaverRecord( void ){};

  virtual bool operator<( const ParaverRecord& T1 )
  {
    return this->Timestamp < T1.Timestamp;
    /*if (this->Timestamp < T1.Timestamp)
      return true;
    else
      return false;
    */
  };

  virtual bool operator>( const ParaverRecord& T1 )
  {
    return this->Timestamp > T1.Timestamp;
    /*
    if (this->Timestamp > T1.Timestamp)
      return true;
    else
      return false;
    */
  };

  virtual bool operator==( const ParaverRecord& T1 )
  {
    return this->Timestamp == T1.Timestamp;
    /*
    if (this->Timestamp == T1.Timestamp)
      return true;
    else
      return false;
    */
  };

  virtual UINT64 GetRecordCount( void )
  {
    return RecordCount;
  };
  virtual UINT64 GetTimestamp( void )
  {
    return Timestamp;
  };
  virtual INT32 GetCPU( void )
  {
    return CPU;
  };
  virtual INT32 GetAppId( void )
  {
    return AppId;
  };
  virtual INT32 GetTaskId( void )
  {
    return TaskId;
  };
  virtual INT32 GetThreadId( void )
  {
    return ThreadId;
  };

  static UINT64 NewRecordCount( void );

  virtual void Write( ostream& os ) const {};
};

typedef ParaverRecord* ParaverRecord_t;

ostream& operator<<( ostream& os, const ParaverRecord& Comm );

class ParaverRecordCompare
{
 public:
  bool operator()( ParaverRecord_t R1, ParaverRecord_t R2 )
  {
    return R1->GetTimestamp() < R2->GetTimestamp();
  }
};

/*****************************************************************************
 * class State
 ****************************************************************************/
class State : public virtual ParaverRecord
{
 private:
  UINT64 TimestampEnd;
  INT32 StateValue;

 public:
  State(){};
  State( INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId, UINT64 BeginTime, UINT64 EndTime, INT32 StateValue );

  UINT64 GetBeginTime( void )
  {
    return Timestamp;
  };
  UINT64 GetEndTime( void )
  {
    return TimestampEnd;
  };
  INT32 GetStateValue( void )
  {
    return StateValue;
  };

  void Write( ostream& os ) const;
};

ostream& operator<<( ostream& os, const State& Comm );

typedef State* State_t;

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

  EventTypeValue( INT32 Type, INT64 Value )
  {
    this->Type       = Type;
    this->Value      = Value;
    this->TraceOrder = EventTypeValue::NewTraceOrder();
  }

  INT32 GetType( void )
  {
    return Type;
  };
  INT64 GetValue( void )
  {
    return Value;
  };
  INT64 GetTraceOrder( void )
  {
    return TraceOrder;
  };

  bool ToDimemas( FILE* DimemasTrace, INT32 TaskId, INT32 ThreadId );

  bool IsMPIEvent( void )
  {
    if ( MPIEventEncoding_Is_MPIBlock( this->Type ) == TRUE )
      return true;
    else
      return false;
  }

  bool IsUserBlockBegin( void );
  bool IsMPIBlockBegin( void );
  bool IsUserBlockEnd( void );
  bool IsMPIBlockEnd( void );
  bool IsCaller( void );
  bool IsCallerLine( void );

  static INT64 NewTraceOrder( void );
};
typedef EventTypeValue* EventTypeValue_t;

class Event : public virtual ParaverRecord
{
 private:
  vector<EventTypeValue_t> Content;
  bool ContentPresent;

 public:
  Event()
  {
    ContentPresent = false;
  };
  ~Event();

  Event( UINT64 Timestamp, INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId );

  void AddTypeValue( INT32 Type, INT64 Value );

  UINT32 GetTypeValueCount( void );

  INT32 GetFirstType( void );
  INT32 GetType( UINT32 Index );

  INT64 GetFirstValue( void );
  INT64 GetValue( UINT32 Index );

  INT64 GetFirstTraceOrder( void );
  INT64 GetTraceOrder( UINT32 Index );

  bool IsUserBlockBegin( void );
  bool IsMPIBlockBegin( void );
  bool IsUserBlockEnd( void );
  bool IsMPIBlockEnd( void );

  bool IsCaller( void );
  bool IsCallerLine( void );

  void Write( ostream& os ) const;
};
typedef Event* Event_t;

ostream& operator<<( ostream& os, const Event& Comm );

/*****************************************************************************
 * class Communication
 ****************************************************************************/
class Communication : public virtual ParaverRecord
{
 private:
  INT32 DestCPU, DestAppId, DestTaskId, DestThreadId;
  UINT64 PhysicalSend, LogicalRecv, PhysicalRecv;
  INT32 Size;
  INT32 Tag;
  UINT64 TraceOrder;

  static UINT64 CurrentTraceOrder;

 public:
  Communication( UINT64 LogSend,
                 UINT64 PhySend,
                 UINT64 LogRecv,
                 UINT64 PhyRecv,
                 INT32 SrcCPU,
                 INT32 SrcAppId,
                 INT32 SrcTaskId,
                 INT32 SrcThreadId,
                 INT32 DstCPU,
                 INT32 DstAppId,
                 INT32 DstTaskId,
                 INT32 DstThreadId,
                 INT32 Size,
                 INT32 Tag );

  UINT64 GetLogicalSend( void )
  {
    return Timestamp;
  };
  UINT64 GetPhysicalSend( void )
  {
    return PhysicalSend;
  };
  UINT64 GetLogicalRecv( void )
  {
    return LogicalRecv;
  };
  UINT64 GetPhysicalRecv( void )
  {
    return PhysicalRecv;
  };

  INT32 GetSrcCPU( void )
  {
    return CPU;
  };
  INT32 GetSrcAppId( void )
  {
    return AppId;
  };
  INT32 GetSrcTaskId( void )
  {
    return TaskId;
  };
  INT32 GetSrcThreadId( void )
  {
    return ThreadId;
  };
  INT32 GetDstCPU( void )
  {
    return DestCPU;
  };
  INT32 GetDstAppId( void )
  {
    return DestAppId;
  };
  INT32 GetDstTaskId( void )
  {
    return DestTaskId;
  };
  INT32 GetDstThreadId( void )
  {
    return DestThreadId;
  };
  INT32 GetSize( void )
  {
    return Size;
  };
  INT32 GetTag( void )
  {
    return Tag;
  };
  INT64 GetTraceOrder( void )
  {
    return TraceOrder;
  };

  static UINT64 NewTraceOrder( void );

  void Write( ostream& os ) const;
};
typedef Communication* Communication_t;

ostream& operator<<( ostream& os, const Communication& Comm );


/*****************************************************************************
 * class GlobalOp
 ****************************************************************************/

#define UNKNOWN_ROOT -1

class GlobalOp : public virtual ParaverRecord
{
 private:
  INT32 CommunicatorId;
  INT32 SendSize, RecvSize;
  INT32 GlobalOpId;
  INT32 RootTaskId;

  INT32 Synchronize;
  bool Root;

 public:
  GlobalOp( UINT64 Timestamp,
            INT32 CPU,
            INT32 AppId,
            INT32 TaskId,
            INT32 ThreadId,
            INT32 CommunicatorId,
            INT32 SendSize,
            INT32 RecvSize,
            INT32 GlobalOpId,
            INT32 RootTaskId );

  GlobalOp( UINT64 Timestamp,
            INT32 CPU,
            INT32 AppId,
            INT32 TaskId,
            INT32 ThreadId,
            INT32 CommunicatorId,
            INT32 SendSize,
            INT32 RecvSize,
            INT32 GlobalOpId,
            bool IsRoot );

  ~GlobalOp( void ){};

  void SetCommunicatorId( INT32 CommunicatorId )
  {
    this->CommunicatorId = CommunicatorId;
  };
  INT32 GetCommunicatorId( void )
  {
    return CommunicatorId;
  };

  void SetSendSize( INT32 SendSize )
  {
    this->SendSize = SendSize;
  };

  INT32 GetSendSize( void )
  {
    return SendSize;
  };

  void SetRecvSize( INT32 RecvSize )
  {
    this->RecvSize = RecvSize;
  };

  INT32 GetRecvSize( void )
  {
    return RecvSize;
  };

  void SetGlobaOpId( INT32 GlobaOpId )
  {
    this->GlobalOpId = GlobalOpId;
  }

  INT32 GetGlobalOpId( void )
  {
    if ( GlobalOpId >= GLOP_ID_IMMEDIATE )
      return GlobalOpId - GLOP_ID_IMMEDIATE;
    else
      return GlobalOpId;
  };

  void SetRootTaskId( INT32 RootTaskId )
  {
    this->RootTaskId = RootTaskId;
  };

  INT32 GetRootTaksId( void )
  {
    return RootTaskId;
  };

  void SetIsRoot( bool Root )
  {
    this->Root = Root;
  };

  bool GetIsRoot( void )
  {
    return Root;
  };

  INT32 GetSynch( void )
  {
    return Synchronize;
  };

  void Write( ostream& os ) const;
};

typedef GlobalOp* GlobalOp_t;

ostream& operator<<( ostream& os, const GlobalOp& Comm );

#endif /* _PARAVERRECORD_H */
