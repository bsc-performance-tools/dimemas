#ifndef _BINFMT_H
#define _BINFMT_H
/*****************************************************************************
 *                                                                           *
 * VAMPIRtrace MPI Profiling Library - GENERIC Module                        *
 *                                                                           *
 * Copyright (c) PALLAS GmbH 1996-1999                                       *
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
 * $Log: binfmt.h,v $
 * Revision 1.2  2005/05/24 15:41:40  paraver
 * Modificacion global para retomar la version Stable como version de desarrollo
 *
 * Revision 1.1.1.1  1999/04/01 11:24:59  astein
 * Initial checkin
 *
 *
 *****************************************************************************/

/**************************************************************************/
/*                                                                        */
/*            Definition des PARvis-Binaer-Traceformats                   */
/*                                                                        */
/* 30.11.1995 Recordtypen fuer Kommentar und Erzeuger hinzugefuegt (aa)   */
/* 19. 2.1997 Recordtypen fuer absolute Zeiten hinzugefuegt (aa)          */
/*                                                                        */
/**************************************************************************/

/*========================================================================*/
/*                                                                        */
/* LongInt = signed Integer, 32 Bit                                       */
/* Word    = unsigned Integer, 16 Bit                                     */
/* Byte    = unsigned Integer, 8 Bit                                      */
/* Char    = single ASCII-char, 8 Bit                                     */
/* Double  = IEEE-Double-Precision-Float, 64 Bit                          */
/* String  = (1) one Word, as length byte                                 */
/*           (2) array of (1) Chars, to form the string                   */
/*                                                                        */
/* multibyte values are saved in Little-Endian-Format                     */

/*========================================================================*/
/* Definition des Dateikopfes:                                            */
/*                                                                        */
/* (1) ein LongInt mit dem Wert 0x5aa5aa55                                */
/* (2) vier Chars der Werte 'B','P','V','I'                               */

#define BINFMT_HEADER {0x55,0xaa,0xa5,0x5a,'B','P','V','I'}

/*========================================================================*/
/* Definition eines Recordkopfes:                                         */
/*                                                                        */
/* (1) ein LongInt, der die Recordlaenge (ausgenommen (1) und (2) )       */
/*     angibt                                                             */
/* (2) ein Word, das den Typ des Records definiert                        */

/*========================================================================*/
/* Dieser Record spezifiziert, dass die folgenden Events einen unge-      */
/* mergten Trace darstellen.  PARvis wird die Benutzung eines Traces      */
/* weigern, wenn dieser Record ALS ALLERERSTER in der Datei steht.        */
/*                                                                        */
/* kein Recordaufbau                                                      */

#define BINFMT_RECTYPE_UNMERGED 5

/*========================================================================*/
/* Dieser Record definiert einen Kommentar.  Er darf ueberall auftreten.  */
/* Er beinhaltet auch einen Zeitstempel, damit die Sortierroutinen in     */
/* pvmerge Kommentare an der korrekten Stelle belassen!                   */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Zeit angibt                                    */
/* (4) ein String, der den Kommentar selbst angibt                        */

#define BINFMT_RECTYPE_COMMENT 0

/*========================================================================*/
/* Dieser Record definiert den Erzeuger des Tracefiles, d.h. das verwen-  */
/* dete Instrumentierungstool.  PARvis/vampir koennen diesen Eintrag      */
/* auswerten und daraufhin bestimmte Elemente der Visualisierung automa-  */
/* tisch sperren.                                                         */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein String, der den Erzeuger beschreibt                            */

#define BINFMT_RECTYPE_CREATOR 1

/*========================================================================*/
/* Dieser Record definiert die Version des Tracefiles.  Version ist >=1.  */
/* Wenn nicht vorhanden, gilt Version=1.                                  */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der die Version angibt                                */

#define BINFMT_RECTYPE_VERSION 2

/*========================================================================*/
/* Dieser Record definiert ein Token fuer eine Aktivitaet.  Sinn ist,     */
/* dass bei den Exchange-Events nur ein Token anstelle eines Strings ge-  */
/* lesen werden muss.  Die Token-Werte sollten positiv sein, 0 gilt als   */
/* "keine Aktivitaet".  Der Record darf ueberall auftreten.               */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der das Token angibt                                  */
/* (4) ein String, der den Namen der zugehoerigen Aktivitaet angibt       */

#define BINFMT_NOACT 0
#define BINFMT_NOACTNUM -1
#define BINFMT_RECTYPE_DEFTOKEN 10

/*========================================================================*/
/* Dieser Record definiert ein Symbol aus Aktivitaet und Nummer und       */
/* ordnet dem Paar ein PARvis-Symbol zu.  Der Record darf ueberall        */
/* auftreten.                                                             */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der die Aktivitaet angibt                             */
/* (4) ein LongInt, der die Nummer angibt (!=BINFMT_NOACTNUM)             */
/* (5) ein String, der den Symbolnamen angibt                             */
/* (6) optional zwei Longints fuer Quellposition                          */

#define BINFMT_RECTYPE_DEFSYMBOL 20

/*========================================================================*/
/* Dieser Record definiert ein Nachrichten-Symbol, das einem Tag einen    */
/* symbolischen Namen zuordnet.  Der Record darf ueberall auftreten.      */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der das Tag angibt                                    */
/* (4) ein String, der den Symbolnamen angibt                             */
/* (5) optional ein LongInt, der das Symbol auf einen bestimmten Kommuni- */
/*     kator einschraenkt                                                 */

#define BINFMT_NOCOMMUNICATOR -0x7fff
#define BINFMT_RECTYPE_DEFMSGSYMBOL 30

/*========================================================================*/
/* Dieser Record definiert ein Symbol, das einem Prozessor einen symboli- */
/* schen Namen zuordnet.  Der Record darf ueberall auftreten.             */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der den Prozessor angibt                              */
/* (4) ein String, der den symbolischen Namen angibt                      */

#define BINFMT_RECTYPE_DEFCPUSYMBOL 100

/*========================================================================*/
/* Dieser Record definiert die Zahl von Prozessoren, die in dem Trace     */
/* enthalten sind.  CPU-Nummern werden spaeter immer in der Form 0..n-1   */
/* angegeben.  Dieser Record darf nur am Anfang, vor einem Event-Record   */
/* auftreten!                                                             */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der die Zahl der Gruppen angibt.                      */
/* (4) ein Feld von (3) LongInts, welches die Anzahl CPUs in den einzel-  */
/*     nen Gruppen angibt.  Die Gesamtsumme darf fuer PARvis momentan     */
/*     nicht 512 uebersteigen, das ist aber kein Problem des Treibers.    */
 
#define BINFMT_RECTYPE_NCPUS 40

/*========================================================================*/
/* Dieser Record definiert die Namen der einzelnen Prozessorgruppen.  Er  */
/* darf nur am Anfang und sollte hinter dem NCPUS-Record stehen.          */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der die Zahl der Gruppen angibt.                      */
/* (4) ein Feld von (3) Strings, das die Namen angibt.                    */

#define BINFMT_RECTYPE_CPUNAMES 50 

/*========================================================================*/
/* Dieser Record definiert die Laenge einer Taktperiode in Sekunden. Er   */
/* darf nur am Anfang stehen.                                             */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Periodenlaenge angibt                          */

#define BINFMT_RECTYPE_CLKPERIOD 60

/*========================================================================*/
/* Dieser Record definiert den absoluten Offset aller Timestamps in Se-   */
/* kunden seit dem 1.1.1970 (reicht erstmal knapp 70 Jahre...). Er darf   */
/* nur am Anfang stehen.                                                  */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der den Offset angibt                                 */

#define BINFMT_RECTYPE_TIMEOFFSET 110

/*========================================================================*/
/* Dieser Record definiert den Zustandswechsel eines Prozessors.  Die     */
/* alte Aktivitaet darf auf NOACT gesetzt werden, um Inkonsistenzen zu    */
/* unterdruecken.                                                         */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Zeit in Takten angibt [6]                      */
/* (4) ein LongInt, der den Prozessor angibt [14]                         */
/* (5) ein LongInt, der die alte Aktivitaet angibt [18]                   */
/* (6) ein LongInt, der die alte Aktivitaetsnummer angibt [22]            */
/* (7) ein LongInt, der die neue Aktivitaet angibt [26]                   */
/* (8) ein LongInt, der die neue Aktivitaetsnummer angibt [30]            */
/* (9) ein LongInt, der die Jobnummer angibt [34]                         */
/* (10) optional ein Byte, das Ein/Ausspruenge spezifiziert [38]          */
/* (11) optional zwei LongInts, die Quelldatei und -zeile angeben [38+42/39+43] */

#define BINFMT_NOJOB -9999999
#define BINFMT_RECTYPE_EXCHANGE 70
#define BINFMT_EXTYPE_UNKNOWN 0
#define BINFMT_EXTYPE_CALL 1
#define BINFMT_EXTYPE_RETURN 2

/*========================================================================*/
/* Dieser Record definiert den Versand einer Nachricht.                   */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Zeit in Takten angibt [6]                      */
/* (4) ein LongInt, der den Prozessor angibt [14]                         */
/* (5) ein LongInt, der den Zielprozessor angibt [18]                     */
/* (6) ein LongInt, der den Kommunikator angibt [22]                      */
/* (7) ein LongInt, der den Nachrichtentyp (Tag) angibt [26]              */
/* (8) ein LongInt, der die Nachrichtenlaenge angibt [30]                 */
/* (9) optional zwei LongInts, die Quelldatei und -zeile angeben [34+38]  */

#define BINFMT_RECTYPE_SENDMSG 80

/*========================================================================*/
/* Dieser Record definiert den Empfang einer Nachricht.                   */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Zeit in Takten angibt                          */
/* (4) ein LongInt, der den Prozessor angibt                              */
/* (5) ein LongInt, der den Quellprozessor angibt                         */
/* (6) ein LongInt, der den Kommunikator angibt                           */
/* (7) ein LongInt, der den Nachrichtentyp (Tag) angibt                   */
/* (8) ein LongInt, der die Nachrichtenlaenge angibt                      */
/* (9) optional zwei LongInts, die Quelldatei und -zeile angeben          */

#define BINFMT_RECTYPE_RECVMSG 90

/*========================================================================*/
/* Dieser Record definiert ein Datei-Token, d.h. eine Kurzschreibweise    */
/* fuer einen Dateinamen.  Ein solches Token muss VOR der ersten Benut-   */
/* zung definiert werden!                                                 */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein LongInt, der das Token angibt                                  */
/* (4) ein String, der den Dateinamen selber angibt                       */

#define BINFMT_RECTYPE_FILETOKEN 120


/*========================================================================*/
/* This record defines a global operation token. This definition must     */
/* defined before the first usage !                                       */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/* (3) one LongInt, to define the token value                             */
/* (4) String, to define the name of the global operation                 */

#define BINFMT_RECTYPE_GLOBALOPTOKEN 200

/*========================================================================*/
/* This record defines a global reduce function token. This definition    */
/* must defined before the first usage !                                  */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/* (3) one LongInt, to define the token value                             */
/* (4) String, to define the name of the global function                  */

#define BINFMT_RECTYPE_REDFUNC 210

/*========================================================================*/
/* This record  defines a global operation data record.                   */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/*  (3) one LongInt, <Global operation token>                             */
/*  (4) one LongInt, <Process ID (MPI global rank)>                       */
/*  (5) one LongInt, <communicator ID>                                    */
/*  (6) one LongInt, <root process ID of global operation>                */
/*  (7) one LongInt, <Bytes Send>                                         */
/*  (8) one LongInt, <Bytes Received>                                     */
/*  (9) one LongInt, <Reduce function token)>                             */
/* (10) one Double,  <Delta t>                                            */

#define BINFMT_RECTYPE_GLOBALOP 220

/*========================================================================*/
/* This record  defines a MPI-IO file token. This definition must defined */
/* before the first usage !                                               */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/* (3) one LongInt, to define the token value                             */
/* (4) one LongInt, to define the appropriate communicator                */
/* (5) String, to define the filename                                     */

#define BINFMT_RECTYPE_MPIOFTOKEN 230

/*========================================================================*/
/* This record defines a MPI-IO file access.                              */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/*  (3) one Double,  actual timestamp                                     */
/*  (4) one LongInt, <READ|WRITE> access                                  */
/*  (5) one LongInt, <CPUID>                                              */
/*  (6) one LongInt, <FILETOKEN>                                          */
/*  (7) one LongInt, <COUNT bytes transferred>                            */

#define BINFMT_RECTYPE_FILEIOBEGIN 240

/*========================================================================*/
/* This record defines a MPI-IO file access.                              */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/*  (3) one Double,  actual timestamp                                     */
/*  (4) one LongInt, <READ|WRITE> access                                  */
/*  (5) one LongInt, <CPUID>                                              */
/*  (6) one LongInt, <FILETOKEN>                                          */
/*  (7) one LongInt, <COUNT bytes transferred>                            */

#define BINFMT_RECTYPE_FILEIOEND 245

#define BINFMT_RECTYPE_FILEIO_READ	1
#define BINFMT_RECTYPE_FILEIO_WRITE	2

/*========================================================================*/
/* This record defines a MPI communicator.                                */
/*                                                                        */
/* record structure:                                                      */
/*                                                                        */
/*  (3) one LongInt, <COMMID>                                             */
/*  (4) one LongInt, <COMMSIZE>                                           */
/*  (5)\                                                                  */
/*   :  three LongInts to define a triplet <LOWER>:<UPPER>:<INCREMENT>    */
/*  (N)/                                                                  */

#define BINFMT_RECTYPE_COMDEF 250


/*========================================================================*/
/* Dieser Record definiert als Kompatibilitaet zu Pallas-NEC-Tracefiles   */
/* Quellcodeinformationen fuer das vorausgegangene (auweia...) Event.     */
/*                                                                        */
/* Recordaufbau:                                                          */
/*                                                                        */
/* (3) ein Double, der die Zeit in Takten angibt                          */
/* (4) ein LongInt, der den Prozessor angibt                              */
/* (5) ein LongInt, der den Dateinamen angibt                             */
/* (6) ein LongInt, der die Zeilennummer angibt                           */

#define BINFMT_RECTYPE_SRCINFO	32767
/*
 * traditional name
 */
#define BINFMT_RECTYPE_PC 	32767

#endif /* _BINFMT_H */
