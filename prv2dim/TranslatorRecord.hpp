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

#ifndef _TRANSLATORRECORD_H
#define _TRANSLATORRECORD_H

#include <iostream>
using std::cout;
using std::endl;
using std::ostream;
#include "ParaverCommunicator.hpp"
#include "ParaverRecord.hpp"

/*****************************************************************************
 * class PartialCommunication
 ****************************************************************************/

/* Communication record identifiers */
#define LOGICAL_SEND  0
#define PHYSICAL_SEND 1
#define LOGICAL_RECV  2
#define PHYSICAL_RECV 3

class PartialCommunication : virtual public ParaverRecord
{
 private:
  INT32 Type;
  INT32 PartnerCPU, PartnerAppId, PartnerTaskId, PartnerThreadId;
  INT32 Size;
  INT32 Tag;
  INT32 CommId; /* Pseudo-Communicator ID for Dimemas coherence */
  UINT64 TraceOrder;

 public:
  PartialCommunication( INT32 Type,
                        UINT64 Timestamp,
                        INT32 SrcCPU,
                        INT32 SrcAppId,
                        INT32 SrcTaskId,
                        INT32 SrcThreadId,
                        INT32 DstCPU,
                        INT32 DstAppId,
                        INT32 DstTaskId,
                        INT32 DstThreadId,
                        INT32 Size,
                        INT32 Tag,
                        INT32 CommId,
                        UINT64 TraceOrder );

  PartialCommunication( INT32 Type, Communication_t Comm, INT32 CommId );

  INT32 GetType( void )
  {
    return Type;
  };
  INT32 GetPartnerTaskId( void )
  {
    return PartnerTaskId;
  };
  INT32 GetPartnerThreadId( void )
  {
    return PartnerThreadId;
  };
  INT32 GetSize( void )
  {
    return Size;
  };
  INT32 GetTag( void )
  {
    return Tag;
  };
  INT32 GetCommId( void )
  {
    return CommId;
  };
  UINT64 GetTraceOrder( void )
  {
    return TraceOrder;
  };

  void Write( ostream& os ) const;

  bool ToDimemas( FILE* DimemasTrace )
  {
    return true;
  };

 private:
  void
  InitFields( INT32 SrcCPU, INT32 SrcAppId, INT32 SrcTaskId, INT32 SrcThreadId, INT32 DstCPU, INT32 DstAppId, INT32 DstTaskId, INT32 DstThreadId );
};
typedef PartialCommunication* PartialCommunication_t;

ostream& operator<<( ostream& os, const PartialCommunication& Comm );

/*****************************************************************************
 * class TranslationRecordCompare
 ****************************************************************************/

class OutBlockComparison
{
 public:
  bool operator()( ParaverRecord_t R1, ParaverRecord_t R2 )
  {
    Event_t EventR1, EventR2;
    PartialCommunication_t CommR1, CommR2;
    GlobalOp_t GlobalOpR1, GlobalOpR2;

    // printf("%d : %d \n", EventR1, EventR2 );
    if ( R1->GetTimestamp() != R2->GetTimestamp() )
      return R1->GetTimestamp() < R2->GetTimestamp();

    /* Records at the same time */
    if ( IsEvent( R1 ) && IsEvent( R2 ) )
    {
      EventR1 = dynamic_cast<Event_t>( R1 );
      EventR2 = dynamic_cast<Event_t>( R2 );

      /* Special case for PROBE counters */
      if ( EventR1->GetFirstType() == MPITYPE_PROBE_SOFTCOUNTER && EventR2->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER )
      {
        return true;
      }

      if ( EventR1->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER && EventR2->GetFirstType() == MPITYPE_PROBE_SOFTCOUNTER )
      {
        return false;
      }

      if ( ( EventR1->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER || EventR1->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER ) &&
           EventR2->GetFirstType() == CLUSTER_ID_EV )
      {
        return true;
      }

      if ( EventR1->GetFirstType() == CLUSTER_ID_EV &&
           ( EventR2->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER || EventR2->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER ) )
      {
        return false;
      }

      /* Preserve trace order */
      return EventR1->GetFirstTraceOrder() < EventR2->GetFirstTraceOrder();
    }
    else if ( IsEvent( R1 ) && IsCommunication( R2 ) )
    {
      EventR1 = dynamic_cast<Event_t>( R1 );
      CommR2  = dynamic_cast<PartialCommunication_t>( R2 );

      if ( EventR1->IsMPIBlockEnd() )
        return false;

      return true;
    }
    else if ( IsEvent( R1 ) && IsGlobalOp( R2 ) )
    {
      EventR1    = dynamic_cast<Event_t>( R1 );
      GlobalOpR2 = dynamic_cast<GlobalOp_t>( R2 );

      if ( EventR1->IsMPIBlockBegin() )
        return true;
      else if ( EventR1->IsMPIBlockEnd() )
        return false;
      else
        return true;
    }
    else if ( IsCommunication( R1 ) && IsEvent( R2 ) )
    {
      CommR1  = dynamic_cast<PartialCommunication_t>( R1 );
      EventR2 = dynamic_cast<Event_t>( R2 );

      /*
      if (EventR2->IsDimemasBlockBegin())
        return false;

      if (EventR2->IsMPIBlockBegin())
        return true;
      */

      /*
      if (EventR2->IsCaller() || EventR2->IsCallerLine())
        return false;
      */

      return false;
    }
    else if ( IsGlobalOp( R1 ) && IsEvent( R2 ) )
    {
      GlobalOpR1 = dynamic_cast<GlobalOp_t>( R1 );
      EventR2    = dynamic_cast<Event_t>( R2 );

      if ( EventR2->IsMPIBlockBegin() )
        return false;
      else if ( EventR2->IsMPIBlockEnd() )
        return true;
      else
        return false;
    }
    else if ( IsCommunication( R1 ) && IsCommunication( R2 ) )
    {
      CommR1 = dynamic_cast<PartialCommunication_t>( R1 );
      CommR2 = dynamic_cast<PartialCommunication_t>( R2 );

      if ( CommR1->GetType() == LOGICAL_RECV )

        return false;

      if ( CommR1->GetType() == LOGICAL_SEND && CommR2->GetType() == PHYSICAL_SEND )
        return true;

      if ( CommR1->GetType() == PHYSICAL_SEND && CommR2->GetType() == PHYSICAL_RECV )
        return true;

      return false;
    }
    else if ( IsCommunication( R1 ) && IsGlobalOp( R2 ) )
    {
      return true;
    }
    else if ( IsGlobalOp( R1 ) && IsCommunication( R2 ) )
    {
      return true;
    }
    else if ( IsGlobalOp( R1 ) && IsGlobalOp( R2 ) )
    {
      return true;
    }

    return true;
  };

 private:
  bool IsCommunication( ParaverRecord_t R1 )
  {
    PartialCommunication_t Comm;

    if ( ( Comm = dynamic_cast<PartialCommunication_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  };

  bool IsEvent( ParaverRecord_t R1 )
  {
    Event_t Evt;

    if ( ( Evt = dynamic_cast<Event_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  }

  bool IsGlobalOp( ParaverRecord_t R1 )
  {
    GlobalOp_t GlobOp;

    if ( ( GlobOp = dynamic_cast<GlobalOp_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  }
};

class InBlockComparison
{
 public:
  bool operator()( ParaverRecord_t R1, ParaverRecord_t R2 )
  {
    Event_t EventR1, EventR2;
    PartialCommunication_t CommR1, CommR2;
    GlobalOp_t GlobalOpR1, GlobalOpR2;

    if ( R1->GetTimestamp() != R2->GetTimestamp() )
      return R1->GetTimestamp() < R2->GetTimestamp();

    /* Records at the same time */
    if ( IsEvent( R1 ) && IsEvent( R2 ) )
    {
      EventR1 = dynamic_cast<Event_t>( R1 );
      EventR2 = dynamic_cast<Event_t>( R2 );

      /* Special case for CALLERS */
      if ( ( ( EventR1->GetFirstType() >= MPI_CALLER_EV ) && ( EventR1->GetFirstType() <= MPI_CALLER_EV_END ) ) &&
           ( ( EventR2->GetFirstType() >= MPI_CALLER_LINE_EV ) && ( EventR2->GetFirstType() <= MPI_CALLER_LINE_EV_END ) ) )
        return true;

      if ( ( ( EventR1->GetFirstType() >= MPI_CALLER_EV ) && ( EventR1->GetFirstType() <= MPI_CALLER_EV_END ) ) ||
           ( ( EventR1->GetFirstType() >= MPI_CALLER_LINE_EV ) && ( EventR1->GetFirstType() <= MPI_CALLER_LINE_EV_END ) ) )
        return false;

      if ( ( ( EventR2->GetFirstType() >= MPI_CALLER_EV ) && ( EventR2->GetFirstType() <= MPI_CALLER_EV_END ) ) ||
           ( ( EventR2->GetFirstType() >= MPI_CALLER_LINE_EV ) && ( EventR2->GetFirstType() <= MPI_CALLER_LINE_EV_END ) ) )
        return true;

      /* Special case for PROBE counters */
      if ( EventR1->GetFirstType() == MPITYPE_PROBE_SOFTCOUNTER && EventR2->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER )
        return true;

      if ( EventR1->GetFirstType() == MPITYPE_PROBE_TIMECOUNTER && EventR2->GetFirstType() == MPITYPE_PROBE_SOFTCOUNTER )
        return false;

      /* Preserve trace order */
      return EventR1->GetFirstTraceOrder() < EventR2->GetFirstTraceOrder();
    }
    else if ( IsEvent( R1 ) && IsCommunication( R2 ) )
    {
      return false; /* TESTING */
    }
    else if ( IsEvent( R1 ) && IsGlobalOp( R2 ) )
    {
      return false; /* TESTING */
    }
    else if ( IsCommunication( R1 ) && IsEvent( R2 ) )
    {
      return true;
    }
    else if ( IsGlobalOp( R1 ) && IsEvent( R2 ) )
    {
      return true;
    }
    else if ( IsCommunication( R1 ) && IsCommunication( R2 ) )
    {
      CommR1 = dynamic_cast<PartialCommunication_t>( R1 );
      CommR2 = dynamic_cast<PartialCommunication_t>( R2 );

      if ( ( CommR1->GetType() == LOGICAL_RECV && CommR2->GetType() == LOGICAL_RECV ) ||
           ( CommR1->GetType() == PHYSICAL_RECV && CommR2->GetType() == PHYSICAL_RECV ) ||
           ( CommR1->GetType() == LOGICAL_SEND && CommR2->GetType() == LOGICAL_SEND ) ||
           ( CommR1->GetType() == PHYSICAL_SEND && CommR2->GetType() == PHYSICAL_SEND ) )
      {
        return CommR1->GetTraceOrder() < CommR2->GetTraceOrder();
        // return CommR1->GetTaskId() < CommR2->GetTaskId();
      }

      if ( CommR1->GetType() == PHYSICAL_SEND && CommR2->GetType() == PHYSICAL_SEND )
      {
        return CommR1->GetTraceOrder() < CommR2->GetTraceOrder();
        // return CommR1->GetTaskId() < CommR2->GetTaskId();
      }

      if ( CommR1->GetType() == LOGICAL_RECV )
        return true;

      if ( CommR1->GetType() == LOGICAL_SEND && CommR2->GetType() == PHYSICAL_SEND )
        return true;

      if ( CommR1->GetType() == PHYSICAL_SEND && CommR2->GetType() == PHYSICAL_RECV )
        return true;

      if ( CommR1->GetType() == LOGICAL_SEND && CommR2->GetType() == PHYSICAL_RECV )
        return true;

      return false;
    }
    else if ( IsCommunication( R1 ) && IsGlobalOp( R2 ) )
    {
      return true;
    }
    else if ( IsGlobalOp( R1 ) && IsCommunication( R2 ) )
    {
      return true;
    }
    else if ( IsGlobalOp( R1 ) && IsGlobalOp( R2 ) )
    {
      return true;
    }

    return true;
  };

 private:
  bool IsCommunication( ParaverRecord_t R1 )
  {
    PartialCommunication_t Comm;

    if ( ( Comm = dynamic_cast<PartialCommunication_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  };

  bool IsEvent( ParaverRecord_t R1 )
  {
    Event_t Evt;

    if ( ( Evt = dynamic_cast<Event_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  }

  bool IsGlobalOp( ParaverRecord_t R1 )
  {
    GlobalOp_t GlobOp;

    if ( ( GlobOp = dynamic_cast<GlobalOp_t>( R1 ) ) != NULL )
      return true;
    else
      return false;
  }
};

/*****************************************************************************
 * class TranslationCommunicator
 ****************************************************************************/

class TranslationCommunicator : public Communicator
{
 private:
  bool PendingGlobalOp;
  bool FinishedGlobalOp;
  set<INT32> TaskIdArrived;
  vector<GlobalOp_t> GlobalOpsArrived;
  INT32 RootTaskId;

 public:
  TranslationCommunicator( Communicator_t PrvCommunicator ) : Communicator( *PrvCommunicator )
  {
    set<INT32> Tasks;
    set<INT32>::iterator it;

    Tasks = PrvCommunicator->GetCommunicatorTasks();
    it    = CommunicatorTasks.begin();

    RootTaskId       = (INT32)*it;
    PendingGlobalOp  = false;
    FinishedGlobalOp = false;
  };

  void SetPendingGlobalOp( bool PendingGlobalOp )
  {
    this->PendingGlobalOp = PendingGlobalOp;
  };
  bool GetPendingGlobalOp( void )
  {
    return PendingGlobalOp;
  };
  bool GetFinisehdGlobalOp( void )
  {
    return FinishedGlobalOp;
  };
  INT32 GetRootTaskId( void )
  {
    return RootTaskId;
  };
};
typedef TranslationCommunicator* TranslationCommunicator_t;

#endif /* _TRANSLATORRECORD_H */
