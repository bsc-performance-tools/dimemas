#ifndef RCS_ts_h
#define RCS_ts_h
#endif
struct Str
{
   char           *str;
   struct Str     *next_str;
};

struct t_entry
{
   int             entryid;
   char           *name;
   struct t_queue *types;
};

struct t_element
{
   struct t_array *i3;
   int             type;
};

struct t_array
{
   char           *i1;
   int             dimension;
};

struct t_arr
{
   struct t_queue *q;
   int             dim1,
                   dim2;
};

struct t_field
{
   int             tipo;
   union
   {
      int             dec;
      double           real;
      char           *string;
      struct t_arr    arr;
   }               value;
};
