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
 * $Log: binlib_public.h,v $
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.1.1.1  1999/04/01 11:24:59  astein
 * Initial checkin
 *
 *
 *****************************************************************************/


extern void *BinFmt_CreateFile(char *Name);

extern void BinFmt_CloseWriteFile(void *File);

extern void BinFmt_WriteActDef(void *File, char *Activity, LongInt Token);

extern void BinFmt_WriteSymbol(void *File, char *Activity, LongInt Num,
                               char *SymName);

extern void BinFmt_WriteMsgSymbol(void *File, LongInt Type, char *Name);

extern void BinFmt_WriteNCPUs(void *File, LongInt NumCnt, LongInt *Nums);

extern void BinFmt_WriteCPUNAMES(void *File, LongInt NameCnt, char **Names);

extern void BinFmt_WriteCLKPERIOD(void *File, Double ClkPeriod);

extern void BinFmt_WriteEXCHANGE(void *File, Double Time, LongInt CPU,
                                 char *OldAct, LongInt OldActNum,
                                 char *NewAct, LongInt NewActNum,
                                 LongInt JobNum);

extern void BinFmt_WriteSENDMSG(void *File, Double Time,
                                LongInt Sender, LongInt Receiver,
                                LongInt Communicator, LongInt Type,
                                LongInt Length);

extern void BinFmt_WriteRECVMSG(void *File, Double Time, 
                                LongInt Receiver, LongInt Sender,
                                LongInt Communicator, LongInt Type,
                                LongInt Length);

