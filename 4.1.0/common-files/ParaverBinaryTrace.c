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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define _PARAVER_BINARY_TRACE_TYPE

#include "Macros.h"

#ifndef DIMEMAS_COMPILATION
// extern int **EnabledTasks;
// extern unsigned long long **EnabledTasks_time;
#endif

#define FALSE 0
#define TRUE  1

/* ---------------------------------------------------- Macros --------------*/
#define MAX( a, b )  ((a) > (b)) ? (a) : (b)

/* ---------------------------------------------------- Macros --------------*/
#define WRITE( channel, record, type ) \
{ \
  if (write( (int) (channel), (const void *) (record), (size_t) sizeof(type) ) == -1) \
  { \
    perror( "Writing to disk the tracefile" ); \
    exit( 1 ); \
  } \
}
 
/* ---------------------------------------------------- Constants -----------*/
#define ALLOC_FILE_NUMBER       (500)
#define ALLOC_TRACE_NUMBER      (ALLOC_FILE_NUMBER / 2)
#define CURRENT_ENV_DIR         ("PWD")
#define TEMPORAL_ENV_VAR        ("MPTRACE_DIR")

#define MIDA_MAXIMA_BINRECORD 4096

/* ---------------------------------------------------- Data Types ----------*/
typedef enum { BINSTATE,
               BINEVENT,
               BINMLTEVENT,
               BINCOMMUNICATION,
               BINCOLLECTIVECOMM
             } BinType;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;
  int        cpu, ptask, task, thread;
} _BinRecord;

#define BINRECORD_TYPE( x )  ( ((BinRecord) (x))->Type )
#define BINRECORD_TIME( x )  ( ((BinRecord) (x))->Time )

/* ---------------------------------------------------- Macros --------------*/
#define ADD_RECORDSIZE( CurRec, Type, Size ) \
  ( (CurRec) = (Type) ( ((char *) CurRec) + Size ) )

#define BINRECORD_UPDATENEXT( record ) \
{ \
  switch( BINRECORD_TYPE( (record) ) ) \
  { \
    case BINSTATE:  \
    { \
      StateRecord CurRec = (StateRecord) (record); \
       \
      ADD_RECORDSIZE( CurRec, StateRecord, sizeof( _StateRecord ) ); \
      (record) = (BinRecord) CurRec; \
      break; \
    } \
    case BINEVENT:  \
    { \
      EventRecord CurRec = (EventRecord) (record); \
       \
      ADD_RECORDSIZE( CurRec, EventRecord, sizeof( _EventRecord ) ); \
      (record) = (BinRecord) CurRec; \
      break; \
    } \
    case BINMLTEVENT:  \
    { \
      MultipleEventRecord CurRec = (MultipleEventRecord) (record); \
      size_t      Size           =  sizeof( _MultipleEventRecord ); \
      \
      Size += sizeof( long long ) * (MULTEVENTRECORD_NUM( CurRec ) - 2); \
      \
      ADD_RECORDSIZE( CurRec, MultipleEventRecord, Size ); \
      (record) = (BinRecord) CurRec; \
      break; \
    } \
    case BINCOMMUNICATION:  \
    { \
      CommunicationRecord CurRec = (CommunicationRecord) (record); \
       \
      ADD_RECORDSIZE( CurRec, CommunicationRecord, sizeof( _CommunicationRecord ) ); \
      (record) = (BinRecord) CurRec; \
      break; \
    } \
    case BINCOLLECTIVECOMM:  \
    { \
      CollectiveCommRecord CurRec = (CollectiveCommRecord) (record); \
       \
      ADD_RECORDSIZE( CurRec, CollectiveCommRecord, sizeof( _CollectiveCommRecord ) ); \
      (record) = (BinRecord) CurRec; \
      break; \
    } \
  } \
}


/* Macro que indica si s'ha arribat al final del "BinFile *" donat */
#ifdef USE_MMAPS
#define ENDOFBINFILE(bf) (((bf)->Current >= (bf)->Last) ? 1 : 0)
  
#else
#define ENDOFBINFILE(bf) (((bf)->curr_pos>=(bf)->size) ? 1 : 0)
#endif /*USE_MMAPS*/


/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;
  int        cpu, ptask, task, thread;
  
  long long  EndTime;
  long long  State;
} _StateRecord;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;
  int        cpu, ptask, task, thread;
  
  long long  TypeEv;
  long long  ValueEv;
} _EventRecord;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;
  int        cpu, ptask, task, thread;
  
  int        NumFields;
  
  long long  TypeEv;
  long long  ValueEv;

} _MultipleEventRecord;

#define MULTEVENTRECORD_NUM( x )  ( ((MultipleEventRecord) (x))->NumFields )

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;    /* LogSend */
  int        cpu, ptask, task, thread;
  
  long long  PhySend;
  long long  LogRecv;
  long long  PhyRecv;
  int        cpu_r, ptask_r, task_r, thread_r;

  long long  Tag;
  long long  Size;
} _CommunicationRecord;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  BinType    Type;
  long long  Time;                       /* LogSend */
  int        cpu, ptask, task, thread;
  
  long long  SendSize;
  long long  RecvSize;
  long long  CommID;
  int        GlobID;
  int        Root;
} _CollectiveCommRecord;

/* ---------------------------------------------------- Data Types ----------*/
typedef _BinRecord*             BinRecord;
typedef _StateRecord*           StateRecord;
typedef _EventRecord*           EventRecord;
typedef _CommunicationRecord*   CommunicationRecord;
typedef _CollectiveCommRecord*  CollectiveCommRecord;
typedef _MultipleEventRecord*   MultipleEventRecord;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  int         fd;
  BinRecord   First, Current, Last;
  size_t      size;
  size_t      curr_pos, next_pos;
  
} BinFile;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  int StateFile, CommFile;
  
} BinaryFile;

#define MAX_BIN_TASKS 4096

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  int id;
  int num_tasks;
  int tasks[ MAX_BIN_TASKS ];
   
} Communicator;

/* ---------------------------------------------------- Data Types ----------*/
typedef int BinHandle;

/* ---------------------------------------------------- Include Files -------*/
#include "ParaverBinaryTrace.h"

/* ---------------------------------------------------- Global Variables ----*/

/*------------------------------------------------ Static Variables ---------*/
static BinaryFile *TraceList = NULL;
static int         nTraces = 0;
static int         nAllocTraces = 0;

static char       **Temporal_FileNames = NULL;
static BinFile    *FileList = NULL;
static int         nFiles = 0;
static int         nAllocFiles = 0;

static char        *TempDirectory = NULL;

#define MAX_BIN_PTASKS 10
#define MAX_BIN_COMM   2000
static Communicator   Comm[ MAX_BIN_PTASKS ][ MAX_BIN_COMM ];
static int            nComm[ MAX_BIN_PTASKS ];

/* ------------------------------------------------- Static Variables -------*/
static size_t MaxTraceSize = 0;
static size_t PercentatgeProcessed = 0;


/* ------------------------------------------------- GZIP support ----------*/

#include "zlib.h"

struct fdz_fitxer                                                       
{                                                                       
    FILE   *handle;                                                     
    gzFile handleGZ;                                                    
};                                                                      
                                                                        
#define FDZ_CLOSE(x) \
    (x.handleGZ!=NULL)?gzclose(x.handleGZ):fclose(x.handle)
#define FDZ_WRITE(x,buffer) \
    (x.handleGZ!=NULL)?gzputs(x.handleGZ,buffer):fputs(buffer,x.handle) 

/* ---------------------------------------------------- Function Prototypes -*/
static int ParaverTrace_OpenTemporalFile( char **FileName );

static int ParaverTrace_MapBinaryFiles( void );

static void ParaverTrace_UnmapBinaryFiles( void );

static BinRecord ParaverTrace_NextBinaryRecord( void );

static void ParaverTrace_GlobalComm( struct fdz_fitxer prv_fd,
                                     int cpu, int ptask, int task, int thread,
                                     unsigned long long time,
                                     long long sendsize, long long recvsize,
                                     int commid,
                                     int gOP,
                                     int root );
                                     
static void ParaverTrace_Comm( struct fdz_fitxer prv_fd,
                               int cpu_s, int ptask_s, int task_s, int thread_s,
                               unsigned long long log_s,
                               unsigned long long phy_s,
                               int cpu_r, int ptask_r, int task_r, int thread_r,
                               long long log_r, long long phy_r,
                               long long size, long long tag );
                               
static void ParaverTrace_Event( struct fdz_fitxer prv_fd,
                                int cpu, int ptask, int task, int thread,
                                unsigned long long time,
                                long long type, long long value );

static void ParaverTrace_State( struct fdz_fitxer prv_fd,
                                int cpu,int ptask, int task, int thread,
                                unsigned long long ini_time,
                                unsigned long long end_time,
                                int state );

static void ParaverTrace_MultipleEvent( struct fdz_fitxer prv_fd,
                                        int cpu, int ptask, int task, int thread,
                                        long long  time,
                                        short      NumFields,
                                        long long  *Values );


/******************************************************************************
 **      Function name : BinRecord_Alloc
 **
 **      Description :
 ******************************************************************************/
static BinRecord BinRecord_Alloc( void )
{
  BinRecord New = NULL;

  New = (BinRecord) malloc (MIDA_MAXIMA_BINRECORD);
  if (New==NULL)
  {
    fprintf(stderr,"Not enough memory\n");
    exit(1);
  }
  return( New );
}

/******************************************************************************
 **      Function name : BinRecord_Delete
 **
 **      Description :
 ******************************************************************************/
static void BinRecord_Delete( BinRecord rec )
{
  free(rec);
}

/******************************************************************************
 **      Function name : GetBinRecordSize
 **
 **      Description :
 ******************************************************************************/

static size_t GetBinRecordSize(BinRecord rec)
{ 
  size_t mida;
  
  switch( BINRECORD_TYPE( rec ) )
  {
    case BINSTATE:
      return(sizeof(_StateRecord));
      break;
    
    case BINEVENT:
      return(sizeof(_EventRecord));
      break;
    
    case BINMLTEVENT:
      mida=sizeof(_MultipleEventRecord);
      mida+= sizeof( long long ) * (MULTEVENTRECORD_NUM( rec ) - 2);
      return(mida);
      break;
    
    case BINCOMMUNICATION:
      return(sizeof(_CommunicationRecord));
      break;
    
    case BINCOLLECTIVECOMM:
      return(sizeof(_CollectiveCommRecord));
      break;
    
    default:  
      fprintf( stderr, "Unknown BinRecord type: %dn",  BINRECORD_TYPE( rec ) ); 
      fflush( stderr ); 
      break; 
  }
  return(0);
}



/******************************************************************************
 **      Function name : BinRecord_Copy
 **
 **      Description :
 ******************************************************************************/
static void BinRecord_Copy( BinRecord desti, BinRecord origen )
{
  memcpy(desti, origen, GetBinRecordSize(origen));
}



 /******************************************************************************
 **      Function name : ParaverTrace_ReadBinRecord
 **
 **      Description : 
 ******************************************************************************/
static int ParaverTrace_ReadBinRecord( BinFile *bf, BinRecord rec)
{
  int res;

  if (bf->next_pos>=bf->size)
  {
    /* Ja estem a l'ultim record */
    bf->curr_pos=bf->next_pos;
    return(-1);
  }
  
  /* Ens situem a l'inici del seguent */
  lseek(bf->fd, bf->next_pos, SEEK_SET);
 
/*  res = fread( rec, 1, MIDA_MAXIMA_BINRECORD, bf->fd);*/
  res = read( bf->fd, rec, MIDA_MAXIMA_BINRECORD);
  if (res <= 0)
  {
    fprintf( stderr, "Error reading a record\n");
    return( -1 );
  }
  if (res<GetBinRecordSize(rec))
  {
    /* S'han llegit menys bytes de la mida d'aquest tipus de record */
    fprintf( stderr, "Error reading a record!\n");
    return( -1 );
  }

  /* S'ha pogut llegir correctament */

  /* Cal tornar enrera els bytes de mes que hem llegit i guardar la
     nova posicio dins del fitxer */
  bf->curr_pos=bf->next_pos; /* Hem saltat un record */
  /* Calculem on comenc,ara el seguent */
  bf->next_pos=bf->curr_pos+GetBinRecordSize(rec);
  
  /* Es retorna que tot ha anat be */
  return(0);
}




/******************************************************************************
 **      Function name : ParaverTrace_AddCommunicator
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_AddCommunicator( int appl, int commid, int ntasks,
                                   int *tasks )
{
  int ii;
  
  appl--; /* Ens ho hauran passat de 1 a N */

  ASSERT( ntasks < MAX_BIN_TASKS );
  ASSERT( tasks != NULL );
  ASSERT( appl < MAX_BIN_PTASKS );
  ASSERT( appl >= 0 );
  
#if defined( DEBUG_INFO )
  fprintf( stderr, "<I> Comm: appl = %d commid = %d ntasks = %d\n", appl, commid, ntasks );
#endif

  Comm[ appl ][ nComm[ appl ] ].id        = commid;
  Comm[ appl ][ nComm[ appl ] ].num_tasks = ntasks;
  
  for (ii= 0; ii< ntasks; ii++ )
  {
    Comm[ appl ][ nComm[ appl ] ].tasks[ ii ] = tasks[ ii ];
  }
  nComm[ appl ]++;
  
  ASSERT( nComm[ appl ] < MAX_BIN_COMM );
}

/******************************************************************************
 **      Function name :
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_Initialize( void )
{
  int ii = 0;
  
  for (ii= 0; ii< nFiles; ii++)
    FREE( Temporal_FileNames[ ii ] );
  FREE( Temporal_FileNames );
  
  if (TraceList)
    FREE( TraceList );

  if (FileList)
    FREE( FileList );

  TraceList            = NULL;
  FileList             = NULL;
  nTraces              = 0;
  nAllocTraces         = 0;
  nFiles               = 0;
  nAllocFiles          = 0;
  MaxTraceSize         = 0;
  PercentatgeProcessed = 0;
  
  for (ii= 0; ii< MAX_BIN_PTASKS; ii++)
    nComm[ ii ] = 0;
}

/******************************************************************************
 **      Function name : ParaverTrace_AllocBinaryFile
 **      
 **      Description : 
 ******************************************************************************/

BinHandle ParaverTrace_AllocBinaryFile( void )
{
  int ii;
  char *FileName;
  
  if (FileList == NULL)
  {
    nAllocFiles = ALLOC_FILE_NUMBER;
    nAllocTraces =  ALLOC_TRACE_NUMBER;
    nFiles = 0;
    nTraces = 0;
    
    FileList = ALLOC( nAllocFiles, BinFile, "FileList" );
    Temporal_FileNames = ALLOC( nAllocFiles, char*, "Temporal_FileNames" );
    TraceList = ALLOC( nAllocTraces, BinaryFile, "TraceList" );
	
	/* HSG,
	   a peticio de la Judit, els temporals no han d'anar a MPTRACE_DIR, han
     de ser a TMPDIR si esta definida, o sino al directori actual.

    if ((TempDirectory = getenv( TEMPORAL_ENV_VAR )) == NULL)
      if ((TempDirectory = getenv( CURRENT_ENV_DIR )) == NULL)
    */
    if ((TempDirectory = getenv( "TMPDIR" )) == NULL)
      TempDirectory = "./";
  
    for (ii= 0; ii< MAX_BIN_PTASKS; ii++)
      nComm[ ii ] = 0;
    
  }  

  FileList[ nFiles ].fd = ParaverTrace_OpenTemporalFile( &FileName );
  Temporal_FileNames[ nFiles ] = FileName;
  TraceList[ nTraces ].StateFile = nFiles;
#if defined( DEBUG_INFO )
  fprintf( stderr, "<I> State/event a temporal file openned ii = %d fd = %d\n",
                               nTraces, FileList[ nFiles ].fd );
#endif
  nFiles++;
 
  FileList[ nFiles ].fd = ParaverTrace_OpenTemporalFile( &FileName );
  Temporal_FileNames[ nFiles ] = FileName;
  TraceList[ nTraces ].CommFile = nFiles;
#if defined( DEBUG_INFO )
  fprintf( stderr, "<I> Communication a temporal file openned ii = %d fd = %d\n",
                               nTraces, FileList[ nFiles ].fd );
#endif
  nFiles++;
  nTraces++;
  
  if (nTraces >= nAllocTraces)
  {
    nAllocFiles   +=  ALLOC_FILE_NUMBER;
    nAllocTraces  +=  ALLOC_TRACE_NUMBER;
    
    FileList = REALLOC( FileList, nAllocFiles, BinFile, "FileList" );
    TraceList = REALLOC( TraceList, nAllocTraces, BinaryFile, "TraceList" );
    Temporal_FileNames = REALLOC( Temporal_FileNames, nAllocFiles, char*, "Temporal_FileNames" );
  }
  return( (BinHandle) (nTraces - 1) );
}

/******************************************************************************
 **      Function name : ParaverTrace_BinaryState
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_BinaryState( BinHandle file,
                               int cpu, int ptask, int task, int thread,
                               long long ini_time,
                               long long end_time,
                               int state )
{
  int          fd = FileList[ TraceList[ file ].StateFile ].fd;
  _StateRecord  Rec;

#ifndef DIMEMAS_COMPILATION
//  if (EnabledTasks != NULL) 
//    if (!EnabledTasks[ptask - 1][task - 1]) 
//      return;
#endif

  if (ini_time == end_time) return;

  Rec.Type    = BINSTATE;
  Rec.Time    = ini_time;
  Rec.cpu     = cpu;
  Rec.ptask   = ptask;
  Rec.task    = task;
  Rec.thread  = thread;
  Rec.EndTime = end_time;
  Rec.State   = (long long) state;

  WRITE( fd, &Rec, _StateRecord );
}

/******************************************************************************
 **      Function name : ParaverTrace_BinaryEvent
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_BinaryEvent( BinHandle file,
                               int cpu, int ptask, int task, int thread,
                               long long time,
                               long long type, long long value )
{                     
  int           fd = FileList[ TraceList[ file ].StateFile ].fd;
  _EventRecord  Rec;

#ifndef DIMEMAS_COMPILATION
//  if (EnabledTasks != NULL) 
//    if (!EnabledTasks[ptask - 1][task - 1]) 
//      return;
#endif

  Rec.Type     = BINEVENT;
  Rec.Time     = time;
  Rec.cpu      = cpu;
  Rec.ptask    = ptask;
  Rec.task     = task;
  Rec.thread   = thread;
  Rec.TypeEv   = type;
  Rec.ValueEv  = value;
  
  WRITE( fd, &Rec, _EventRecord );
}

/******************************************************************************
 **      Function name : ParaverTrace_BinaryMultipleEvent
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_BinaryMultipleEvent( BinHandle file,
                                       int cpu, int ptask, int task, int thread,
                                       long long  time,
                                       int        NumEvents,
                                       long long  *EvTypes,
                                       long long  *EvValues )
{                     
  int                    fd = FileList[ TraceList[ file ].StateFile ].fd;
  _MultipleEventRecord   Rec;
  int                    ii;

#ifndef DIMEMAS_COMPILATION
//  if (EnabledTasks != NULL) 
//    if (!EnabledTasks[ptask - 1][task - 1]) 
//      return;
#endif
 
  ASSERT( NumEvents > 0 );

  Rec.Type      = BINMLTEVENT;
  Rec.Time      = (long long) time;
  Rec.cpu       = cpu;
  Rec.ptask     = ptask;
  Rec.task      = task;
  Rec.thread    = thread;
  Rec.NumFields = (short) NumEvents * 2;
  Rec.TypeEv    = (long long) EvTypes[ 0 ];
  Rec.ValueEv   = (long long) EvValues[ 0 ];
  
  WRITE( fd, &Rec, _MultipleEventRecord );
  
  for (ii= 1; ii< NumEvents; ii++)
  {
    WRITE( fd, &(EvTypes[ ii ]), long long );
    WRITE( fd, &(EvValues[ ii ]), long long );
  }
}

/*****************************************************************************
 **      Function name : ParaverTrace_BinaryComm
 **      
 **      Description : 
 ****************************************************************************/

void ParaverTrace_BinaryComm( BinHandle file,
                              int cpu_s, int ptask_s, int task_s, int thread_s,
                              long long log_s,
                              long long phy_s,
                              int cpu_r, int ptask_r, int task_r, int thread_r,
                              long long log_r,
                              long long phy_r, long long size,
                              long long tag )
{
  int                    fd = FileList[ TraceList[ file ].CommFile ].fd;
  _CommunicationRecord   Rec;

#ifndef DIMEMAS_COMPILATION
//  if (EnabledTasks != NULL) 
//    if ((!EnabledTasks[ptask_s - 1][task_s - 1]) && (!EnabledTasks[ptask_r - 1][task_r - 1])) 
//      return;
#endif

  Rec.Type     = BINCOMMUNICATION;
  Rec.Time     = log_s;
  Rec.cpu      = cpu_s;
  Rec.ptask    = ptask_s;
  Rec.task     = task_s;
  Rec.thread   = thread_s;
  Rec.PhySend  = phy_s;
  Rec.LogRecv  = log_r;
  Rec.PhyRecv  = phy_r;
  Rec.cpu_r    = cpu_r;
  Rec.ptask_r  = ptask_r;
  Rec.task_r   = task_r;
  Rec.thread_r = thread_r;
  Rec.Size     = size;
  Rec.Tag      = tag;

  WRITE( fd, (&Rec), _CommunicationRecord );
}

/******************************************************************************
 **      Function name : ParaverTrace_BinaryGlobalComm
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_BinaryGlobalComm( BinHandle file,
                                    int cpu_s, int ptask_s, int task_s,
                                    int thread_s,
                                    long long time,
                                    int commid,
                                    long long sendsize,
                                    long long recvsize,
                                    int gOP,
                                    int root )
{
  int                      fd = FileList[ TraceList[ file ].CommFile ].fd;
  _CollectiveCommRecord    Rec;

#ifndef DIMEMAS_COMPILATION
//  if (EnabledTasks != NULL)
//    if (!EnabledTasks[ptask_s - 1][task_s - 1])
//      return;
#endif

  Rec.Type      = BINCOLLECTIVECOMM;
  Rec.Time      = time;
  Rec.cpu       = cpu_s;
  Rec.ptask     = ptask_s;
  Rec.task      = task_s;
  Rec.thread    = thread_s;
  Rec.SendSize  = sendsize;
  Rec.RecvSize  = recvsize;
  Rec.CommID    = commid;
  Rec.GlobID    = gOP;
  Rec.Root      = root;

  WRITE( fd, &Rec, _CollectiveCommRecord );
}

/******************************************************************************
 **      Function name : ParaverTrace_WriteCommunicators
 **      
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_WriteCommunicators( struct fdz_fitxer prv_fd, int Appl )
{
  char buffer[1024];
  int ii, jj, nTasks;

  
  for (jj= 0; jj< nComm[ Appl ]; jj++)
  {
    sprintf( buffer, "C:%d:%d:%d:", Appl + 1,
                                Comm[ Appl ][ jj ].id,
                                Comm[ Appl ][ jj ].num_tasks );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "<I> Communicactors are --> C:%d:%d:%d:\n", Appl + 1,
                                    Comm[ Appl ][ jj ].id,
                                    Comm[ Appl ][ jj ].num_tasks );
#endif
    nTasks = Comm[ Appl ][ jj ].num_tasks;
    for (ii= 0; ii< nTasks-1; ii++)
    {
      sprintf( buffer, "%d:", Comm[ Appl ][ jj ].tasks[ ii ] );
      FDZ_WRITE(prv_fd,buffer);
                        
#if defined( DEBUG_INFO )
      fprintf( stderr, "%d:", Comm[ Appl ][ jj ].tasks[ ii ] );
      fflush( stderr );
#endif
    }
    sprintf( buffer, "%d\n", Comm[ Appl ][ jj ].tasks[ nTasks-1 ] );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "%d\n", Comm[ Appl ][ jj ].tasks[ nTasks-1 ] );
#endif
  }
}

/******************************************************************************
 **      Function name : ParaverTrace_WriteParaverHeader
 **      
 **      Description : 
 ******************************************************************************/

static int ParaverTrace_WriteParaverHeader( long long FinalTime,
                                            struct fdz_fitxer prv_fd,
                                            ProcessHierarchy  *Process,
                                            ResourceHierarchy *Resource,
                                            char TimeUnits[3] )
{
  time_t h;
  char Date[80];
  int ii, task, appl;
  char buffer[1024];
  
  int nAppl, nNodes, nTasks;
  struct tm * tm_str;

  ASSERT( Process != NULL );

#if defined( DEBUG_INFO )
  fprintf( stderr, "<I> Writing paraver header FP = 0x%lx\n", prv_fd );
#endif
  
  time( &h );
  tm_str = localtime( &h );
  strftime( Date, 80, "%d/%m/%y at %H:%M", tm_str );

  nAppl = Process->nappls;

#if defined( DEBUG_INFO )
  fprintf( stderr, "<I> Header is --> #Paraver (%s):%llu_%s:", Date, FinalTime,
                                                               TimeUnits );
#endif  
  sprintf( buffer, "#Paraver (%s):%llu_%s:", Date, FinalTime, TimeUnits );
  FDZ_WRITE(prv_fd,buffer);
  
  if (Resource == NULL)
  {
    sprintf( buffer, "%d:%d:", 0, nAppl );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "%d:%d:", 0, nAppl );
#endif
  }
  else
  {
    ASSERT( Resource->nnodes > 0 && Resource->ncpus != NULL );
    nNodes = Resource->nnodes;
    
    sprintf( buffer, "%d(", nNodes );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "%d(", nNodes );
#endif
    for (ii= 0; ii< nNodes-1; ii++)
    {
      sprintf(buffer, "%d,", Resource->ncpus[ ii ] );
      FDZ_WRITE(prv_fd,buffer);
      
#if defined( DEBUG_INFO )
      fprintf( stderr, "%d,", Resource->ncpus[ ii ] );
#endif
    }
    
    sprintf( buffer, "%d):%d:", Resource->ncpus[ nNodes-1 ], nAppl );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "%d):%d:", Resource->ncpus[ nNodes-1 ], nAppl );
#endif
  }

#if defined( DEBUG_INFO )
  fflush( stderr );
#endif

  for (appl= 0; appl < nAppl; appl++)
  {
    nTasks = Process->appls[ appl ].ntasks;
    
    sprintf( buffer, "%d(", nTasks );
    FDZ_WRITE(prv_fd,buffer);
    
#if defined( DEBUG_INFO )
    fprintf( stderr, "%d(", nTasks );
#endif

    for (task= 0; task < nTasks - 1; task++)
    {
      sprintf( buffer, "%d:%d,", Process->appls[ appl ].nthreads[ task ],
                                 0 );
      FDZ_WRITE(prv_fd,buffer);

#if defined( DEBUG_INFO )
      fprintf( stderr, "%d:%d,", Process->appls[ appl ].nthreads[ task ],
                                 0 );
#endif
    }
    sprintf( buffer, "%d:%d)", Process->appls[ appl ].nthreads[ nTasks - 1 ],
                               0 );
    FDZ_WRITE(prv_fd,buffer);

#if defined( DEBUG_INFO )
    fprintf( stderr, "%d:%d)", Process->appls[ appl ].nthreads[ nTasks - 1 ],
                               0 );
#endif
    
    /* Each Application have the MPI_COMM_WORLD and MPI_COMM_SELF communicators
       plus the other ones defined during the execution. */

    if (nComm[ appl ])
    {
      sprintf( buffer, ",%d", nComm[ appl ] );
      FDZ_WRITE(prv_fd,buffer);

#if defined( DEBUG_INFO )
      fprintf( stderr, ",%d", nComm[ appl ] );
#endif
    }
    
    if ((appl+1) != nAppl)
    {
      sprintf( buffer, ":" );
      FDZ_WRITE(prv_fd,buffer);
    }
  }
  FDZ_WRITE(prv_fd,"\n");

#if defined( DEBUG_INFO )
  fprintf( stderr, "\n" );
  fflush( stderr );
#endif

  for (appl= 0; appl< nAppl; appl++)
    if (nComm[ appl ])
      ParaverTrace_WriteCommunicators( prv_fd, appl );
  
  return( 0 );
}

/******************************************************************************
 **      Function name : ParaverTrace_MergeBinaryFiles
 **      
 **      Description : 
 ******************************************************************************/

void ParaverTrace_MergeBinaryFiles( char *prvName,
                                    long long Ftime,
                                    ProcessHierarchy  *Process,
                                    ResourceHierarchy *Resource )
{
  ParaverTrace_MergeBinaryFiles_TimeUnits( prvName, Ftime, Process, Resource,
                                           "ns" );
}



/******************************************************************************
 **      Function name : ParaverTrace_MergeBinaryFiles_TimeUnits
 **      
 **      Description : 
 ******************************************************************************/
void ParaverTrace_MergeBinaryFiles_TimeUnits( char              *prvName,
                                              long long          Ftime,
                                              ProcessHierarchy  *Process,
                                              ResourceHierarchy *Resource,
                                              char               TimeUnits[3])
{
  struct fdz_fitxer prv_fd;
  int        error;
  BinRecord  Event;

  if (strlen(prvName) >= 7 &&
      strncmp(&(prvName[strlen(prvName)-7]), ".prv.gz", 7) == 0)
  {
    /* Trying to generate gzipped file */
    prv_fd.handleGZ = gzopen (prvName, "wb6");
    prv_fd.handle   = NULL;
    if (prv_fd.handleGZ == NULL)
    {
      fprintf (stderr,
               "mpi2prv ERROR: creating GZ paraver tracefile : %s\n",
               prvName);
      return;
    }
  }
  else
  {
    /* Trying to generate ASCII file without compression */
    prv_fd.handle = fopen(prvName, "w");
    prv_fd.handleGZ = NULL;
    if (prv_fd.handle == NULL)
    {
      fprintf(stderr,
              "mp2prv ERROR: Creating paraver tracefile : %s\n",
              prvName);
      return;
    }
  }

  error = ParaverTrace_WriteParaverHeader(Ftime,
                                          prv_fd,
                                          Process,
                                          Resource,
                                          TimeUnits );

  if (!error)
  {
    error = ParaverTrace_MapBinaryFiles( );
  }
 
  Event = ParaverTrace_NextBinaryRecord( );
  if (Event == NULL)
  {
    fprintf(stderr,"There isn't any record!\n");
    exit(1);
  }
  
  do
  {
    switch(BINRECORD_TYPE(Event))
    {
      case BINSTATE:
      {
        StateRecord StateRec = (StateRecord) Event;
                          
        ParaverTrace_State(prv_fd,
                           StateRec->cpu,
                           StateRec->ptask,
                           StateRec->task,
                           StateRec->thread,
                           StateRec->Time,
                           StateRec->EndTime,
                           StateRec->State );
        break;
      }
 
      case BINEVENT:
      {
        EventRecord EventRec = (EventRecord) Event;

        ParaverTrace_Event(prv_fd,
                           EventRec->cpu,
                           EventRec->ptask,
                           EventRec->task,
                           EventRec->thread,
                           EventRec->Time,
                           EventRec->TypeEv,
                           EventRec->ValueEv);
        break;
      }
 
      case BINMLTEVENT:
      {
        MultipleEventRecord EventRec = (MultipleEventRecord) Event;

        ParaverTrace_MultipleEvent(prv_fd,
                                   EventRec->cpu,
                                   EventRec->ptask,
                                   EventRec->task,
                                   EventRec->thread,
                                   EventRec->Time,
                                   EventRec->NumFields,
                                   (long long *) &(EventRec->TypeEv));
        break;
      }

      case BINCOMMUNICATION:
      {
        CommunicationRecord CommRec = (CommunicationRecord) Event;

        ParaverTrace_Comm(prv_fd,
                          CommRec->cpu,
                          CommRec->ptask,
                          CommRec->task,
                          CommRec->thread,
                          CommRec->Time,
                          CommRec->PhySend,
                          CommRec->cpu_r,
                          CommRec->ptask_r,
                          CommRec->task_r,
                          CommRec->thread_r,
                          CommRec->LogRecv,
                          CommRec->PhyRecv,
                          CommRec->Size,
                          CommRec->Tag);
        break;
      }

      case BINCOLLECTIVECOMM:
      {
        CollectiveCommRecord CommRec = (CollectiveCommRecord) Event;

        ParaverTrace_GlobalComm(prv_fd,
                                CommRec->cpu,
                                CommRec->ptask,
                                CommRec->task,
                                CommRec->thread,
                                CommRec->Time,
                                CommRec->SendSize,
                                CommRec->RecvSize,
                                CommRec->CommID,
                                CommRec->GlobID,
                                CommRec->Root);
        break;
      }
    }

#ifndef USE_MMAPS
    BinRecord_Delete(Event);
#endif /* USE_MMAPS */
    Event = ParaverTrace_NextBinaryRecord( );
 
  } while (Event && !error);

  FDZ_CLOSE(prv_fd);
 
  ParaverTrace_UnmapBinaryFiles( );

  ParaverTrace_Initialize( );
 
  if (error)
  {
    unlink( prvName );
    exit( 1 );
  }
}

/******************************************************************************
 **      Function name : ParaverTrace_OpenTemporalFile
 **      
 **      Description : 
 ******************************************************************************/

#if defined( sgi )

/* A silicon la crida mkstemp es incapaç de generar més de 26 fitxers!
   Per aixo cal una rutina ParaverTrace_OpenTemporalFile diferent que utilitzi
   la crida tempnam tot i que és obsoleta. */

static int ParaverTrace_OpenTemporalFile(char **FileName )
{
  char *tmpFile;
  int fd;
 
  ASSERT( TempDirectory != NULL );

  tmpFile = tempnam( TempDirectory, "TMP" );
  if (tmpFile == NULL)
  {
    fprintf( stderr, "Can't get a temporal file name!\n" );
    exit( 1 );
  }
  
  fd = open( tmpFile, O_RDWR | O_EXCL | O_CREAT, 0600 );
  if (fd == -1)
  {
    fprintf( stderr, "Error while openning a temporal file (name %s)\n", tmpFile );
    fprintf( stderr, "Please, check user quota or file descriptors limit.\n" );
    exit( 1 );
  }
  
#if defined( DEBUG_INFO )
  fprintf( stderr,
          "<I> Openning a temporal file (mkstemp) name = %s fd = %d\n",
          tmpFile,
          fd );
#endif /* DEBUG_INFO */

  *FileName = tmpFile;
  
  return( fd );
}

#else

static int ParaverTrace_OpenTemporalFile( char **FileName )
{
  char tmpFile[ 1024 ];
  char Tmp[ 10 ] = "XXXXXX";
  int fd;
 
  ASSERT( TempDirectory != NULL );
  ASSERT( strlen( TempDirectory ) + strlen( Tmp ) + 1 < 1024 );
  
  sprintf( tmpFile, "%s/%s", TempDirectory, Tmp  );
  
  if ((fd = mkstemp( tmpFile )) == -1)
  {
    fprintf( stderr, "Error while openning a temporal file (name %s)\n", tmpFile );
    fprintf( stderr, "Please, check user quota or file descriptors limit.\n", tmpFile );
    exit( 1 );
  }
  
#if defined( DEBUG_INFO )
  fprintf(stderr,
          "<I> Openning a temporal file (mkstemp) name = %s fd = %d\n",
          tmpFile,
          fd);
#endif

  *FileName = ALLOC( strlen( tmpFile ) + 1, char, "FileName" );
  strcpy( *FileName, tmpFile );
  return( fd );
}
#endif

/******************************************************************************
 **      Function name : ParaverTrace_MapBinaryFiles
 **      
 **      Description : 
 ******************************************************************************/

static int ParaverTrace_MapBinaryFiles( void )
{
  int ii, res;
  char *tmp;

  for (ii= 0; ii < nFiles; ii++)
  {
#if defined( DEBUG_INFO )
    
    fprintf(stderr,
            "<I> Rewind tmp file ii = %d size = %ld fd = %d\n",
            ii,
            FileList[ ii ].size,
            FileList[ ii ].fd);
#endif

    FileList[ ii ].size = lseek( FileList[ ii ].fd, 0, SEEK_END);
    lseek( FileList[ ii ].fd, 0, SEEK_SET);
    FileList[ ii ].curr_pos=0;
    FileList[ ii ].next_pos=0;
       
    if (FileList[ ii ].size <= 0)
    {
      FileList[ ii ].size = 0;
      FileList[ ii ].First = NULL;
      FileList[ ii ].Current = NULL;
      FileList[ ii ].Last = NULL;
    }
    else
    {
      MaxTraceSize = MAX( MaxTraceSize, FileList[ ii ].size );

#if !defined( DIMEMAS_COMPILATION )
      fprintf(stderr,
              "<I> Mapping tmp file ii = %d size = %ld fd = %d\n",
              ii,
              FileList[ ii ].size,
              FileList[ ii ].fd);
#endif

#ifdef USE_MMAPS
      FileList[ ii ].First = (BinRecord) mmap(NULL,
                                              FileList[ ii ].size,
                                              PROT_READ | PROT_WRITE,
#if defined( sgi ) || defined (__ia64)
                                              MAP_PRIVATE,
#else
                                              0,
#endif
                                              FileList[ ii ].fd,
                                              0);
 
      if (FileList[ ii ].First == (BinRecord) -1)
      {
        perror( "mmap at ParaverTrace_MapBinaryFiles function failed." );
        return ( -1 );
      }
           
      tmp = (char *) FileList[ ii ].First;
      tmp = tmp + FileList[ ii ].size;
      FileList[ ii ].Last = (BinRecord) tmp;
      FileList[ ii ].Current = FileList[ ii ].First;

#else
      /* Com que no es torna mai endarrera, en principi no necessito aixo */
      FileList[ ii ].First = NULL;
      FileList[ ii ].Last  = NULL;
      /****************************************/
      
      FileList[ ii ].Current = BinRecord_Alloc();
     
      res=ParaverTrace_ReadBinRecord( &FileList[ ii ],
                                      FileList[ ii ].Current);
      if (res!=0) return(res);

#endif /* USE_MMAPS */

    }
  }
  
  return( 0 );
}

/******************************************************************************
 **      Function name : ParaverTrace_UnmapBinaryFiles
 **      
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_UnmapBinaryFiles( void )
{
  int ii;

  for (ii= 0; ii < nFiles; ii++)
  {
#ifdef USE_MMAPS
    munmap( (void *) FileList[ ii ].First, FileList[ ii ].size );
#endif /* USE_MMAPS */
    close( FileList[ ii ].fd );
    unlink( Temporal_FileNames[ ii ] );

#if !defined( DIMEMAS_COMPILATION )
    fprintf(stderr,
            "<I> Deleting temporal file: %s\n",
            Temporal_FileNames[ ii ]);
#endif
  }
}

/******************************************************************************
 **      Function name : ParaverTrace_NextBinaryRecord
 **      
 **      Description : 
 ******************************************************************************/

static BinRecord ParaverTrace_NextBinaryRecord( void )
{
  BinRecord  MinBinRec = NULL, current = NULL;
  int        Min = 0, ii;
  size_t     CurSize, Percentatge;

  
  for (ii= 0; ii< nFiles; ii++)
  {
    if (ENDOFBINFILE(&FileList[ii])) current=NULL;
    else current = FileList[ ii ].Current;
     
    if (!MinBinRec && current)
    {
#ifdef USE_MMAPS
      MinBinRec = current;
#else
      MinBinRec = BinRecord_Alloc();
      BinRecord_Copy(MinBinRec,current);
#endif /* USE_MMAPS */
      Min = ii;
    }
    else if (MinBinRec && current)
    {
      if (BINRECORD_TIME( MinBinRec ) > BINRECORD_TIME( current ))
      {
#ifdef USE_MMAPS
        MinBinRec = current;
#else
        BinRecord_Copy(MinBinRec,current);
#endif /* USE_MMAPS */
        Min = ii;
      }
      else if (BINRECORD_TIME( MinBinRec ) == BINRECORD_TIME( current ))
      {
        if (BINRECORD_TYPE( MinBinRec ) < BINRECORD_TYPE( current ) )
        {
#ifdef USE_MMAPS
          MinBinRec = current;
#else
          BinRecord_Copy(MinBinRec,current);
#endif /* USE_MMAPS */
          Min = ii;
        }
      }
    } 
  }
  
  if (MinBinRec == NULL)
    return( NULL );
 
  if (!ENDOFBINFILE(&FileList[Min]))
#ifdef USE_MMAPS
    BINRECORD_UPDATENEXT( FileList[ Min ].Current );
#else
    ParaverTrace_ReadBinRecord( &FileList[Min], FileList[ Min ].Current);
#endif /* USE_MMAPS */

#if !defined( DIMEMAS_COMPILATION )
#ifdef USE_MMAPS
  CurSize = MinBinRec - FileList[ Min ].First;
#else
  CurSize = FileList[ Min ].curr_pos;
#endif /* USE_MMAPS */
  Percentatge = ((((CurSize*100)/MaxTraceSize))/10);

  if (Percentatge > PercentatgeProcessed)
  {
    PercentatgeProcessed = Percentatge;
#if !defined( DIMEMAS_COMPILATION )
//    fprintf( stderr, "\t processed ... %2ld %%\n", PercentatgeProcessed * 10 );
#endif
  }
#endif
  
  return( MinBinRec );
}


/******************************************************************************
 **      Function name : ParaverTrace_State
 **
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_State( struct fdz_fitxer prv_fd,
                                int cpu, int ptask, int task, int thread,
                                unsigned long long ini_time,
                                unsigned long long end_time,
                                int state )
{
  int ret;
  char buffer[1024];

  /*
   *  State line format is :
   *      1:cpu:ptask:task:thread:ini_time:end_time:state
   */
  sprintf( buffer, "1:%d:%d:%d:%d:%llu:%llu:%d\n",
                     cpu, ptask, task, thread,
                     ini_time, end_time,
                     state );
  ret = FDZ_WRITE(prv_fd,buffer);

  if (ret < 0)
  {
    fprintf( stderr, "ERROR : Writing to disk the tracefile "
                     "(PARAVER_PRVState_Rec)\n" );
    exit( 1 );
  }
}

/******************************************************************************
 **      Function name : ParaverTrace_Event
 **
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_Event( struct fdz_fitxer prv_fd,
                                int cpu, int ptask, int task, int thread,
                                unsigned long long time,
                                long long type, long long value )
{
  int ret;
  char buffer[1024];

  /*
   * Format event line is :
   *      2:cpu:ptask:task:thread:time:type:value
   */
  sprintf( buffer, "2:%d:%d:%d:%d:%llu:%lld:%lld\n",
                     cpu, ptask, task, thread,
                     time,
                     type, value );
  ret = FDZ_WRITE(prv_fd,buffer);

  if ( ret < 0 )
  {
    fprintf( stderr, "ERROR : Writing to disk the tracefile "
                     "(PARAVER_PRVEvent_Rec)\n" );
    exit( 1 );
  }      
}

/******************************************************************************
 **      Function name : ParaverTrace_MultipleEvent
 **
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_MultipleEvent( struct fdz_fitxer prv_fd,
                                        int cpu, int ptask, int task, int thread,
                                        long long  time,
                                        short      NumFields,
                                        long long  *Values )
{
  char buffer[1024];
  int ii;

  ASSERT( NumFields > 0 );
  ASSERT( NumFields % 2 == 0 );
  /*
   * Format event line is :
   *      2:cpu:ptask:task:thread:time:type:value
   */

  sprintf( buffer, "2:%d:%d:%d:%d:%llu", cpu, ptask, task, thread, time );
  FDZ_WRITE(prv_fd,buffer);

  for (ii= 0; ii< NumFields; ii+= 2)
  {
    sprintf( buffer, ":%lld:%lld", Values[ ii ], Values[ ii+1 ] );
    FDZ_WRITE(prv_fd,buffer);
  }

  FDZ_WRITE(prv_fd,"\n");
}

/******************************************************************************
 **      Function name : ParaverTrace_Communication
 **
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_Comm(struct fdz_fitxer  prv_fd,
                              int                cpu_s,
                              int                ptask_s,
                              int                task_s,
                              int                thread_s,
                              unsigned long long log_s,
                              unsigned long long phy_s,
                              int                cpu_r,
                              int                ptask_r,
                              int                task_r,
                              int                thread_r,
                              long long          log_r,
                              long long          phy_r,
                              long long          size,
                              long long          tag)
{
  int ret;
  char buffer[1024];

  /*
   * Format event line is :
   *   3:cpu_s:ptask_s:task_s:thread_s:log_s:phy_s:cpu_r:ptask_r:task_r:
         thread_r:log_r:phy_r:size:tag
   */
  sprintf( buffer, "3:%d:%d:%d:%d:%llu:%llu:%d:%d:%d:%d:%lld:%lld:%lld:%lld\n",
                     cpu_s, ptask_s, task_s, thread_s,
                     log_s, phy_s,
                     cpu_r, ptask_r, task_r, thread_r,
                     log_r, phy_r,
                     size, tag );
  ret = FDZ_WRITE(prv_fd,buffer);

  if (ret < 0)
  {
    fprintf( stderr, "ERROR : Writing to disk the tracefile "
                     "(PARAVER_PRVComm_Rec)\n" );
    exit( 1 );
  }      
}

/******************************************************************************
 **      Function name : ParaverTrace_GlobalComm
 **
 **      Description : 
 ******************************************************************************/

static void ParaverTrace_GlobalComm(struct fdz_fitxer prv_fd,
                                    int cpu,
                                    int ptask,
                                    int task,
                                    int thread,
                                    unsigned long long time,
                                    long long sendsize,
                                    long long recvsize,
                                    int commid,
                                    int gOP,
                                    int root )
{
  int ret;
  char buffer[1024];

  sprintf( buffer,
                 "4:%d:%d:%d:%d:%lld:%d:%lld:%lld:%d:%d\n",
                 cpu, ptask, task, thread,
                 time,
                 commid,
                 sendsize, recvsize,
                 gOP,
                 root );
  ret = FDZ_WRITE(prv_fd,buffer);  
                   
  if (ret <  0)
  {
    fprintf( stderr, "ERROR : Writing to disk the tracefile "
                     "(PARAVER_PRVGlob_Rec)\n" );
    exit( 1 );
  }     
}

