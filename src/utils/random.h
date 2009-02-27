#ifndef __random_h
#define __random_h

struct t_randomness {
  struct t_rand_type processor_ratio;
  struct t_rand_type network_bandwidth;
  struct t_rand_type network_latency;
  struct t_rand_type memory_bandwidth;
  struct t_rand_type memory_latency;
  struct t_rand_type external_network_bandwidth;
  struct t_rand_type external_network_latency;
};

extern struct t_randomness randomness;

extern void RANDOM_init(char *fichero_random);
extern double random_dist(struct t_rand_type *);

#endif
