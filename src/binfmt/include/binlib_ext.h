/*****************************************************************************
 *                                                                           *
 * VAMPIRtrace MPI Profiling Library - GENERIC MODULE                        *
 *                                                                           *
 * Copyright (c) PALLAS GmbH 1996-1999                                       *
 *                                                                           *
 * VAMPIRtrace main include file 
 *                                                                           *
 * $Revision: 1.2 $
 *                                                                           *
 * $Date: 2005/05/24 15:41:40 $
 *                                                                           *
 * $State: Exp $
 *                                                                           *
 * $Author: paraver $
 *                                                                           *
 * $Locker:  $
 *                                                                           *
 *****************************************************************************/
 
/*****************************************************************************
 *                                                                           *
 * $Log: binlib_ext.h,v $
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.1.1.1  1999/04/01 11:24:59  astein
 * Initial checkin
 *
 *
 *****************************************************************************/

#ifndef _BINLIB_EXT_H
#define _BINLIB_EXT_H

/* Struktur eines zwischengespeicherten Records: */

typedef struct
         {
          LongInt Length;
          LongInt Type;
          Byte *Data; 
         } BinRecord;

extern void *BinFmt_OpenFile(char *Name);

extern void BinFmt_CloseReadFile(void *File);

extern Double BinFmt_GetPos(void *File);

extern LongInt BinFmt_GetAbsPos(void *File);

extern Booli BinFmt_SetAbsPos(void *File, LongInt Pos);

extern BinRecord *BinFmt_ReadRecord(void *File);

extern Double BinFmt_GetRecordTime(BinRecord *Set);

extern void BinFmt_SetRecordTime(BinRecord *Set, Double NewTime);

extern LongInt BinFmt_GetNCPUs(BinRecord *Set);

extern Double BinFmt_GetCLKPERIOD(BinRecord *Set);

extern LongInt BinFmt_GetVersion(BinRecord *Set);

extern void BinFmt_FreeRecord(BinRecord *Set);

extern void BinFmt_WriteRecord(void *File, BinRecord *Set);

extern void BinFmt_WriteRecordOffset(void *File, BinRecord *Set, LongInt offset, 
				     Double TimeOffset);

extern Booli BinFmt_TranslateRecord(void *Dest, void *Src, BinRecord *Set);

extern void BinFmt_CorrTimestamp(BinRecord *Rec, Double Offset);

#endif /* _BINLIB_EXT_H */
