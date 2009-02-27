#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "define.h"

#ifdef ARCH_HPUX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/utsname.h>
#endif

#ifdef ARCH_LINUX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>                                                   
#include <string.h>
#include <netdb.h>
#endif

#ifdef ARCH_CYGWIN
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef ARCH_SGI
#include <sys/systeminfo.h>
#endif 

#ifdef ARCH_ALPHA
#include <unistd.h>
#endif

#define TELNET_PORT 23

#define MACH_SGI   1
#define MACH_HP    2
#define MACH_LINUX 3
#define MACH_IBM   4
/* Falta la versio SUN? */
#define MACH_ALPHA 6
#define MACH_WINDOWS 7

static char buf[BUFSIZE];
static char license_file[1024];
static char license_error[1024];
static int  is_license_error=0;

static unsigned int All[256];
static unsigned int Final[256];
static int          local_index=0;

static int Coeff_Matrix[16][16] = 
{ {10, 7, 12, 4, 16, 1, 3, 14, 6, 8, 11, 15, 13, 9, 5, 2},
  {7, 2, 3, 5, 1, 9, 13, 6, 12, 16, 4, 8, 15, 11, 10, 14},
  {4, 8, 5, 3, 11, 14, 6, 13, 10, 16, 2, 15, 12, 7, 1, 9},
  {5, 15, 12, 16, 10, 9, 6, 3, 4, 1, 7, 2, 13, 11, 8, 14},
  {12, 2, 8, 1, 11, 7, 13, 6, 9, 4, 3, 10, 14, 15, 16, 5},
  {12, 11, 5, 14, 8, 7, 15, 3, 16, 10, 1, 9, 4, 13, 2, 6},
  {3, 15, 10, 1, 8, 16, 6, 9, 2, 7, 13, 11, 5, 14, 12, 4},
  {14, 3, 10, 5, 8, 1, 11, 2, 16, 4, 13, 7, 9, 15, 12, 6},
  {11, 9, 15, 8, 14, 1, 2, 10, 5, 12, 7, 6, 3, 16, 4, 13},
  {2, 3, 6, 11, 15, 4, 14, 7, 13, 12, 5, 1, 8, 16, 10, 9},
  {16, 1, 2, 15, 7, 6, 4, 11, 10, 12, 13, 9, 14, 3, 5, 8},
  {12, 13, 5, 7, 4, 15, 1, 8, 10, 14, 2, 11, 6, 16, 3, 9},
  {16, 5, 4, 6, 3, 14, 7, 13, 12, 11, 9, 2, 1, 10, 8, 15},
  {12, 8, 6, 10, 9, 3, 4, 7, 2, 13, 1, 16, 5, 11, 14, 15},
  {8, 15, 5, 9, 11, 14, 10, 13, 6, 3, 12, 16, 7, 4, 2, 1},
  {5, 9, 12, 2, 14, 10, 7, 3, 15, 8, 13, 16, 4, 1, 6, 11}};


static int  Q1 = 13,
            Q2 = 2,
            S1 = 12,
            S2 = 17,
            P1mS1 = 19,
            P2mS2 = 12,
            P1mP2 = 2;

static unsigned int I1,
                    I2,
                    b,
                    Mask1 = 2147483647,
                    Mask2 = 536870911;

static double Norm = 4.656612873e-10;

static void
srandomine (unsigned int seed1, unsigned int seed2)
{
  seed1 = seed1 + 2147483;
  seed2 = seed2 + 536870;
  I1 = seed1 & Mask1;
  I2 = seed2 & Mask2;
}

static float
randomine ()
{
  b  = ((I1 << Q1) ^ I1) & Mask1;
  I1 = ((I1 << S1) ^ (b >> P1mS1)) & Mask1;
  b  = ((I2 << Q2) ^ I2) & Mask2;
  I2 = ((I2 << S2) ^ (b >> P2mS2)) & Mask2;

  return ((float) (I1 ^ (I2 << P1mP2)) * Norm);
}


static int
asci_to_version(char *s)
{
  int re=0;
  int order=1;
  
  while (*s)
  {
    if ((*s>='0') && (*s<='9'))
    {
      re = re * order + (*s)-'0';
      order = order*10;
    }
    s++;
  }
  return (re);
}

static double
uniform(double min, double max)
{
  double  rnum, v_funcion;

  rnum      = randomine ();
  v_funcion = min + ((max - min) * rnum);

  return v_funcion;
}

static unsigned int
get_date ()
{
  char            time[64];
  struct timeval  tv1;
  int             mm,yy,dd;

  gettimeofday(&tv1, (struct timezone*)0);
  strftime(time,20,"%m/%d/%Y", gmtime(&tv1.tv_sec));
  sscanf(time,"%d/%d/%d",&mm,&dd, &yy);
  return ((yy-1995)*500+mm*40+ dd);
}

static void 
license_error_format()
{
  sprintf (license_error,
           "%s%s%s",
           "Invalid format or data in license file\n",
           "Contact the license service at\n",
           "dimemas@cepba.upc.es\n");
  is_license_error++;
}

static int
get_line (char *buf, int si, FILE *fp)
{
  if (feof(fp))
    return (-1);

  if (fgets (buf, si, fp)==(char *)0)
  {
    license_error_format();
    return(-1);
  }

  return(0);
}

static void
read_file (unsigned int *version_file,
           unsigned int *machine, 
           unsigned int *hostid_file,
           unsigned int *serialid_file, 
           unsigned int *dat_file,
           char         *KEY_file,
           int           line)
{
  char         *prom_home;
  int           dd, mm, yy;
  FILE         *fl;
  unsigned long hostid;
  char         *pathCWD;
  char          string_version[256];
  int           i = 0;

  if ((prom_home=getenv ("DIMEMAS_HOME"))==(char *)0)
  {
    sprintf (license_error,
             "%s%s",
             "Unable to locate license file\n",
             "Check the definition of variable DIMEMAS_HOME\n");
    is_license_error++;
    return;
  }
  
  sprintf (license_file, "%s/etc/license.dat", getenv ("DIMEMAS_HOME"));
  
  if ((fl = MYFOPEN(license_file,"r"))==(FILE *)0)
  {
    sprintf (license_error,
             "%s%s\n%s",
             "Unable to open License file ",
             license_file,
             "Check file permission and name\n");
    is_license_error++;
    return;
  }

  while (i <= line)
  {
    if (get_line(buf, BUFSIZE, fl)==-1)
    {
      sprintf (license_error,
               "%s\n%s",
               "Unable to find a license in file ",
               license_file);
      is_license_error++;
      return;
    }

    if (strcmp (buf, "Dimemas License file\n")==0)
    {
      i++;
    }
  }

  get_line(buf, BUFSIZE, fl);

  if (sscanf (buf, "Version           : %s\n",string_version)!=1)
  {
    license_error_format();
    if (fl !=(FILE *)0)
      fclose(fl);
    return;
  }
  else
  {
    *version_file = asci_to_version(string_version);
  }

  get_line(buf, BUFSIZE, fl);
  if (sscanf (buf, "Machine type      : %s\n",string_version)!=1)
  {
    license_error_format();
    if (fl !=(FILE *)0)
      fclose(fl);
    return;
  }
  else
  {
    *machine = 0;
    if (strcmp(string_version,"SGI")==0)
      *machine = MACH_SGI;
    if (strcmp(string_version,"HP")==0)
      *machine = MACH_HP;
    if (strcmp(string_version,"Linux")==0)
      *machine = MACH_LINUX;
    if (strcmp(string_version,"Windows")==0)
      *machine = MACH_WINDOWS;
    if (strcmp(string_version,"IBM")==0)
      *machine = MACH_IBM;
    if (strcmp(string_version,"Alpha")==0)
      *machine = MACH_ALPHA;
  }
                                                                                
  get_line(buf, BUFSIZE, fl);
  if (sscanf (buf, "Host identificator: %X\n",hostid_file)!=1)
  {
    license_error_format();
    if (fl !=(FILE *)0)
      fclose(fl);
    return;
  }

  get_line(buf, BUFSIZE, fl);
  if (sscanf (buf, "Serial host number: %X\n",serialid_file)!=1)
  {
    license_error_format();
    if (fl !=(FILE *)0)
      fclose(fl);
    return;
  }
  get_line(buf, BUFSIZE, fl);
  if (sscanf (buf, "Limit date        : %d/%d/%d\n",&dd,&mm,&yy)!=3)
  {
    if (strcmp(buf,"Limit date        : Unlimited\n")!=0)
    {
      license_error_format();
      if (fl !=(FILE *)0)
        fclose(fl);
      return;
    }
    else
      *dat_file = 0;
  }
  else
  {
    if ((dd<1) || (dd>31) || (mm<1) || (mm>12) || (yy<1995) || (yy>2025))
    {
      license_error_format();
      if (fl !=(FILE *)0)
        fclose(fl);
      return;
    }
    *dat_file = (yy-1995)*500+mm*40+ dd;
  }

  get_line(buf, BUFSIZE, fl);
  if (sscanf (buf, "License Key       : %s\n", KEY_file)!=1)
  {
    license_error_format();
    if (fl !=(FILE *)0)
      fclose(fl);
    return;
  }
  if (fl !=(FILE *)0)
    fclose(fl);
}

#ifdef ARCH_AIX 
#include <sys/utsname.h>

unsigned long get_hardware_serial_number_IBM () {
struct utsname name;

  unsigned long serialid;

  uname(&name);
  serialid = strtoul(name.machine+(strlen(name.machine)-8), (char **)0, 16);
  return(serialid);
}
#endif

#ifdef ARCH_ALPHA
#   include <string.h>
#   include <errno.h>

#define BUFFER_NAME 1024

char device[300], IPaddr[300], mask[300], Addr1[300], Rest[700];

unsigned long
SerialNumberFromNETSTAT(int fd) {

  int           bytes, ret;
  char         *pointer = NULL;
  char         *piece = NULL, *middel = NULL;
  char         *bigpointer3 = NULL;
  int           old = 0, found;
  unsigned long seriall;
  unsigned long serial;

  piece = (char *) malloc (BUFFER_NAME);

  while ((bytes = read(fd,piece,BUFFER_NAME)) != 0)
  {
    if ((bytes == -1) && (errno==EINTR))
      continue;

    piece[bytes] = 0;
    if (old != 0)
    {
      middel=(char *) malloc(old+1);
      strcpy(middel,bigpointer3);
      free(bigpointer3);
    }
    
    bigpointer3 = (char *) malloc(old+bytes+1);
    if (old != 0)
    {
       strcpy(bigpointer3,middel);
       strcat(bigpointer3,piece);
       free(middel);
    }
    else strcpy(bigpointer3,piece);
    old = old + bytes;
  }

  free(piece);

  found = 0;
  if (old)
  {
    piece = strtok(bigpointer3,"\n");
    if (piece != NULL)
    {
      piece = strtok(NULL,"\n");
      if (piece != NULL)
      {
        sscanf(piece,"%s %s %s %s [^\n]\n",device,IPaddr,mask,Addr1,Rest);

        piece  = (char *) malloc (BUFFER_NAME);
        middel = strtok(Addr1,":");
        sprintf(piece,"0x");

        while (middel != NULL)
        {
          sprintf(piece,"%s%s", piece, middel);
          middel = strtok(NULL,":");
        }

        sscanf(piece,"%lx",&(seriall));
        seriall = seriall & 0x00000000FFFFFFFF;
        serial = (unsigned long) seriall;

        free(piece);
      }
    }
  }
  return serial;
}


static unsigned long get_hardware_serial_number_ALPHA () {
        int             pipefd[2];
        unsigned long serial;
        pid_t           pidChild;
        int             statptr;

        if (pipe (pipefd) == -1){
            fprintf(stderr,"ERROR at pipe: %s\n",strerror(errno));
            return 1;
        }

        switch (pidChild=fork()) {
            case 0: /* child process */
                    close(1); /* close standard output */

                    /** Redirect standar output to the pipe */
                    if (dup(pipefd[1])== -1){
                         fprintf(stderr,"ERROR at dup: %s\n",strerror(errno));
                         close (pipefd[0]);
                         close (pipefd[1]);
                         exit(-1); /** Error */
                    }

                    if (execlp("/usr/sbin/netstat","/usr/sbin/netstat","-ia",NULL) == -1){
                         fprintf(stderr,"ERROR at execlp: %s\n",strerror(errno));
                         close (pipefd[0]);
                         close (pipefd[1]);

                        exit(-1); /** Error */
                    }
                    exit(0);
                    break;

        case -1: /* some error forking */
                fprintf(stderr,"ERROR at fork: %s\n",strerror(errno));
                close (pipefd[0]);
                close (pipefd[1]);
                return 1;

        default: /* parent process, everything ok */
                 close (pipefd[1]);           /** Close the writing pipe */

                 waitpid(pidChild,&(statptr),0);   /* wait for the child */

                 if (statptr!=-1){
                     serial = SerialNumberFromNETSTAT(pipefd[0]);
                     close (pipefd[0]);
                     return serial;
                 }
                 else {  /** Some error assume the process is alive */
                    fprintf(stderr,"ERROR in child\n");
                    close (pipefd[0]);
                    exit(1);
                 }
        }
        return 1;
}
#endif



#ifdef ARCH_LINUX
int 
find_machineLinux (int *hostid, int *serialid)
{
  int pi[2];
  int i;
  char buf[512];
  int p2[2];
  char *es;
  char f[10];
  char s[10];
  struct hostent *hp; 
  char buff[200];
  unsigned long adr;
  
  long hostid_up;
  long hostid_down;
  
  
  pipe (pi);
  
  if (fork()==0)
  {
    pipe (p2);
    if (fork()==0)
    {
      dup2 (p2[0], 0);
      dup2 (pi[1], 1);
      close (pi[1]); close (pi[0]);
      close (p2[1]); close (p2[0]);
      if (execlp ("grep", "grep", "HWaddr", (char *)0)==-1)
         perror ("FAILS");
      exit(1);
    }
    else
    {
      dup2 (p2[1], 1);
      close (pi[1]); close (pi[0]);
      close (p2[1]); close (p2[0]);
      if (execlp ("/sbin/ifconfig", "ifconfig", "-a", (char *)0)==-1)
         perror ("FAILS");
      exit(1);
    }
  }
  close (pi[1]);
  i = read (pi[0], buf, 512);
  if (i != 512)
  {
    buf[i] = (char)0;
    es = (char *)strstr(buf, "HWaddr");
    es = (char *)strstr(es, " ");
    es = es+1;
    f[0] = es[0]; f[1] = es[1];
    f[2] = es[3]; f[3] = es[4];
    f[4] = es[6]; f[5] = es[7];
    f[6] = 0;
    s[0] = es[6]; s[1] = es[7];
    s[2] = es[9]; s[3] = es[10];
    s[4] = es[12]; s[5] = es[13];
    s[6] = es[15]; s[7] = es[16];
    s[8] = 0;
    *hostid   = strtoul(f,NULL,16);
    *serialid = strtoul(s,NULL,16);

    hostid_up   = gethostid();
    hostid_down = hostid_up;
    
    hostid_up   = hostid_up   << 16;
    hostid_down = hostid_down >> 16;
    
    *hostid = (int) ( hostid_up & 0xFFFF0000) | ( hostid_down & 0x0000FFFF);

    /* DEBUG 
    gethostname(buff,sizeof(buff));
    hp = gethostbyname(buff);
    memcpy(hostid,hp->h_addr,hp->h_length);
    */

  }
  wait (0);
}        
#endif



#ifdef ARCH_CYGWIN
int find_machineWindows (int *hostid, int *serialid)
{
  char *nom_temporal, *buff;
  char comanda[256];
  char mac[12];
  int ip[4];
  FILE *fd_temp;
  int res, final_mac=0, final_ip=0;
  struct hostent *hp;
  
  /* S'obte un nom de fitxer temporal */
  nom_temporal = tempnam(".","MAC_");
  if (nom_temporal==NULL) exit(1);

  /* Es reserva memoria pel buffer de lectura */
  buff = (char *) malloc(4096);
  if (buff == NULL) exit(1);

  /* Es genera la comanda per obtenir l'adrec,a MAC */
  sprintf(comanda,"ipconfig /all > %s", nom_temporal);
  
  /* S'executa la comanda */
  res = system(comanda);
  if (res<0) exit(1);

  /* Cal parsejar el contingut del fitxer per llegir l'adreça MAC */
  fd_temp = MYFOPEN(nom_temporal,"r");
  if (fd_temp<0) exit(1);
  
  /*printf("Just abans de parsejar la sortida\n");*/

  while ((!final_mac) || (!final_ip))
  {
    if (feof(fd_temp)) exit(1);
    res = fscanf(fd_temp, "%[^\n]\n",buff);
    if (res == -1) exit(1);
    /*printf("Linia: %s\n",buff);*/
    if (!final_mac)
    {
      res=sscanf(buff,"\tPhysical Address. . . . . . . . . : %c%c-%c%c-%c%c-%c%c-%c%c-%c%c",
                 &mac[11],&mac[10],&mac[9],&mac[8],&mac[0],&mac[1],
                 &mac[2],&mac[3],&mac[4],&mac[5],&mac[6],&mac[7]);
      if (res==12) final_mac=1;
    }
    if (!final_ip)
    {
      res=sscanf(buff,"\tIP Address. . . . . . . . . . . . : %d.%d.%d.%d",
                 &ip[0],&ip[1],&ip[2],&ip[3]);
      if (res==4) final_ip=1;
    }

  }

  mac[8]='\0';
  *serialid = strtoul(mac,NULL,16);

  *hostid=(ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3]; 
  
  /* Es tanca i s'elimina el fitxer */
  fclose(fd_temp);
  unlink(nom_temporal);

/*  printf("El serial es: %X i el hostid es: %X\n", *serialid, *hostid); */
  
  /* S'allibera el buffer temporal */
  free(buff);
  return(0);
}        
#endif



#ifdef ARCH_HPUX
#define MAXHOSTNAMELEN 256

int 
find_machineHP () /* this is the IP number of the machine */
{
   struct hostent *host;
   char           *name;
   struct in_addr  ad;
   int a,b,c,d, hostid;

   name = (char *)malloc(MAXHOSTNAMELEN);
   gethostname (name, MAXHOSTNAMELEN);
   host = gethostbyname (name);
   free (name);
   bcopy (host->h_addr, (char *) &ad, host->h_length);
   name = malloc (strlen(inet_ntoa(ad))+1);
   bcopy ((char *)host->h_addr, (char *) &ad, host->h_length);
   strcpy (name, inet_ntoa(ad));
   sscanf(name, "%d.%d.%d.%d",&a,&b,&c,&d);

   hostid = a;
   hostid = (hostid<<8)+b;
   hostid = (hostid<<8)+c;
   hostid = (hostid<<8)+d;
   return (hostid);
}
#endif

static void
get_system_info(unsigned int *hostid_system, unsigned int *sysinfo_system)
{
  
#ifdef ARCH_HPUX
  struct utsname name;
#endif
  unsigned int serialid,hostid;
  static char buf[BUFSIZE]; 

#ifdef ARCH_HPUX
  hostid = find_machineHP();
  uname(&name);
  serialid = strtoul(name.idnumber ,(char **)0, 10);
#endif

#ifdef ARCH_LINUX
  find_machineLinux(&hostid,&serialid);
#endif

#ifdef ARCH_CYGWIN
  find_machineWindows(&hostid,&serialid);
#endif

#ifdef ARCH_SGI
  (void) sysinfo(_MIPS_SI_HOSTID, buf, BUFSIZE);
  hostid = strtoul(buf, (char **)0, 16);
  
  (void) sysinfo(SI_HW_SERIAL, buf, BUFSIZE);
  serialid = strtoul(buf, (char **)0, 10);
#endif

#ifdef ARCH_AIX
  hostid   = gethostid();
  serialid = get_hardware_serial_number_IBM();
#endif

#ifdef ARCH_ALPHA
  hostid   = (unsigned int) gethostid();
  serialid = (unsigned int) get_hardware_serial_number_ALPHA();
#endif

  *hostid_system = hostid;
  *sysinfo_system = serialid;
}


/* Checks if the master server is running where it should be according to the 
 * license */
/* hostid_file and serialid_file are the values specified in the license file */
static int
CheckMasterServer (unsigned int hostid_file,
                   unsigned int serialid_file,
                   int version_file)
{
  unsigned int serialid,hostid;
  
  get_system_info (&hostid, &serialid);
  
  if ((hostid!=hostid_file) || 
      (serialid!=serialid_file) ||
      (version_file!=VERSION*10+SUBVERSION))
  {
    return 0;
  }
  else
    return 1;
}

char*
check_license ()
{
  unsigned int hostid_system, sysinfo_system;
  unsigned int dat;
  int machine_type;
  unsigned int version_file, machine_file;
  unsigned int hostid_file, serialid_file;
  unsigned int dat_file;
  char KEY[256], KEY_file[256];
  char string_tmp[32];
  int i, j, limit, index;
  int parjobs;
  int portmaster_file, portslave_file, portSuperManager_file;
  int tmp1;
#ifdef ARCH_HPUX
  struct utsname name;
#endif

  machine_type = -1;

#ifdef ARCH_SGI
  machine_type = MACH_SGI;
#endif

#ifdef ARCH_HPUX
  machine_type = MACH_HP;
#endif

#ifdef ARCH_LINUX
  machine_type = MACH_LINUX;
#endif

#ifdef ARCH_CYGWIN
  machine_type = MACH_WINDOWS;
#endif

#ifdef ARCH_AIX
  machine_type = MACH_IBM;
#endif

#ifdef ARCH_ALPHA
  machine_type = MACH_ALPHA;
#endif

  for (index=0; ;index++)
  {
    read_file (&version_file,
               &machine_file,
               &hostid_file,
               &serialid_file,
               &dat_file,
               KEY_file,
               index);
    if (is_license_error!=0)
      break;
  
  
    dat = get_date();
    dat = dat_file;
  
    sprintf (KEY,"");
    local_index = 0;
  
    srandomine (dat_file, serialid_file);
    limit = 10000*randomine ();
    for (i = 0; i < limit; i++)
      (void) randomine ();

    All[local_index++] = I1;
    All[local_index++] = I2;
   
    if (dat==0)
      srandomine (I1,hostid_file);
    else
      srandomine (I1,hostid_file);

    limit = 10000*randomine ();
    for (i = 0; i < limit; i++)
      (void) randomine ();

    All[local_index++] = I1;
    All[local_index++] = I2;
   
    if (dat==0)
      srandomine (I1, version_file);
    else
      srandomine (I1, version_file);

    limit = 10000*randomine ();
    for (i = 0; i < limit; i++)
      (void) randomine ();

    All[local_index++] = I1;
    All[local_index++] = I2;
   
    srandomine (I1, machine_file);
    limit = 10000*randomine ();
    for (i = 0; i < limit; i++)
      (void) randomine ();
    All[local_index++] = I1;
    All[local_index++] = I2;
   
    for (i = 0; i < local_index; i++)
    {
      Final[i] = 0;
      for (j=0;j<local_index; j++)
      {
        if ((Mask1>>4) < All[j])
          tmp1 = All[j] - Mask1>>4;
        else 
          tmp1 = All[j]*Coeff_Matrix[i][j];

        if ((Mask1 - tmp1) < Final[i])
          Final[i] = (Mask1 - Final[i]) + tmp1;
        else
          Final[i] += tmp1;
      }
    }
    sprintf (KEY, "");
    for (i = 0; i < local_index/2+1; i++)
    {
      sprintf (string_tmp,"%08x", Final[i]);
      strcat (KEY, string_tmp);
    }

    dat = get_date();
    if ((CheckMasterServer ( hostid_file, serialid_file, version_file)) &&
        (strcmp(KEY_file, KEY)==0) &&
        (machine_file==machine_type) &&
        (dat<=dat_file))
      return ((char *)0);
  }
  get_system_info(&hostid_system, &sysinfo_system);
  sprintf (license_error,
    "%s\n%s\nNeed license for Dimemas Version %d.%d, Hostid: %X, Sysinfo: %X\n",
    "Unable to find a valid license in $DIMEMAS_HOME/etc/license.dat", 
    "Contact the license service at dimemas@cepba.upc.es",
    VERSION, SUBVERSION, hostid_system, sysinfo_system);
/******************************** Aixo abans era aixi: **********************
  sprintf (license_error, "%s\n%s\nNeed license for Dimemas Version %d.%d, Hostid: %X, Sysinfo: %X %X %X\n",
           "Unable to find a valid license in $DIMEMAS_HOME/etc/license.dat",
           "Contact the license service at dimemas@cepba.upc.es",
           VERSION, SUBVERSION, hostid_system, sysinfo_system,
            hostid_file, serialid_file);
*****************************************************************************/
  return(license_error);
}
