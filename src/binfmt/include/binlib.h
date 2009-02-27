#ifndef _BINLIB_H 
#define _BINLIB_H
/*****************************************************************************
 *                                                                           *
 * VAMPIRtrace MPI Profiling Library - Trace Writing Module                  *
 *                                                                           *
 * Copyright (c) Pallas GmbH 1996 - 1999                                     *
 *                                                                           *
 * VAMPIRtrace internal routines                                             *
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
 * $Log: binlib.h,v $
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.1.1.1  1999/04/01 11:24:59  astein
 * Initial checkin
 *
 *
 *****************************************************************************/

#include <binfmt.h>
#include <pal_env.h>

extern LongInt BinFmt_ReadInteger(Byte *src);

extern LongInt BinFmt_ReadLongInt(Byte *src);

extern Double BinFmt_ReadDouble(Byte *src);

extern char *BinFmt_ReadString(Byte *src);


extern void *BinFmt_CreateFile(char *Name, LongInt Binary);

extern void *BinFmt_CreateUnmergedFile(char *Name, LongInt Binary);

extern void BinFmt_SetBufferSize(void *File, LongInt BufferSize);

extern void BinFmt_CloseWriteFile(void *File);

extern void BinFmt_WriteUnmergedFlag(void *File);

extern void BinFmt_WriteComment(void *File, Double Time, char *Comment);

extern void BinFmt_WriteCreator(void *File, char *Creator);

extern void BinFmt_WriteVersion(void *File, LongInt Version);

extern void BinFmt_WriteActDef(void *File, char *Activity, LongInt Token);

extern void BinFmt_WriteSymbol(void *File, char *Activity, LongInt Num,
                               char *SymName);

extern void BinFmt_WriteSymbol2(void *File, char *Activity, LongInt Num,
                               char *SymName, LongInt FileNo, LongInt LineNo);

extern void BinFmt_WriteMsgSymbol(void *File, LongInt Type, LongInt Communicator, char *Name);

extern void BinFmt_WriteCPUSymbol(void *File, LongInt CPUNo, char *Name);

extern void BinFmt_WriteNCPUs(void *File, LongInt NumCnt, LongInt *Nums);

extern void BinFmt_WriteCPUNAMES(void *File, LongInt NameCnt, char **Names);

extern void BinFmt_WriteCLKPERIOD(void *File, Double ClkPeriod);

extern void BinFmt_WriteTIMEOFFSET(void *File, LongInt Offset);

extern void BinFmt_WriteFILETOKEN(void *File, LongInt Token, char *Name);

extern void BinFmt_WriteGLOBALOPTOKEN(void *File, LongInt Token, char *Name);

extern void BinFmt_WriteREDFUNCTOKEN(void *File, LongInt Token, char *Name);

extern void BinFmt_WriteSRCINFO(void *File, Double Time,
                                LongInt CPU, LongInt FileNr, LongInt LineNum);

extern void BinFmt_SetSrcInfo(LongInt CPU, LongInt FileNo, LongInt LineNo);

extern void BinFmt_WriteEXCHANGE(void *File, Double Time, LongInt CPU,
                                 char *OldAct, LongInt OldActNum,
                                 char *NewAct, LongInt NewActNum,
                                 LongInt JobNum);

extern void BinFmt_WriteEXCHANGE2(void *File, Double Time, LongInt CPU,
                                  char *OldAct, LongInt OldActNum,
                                  char *NewAct, LongInt NewActNum,
                                  LongInt JobNum, Byte CallType);

extern void BinFmt_WriteSENDMSG(void *File, Double Time,
                                LongInt Sender, LongInt Receiver,
                                LongInt Communicator, LongInt Type,
                                LongInt Length);

extern void BinFmt_WriteRECVMSG(void *File, Double Time, 
                                LongInt Receiver, LongInt Sender,
                                LongInt Communicator, LongInt Type,
                                LongInt Length);

extern void BinFmt_WriteGlobalOp(void   *File,         Double  Time,
                                 LongInt OpToken,      LongInt ProcessID,
                                 LongInt Communicator, LongInt OpRoot,
                                 LongInt BytesSend,    LongInt BytesRecv,
                                 Double  Delta);

extern void BinFmt_WritePC(void *File,     Double Time,     LongInt CPU,
                           LongInt FileNR, LongInt LineNR);

extern void BinFmt_WriteMPIOFTOKEN(void    *File,
                                   LongInt  Token, LongInt CommId, char *Name);

extern void BinFmt_WriteCOMDEF(void    *File,
                               LongInt  CommId,    LongInt  CommSize,
                               LongInt  TripCount, LongInt *Trips );

extern void BinFmt_WriteFILEIOBEGIN(void    *File,   Double  Time,
                                    LongInt  CPU,    LongInt Type,
                                    LongInt  FToken, LongInt Count);

extern void BinFmt_WriteFILEIOEND(void    *File,   Double  Time,
                                  LongInt  CPU,    LongInt Type,
                                  LongInt  FToken, LongInt Count);

#endif /* _BINLIB_H */
