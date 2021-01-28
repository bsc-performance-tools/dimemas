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

#ifndef _PARAVERAPPLICATIONDESCRIPTION_H
#define _PARAVERAPPLICATIONDESCRIPTION_H

#include "ParaverCommunicator.hpp"

#include <Error.hpp>
#include <basic_types.h>
#include <ostream>
using std::endl;
using std::ostream;
#include <vector>
using std::vector;

/*****************************************************************************
 * class TaskDescription
 ****************************************************************************/

class TaskDescription
{
 private:
  INT32 TaskId;
  INT32 ThreadCount;
  INT32 Node;

 public:
  TaskDescription( INT32 TaskId, INT32 ThreadCount, INT32 Node = 0 )
  {
    this->TaskId      = TaskId;
    this->ThreadCount = ThreadCount;
    this->Node        = Node;
  };

  void Write( ostream& os ) const;

  INT32 GetTaskId( void )
  {
    return TaskId;
  };
  INT32 GetThreadCount( void )
  {
    return ThreadCount;
  };
  INT32 GetNode( void )
  {
    return Node;
  };
  void SetThreadCount( INT32 ThreadCount )
  {
    this->ThreadCount = ThreadCount;
  };
};
typedef TaskDescription* TaskDescription_t;

ostream& operator<<( ostream& os, const TaskDescription& Task );

/*****************************************************************************
 * class ApplicationDescription
 ****************************************************************************/

class ApplicationDescription : public Error
{
 private:
  INT32 ApplicationId;

  INT32 TaskCount;
  vector<TaskDescription_t> TaskInfo;

  INT32 CommunicatorCount;
  vector<Communicator_t> Communicators;

 public:
  ApplicationDescription( INT32 ApplicationId, INT32 TaskCount, INT32 CommunicatorCount );

  ApplicationDescription( INT32 ApplicationId, INT32 TaskCount, INT32 CommunicatorCount, char* ASCIITaskInfo );

  INT32 GetApplicationId( void )
  {
    return ApplicationId;
  };
  INT32 GetTaskCount( void )
  {
    return TaskCount;
  };
  vector<TaskDescription_t> GetTaskInfo( void )
  {
    return TaskInfo;
  };

  INT32 GetCommunicatorCount( void )
  {
    return CommunicatorCount;
  };
  vector<Communicator_t> GetCommunicators( void )
  {
    return Communicators;
  }
  INT32 GetCOMM_WORLD_Id( void );

  void Write( ostream& os ) const;

  bool AddTaskDescription( TaskDescription_t NewTaskDescription )
  {
    TaskInfo.push_back( NewTaskDescription );
    return true;
  };

  bool AddCommunicator( Communicator_t NewCommunicator )
  {
    Communicators.push_back( NewCommunicator );
    return true;
  };

  bool AddTaskDescription( INT32 TaskId, INT32 ThreadCount, INT32 Node = 0 )
  {
    TaskDescription_t NewTaskDescription = new TaskDescription( TaskId, ThreadCount, Node );

    AddTaskDescription( NewTaskDescription );
    return true;
  };
};
typedef ApplicationDescription* ApplicationDescription_t;

ostream& operator<<( ostream& os, const ApplicationDescription& App );

#endif /* _PARAVERAPPLICATIONDESCRIPTION_H */
