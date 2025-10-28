#ifndef __read_h
#define __read_h

#include <pthread.h>

/*
 * MACROS PARA LA FUNCION 'show_individual_statistics_pallas'
 */

#define SP_TOTAL_INDEX 0
#define SP_AVG_INDEX   1
#define SP_MAX_INDEX   2
#define SP_MIN_INDEX   3
#define SP_STDEV_INDEX 4

/* Macro que inicialitza totes les estadistiques totals */
#define SP_INIT_ESTADISTIQUES()                                         \
  {                                                                     \
    int i;                                                              \
                                                                        \
    bzero( totals, sizeof( double ) * NUM_COLS_EST * NUM_ESTADISTICS ); \
    for ( i = 0; i < NUM_COLS_EST; i++ )                                \
    {                                                                   \
      totals[ i ][ SP_MIN_INDEX ] = DBL_MAX;                            \
      totals[ i ][ SP_MAX_INDEX ] = -DBL_MAX;                           \
    }                                                                   \
  }

/* Macro que calcula totes les estadistiques d'una columna */
#define SP_ESTADISTIQUES( index, valor )                                        \
  {                                                                             \
    totals[ index ][ SP_TOTAL_INDEX ] += ( valor );                             \
    totals[ index ][ SP_AVG_INDEX ]++; /* De moment conta el numero de files */ \
    if ( ( valor ) < totals[ index ][ SP_MIN_INDEX ] )                          \
      totals[ index ][ SP_MIN_INDEX ] = ( valor );                              \
    if ( ( valor ) > totals[ index ][ SP_MAX_INDEX ] )                          \
      totals[ index ][ SP_MAX_INDEX ] = ( valor );                              \
    /* Acumulem els valors al quadrat */                                        \
    totals[ index ][ SP_STDEV_INDEX ] += ( ( valor ) * ( valor ) );             \
  }

/* Macro que fa els ultims tractaments a totes les estadistiques totals */
#define SP_FINI_ESTADISTIQUES()                                                \
  {                                                                            \
    int i;                                                                     \
    double num_files, aux;                                                     \
                                                                               \
    for ( i = 0; i < NUM_COLS_EST; i++ )                                       \
    {                                                                          \
      num_files = totals[ i ][ SP_AVG_INDEX ];                                 \
      /* Es calcula la mitjana */                                              \
      totals[ i ][ SP_AVG_INDEX ] = totals[ i ][ SP_TOTAL_INDEX ] / num_files; \
      /* Es calcula la variança */                                            \
      totals[ i ][ SP_STDEV_INDEX ] /= num_files;                              \
      aux = ( totals[ i ][ SP_AVG_INDEX ] * totals[ i ][ SP_AVG_INDEX ] );     \
      totals[ i ][ SP_STDEV_INDEX ] -= aux;                                    \
      /* Si el valor es -0 es passa a positiu */                               \
      if ( totals[ i ][ SP_STDEV_INDEX ] == 0.0 )                              \
        totals[ i ][ SP_STDEV_INDEX ] = 0.0;                                   \
      totals[ i ][ SP_STDEV_INDEX ] = sqrt( totals[ i ][ SP_STDEV_INDEX ] );   \
    }                                                                          \
  }


void show_statistics();

void calculate_execution_end_time();

void reload_new_Ptask( struct t_Ptask *Ptask );

void READ_get_next_action( struct t_thread *thread );

void READ_create_action( struct t_action **action );

void READ_copy_action( struct t_action *src_action, struct t_action *dst_action );

void READ_free_action( struct t_action *action );

void READ_Init_asynch( struct t_Ptask *ptask, int max_memory, int threads_count );
#endif
