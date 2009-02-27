/*
 *This file contains the arrays for description and checking of
 *the records in the configuration file and trace file
 */

typedef enum {
  CONFIGURATION,
  TRACEFILE
} t_configuration_or_tracefile;

typedef enum {
  TYPES_INTEGER,
  TYPES_DOUBLE,
  TYPES_CHAR
} t_variable_types;
                
struct t_record_elements{
  char *name;
  t_variable_types variable_type;
  int dimension;
};        

struct t_record{
  t_configuration_or_tracefile configuration_or_tracefile;
  int   number;
  char *name;
  int   num_elements;
  struct t_queue Elements;
};
