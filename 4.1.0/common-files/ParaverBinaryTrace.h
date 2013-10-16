/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
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
  $Rev:: 563                                      $:  Revision of last commit
  $Author: jgonzale $:  Author of last commit
  $Date: 2012/02/03 11:02:40 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _PARAVER_BINARY_TRACE_H
#define _PARAVER_BINARY_TRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined( _PARAVER_BINARY_TRACE_TYPE )
   typedef int BinHandle;
#endif

typedef struct
{
   int ntasks;
   int *nthreads;
   
}Application;

typedef struct
{
  int nappls;
  Application *appls;
   
} ProcessHierarchy;

typedef struct
{
  int nnodes;
  int *ncpus;
   
} ResourceHierarchy;

void ParaverTrace_BinaryState( BinHandle file,
                               int cpu, int ptask, int task, int thread,
                               long long ini_time,
                               long long end_time,
                               int state );
                               
void ParaverTrace_BinaryEvent( BinHandle file,
                               int cpu, int ptask, int task, int thread,
                               long long time,
                               long long type, long long value );
                               
void ParaverTrace_BinaryMultipleEvent( BinHandle file,
                                       int cpu, int ptask, int task, int thread,
                                       long long  time,
                                       int        NumEvents,
                                       long long  *EvTypes,
                                       long long  *EvValues );

void ParaverTrace_BinaryGlobalComm( BinHandle file,
                                    int cpu_s, int ptask_s, int task_s,
                                    int thread_s,
                                    long long time,
                                    int commid,
                                    long long sendsize,
                                    long long recvsize,
                                    int gOP,
                                    int root );
                              
void ParaverTrace_MergeBinaryFiles( char *prvName,
                                    long long Ftime,
                                    ProcessHierarchy  *Process,
                                    ResourceHierarchy *Resource );

void ParaverTrace_MergeBinaryFiles_TimeUnits( char *prvName,
                                              long long Ftime,
                                              ProcessHierarchy  *Process,
                                              ResourceHierarchy *Resource,
                                              char TimeUnits[3] );
                                    
BinHandle ParaverTrace_AllocBinaryFile( void );

void ParaverTrace_AddCommunicator( int appl, int commid, int ntasks,
                                   int *tasks );

#ifdef __cplusplus
}
#endif
                               
#endif /* _PARAVER_BINARY_TRACE_H */

