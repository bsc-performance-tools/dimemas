#define EEE_DEBUG 0
#define EEE_UP    1
#define EEE_DOWN  -1

extern int eee_enabled; // 1 for enabled; 0 for disabled
extern double *T_w;     // EEE link wake time
extern double *T_s;     // EEE link sleep time
extern double *T_pdt;   // EEE PowerDown Threshold

extern int N_levels;      // Number of levels in the Network
extern double *T_eee_lat; // Latency at each level 1,2,3...
extern double *T_eee_bw;  // Bandwidth at each network level 1,2,3...
extern double *T_eee_hop_lat;
extern int N_nodes;                 // Node count
extern const char *eee_config_file; // EEE config File - contains specific format!
extern int eee_frame_header_size;

//----------------------------------------------------------------------------
// Network state parameters go here

extern int *switches_per_level;
extern struct switches **eee_switches; // eee_switches[level][switch_id]

extern void EEE_Init();
