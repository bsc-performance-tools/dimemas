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

#include "ParaverCommunicator.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using std::cout;
using std::endl;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

Communicator::Communicator( char* ASCIICommunicator )
{
  INT32 AppId, CommId, TaskCount;
  char* TaskList = (char*)calloc( strlen( ASCIICommunicator ) + 1, sizeof( char ) );

  if ( sscanf( ASCIICommunicator, "C:%d:%d:%d:%[^\n]\n", &AppId, &CommId, &TaskCount, TaskList ) == 4 )
  {
    CommunicatorId = CommId;
    ApplicationId  = AppId;
  }
  else if ( sscanf( ASCIICommunicator, "c:%d:%d:%d:%[^\n]\n", &AppId, &CommId, &TaskCount, TaskList ) == 4 )
  {
    CommunicatorId = CommId;
    ApplicationId  = AppId;
  }
  else if ( sscanf( ASCIICommunicator, "c:%d:%d:%d\n", &AppId, &CommId, &TaskCount ) == 3 )
  {
    // Special case for void communicators
    //
    CommunicatorId = CommId;
    ApplicationId  = AppId;
    if ( TaskCount != 0 )
    {
      SetError( true );
      LastError = "wrong communicator format";
      return;
    }
  }
  else
  {
    SetError( true );
    LastError = "wrong communicator format";
    return;
  }

  ParseTaskList( TaskList );

  if ( CommunicatorTasks.size() != TaskCount )
  {
    SetError( true );
    LastError = "number of task involved differs from task count";
    return;
  }

  if ( CommunicatorTasks.size() == 1 && CommunicatorTasks.count( -1 ) == 1 )
    COMM_SELF = true;

  return;
}

Communicator::Communicator( const Communicator& Comm )
{
  CommunicatorId    = Comm.CommunicatorId;
  ApplicationId     = Comm.ApplicationId;
  CommunicatorTasks = Comm.CommunicatorTasks;
  COMM_SELF         = Comm.COMM_SELF;
}

void Communicator::AddTask( INT32 TaskId )
{
  if ( IsTaskIncluded( TaskId ) )
    return;

  CommunicatorTasks.insert( TaskId );
}

bool Communicator::IsTaskIncluded( INT32 TaskId )
{
  return ( CommunicatorTasks.count( TaskId ) == 1 );
}

void Communicator::Write( ostream& os ) const
{
  os << "CommId: " << CommunicatorId << " ";
  os << "AppId: " << ApplicationId << " ";
  os << "Tasks: " << CommunicatorTasks.size() << " ";
  os << "COMM_SELF: " << COMM_SELF << endl;
}

ostream& operator<<( ostream& os, const Communicator& Comm )
{
  Comm.Write( os );
  return os;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool Communicator::ParseTaskList( char* ASCIITaskList )
{
  char* CurrentTaskIdStr;
  INT32 CurrentTaskIdInt;

  CurrentTaskIdStr = strtok( ASCIITaskList, ":" );

  while ( CurrentTaskIdStr != NULL )
  {
    CurrentTaskIdInt = atoi( CurrentTaskIdStr );
    AddTask( CurrentTaskIdInt );

    CurrentTaskIdStr = strtok( NULL, ":" );
  }

  return true;
}
