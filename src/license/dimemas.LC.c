#ifdef PAL_LC
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#ifdef VT_LCNETID
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "dimemas_defs.h"
#include "pal_env.h"
#include "licensecheck.c"

#ifdef LC_TEST
int main(void)
#else
void LC_check(void)
#endif
{ 
  pm_int result;
  int lc_ok = 0;

  result = PM_LICENSECHECK;

  switch (result) {
  case PM_LC_ACCCHECK:
    printf("%s: could not access license file.\n", PAL_TOOLNAME);
    break;
  case PM_LC_NEGCHECK:
    printf("%s: could not find matching license.\n", PAL_TOOLNAME);
    break;
  case PM_LC_POSCHECK:
#ifdef LC_TEST
    printf("%s: valid license record found.\n", PAL_TOOLNAME);
#endif
    lc_ok = 1;			/* license check OK */
    break;
  default:
    printf("%s: invalid licensecheck result.\n", PAL_TOOLNAME);
    break;
  }

  if (!lc_ok) {
    char *product, *arch, *mclass, *hostid, *uid, *env;
#ifdef VT_LCNETID
    char *netid;
#endif

    product = pm_LCgetinfo(LCPRODUCT);
    arch = pm_LCgetinfo(LCARCH);
    mclass = pm_LCgetinfo(LCMCLASS);
    hostid = pm_LCgetinfo(LCHOSTID);
    uid = pm_LCgetinfo(LCUID);
#ifdef VT_LCNETID
    netid = pm_LCgetinfo(LCNETID);
#endif

    printf("%s: need license for product %s, arch %s,\n\t hostid %s, uid %s.\n",
	   PAL_TOOLNAME, product, arch, hostid, uid);
#ifdef VT_LCNETID
    if (netid != NULL)
      printf("%s: network address %s\n",PAL_TOOLNAME,netid);
    else
      printf("%s: no network address found\n",PAL_TOOLNAME);
#endif

    free(product); free(mclass); free(hostid); free(uid);
    
    if ((env = getenv(PAL_ROOT_VAR)) == NULL)
      printf("%s: environment var %s not set.\n", PAL_TOOLNAME, PAL_ROOT_VAR);
    else
      printf("%s: environment var %s set to\n\t %s.\n", PAL_TOOLNAME,
	     PAL_ROOT_VAR, env);
    if ((env = getenv(PAL_LICENSEFILE_VAR)) == NULL)
      printf("%s: environment var %s not set.\n", PAL_TOOLNAME,
	     PAL_LICENSEFILE_VAR);
    else
      printf("%s: environment var %s set to\n\t%s.\n", PAL_TOOLNAME,
	     PAL_LICENSEFILE_VAR, env);
    exit(-1);
  }

  return;
}
#else
void
LC_check(void)
{
}
#endif
