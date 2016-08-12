#ifndef _DIMEMAS_2_PRV
#define _DIMEMAS_2_PRV 

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef OLD_PRV_FORMAT

/* S'utilitza el format vell dels .prv que generava el dimemas, es a dir,
   amb events de tipus 40 i el valor igual que l'id del block del .trf. */
#define BLOCK_TRF2PRV(ttrf, tprv, vprv) \
  { \
    tprv=40; \
    vprv=ttrf; \
  }
  
/* MACRO desglossada en obtencio unicament del tipus o del valor */
#define BLOCK_TRF2PRV_TYPE(ttrf) 40

#define BLOCK_TRF2PRV_VALUE(ttrf) ttrf

/* MACROS per obtenir facilment dades d'un block */
#define PRV_BLOCK_TYPE(i) 40
#define PRV_BLOCK_LABEL(i) "Block Event"
#define PRV_BLOCK_COLOR(i) 9
    
#else /* !OLD_PRV_FORMAT */

/*************************************************************************
 * S'utilitza el format nou dels .prv, que genera diferents tipus i estan
 * definits en un fitxer de constants.
 *************************************************************************/

/* D'afegeix el fitxer on hi ha totes les constants */
#include "EventEncoding.h"

#define MPITYPE_FLAG_COLOR 9

#define NUM_BLOCK_GROUPS   11 /* Dels 14, de moment nomes 11 son diferents */


struct t_block_dimemas2prv
{
  int tipus_prv;
  int valor_prv;
};

struct t_prv_type_info
{
  int  type;
  char *label;
  int  flag_color;
};

/* Els id minim i maxim dels blocks predefinits que apareixen a
   les traces .trf */
#define MIN_TIPUS_MPI_TRF 1
#define MAX_TIPUS_MPI_TRF 155
#define NUM_MPI_ELEMENTS (MAX_TIPUS_MPI_TRF-MIN_TIPUS_MPI_TRF+1)


extern struct  t_block_dimemas2prv block_dimemas2prv[];
extern struct  t_prv_type_info     prv_block_groups[];


/* MACRO per accedir fàcilment a la taula de conversió de tipus/valors */
void Block_Dimemas2Paraver_TypeAndValue(long long  DimemasBlockId,
                                        long long* ParaverType,
                                        long long* ParaverValue);
#define BLOCK_TRF2PRV(ttrf, tprv, vprv) \
  { \
    if ((ttrf >= MIN_TIPUS_MPI_TRF) && (ttrf <= MAX_TIPUS_MPI_TRF)) \
    { \
      tprv = block_dimemas2prv[ttrf-MIN_TIPUS_MPI_TRF].tipus_prv; \
      vprv = block_dimemas2prv[ttrf-MIN_TIPUS_MPI_TRF].valor_prv; \
    } \
    else if (ttrf < BASE_CLUSTER_BLOCK) \
    { \
      tprv = USER_FUNCTION; \
      vprv = ttrf-BASE_USERFUNCTION; \
    } \
    else if (ttrf < BASE_USERCALL) \
    { \
      tprv = CLUSTER_ID_EV; \
      vprv = ttrf - BASE_CLUSTER_BLOCK; \
    } \
    else if (ttrf < BASE_USERBLOCK) \
    { \
      tprv = USER_CALL; \
      vprv = ttrf-BASE_USERCALL; \
    } \
     { \
      tprv = USER_BLOCK; \
      vprv = ttrf - BASE_CLUSTER_BLOCK; \
    } \
  }

/* MACRO desglossada en obtencio unicamaent del tipus o del valor */

long long Block_Dimemas2Paraver_Type(long long DimemasBlockId);

/* Old way to do it, via MACRO */
#define BLOCK_TRF2PRV_TYPE(ttrf) \
  ( \
    ((ttrf >= MIN_TIPUS_MPI_TRF) && (ttrf<=MAX_TIPUS_MPI_TRF)) ? \
      block_dimemas2prv[ttrf-MIN_TIPUS_MPI_TRF].tipus_prv : \
        ((ttrf < BASE_USERCALL) ? USER_FUNCTION : \
          ((ttrf<BASE_USERBLOCK) ? USER_CALL : USER_BLOCK)) \
  )

long long Block_Dimemas2Paraver_Value(long long DimemasBlockId);

/* Old way to do it, via MACRO */
#define BLOCK_TRF2PRV_VALUE(ttrf) \
  ( \
    ((ttrf>=MIN_TIPUS_MPI_TRF) && (ttrf<=MAX_TIPUS_MPI_TRF)) ? \
      block_dimemas2prv[ttrf-MIN_TIPUS_MPI_TRF].valor_prv : \
        ((ttrf<BASE_USERCALL) ? ttrf-BASE_USERFUNCTION : \
          ((ttrf<BASE_USERBLOCK) ? ttrf-BASE_USERCALL : \
            ttrf-BASE_USERBLOCK)) \
  )


/* MACRO per obtenir facilment un tipus de block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_TYPE(i)  prv_block_groups[i].type
/* MACRO per obtenir facilment una etiqueta de block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_LABEL(i) prv_block_groups[i].label
/* MACRO per obtenir facilment el color d'un block (i=[0..NUM_BLOCK_GROUPS-1])*/
#define PRV_BLOCK_COLOR(i) prv_block_groups[i].flag_color


#endif /* OLD_PRV_FORMAT */

#ifdef __cplusplus
}
#endif

#endif /* _DIMEMAS_2_PRV */
