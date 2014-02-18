#ifndef MODULES_MAP_H
#define MODULES_MAP_H

/*** CONTAINER FOR NEW MODULES ****/

#ifdef __cplusplus
extern "C" {
#endif

void create_modules_map(modules_map *mm);

void* find_module(modules_map      *mm,
                  unsigned long int type,
                  unsigned long int value);

void insert_module(modules_map      *mm,
                   unsigned long int type,
                   unsigned long int value,
                   void             *new_module);

t_count count_map(modules_map* mm);

void *head(modules_map* mm);

void *next(modules_map* mm);

#ifdef __cplusplus
}
#endif
#endif
