/*****************************************************************************
 *                                                                           *
 * VAMPIRtrace MPI Profiling Library - Trace Writing Module                  *
 *                                                                           *
 * Copyright (c) Pallas GmbH 1996 - 1999                                     *
 *                                                                           *
 * VAMPIR Performance Analysis System                                        *
 *                                                                           *
 * $Revision: 1.4 $
 *                                                                           *
 * $Date: 2005/12/23 10:44:14 $
 *                                                                           *
 * $State: Exp $
 *                                                                           *
 * $Author: jgonzale $
 *                                                                           *
 * $Locker:  $
 *                                                                           *
 *****************************************************************************/
 
static char rcsid[] = "@(#) $Id: binlib.c,v 1.4 2005/12/23 10:44:14 jgonzale Exp $";
 
/*****************************************************************************
 *                                                                           *
 * $Log: binlib.c,v $
 * Revision 1.4  2005/12/23 10:44:14  jgonzale
 * Modifications to use new file open macros (MYOPEN, ecc)
 *
 * Revision 1.3  2005/11/25 16:03:27  paraver
 * MacOS X support!
 *
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.1.1.1  1999/04/01 11:24:59  astein
 * Initial checkin
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef ARCH_MACOSX
#include <malloc.h>
#endif

#include <pal_env.h>

#include "binlib.h"
#include "binlib_ext.h"

/* Needed to correctly define 'MYFOPEN' and similar macros */
#include "define.h"

#define ReadInteger(arg) BinFmt_ReadInteger(arg)
#define ReadLongInt(arg) BinFmt_ReadLongInt(arg)
#define ReadDouble(arg)  BinFmt_ReadDouble(arg)
#define ReadString(arg)  BinFmt_ReadString(arg)

#define LINELEN	300

#define SHUFFLE_4(to, from) \
  /* reshuffle 4 bytes from little to big-endian */ \
  { char *t_ptr = (char *)to, *f_ptr = (char *)from; \
    t_ptr += 3; \
    *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; \
    *t_ptr = *f_ptr; \
  }

#define SHUFFLE_8(to, from) \
  /* reshuffle 8 bytes from little to big-endian */ \
  { char *t_ptr = (char *)to, *f_ptr = (char *)from; \
    t_ptr += 7; \
    *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; \
    *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; *t_ptr-- = *f_ptr++; \
    *t_ptr-- = *f_ptr++; *t_ptr = *f_ptr; \
  }


/*---------------------------------------------------------------------*/
/* Filedescription record */

typedef struct {
   FILE *Datei;
   LongInt TokenCnt;
   char **Tokens;
   Booli IsBinary,IsOutput;
   off_t Size;
   LongInt BufferSize,BufferFill;
   Byte *Buffer;
} TBinFmt_FileRec,*PBinFmt_FileRec;

/*---------------------------------------------------------------------*/


static char *ExNames[3]={"TO","DOWNTO","UPTO"};

static LongInt *Src_FileNo=NULL, *Src_LineNo=NULL;
static int Src_disable = 1;

/*---------------------------------------------------------------------*/

/**************************************************************************
 *
 * buffered write
 *
 **************************************************************************/
static void BinFmt_FlushBuffer(PBinFmt_FileRec Rec)
{
  if ((Rec->BufferSize>0) && (Rec->BufferFill>0) && (Rec->Buffer!=NULL)) {
    fwrite(Rec->Buffer,1,Rec->BufferFill,Rec->Datei);
    Rec->BufferFill=0;
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void Wr(PBinFmt_FileRec Rec, void *Data, LongInt Count)
{
  if (Rec->Buffer==NULL)
    fwrite(Data,1,Count,Rec->Datei);
  else {
    if (Count>Rec->BufferSize)
      fwrite(Data,1,Count,Rec->Datei);
    else {
      if (Rec->BufferFill+Count>Rec->BufferSize)
        BinFmt_FlushBuffer(Rec);

      memcpy(Rec->Buffer+Rec->BufferFill,Data,Count);
      Rec->BufferFill+=Count;
    }
  }
}

/*---------------------------------------------------------------------*/
/* binary input */

/**************************************************************************
 *
 *
 *
 **************************************************************************/
 LongInt BinFmt_ReadInteger(Byte *src)
{
  return (((LongInt) *src)+(((LongInt) src[1])<<8));
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
LongInt BinFmt_ReadLongInt(Byte *src)
{
  union {
    LongInt Int;
    char Vals[4];
  } TwoFace;

#ifdef BIGENDIAN
   SHUFFLE_4(TwoFace.Vals,src);
#else
   memcpy(TwoFace.Vals,src,4);
#endif
   return TwoFace.Int;
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
Double BinFmt_ReadDouble(Byte *src)
{
  Double val1;
  union {
    Double Float;
    char Vals[8];
  } TwoFace;

#ifdef BIGENDIAN
  SHUFFLE_8(TwoFace.Vals,src);
#else
  memcpy(TwoFace.Vals,src,8);
#endif

  /*********************************************************
   * convert from IEEE -> machine specific floating format
   * (macro defined in pal_env.h)
   *********************************************************/
#ifndef HAS_IEEE
  DOUBLE_FROM_IEEE(&val1, &TwoFace.Float);
  TwoFace.Float = val1;
#endif

  return TwoFace.Float;
}



/**************************************************************************
 *
 *
 *
 **************************************************************************/
char *BinFmt_ReadString(Byte *src)
{
   LongInt Length;
   static char Line[LINELEN+1];

   Length=ReadInteger(src);
   if (Length>LINELEN)
     Length=LINELEN;

   memcpy(Line,src+2,Length);
   Line[Length]='\0';
   return Line;
}

/*-------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_SetSrcInfo(LongInt CPU, LongInt FileNo, LongInt LineNo)
{
  if (!Src_disable) {
    Src_FileNo[CPU]=FileNo;
    Src_LineNo[CPU]=LineNo;
  }
}

/*-------------------------------------------------------------------------*/
/* binary output */

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void PutInteger(Byte *dest, LongInt Value)
{
  dest[0]=Value&0xff; dest[1]=Value>>8;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void BinFmt_WriteInteger(PBinFmt_FileRec Rec, LongInt Num)
{
  Byte Vals[2];

  Vals[0]=Num&0xff; Vals[1]=Num>>8;
  Wr(Rec,Vals,2);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void PutLongInt(Byte *dest, LongInt Value)
{
  union {
    LongInt Int;
    Byte Vals[4];
  } TwoFace;
#ifdef BIGENDIAN
  SHUFFLE_4(TwoFace.Vals,(char *)&Value);
  memcpy(dest,TwoFace.Vals,4);
#else
  memcpy(dest,(char *)&Value,4);
#endif
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/

static void BinFmt_WriteLongInt(PBinFmt_FileRec Rec, LongInt Num)
{
  Byte Vals[4];

  PutLongInt(Vals,Num);
  Wr(Rec,Vals,4);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void PutDouble(Byte *dest, Double Num)
{
  Double val1;
  union {
    Double Float;
    char Vals[8];
  } TwoFace;
  /********************************************************
   * convert from IEEE -> machine specific floating format
   * (macro defined in pal_env.h)
   ********************************************************/
#ifndef HAS_IEEE
  IEEE_FROM_DOUBLE(&val1, Num);
  Num = val1;
#endif

#ifdef BIGENDIAN
  SHUFFLE_8(TwoFace.Vals,(char *)&Num);
  memcpy(dest,TwoFace.Vals,8);
#else
  memcpy(dest,(char *)&Num,8);
#endif
  
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void PutString(Byte *dest, char *Line)
{
  LongInt len=strlen(Line);

  PutInteger(dest,len);
  memcpy(dest+2,Line,len);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void BinFmt_WriteDouble(PBinFmt_FileRec Rec, Double Num)
{
  Byte Vals[8];

  PutDouble(Vals,Num);
  Wr(Rec,Vals,8);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void BinFmt_WriteString(PBinFmt_FileRec Rec, char *Line)
{
  LongInt len;

  BinFmt_WriteInteger(Rec,len=strlen(Line));
  Wr(Rec,Line,len);
}

/*---------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static LongInt BinFmt_GetToken(void *File, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z;

  if (Name==NULL) {
    fprintf(stderr,"Fatal: BinFmt_GetToken: NULL pointer passed as "
                   "token name!\n");
    exit(255);
  }

  for (z=0; z<Rec->TokenCnt; z++)
    if (strcmp(Name,Rec->Tokens[z])==0)
      return z;

  Rec->Tokens=(char **) realloc(Rec->Tokens,sizeof(char *)*(Rec->TokenCnt+1));
  Rec->Tokens[Rec->TokenCnt]=strdup(Name);
  BinFmt_WriteActDef(File,Name,Rec->TokenCnt++);
  return z;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void BinFmt_SetToken(void *File, char *Name, LongInt Token)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z;

  if (Name==NULL) {
    fprintf(stderr,"Fatal: BinFmt_SetToken: NULL pointer passed as token name!");
    exit(255);
  }

  if (Token>=Rec->TokenCnt) {
    Rec->Tokens=(char **) realloc(Rec->Tokens,sizeof(char *)*(Token+1));
    for (z=Rec->TokenCnt; z<=Token; Rec->Tokens[z++]=NULL)
      ;

    Rec->TokenCnt=Token+1;
  }
   
  if (Rec->Tokens[Token]==NULL)
    Rec->Tokens[Token]=strdup(Name);
}

/*--------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void CheckRecordTokens(PBinFmt_FileRec Rec, BinRecord *Set, char *Msg)
{
  LongInt TInt;

  switch (Set->Type) {
     case BINFMT_RECTYPE_EXCHANGE:
      TInt=ReadLongInt(Set->Data+12);
      if ((TInt<0) || (TInt>=Rec->TokenCnt))
       fprintf(stderr,"%s: invalid EXCHANGE/old activity token %d out of"
                      " range 0..%d\n",Msg,TInt,Rec->TokenCnt);
      TInt=ReadLongInt(Set->Data+20);
      if ((TInt<0) || (TInt>=Rec->TokenCnt))  
       fprintf(stderr,"%s: invalid EXCHANGE/new activity token %d out of"
                      " range 0..%d\n",Msg,TInt,Rec->TokenCnt);
      break;
     case BINFMT_RECTYPE_DEFSYMBOL:
      TInt=ReadLongInt(Set->Data);
      if ((TInt<0) || (TInt>=Rec->TokenCnt))
       fprintf(stderr,"%s: invalid DEFSYMBOL/token %d out of range"
                      " 0..%d\n",Msg,TInt,Rec->TokenCnt);
      break;
  }
} 

/**************************************************************************
 *
 *
 *
 **************************************************************************/
static void CheckRecordStamp(BinRecord *Set, char *Msg)
{
  Double Stamp;
   
  switch (Set->Type) {
     case BINFMT_RECTYPE_EXCHANGE:
     case BINFMT_RECTYPE_COMMENT:
     case BINFMT_RECTYPE_SENDMSG:
     case BINFMT_RECTYPE_RECVMSG:
     case BINFMT_RECTYPE_SRCINFO:
      Stamp=ReadDouble(Set->Data);
      if (Stamp<0) {
        fprintf(stderr,"%s: Warning: record type %d contains negative "
                       "timestamp, setting to 0, strange things may "
                       "happen from now on...\n",Msg,Set->Type);
        PutDouble(Set->Data,0.0);
      }
  }
}

/*---------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void *BinFmt_OpenFile(char *Name)
{
  PBinFmt_FileRec Rec;
  FILE *Datei;
  unsigned char Header[8]=BINFMT_HEADER,ReadHeader[8];
  LongInt z;

  if (strcmp(Name,"-")==0)
    Datei=stdin;
  else
    Datei=MYFOPEN(Name,"r");

  if (Datei==NULL)
    return NULL;

  Rec=(PBinFmt_FileRec) malloc(sizeof(TBinFmt_FileRec));
  if (Rec==NULL) {
    fclose(Datei);
    return NULL;
  }

  Rec->Datei=Datei;
  Rec->IsBinary=TRUE;
  Rec->IsOutput=FALSE; 
  if (MYFSEEK(Datei,0,SEEK_END)==0) {
    Rec->Size=MYFTELL(Datei);
    rewind(Datei);
  }
  else
    Rec->Size=0;

  Rec->TokenCnt=1;  
  Rec->Tokens=(char **) malloc(sizeof(char *));
  if (Rec->Tokens==NULL) {
    free(Rec);
    fclose(Datei);
    return NULL;
  }
  Rec->Tokens[0]=strdup("");
  if (Rec->Tokens[0]==NULL) {
    free(Rec->Tokens);
    free(Rec);
    fclose(Datei);
    return NULL;
  }

  fread(ReadHeader,1,8,Datei);
  for (z=0; z<7; z++)
    if (Header[z]!=ReadHeader[z]) {
      free(Rec->Tokens[0]);
      free(Rec->Tokens);
      free(Rec);
      fclose(Datei);
      return NULL;
    }

  Rec->Buffer=NULL;
  Rec->BufferSize=Rec->BufferFill=0;

  return (void *) Rec;
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_CloseReadFile(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z;

  fclose(Rec->Datei);

  for (z=0; z<Rec->TokenCnt; free(Rec->Tokens[z++]))
    ;
  free(Rec->Tokens);
  free(Rec);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
Double BinFmt_GetPos(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Double Pos=MYFTELL(Rec->Datei);

  if (Pos<0)
    Pos=0; 

  return ((Rec->Size>0) ? Pos/Rec->Size : 0);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
LongInt BinFmt_GetAbsPos(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  return (MYFTELL(Rec->Datei));
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
Booli BinFmt_SetAbsPos(void *File, LongInt Pos)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsOutput) {
    fprintf(stderr,"sorry guys, position must not be set while writing "
                   "to files\n");
    return FALSE;
  }

  return (MYFSEEK(Rec->Datei,Pos,SEEK_SET)==0);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
BinRecord *BinFmt_ReadRecord(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  BinRecord *Set;
  Byte Buffer[4+2];
  char Line[300],*str;
   
  if (fread(Buffer,1,6,Rec->Datei)!=6)
    return NULL;
   
  if ((Set=(BinRecord *) malloc(sizeof(BinRecord)))==NULL)
    return NULL;
   
  Set->Length=ReadLongInt(Buffer);
  Set->Type=ReadInteger(Buffer+4);
  if (Set->Length==0)
    Set->Data=NULL;
  else
    if ((Set->Data=(Byte *) malloc(sizeof(Byte)*Set->Length))==NULL) {
      free(Set);
      return NULL;
    }
   
  if (fread(Set->Data,sizeof(Byte),Set->Length,Rec->Datei)!=Set->Length) {
    free(Set->Data); free(Set); return NULL;
  }

  if (Set->Type==BINFMT_RECTYPE_DEFTOKEN) {
    str=ReadString(Set->Data+4);
    strncpy(Line,str,300);
    BinFmt_SetToken(File,Line,ReadLongInt(Set->Data));
  }

  CheckRecordTokens(Rec,Set,"BinFmt_ReadRecord");
  CheckRecordStamp(Set,"BinFmt_ReadRecord");

  return Set;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
Double BinFmt_GetRecordTime(BinRecord *Set)
{
  switch (Set->Type) {
     case BINFMT_RECTYPE_EXCHANGE:
      return ReadDouble(Set->Data);
     case BINFMT_RECTYPE_COMMENT:
      return ReadDouble(Set->Data);
     case BINFMT_RECTYPE_SENDMSG:
      return ReadDouble(Set->Data);      
     case BINFMT_RECTYPE_RECVMSG:
      return ReadDouble(Set->Data);
     case BINFMT_RECTYPE_SRCINFO:
      return ReadDouble(Set->Data);
     case BINFMT_RECTYPE_DEFTOKEN:
      return -2;
     case BINFMT_RECTYPE_FILETOKEN:
      return -3;
     case BINFMT_RECTYPE_CREATOR:
      return -4;
     case BINFMT_RECTYPE_VERSION:
      return -5;
     case BINFMT_RECTYPE_TIMEOFFSET:
      return -98;
     case BINFMT_RECTYPE_CLKPERIOD:
      return -99;
     case BINFMT_RECTYPE_NCPUS:
      return -100; 
     case BINFMT_RECTYPE_UNMERGED:
      return -200;
     case BINFMT_RECTYPE_FILEIOBEGIN:
     case BINFMT_RECTYPE_FILEIOEND:
      return ReadDouble(Set->Data);
     default:
      return -1;
  }
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_SetRecordTime(BinRecord *Set, Double Time)
{
  switch (Set->Type) {
     case BINFMT_RECTYPE_EXCHANGE:
      PutDouble(Set->Data,Time); return;
     case BINFMT_RECTYPE_COMMENT:
      PutDouble(Set->Data,Time); return;
     case BINFMT_RECTYPE_SENDMSG:
      PutDouble(Set->Data,Time); return;
     case BINFMT_RECTYPE_RECVMSG:
      PutDouble(Set->Data,Time); return;
     case BINFMT_RECTYPE_SRCINFO:
      PutDouble(Set->Data,Time); return;
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
LongInt BinFmt_GetNCPUs(BinRecord *Set)
{
  return ReadInteger(Set->Data+4);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
Double BinFmt_GetCLKPERIOD(BinRecord *Set)
{
  Double d=ReadDouble(Set->Data);

  return d;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
LongInt BinFmt_GetVersion(BinRecord *Set)
{
  return ReadLongInt(Set->Data);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_FreeRecord(BinRecord *Set)
{
  if (Set->Length>0)
    free(Set->Data);

  free(Set);
}

/*--------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_SetBufferSize(void *File, LongInt BufferSize)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (!Rec->IsBinary)
    return;

  BinFmt_FlushBuffer(Rec);
  if (Rec->Buffer!=NULL)
    free(Rec->Buffer);

  Rec->Buffer=(Byte *) malloc(sizeof(Byte)*BufferSize);
  Rec->BufferFill=0;
  Rec->BufferSize=(Rec->Buffer==NULL) ? 0 : BufferSize;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void *BinFmt_CreateFile(char *Name, LongInt Binary)
{
  PBinFmt_FileRec Rec;
  FILE *Datei;
  int n;
  unsigned char Header[8]=BINFMT_HEADER;
   
  if (strcmp(Name,"-")==0)
    Datei=stdout;
  else
    Datei=MYFOPEN(Name,"w"); 

  if (Datei==NULL)
    return NULL;

  Rec=(PBinFmt_FileRec) malloc(sizeof(TBinFmt_FileRec)); 
  if (Rec==NULL)
    return Rec;

  Rec->Datei=Datei;
  Rec->TokenCnt=1;
  Rec->Size=0;
  Rec->Tokens=(char **) malloc(sizeof(char *));
  if (Rec->Tokens==NULL)
    return NULL;

  Rec->Tokens[0]=strdup("");
  if (Rec->Tokens[0]==NULL)
    return NULL;

  Rec->Buffer=NULL;
  Rec->BufferSize=Rec->BufferFill=0;

  if (Rec->IsBinary=Binary)
    Wr(Rec,Header,8);

  /******************************************************
   * as default source code info is disabled.
   * the Src_FileNo, Src_LineNo arrays are initialized
   * in BinFmt_WriteNCPUs().
   ******************************************************/
  Src_FileNo = Src_LineNo = (LongInt *)NULL;
  Src_disable = 1;

  return (void *) Rec;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void *BinFmt_CreateUnmergedFile(char *Name, LongInt Binary)
{
  void *File;
 
  File=BinFmt_CreateFile(Name,Binary);

  if (File!=NULL)
    BinFmt_WriteUnmergedFlag(File);

  return File;
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_CloseWriteFile(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z;

  BinFmt_FlushBuffer(Rec);

  fclose(Rec->Datei);

  for (z=0; z<Rec->TokenCnt; free(Rec->Tokens[z++]))
    ;
  free(Rec->Tokens);
  if (Rec->Buffer!=NULL)
    free(Rec->Buffer);

  Rec->BufferSize=Rec->BufferFill=0;
  free(Rec);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteUnmergedFlag(void *File)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
     BinFmt_WriteLongInt(Rec,0);
     BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_UNMERGED);
  }
  else
    fprintf(Rec->Datei,"           UNMERGED\n");
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteComment(void *File, Double Time, char *Comment)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,8+2+strlen(Comment));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_COMMENT);
    BinFmt_WriteDouble(Rec,Time);
    BinFmt_WriteString(Rec,Comment);
  }
  else
    fprintf(Rec->Datei,"%10.0f C %s\n",Time,Comment);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteCreator(void *File, char *Creator)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,2+strlen(Creator));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_CREATOR);
    BinFmt_WriteString(Rec,Creator);
  }
  else
    fprintf(Rec->Datei,"           CREATOR \"%s\"\n",Creator);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteVersion(void *File, LongInt Version)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_VERSION);
    BinFmt_WriteLongInt(Rec,Version);
  }
  else
    fprintf(Rec->Datei,"           VERSION %d\n",Version);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteActDef(void *File, char *Activity, LongInt Token)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4+2+strlen(Activity));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_DEFTOKEN);
    BinFmt_WriteLongInt(Rec,Token);
    BinFmt_WriteString(Rec,Activity);
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteSymbol(void *File, char *Activity, LongInt Num,
                        char *SymName)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt Act;

  if (Rec->IsBinary) {
    Act=BinFmt_GetToken(File,Activity);
    BinFmt_WriteLongInt(Rec,4+4+2+strlen(SymName));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_DEFSYMBOL);
    BinFmt_WriteLongInt(Rec,Act);
    BinFmt_WriteLongInt(Rec,Num);
    BinFmt_WriteString(Rec,SymName);
  }
  else
    fprintf(Rec->Datei,"           SYMBOL \"%s\" %d \"%s\"\n",Activity,
                                                              Num,SymName);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteSymbol2(void *File, char *Activity, LongInt Num,
                         char *SymName, LongInt FileNo, LongInt LineNo)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt Act;

  if (FileNo<0)
    BinFmt_WriteSymbol(File,Activity,Num,SymName);
  else
    if (Rec->IsBinary) {
      Act=BinFmt_GetToken(File,Activity);
      BinFmt_WriteLongInt(Rec,4+4+2+strlen(SymName)+4+4);
      BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_DEFSYMBOL);
      BinFmt_WriteLongInt(Rec,Act);
      BinFmt_WriteLongInt(Rec,Num);
      BinFmt_WriteString(Rec,SymName);
      BinFmt_WriteLongInt(Rec,FileNo);
      BinFmt_WriteLongInt(Rec,LineNo);
    }
    else
      fprintf(Rec->Datei,"           SYMBOL \"%s\" %d \"%s\" SRC %d:%d\n",
                                          Activity,Num,SymName,FileNo,LineNo);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteMsgSymbol(void *File, LongInt Type, LongInt Communicator,
                           char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  int Length;

  if (Rec->IsBinary) {
    Length=4+2+strlen(Name);
    if (Communicator!=BINFMT_NOCOMMUNICATOR) Length+=4;
    BinFmt_WriteLongInt(Rec,Length);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_DEFMSGSYMBOL);
    BinFmt_WriteLongInt(Rec,Type);
    BinFmt_WriteString(Rec,Name);
    if (Communicator!=BINFMT_NOCOMMUNICATOR) 
      BinFmt_WriteLongInt(Rec,Communicator);
  }
  else {
     fprintf(Rec->Datei,"           MSGTYPE ");
     if (Communicator!=BINFMT_NOCOMMUNICATOR) 
       fprintf(Rec->Datei,"%d ",Communicator);
     fprintf(Rec->Datei,"%d \"%s\"\n",Type,Name);
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteCPUSymbol(void *File, LongInt CPUNo, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4+2+strlen(Name));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_DEFCPUSYMBOL);
    BinFmt_WriteLongInt(Rec,CPUNo);
    BinFmt_WriteString(Rec,Name);
  }
  else
    fprintf(Rec->Datei,"           CPUSYM %d \"%s\"\n",CPUNo+1,Name);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteNCPUs(void *File, LongInt NumCnt, LongInt *Nums)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z;

  /** aSt 980914 
   * init source code info arrays.
   * moved from BinFmt_Open() to avoid use of
   * external VT_numproc.
   **/
  {
    int n, numprocs=0;

    for (n=0; n<NumCnt; n++)
      numprocs += Nums[n];

    Src_FileNo = (LongInt *)malloc(numprocs * sizeof(LongInt));
    Src_LineNo = (LongInt *)malloc(numprocs * sizeof(LongInt));
    if ((Src_FileNo == NULL) || (Src_LineNo == NULL)) {
      Src_disable = 1;
    }
    else {
      for (n=0; n<numprocs; n++)
        Src_FileNo[n]=Src_LineNo[n]=-1;
      Src_disable = 0;
    }
  }
  /* end */

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4*(1+NumCnt));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_NCPUS);
    BinFmt_WriteLongInt(Rec,NumCnt);
    for (z=0; z<NumCnt; z++)
      BinFmt_WriteLongInt(Rec,Nums[z]);
  }
  else {
    fprintf(Rec->Datei,"           NCPUS");
    for (z=0; z<NumCnt; z++)
      fprintf(Rec->Datei," %d",Nums[z]);
    fprintf(Rec->Datei,"\n"); 
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteCPUNAMES(void *File, LongInt NameCnt, char **Names)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt z,Len;

  if (Rec->IsBinary) {
    Len=4;
    for (z=0; z<NameCnt; Len+=2+strlen(Names[z++]))
      ;
    BinFmt_WriteLongInt(Rec,Len);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_CPUNAMES);
    BinFmt_WriteLongInt(Rec,NameCnt);
    for (z=0; z<NameCnt; BinFmt_WriteString(Rec,Names[z++]))
      ;
  }
  else {
    fprintf(Rec->Datei,"           CPUNAMES");
    for (z=0; z<NameCnt; z++)
      fprintf(Rec->Datei," \"%s\"",Names[z]);
    fprintf(Rec->Datei,"\n");
  }
} 

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteCLKPERIOD(void *File, Double ClkPeriod)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,8);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_CLKPERIOD);
    BinFmt_WriteDouble(Rec,ClkPeriod);
  }
  else
    fprintf(Rec->Datei,"           CLKPERIOD %g\n",ClkPeriod);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteTIMEOFFSET(void *File, LongInt Offset)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_TIMEOFFSET);
    BinFmt_WriteLongInt(Rec,Offset);
  }
  else
    fprintf(Rec->Datei,"           TIMEOFFSET %d\n",Offset);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteFILETOKEN(void *File, LongInt Token, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4+2+strlen(Name));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_FILETOKEN);
    BinFmt_WriteLongInt(Rec,Token);
    BinFmt_WriteString(Rec,Name);
  }
  else
    fprintf(Rec->Datei,"           FILETOKEN %d \"%s\"\n",Token,Name);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteGLOBALOPTOKEN(void *File, LongInt Token, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4+2+strlen(Name));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_GLOBALOPTOKEN);
    BinFmt_WriteLongInt(Rec,Token);
    BinFmt_WriteString(Rec,Name);
  }
  else
    fprintf(Rec->Datei,"           GLOBALOPTOKEN %d \"%s\"\n",Token,Name);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteREDFUNCTOKEN(void *File, LongInt Token, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,4+2+strlen(Name));
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_FILETOKEN);
    BinFmt_WriteLongInt(Rec,Token);
    BinFmt_WriteString(Rec,Name);
  }
  else
    fprintf(Rec->Datei,"           REDFUNCTOKEN %d \"%s\"\n",Token,Name);
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteSRCINFO(void *File, Double Time,
                         LongInt CPU, LongInt FileNR, LongInt LineNum)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;

  if (Rec->IsBinary) {
    BinFmt_WriteLongInt(Rec,8+4+4+4);
    BinFmt_WriteInteger(Rec,BINFMT_RECTYPE_SRCINFO);
    BinFmt_WriteDouble(Rec,Time);
    BinFmt_WriteLongInt(Rec,CPU);
    BinFmt_WriteLongInt(Rec,FileNR);
    BinFmt_WriteLongInt(Rec,LineNum);
  }
  else
    fprintf(Rec->Datei,"%10.0f SRC ON CPUID %d FILE %d %d",Time,CPU+1,
                                                           FileNR,LineNum);
}
   
/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteEXCHANGE2(void *File, Double Time, LongInt CPU, 
                           char *OldAct, LongInt OldActNum,
                           char *NewAct, LongInt NewActNum,
                           LongInt JobNum, Byte CallType)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt Old,New,Add;
  Byte Buffer[256];

  if (Rec->IsBinary) {
    Add=(!Src_disable && Src_FileNo[CPU] != -1) ? 8 : 0;
    Old=BinFmt_GetToken(File,OldAct);
    New=BinFmt_GetToken(File,NewAct);

    PutLongInt(Buffer+ 0,8+4+4+4+4+4+4+1+Add);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_EXCHANGE);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,CPU);
    PutLongInt(Buffer+18,Old);
    PutLongInt(Buffer+22,OldActNum);
    PutLongInt(Buffer+26,New);
    PutLongInt(Buffer+30,NewActNum);
    PutLongInt(Buffer+34,JobNum);
    Buffer[38]=CallType;
    if (Add) {
      PutLongInt(Buffer+39,Src_FileNo[CPU]);
      PutLongInt(Buffer+43,Src_LineNo[CPU]);
      Src_FileNo[CPU]=Src_LineNo[CPU]=-1;
    }
    Wr(Rec,Buffer,39+Add);
  }
  else {
    fprintf(Rec->Datei,"%10.0f EXCHANGE ON CPUID %d",Time,CPU+1);
    if (OldAct!=NULL)
      if (strlen(OldAct)!=0) {
        fprintf(Rec->Datei," FROM \"%s\"",OldAct);
        if (OldActNum!=BINFMT_NOACTNUM)
          fprintf(Rec->Datei," %d",OldActNum);
      }
    fprintf(Rec->Datei," %s \"%s\"",ExNames[CallType],
                                    (*NewAct!='\0')?NewAct:"NOACT");
    if (NewActNum!=BINFMT_NOACTNUM)
      fprintf(Rec->Datei," %d",NewActNum);
    fprintf(Rec->Datei," CLUSTER 0");
    if (JobNum!=BINFMT_NOJOB)
      fprintf(Rec->Datei," JOB %d",JobNum);
    if (!Src_disable && Src_FileNo[CPU] != -1) {
      fprintf(Rec->Datei," SRC %d:%d",Src_FileNo[CPU],Src_LineNo[CPU]);
      Src_FileNo[CPU]=Src_LineNo[CPU]=-1;
    }
    fprintf(Rec->Datei,"\n");
  }
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteEXCHANGE(void *File, Double Time, LongInt CPU, 
                          char *OldAct, LongInt OldActNum,
                          char *NewAct, LongInt NewActNum,
                          LongInt JobNum)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  LongInt Old,New,Add;
  Byte Buffer[256];

  if (Rec->IsBinary) {
    Add=(!Src_disable && Src_FileNo[CPU] != -1) ? 8 : 0;
    Old=BinFmt_GetToken(File,OldAct);
    New=BinFmt_GetToken(File,NewAct);

    PutLongInt(Buffer+ 0,8+4+4+4+4+4+4+Add);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_EXCHANGE);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,CPU);
    PutLongInt(Buffer+18,Old);
    PutLongInt(Buffer+22,OldActNum);
    PutLongInt(Buffer+26,New);
    PutLongInt(Buffer+30,NewActNum);
    PutLongInt(Buffer+34,JobNum);
    if (Add) {
      PutLongInt(Buffer+38,Src_FileNo[CPU]);
      PutLongInt(Buffer+42,Src_LineNo[CPU]);
      Src_FileNo[CPU]=Src_LineNo[CPU]=-1;
    }
    Wr(Rec,Buffer,38+Add);
  }
  else {
    fprintf(Rec->Datei,"%10.0f EXCHANGE ON CPUID %d",Time,CPU+1);
    if (OldAct!=NULL)
      if (strlen(OldAct)!=0) {
        fprintf(Rec->Datei," FROM \"%s\"",OldAct);
        if (OldActNum!=BINFMT_NOACTNUM)
          fprintf(Rec->Datei," %d",OldActNum);
      }
    fprintf(Rec->Datei," %s \"%s\"",*ExNames,(*NewAct!='\0')?NewAct:"NOACT");
    if (NewActNum!=BINFMT_NOACTNUM)
      fprintf(Rec->Datei," %d",NewActNum);
    fprintf(Rec->Datei," CLUSTER 0");
    if (JobNum!=BINFMT_NOJOB)
      fprintf(Rec->Datei," JOB %d",JobNum);
    if (!Src_disable && Src_FileNo[CPU] != -1) {
       fprintf(Rec->Datei," SRC %d:%d",Src_FileNo[CPU],Src_LineNo[CPU]);
       Src_FileNo[CPU]=Src_LineNo[CPU]=-1;
    }
    fprintf(Rec->Datei,"\n");
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteSENDMSG(void *File, Double Time, 
                       LongInt Sender, LongInt Receiver,
                       LongInt Communicator, LongInt Type,
                       LongInt Length)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[256];
  LongInt Add;

  if (Rec->IsBinary) {
    Add=(!Src_disable && Src_FileNo[Sender] != -1) ? 8 : 0;
    PutLongInt(Buffer+ 0,8+4+4+4+4+4+Add);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_SENDMSG);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,Sender);
    PutLongInt(Buffer+18,Receiver);
    PutLongInt(Buffer+22,Communicator);
    PutLongInt(Buffer+26,Type);
    PutLongInt(Buffer+30,Length);
    if (Add) {
      PutLongInt(Buffer+34,Src_FileNo[Sender]);
      PutLongInt(Buffer+38,Src_LineNo[Sender]);
      Src_FileNo[Sender]=Src_LineNo[Sender]=-1;
    }
    Wr(Rec,Buffer,34+Add);
  }
  else {
    fprintf(Rec->Datei,"%10.0f SENDMSG ",Time);
    if (Communicator!=BINFMT_NOCOMMUNICATOR)
      fprintf(Rec->Datei," %d",Communicator);
    fprintf(Rec->Datei," %d FROM %d TO %d LEN %d",
             Type,Sender+1,Receiver+1,Length);
    if (!Src_disable && Src_FileNo[Sender] != -1) {
      fprintf(Rec->Datei," SRC %d:%d",Src_FileNo[Sender],Src_LineNo[Sender]);
      Src_FileNo[Sender]=Src_LineNo[Sender]=-1;
    }
    fprintf(Rec->Datei,"\n");
  }
} 

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteRECVMSG(void *File, Double Time, 
                         LongInt Receiver, LongInt Sender,
                         LongInt Communicator, LongInt Type,
                         LongInt Length)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[256];
  LongInt Add;

  if (Rec->IsBinary) {
    Add=(!Src_disable && Src_LineNo[Receiver] != -1) ? 8 : 0;
    PutLongInt(Buffer+ 0,8+4+4+4+4+4+Add);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_RECVMSG);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,Receiver);
    PutLongInt(Buffer+18,Sender);
    PutLongInt(Buffer+22,Communicator);
    PutLongInt(Buffer+26,Type);
    PutLongInt(Buffer+30,Length);   
    if (Add) {
      PutLongInt(Buffer+34,Src_FileNo[Receiver]);
      PutLongInt(Buffer+38,Src_LineNo[Receiver]);
      Src_FileNo[Receiver]=Src_LineNo[Receiver]=-1;
    }
    Wr(Rec,Buffer,34+Add);
  }
  else {
    fprintf(Rec->Datei,"%10.0f RECVMSG ",Time);
    if (Communicator!=BINFMT_NOCOMMUNICATOR)
      fprintf(Rec->Datei," %d",Communicator);
    fprintf(Rec->Datei," %d BY %d FROM %d LEN %d",
             Type,Receiver+1,Sender+1,Length);
    if (!Src_disable && Src_LineNo[Receiver] != -1)  {
      fprintf(Rec->Datei," SRC %d:%d",Src_FileNo[Receiver],Src_LineNo[Receiver]);
      Src_FileNo[Receiver]=Src_LineNo[Receiver]=-1;
    }
    fprintf(Rec->Datei,"\n");
  }
}


/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteGlobalOp(void   *File,         Double  Time,
                          LongInt OpToken,      LongInt ProcessID,
                          LongInt Communicator, LongInt OpRoot,
                          LongInt BytesSend,    LongInt BytesRecv,
                          Double  Delta)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[256];

  if (Rec->IsBinary) {
    PutLongInt(Buffer   ,8+4+4+4+4+4+4+8+2);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_GLOBALOP);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,OpToken);
    PutLongInt(Buffer+18,ProcessID);
    PutLongInt(Buffer+22,Communicator);
    PutLongInt(Buffer+26,OpRoot);
    PutLongInt(Buffer+30,BytesSend);
    PutLongInt(Buffer+34,BytesRecv);
    PutDouble (Buffer+38,Delta);
    PutInteger(Buffer+46,0);
    Wr(Rec,Buffer,48);
  }
  else {
    fprintf(Rec->Datei,"%10.0f GLOBALOP %d ON %d",Time, OpToken, ProcessID+1);
    if (Communicator!=BINFMT_NOCOMMUNICATOR)
      fprintf(Rec->Datei," %d",Communicator);
    fprintf(Rec->Datei," %d %d %d %10.0f\n", OpRoot+1, BytesSend,
                                             BytesRecv,Delta);
  }
}
/*--------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
char *ExtString(char *Dest, char *Src)
{
  Dest=realloc(Dest,strlen(Dest)+strlen(Src)+1);
  if (Dest!=NULL)
    strcat(Dest,Src);
  return Dest;
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteRecordOffset(void *File, BinRecord *Set, LongInt offset, 
			      Double TimeOffset)
{
  switch (Set->Type) {
    case BINFMT_RECTYPE_EXCHANGE : 
         PutLongInt(Set->Data+8,ReadLongInt(Set->Data+8)+offset);
         PutDouble (Set->Data  ,ReadDouble (Set->Data  )+TimeOffset);
         break;
    case BINFMT_RECTYPE_SENDMSG  :
     {
        PutDouble (Set->Data  ,ReadDouble (Set->Data  )+TimeOffset);
        break;
     }
    case BINFMT_RECTYPE_RECVMSG  :
     {
        PutDouble (Set->Data  ,ReadDouble (Set->Data  )+TimeOffset);
        break;
     }
    case BINFMT_RECTYPE_COMMENT  :
     {
        PutDouble (Set->Data  ,ReadDouble (Set->Data  )+TimeOffset);
        break;
     }
    case BINFMT_RECTYPE_DEFCPUSYMBOL  :
     {
        PutLongInt(Set->Data  ,ReadLongInt(Set->Data  )+offset);
        break;
     }
    case BINFMT_RECTYPE_SRCINFO  :
     {
        PutDouble (Set->Data  ,ReadDouble (Set->Data  )+TimeOffset);
        break;
     }
   }
   BinFmt_WriteRecord(File, Set);
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_WriteRecord(void *File, BinRecord *Set)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  char Line[2000],Part[2000],Part2[2000],*run,*c;
  LongInt z,act,pos;

  if (Set->Type==BINFMT_RECTYPE_DEFTOKEN) {
     BinFmt_SetToken(File,ReadString(Set->Data+4),ReadLongInt(Set->Data));
  }  

  if (Rec->IsBinary) {
    CheckRecordTokens(Rec,Set,"BinFmt_WriteRecord");
    CheckRecordStamp(Set,"BinFmt_ReadRecord");
    BinFmt_WriteLongInt(Rec,Set->Length);
    BinFmt_WriteInteger(Rec,Set->Type);
    Wr(Rec,Set->Data,Set->Length);
  }
  else {
     switch (Set->Type) {
       case BINFMT_RECTYPE_DEFTOKEN:
        break;
       case BINFMT_RECTYPE_EXCHANGE:
        run=Line;
        run+=sprintf(run,"%10.0f EXCHANGE ON CPUID %d ",
                     ReadDouble(Set->Data),ReadLongInt(Set->Data+8)+1);
        if ((act=ReadLongInt(Set->Data+12))!=BINFMT_NOACT)
         {
          run+=sprintf(run,"FROM \"%s\" ",Rec->Tokens[act]);
          if ((act=ReadLongInt(Set->Data+16))!=BINFMT_NOACTNUM)
           run+=sprintf(run,"%d ",act);
         }
        run+=sprintf(run,"%s \"",ExNames[((Set->Length==33) || (Set->Length==41)) ?
                                         Set->Data[32] : BINFMT_EXTYPE_UNKNOWN]);
        run+=sprintf(run,"%s\" ",(ReadLongInt(Set->Data+20)==BINFMT_NOACT) ?
                     "NOACT" : Rec->Tokens[ReadLongInt(Set->Data+20)]);
        if ((act=ReadLongInt(Set->Data+24))!=BINFMT_NOACTNUM)
         run+=sprintf(run,"%d ",act);
        run+=sprintf(run,"CLUSTER 0 ");
        if ((act=ReadLongInt(Set->Data+28))!=BINFMT_NOJOB)
         run+=sprintf(run,"JOB %d ",act);
        if (Set->Length>=40)
         {
          z=32+(Set->Length&1);
          run+=sprintf(run," SRC %d:%d",ReadLongInt(Set->Data+z),ReadLongInt(Set->Data+z+4));
         }
        fprintf(Rec->Datei,"%s\n",Line);
        break; 
       case BINFMT_RECTYPE_SENDMSG:
        run=Line;
        run+=sprintf(run,"%10.0f SENDMSG ",ReadDouble(Set->Data));
        if ((act=ReadLongInt(Set->Data+16))!=BINFMT_NOCOMMUNICATOR)
         run+=sprintf(run,"%d ",act);
        run+=sprintf(run,"%d FROM %d TO %d LEN %d",ReadLongInt(Set->Data+20),
                ReadLongInt(Set->Data+8)+1,ReadLongInt(Set->Data+12)+1,
                ReadLongInt(Set->Data+24));
        if (Set->Length>=36)
         run+=sprintf(run," SRC %d:%d",ReadLongInt(Set->Data+28),ReadLongInt(Set->Data+32));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_RECVMSG:
        run=Line;
        run+=sprintf(run,"%10.0f RECVMSG ",ReadDouble(Set->Data));
        if ((act=ReadLongInt(Set->Data+16))!=BINFMT_NOCOMMUNICATOR)
         run+=sprintf(run,"%d ",act);
        run+=sprintf(run,"%d BY %d FROM %d LEN %d",ReadLongInt(Set->Data+20),
                ReadLongInt(Set->Data+8)+1,ReadLongInt(Set->Data+12)+1,
                ReadLongInt(Set->Data+24));
        if (Set->Length>=36)
         run+=sprintf(run," SRC %d:%d",ReadLongInt(Set->Data+28),ReadLongInt(Set->Data+32));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_COMMENT:
        sprintf(Line,"%10.0f C %s",ReadDouble(Set->Data),ReadString(Set->Data+8));
        fprintf(Rec->Datei,"%s\n",Line);
        break; 
       case BINFMT_RECTYPE_DEFSYMBOL:
        run=Line;
        run+=sprintf(run,"           SYMBOL \"%s\" %d \"%s\"",
                Rec->Tokens[ReadLongInt(Set->Data)],ReadLongInt(Set->Data+4),
                ReadString(Set->Data+8));
        z=ReadInteger(Set->Data+8);
        if (Set->Length>=18+z)
         run+=sprintf(run," SRC %d:%d",ReadLongInt(Set->Data+z+10),ReadLongInt(Set->Data+z+14));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_DEFMSGSYMBOL:
        strcpy(Part,ReadString(Set->Data+4));
        if (Set->Length==4+2+strlen(Part)) strcpy(Part2,"");
        else sprintf(Part2,"%d ",ReadLongInt(Set->Data+4+2+strlen(Part)));
        sprintf(Line,"           MSGTYPE %s%d \"%s\"",
                Part2,ReadLongInt(Set->Data),Part);
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_DEFCPUSYMBOL:
        strcpy(Part,ReadString(Set->Data+4));
        sprintf(Line,"           CPUSYM %d \"%s\"",ReadLongInt(Set->Data)+1,Part);
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_NCPUS:
        run=Line;
        run+=sprintf(run,"           NCPUS ");
        for (z=1; z<=ReadLongInt(Set->Data); z++)
         run+=sprintf(run," %d",ReadLongInt(Set->Data+(z*4)));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_CPUNAMES:
        run=Line;
        run+=sprintf(run,"           CPUNAMES");
        for (z=0,pos=4; z<ReadLongInt(Set->Data); z++)
         {
          c=ReadString(Set->Data+pos); run+=sprintf(run," \"%s\"",c);
          pos+=2+strlen(c);
         }
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_CLKPERIOD:
        sprintf(Line,"           CLKPERIOD %g",ReadDouble(Set->Data));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_TIMEOFFSET:
        sprintf(Line,"           TIMEOFFSET %d",ReadLongInt(Set->Data));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_FILETOKEN:
        sprintf(Line,"           FILETOKEN %d \"%s\"",ReadLongInt(Set->Data),
                                                      ReadString(Set->Data+4));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_SRCINFO:
        sprintf(Line,"%10.0f SRC ON CPUID %d FILE %d %d",ReadDouble(Set->Data),
                     ReadLongInt(Set->Data+8),
                     ReadLongInt(Set->Data+12),ReadLongInt(Set->Data+16));
       case BINFMT_RECTYPE_CREATOR:
        sprintf(Line,"           CREATOR \"%s\"",ReadString(Set->Data));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_VERSION:
        sprintf(Line,"           VERSION %d",ReadLongInt(Set->Data));
        fprintf(Rec->Datei,"%s\n",Line);
        break;
       case BINFMT_RECTYPE_UNMERGED:
        sprintf(Line,"           UNMERGED");
        fprintf(Rec->Datei,"%s\n",Line);
        break;
      case BINFMT_RECTYPE_MPIOFTOKEN:
        fprintf(Rec->Datei,"           IOFILE %d %d \"%s\"",
                                           ReadLongInt(Set->Data),
                                           ReadLongInt(Set->Data+4),
                                           ReadString(Set->Data+8));
        break;

      case BINFMT_RECTYPE_COMDEF:
        run=Line;
        run+=sprintf(run,"           COMDEF %d %d",
                           ReadLongInt(Set->Data),
                           ReadLongInt(Set->Data+4));
        for (z=0; z<ReadLongInt(Set->Data+8); z++) {
          run+=sprintf(run," %d:%d:%d",ReadLongInt(Set->Data+12+0+(z*12)),
                                       ReadLongInt(Set->Data+12+4+(z*12)),
                                       ReadLongInt(Set->Data+12+8+(z*12)));
        }
        fprintf(Rec->Datei,"%s\n",Line);
        break;

      case BINFMT_RECTYPE_FILEIOBEGIN:
        fprintf(Rec->Datei,"%10.0f FILEIOBEGIN %s ON CPUID %d %d %d\n",
                           ReadDouble(Set->Data),
                           (ReadInteger(Set->Data+8)==BINFMT_RECTYPE_FILEIO_READ?
                            "READ":"WRITE"),
                           ReadLongInt(Set->Data+10)+1,
                           ReadLongInt(Set->Data+14),
                           ReadLongInt(Set->Data+18));
        break;
      case BINFMT_RECTYPE_FILEIOEND:
        fprintf(Rec->Datei,"%10.0f FILEIOEND %s ON CPUID %d %d %d\n",
                           ReadDouble(Set->Data),
                           (ReadInteger(Set->Data+8)==BINFMT_RECTYPE_FILEIO_READ?
                            "READ":"WRITE"),
                           ReadLongInt(Set->Data+10)+1,
                           ReadLongInt(Set->Data+14),
                           ReadLongInt(Set->Data+18));
        break;
      }
    }
}

/*--------------------------------------------------------------------------*/

/**************************************************************************
 *
 *
 *
 **************************************************************************/
Booli BinFmt_TranslateRecord(void *Dest, void *Src, BinRecord *Set)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) Src;
  LongInt TInt;

  CheckRecordTokens(Rec,Set,"BinFmt_TranslateRecord");
  CheckRecordStamp(Set,"BinFmt_ReadRecord");
  switch (Set->Type) {
     case BINFMT_RECTYPE_DEFTOKEN:
     case BINFMT_RECTYPE_UNMERGED:
      return FALSE;
     case BINFMT_RECTYPE_DEFSYMBOL:
      TInt=ReadLongInt(Set->Data);
      PutLongInt(Set->Data,BinFmt_GetToken(Dest,Rec->Tokens[TInt]));
      return TRUE;
     
     case BINFMT_RECTYPE_SRCINFO:
      TInt=ReadLongInt(Set->Data+8);
      PutLongInt(Set->Data+8,BinFmt_GetToken(Dest,Rec->Tokens[TInt]));
      return TRUE;
     case BINFMT_RECTYPE_EXCHANGE:
      TInt=ReadLongInt(Set->Data+12);
      PutLongInt(Set->Data+12,BinFmt_GetToken(Dest,Rec->Tokens[TInt]));
      TInt=ReadLongInt(Set->Data+20);
      PutLongInt(Set->Data+20,BinFmt_GetToken(Dest,Rec->Tokens[TInt]));
      return TRUE;
     default:
      return TRUE;
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
void BinFmt_CorrTimestamp(BinRecord *Rec, Double Offset)
{
  switch (Rec->Type) {
     case BINFMT_RECTYPE_EXCHANGE:
     case BINFMT_RECTYPE_SENDMSG:
     case BINFMT_RECTYPE_RECVMSG:
     case BINFMT_RECTYPE_COMMENT:
     case BINFMT_RECTYPE_SRCINFO:
     case BINFMT_RECTYPE_FILEIOBEGIN:
     case BINFMT_RECTYPE_FILEIOEND:
      PutDouble (Rec->Data,ReadDouble (Rec->Data)+Offset);
      break;
  }
}

/**************************************************************************
 *
 *
 *
 **************************************************************************/
/*
 * write new record with program counter values
 * aSt 970903
 */
void BinFmt_WritePC(void *File,  Double Time,
                    LongInt CPU, LongInt FileNR, LongInt LineNR)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[LINELEN];

  if (Rec->IsBinary) {
    PutLongInt(Buffer+ 0,8+4+4+4);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_PC);
    PutDouble (Buffer+ 6,Time);
    PutLongInt(Buffer+14,CPU);
    PutLongInt(Buffer+18,FileNR);
    PutLongInt(Buffer+22,LineNR);
    Wr(Rec,Buffer,26);
  }
  else {
    int slen;

    slen = sprintf((char *)Buffer,"%10.0f SRC ON CPUID %d FILE %d %d\n",Time,
                                   CPU+1, FileNR, LineNR);
    Wr(Rec, Buffer, slen);
  }
}

/**************************************************************************
 *
 *  Write MPIO filetoken definition
 *
 **************************************************************************/
void BinFmt_WriteMPIOFTOKEN(void    *File,
                            LongInt  Token, LongInt CommId, char *Name)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[LINELEN];

  if (Rec->IsBinary) {
    int len = strlen(Name);

    PutLongInt(Buffer+ 0,4+4+4+2+len);
    PutInteger(Buffer+ 4,BINFMT_RECTYPE_MPIOFTOKEN);
    PutLongInt(Buffer+ 6,Token);
    PutLongInt(Buffer+10,CommId);
    PutString (Buffer+14,Name);
    Wr(Rec, Buffer, 6+4+4+4+2+len);
  }
  else {
    int slen;

    slen = sprintf((char *)Buffer,"           IOFILE %d %d \"%s\"\n",
                    Token, CommId, Name);
    Wr(Rec, Buffer, slen);
  }
}


/**************************************************************************
 *
 *  Write communicator definition record
 *
 **************************************************************************/
void BinFmt_WriteCOMDEF(void    *File,
                        LongInt  CommId,    LongInt  CommSize,
                        LongInt  TripCount, LongInt *Trips )
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte            Buffer[LINELEN];
  int             n;

  if (Rec->IsBinary) {
    LongInt *p;
    int      len = 6+4+4+4+(4*(TripCount*3));

    PutLongInt(Buffer+ 0, len-6);
    PutInteger(Buffer+ 4, BINFMT_RECTYPE_COMDEF);
    PutLongInt(Buffer+ 6, CommId);
    PutLongInt(Buffer+10, CommSize);
    PutLongInt(Buffer+14, TripCount);
    p = (LongInt *)(Buffer+18);
    for (n=0; n<(TripCount*3); n++) {
      PutLongInt((Byte *)p, Trips[n]);
      p++;
    }
    Wr(Rec, Buffer, len);
  }
  else {

    fprintf(Rec->Datei,"           COMDEF %d %d",CommId,CommSize);
    for (n=0; n<(TripCount*3); n+=3) {
      fprintf(Rec->Datei," %d:%d:%d",Trips[n],Trips[n+1],Trips[n+2]);
    }
    fprintf (Rec->Datei,"\n");
  }
}


/**************************************************************************
 *
 *  Write file IO record
 *
 **************************************************************************/
void BinFmt_WriteFILEIOBEGIN(void    *File,   Double  Time,
                             LongInt  CPU,    LongInt Type,
                             LongInt  FToken, LongInt Count)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[LINELEN];

  if (Rec->IsBinary) {
    PutLongInt(Buffer+ 0, 8+2+4+4+4);
    PutInteger(Buffer+ 4, BINFMT_RECTYPE_FILEIOBEGIN);
    PutDouble (Buffer+ 6, Time);
    PutInteger(Buffer+14, Type);
    PutLongInt(Buffer+16, CPU);
    PutLongInt(Buffer+20, FToken);
    PutLongInt(Buffer+24, Count);
    Wr(Rec,Buffer,28);
  }
  else {
    int slen;

    slen = sprintf((char *)Buffer,"%10.0f FILEIOBEGIN %s ON CPUID %d %d %d\n",
                   Time,(Type==BINFMT_RECTYPE_FILEIO_READ?"READ":"WRITE"),
                   CPU+1, FToken, Count);
    Wr(Rec,Buffer,slen);
  }
}

/**************************************************************************
 *
 *  Write file IO complete record
 *
 **************************************************************************/
void BinFmt_WriteFILEIOEND(void    *File,   Double  Time,
                           LongInt  CPU,    LongInt Type,
                           LongInt  FToken, LongInt Count)
{
  PBinFmt_FileRec Rec=(PBinFmt_FileRec) File;
  Byte Buffer[LINELEN];

  if (Rec->IsBinary) {
    PutLongInt(Buffer+ 0, 8+2+4+4+4);
    PutInteger(Buffer+ 4, BINFMT_RECTYPE_FILEIOEND);
    PutDouble (Buffer+ 6, Time);
    PutInteger(Buffer+14, Type);
    PutLongInt(Buffer+16, CPU);
    PutLongInt(Buffer+20, FToken);
    PutLongInt(Buffer+24, Count);
    Wr(Rec,Buffer,28);
  }
  else {
    int slen;

    slen = sprintf((char *)Buffer,"%10.0f FILEIOEND %s ON CPUID %d %d %d\n",
                   Time,(Type==BINFMT_RECTYPE_FILEIO_READ?"READ":"WRITE"),
                   CPU+1, FToken, Count);
    Wr(Rec,Buffer,slen);
  }
}
