#include <define.h>
#include <types.h>

#include "eee_configuration.h"
#include "dimemas_io.h"

// Karthikeyan 3Level Network Code
//----------------------------------------------------------------------

t_boolean eee_enabled;         // 1 for enabled; 0 for disabled
double   *T_w;                 // EEE link wake time
double   *T_s;                 // EEE link sleep time
double   *T_pdt;               // EEE PowerDown Threshold

int       N_levels;           // Number of levels in the Network
double   *T_eee_lat;          // Latency at each level 1,2,3...
double   *T_eee_bw;           // Bandwidth at each network level 1,2,3...
double   *T_eee_hop_lat;
int       N_nodes;            // Node count
const char     *eee_config_file;    // EEE config File - contains specific format!
int       eee_frame_header_size;

//----------------------------------------------------------------------------
// Network state parameters go here

int              *switches_per_level;
struct switches **eee_switches;       // eee_switches[level][switch_id]

// End of Network state parameters
//----------------------------------------------------------------------------


void EEE_Init(void)
{
  if (!eee_enabled)
    return;

  printf("\n\n\n--------------------------------------------------------------\n");
  printf("DIMEMAS Extension with Multi-Stage Cluster Network\n");
  printf("------------------------------------------------------\n");
  printf("Below are some assumptions with regard to the network\n");
  printf("--->Simulator is currently only tested with 1 process/node\n");
  printf("------> and ONLY 3 network levels\n");
  printf("---> Simulator requires a seperate file - 3leee_network_config.cfg with \n");
  printf("------>input configuration details-EEE T_w,T_s,T_pdt,N_levels and BW,Lat\n");
  printf("Author: Karthikeyan P. Saravanan <karthikeyan.palavedu@bsc.es\n");
  printf("----------------------------------------------------------------\n\n\n");

if(EEE_DEBUG) {
    printf("Karthikeyan: Loading network configuration file\n");
    printf("File to Load: %s\n",eee_config_file);
}

   FILE *config_file;
   char buf[1000],variable[1000];
   float value;
   //config_file = fopen("3leee_network_config.cfg", "r");
   config_file = IO_fopen(eee_config_file, "r");
   if (config_file == NULL) die( "Unable to open EEE config file\n\n");

   fgets(buf, 1000, config_file);
   while (feof(config_file) == 0)
   {
        sscanf(buf,"%s%f",variable,&value);

        if(strcmp(variable, "N_levels")==0) {
            if(EEE_DEBUG) printf("Number of Network Levels: %d\n", (int)value);
            N_levels = (int)value;
            if(N_levels <= 0) { die("Incorrect EEE network value\n");
            } else {
                T_eee_lat       = (double*) malloc (N_levels * sizeof(double));
                T_eee_hop_lat   = (double*) malloc (N_levels * sizeof(double));
                T_eee_bw        = (double*) malloc (N_levels * sizeof(double));
                T_w             = (double*) malloc (N_levels * sizeof(double));
                T_s             = (double*) malloc (N_levels * sizeof(double));
                T_pdt           = (double*) malloc (N_levels * sizeof(double));
            }
            int k_i;
            // Getting Lat,Bw for diff levels of network
            for (k_i = 0; k_i < N_levels; k_i++) {
                // reading Latency
                fgets(buf,1000, config_file);
                sscanf(buf,"%s%f",variable,&value);
                if(strcmp(variable,"Latency")==0) {
                    if(EEE_DEBUG) printf("Latency of Level %d: %f\n",(k_i+1),value);
                    T_eee_lat[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE network configuration parameter,",
                    " Latency 1st!\n");
                }
                // reading HopLatency
                fgets(buf,1000,config_file);
                sscanf(buf, "%s%f",variable,&value);
                if(strcmp(variable,"HopLatency")==0) {
                    if(EEE_DEBUG) printf("HopLatency of Level %d: %f\n",(k_i+1),value);
                    T_eee_hop_lat[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE network configuration parameter,",
                    " HopLat after Lat\n");
                }
                // reading bandwidth
                fgets(buf,1000, config_file);
                sscanf(buf, "%s%f",variable,&value);
                if (strcmp(variable, "Bandwidth")==0) {
                    if(EEE_DEBUG) printf("Bandwidth of Level %d: %f\n",(k_i+1),value);
                    T_eee_bw[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE network configuration parameter,",
                    " BW after HopLat\n");
                }
                //reading T_w
                fgets(buf,1000,config_file);
                sscanf(buf, "%s%f",variable,&value);
                if (strcmp(variable, "T_w")==0) {
                    if(EEE_DEBUG) printf("T_w of Level %d: %f\n",(k_i+1),value);
                    T_w[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE nw config parameter - T_w");
                }
                //reading T_s
                fgets(buf,1000,config_file);
                sscanf(buf, "%s%f",variable,&value);
                if (strcmp(variable, "T_s")==0) {
                    if(EEE_DEBUG) printf("T_s of Level %d: %f\n",(k_i+1),value);
                    T_s[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE nw config parameter - T_s");
                }
                //reading T_pdt
                fgets(buf,1000,config_file);
                sscanf(buf, "%s%f",variable,&value);
                if (strcmp(variable, "T_pdt")==0) {
                    if(EEE_DEBUG) printf("T_pdt of Level %d: %f\n",(k_i+1),value);
                    T_pdt[k_i] = (double)value;
                } else {
                    panic ("Error: Incorrect EEE nw config parameter - T_pdt");
                }
            }

        } else {
            panic("ERROR::EEE Nw cfg parameter specified does not exist!!\n");
        }
        fgets(buf, 1000, config_file);
   }

   fclose(config_file);

if(EEE_DEBUG) printf("Karthikeyan: Config File Load completed - Parameters Assigned\n");
return;
}

void
configure_switches()
{
    if(EEE_DEBUG) printf("Karthikeyan: Configuring Switches\n");
    switches_per_level = (int*) malloc(N_levels * sizeof(int));

    //TODO: MAKE HARDCODED SWITCH CODE BELOW GENERIC
    // constant 2 switches in the top most level
    switches_per_level[2] = 2;
    // level 1 in this case will have 4*2 = 8 switches
    switches_per_level[1] = switches_per_level[2]*2;
    // level 0 switches will depend on the number of nodes
    // in my case, 64,128,256 nodes = 8,16,32 switches
    // reasoning is to have same stucture on higher levels

    switches_per_level[0] = N_nodes/switches_per_level[1];

    if(EEE_DEBUG) {
        printf("Number of nodes: %d\n",N_nodes);
        int eee_i;
        for (eee_i = 0; eee_i<N_levels; eee_i++)
        printf("Switches per Level: %d\n",switches_per_level[eee_i]);
    }

    eee_switches = (struct switches**) malloc(N_levels * sizeof(struct switches*));

    /* C1: Below code calculates the number of links that go into and out of each switch
            and allocates memory for the switches
    */
    //TODO Write Checks in the code when used in more general case
    //TODO Will the code work for NON-Base_2 numbers of nodes/switches etc?
    int eee_i,eee_j;
    // Looping tru switch levels
    for ( eee_i = 0; eee_i < N_levels; eee_i++) { // L1

        // Allocating memory for switches
        eee_switches[eee_i] = (struct switches*) malloc(switches_per_level[eee_i]
                                                        * sizeof(struct switches));

        // looping tru switches per level
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) { // L2

            eee_switches[eee_i][eee_j].switch_level = eee_i;
            eee_switches[eee_i][eee_j].switch_id = eee_j;

            // Level 1 switches
            if ( eee_i == 0) {
                eee_switches[eee_i][eee_j].N_in_links = N_nodes/switches_per_level[eee_i];
                eee_switches[eee_i][eee_j].N_out_links = 1;
            } else if ( eee_i == (N_levels-1) ) {
                eee_switches[eee_i][eee_j].N_in_links = switches_per_level[eee_i-1];
                eee_switches[eee_i][eee_j].N_out_links = 0; // No UP LINK for last level

            } else {
                eee_switches[eee_i][eee_j].N_in_links = switches_per_level[eee_i-1]
                                                            /switches_per_level[eee_i];
                eee_switches[eee_i][eee_j].N_out_links = 1;
                // fat tree system at the highest level
                if ( (eee_i+1) ==  N_levels-1) {
                    eee_switches[eee_i][eee_j].N_out_links = switches_per_level[eee_i+1];
                    // 2nd last level switches get 1 input link from
                    // last level besides their lower level
                    // hence the above code accounts for the extra 1 link
                }
            }
        }// L2
    }// L1
    /* C1 End
    */

    /* C2: Below code allocates memory for in and out links
    */
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
            eee_switches[eee_i][eee_j].in_links
                    = (struct eee_link*) malloc ( eee_switches[eee_i][eee_j].N_in_links
                                                  * sizeof (struct eee_link));

            eee_switches[eee_i][eee_j].out_links
                    = (struct eee_link*) malloc ( eee_switches[eee_i][eee_j].N_out_links
                                                  * sizeof (struct eee_link));
            if (EEE_DEBUG){
            printf("Linking Info [Level:%d][Switch:%d]: Number of in,out_links:\t%d,%d\n"
                                    ,eee_i
                                    ,eee_j
                                    ,eee_switches[eee_i][eee_j].N_in_links
                                    ,eee_switches[eee_i][eee_j].N_out_links);
            }

        }
    }


    /* End of C2 code
    */


    /* C3
    Linking links
    */
    // Configuring INPUT links
    // The below logic basically is to "TAG" partner OUT links to In links of switches

    /*
            THE BELOW IS THE LOGIC FOR LINK CONFIGURATION
            BASICALLY THE WIRING INSIDE A NETWORK SORT OF!

            THE LOGIC BELOW CONTAINS 3 SECTIONS:
              - EDGE LINKS (eee_i == 0) // base level switches and their links
              - AGG LINKS (eee_i != 0,(N_levels-1) // switches between the last and 1st level
              - FAT LINK (eee_i == (N_levels-1) // Last level switch

              --- The last level switch is a fat switch which contains
              --- a copy of input links from all links from previous level
              --- i.e all last level switches are all-all connnected with all
              --- switches in (last level - 1) switch

            The below code tries to link switches in the above explained manner
            TURN ON EEE_DEBUG for DEBUG INFO
    */

    if (EEE_DEBUG) printf("Configuring INPUT LINKS\n");
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
            int eee_partner_j; eee_partner_j = 0;
            for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
                int eee_k;
                for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_in_links ; eee_k++) {
                    // INIT_EEE_VAR -- REMEMBER TO INIT OUT LINKS AS WELL!!!!!!!!!!!!!!
                    eee_switches[eee_i][eee_j].in_links[eee_k].last_inlink_off_time=-1;
                    eee_switches[eee_i][eee_j].in_links[eee_k].last_outlink_off_time=-1;
                    eee_switches[eee_i][eee_j].in_links[eee_k].inlink_next_free_time=0;
                    eee_switches[eee_i][eee_j].in_links[eee_k].outlink_next_free_time=0;
                    eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_inlink_on_time=0;
                    eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_outlink_on_time=0;
                    eee_switches[eee_i][eee_j].in_links[eee_k].link_id = eee_k;
                    eee_switches[eee_i][eee_j].in_links[eee_k].switch_id = eee_j;
                    eee_switches[eee_i][eee_j].in_links[eee_k].level = eee_i;

                    if (eee_i == 0) {
                        eee_switches[eee_i][eee_j].in_links[eee_k].partner_link = NULL;
                        if(EEE_DEBUG){
                        printf("eee_switches[%d][%d].in_links[%d].partner_link = NULL\n"
                                    ,eee_i,eee_j,eee_k);
                        }
                    } else if (eee_i == (N_levels -1)) {
                        eee_switches[eee_i][eee_j].in_links[eee_k].partner_link =
                            &(eee_switches[(eee_i-1)][eee_partner_j
                                    %switches_per_level[eee_i-1]]
                                    .out_links[eee_partner_j/switches_per_level[eee_i-1]]);
                        if(EEE_DEBUG) {
                            printf("eee_switches[%d][%d].in_links[%d].partner_link"
                                    "= &(eee_switches[%d][%d].out_links[%d]);\n"
                                    ,eee_i,eee_j,eee_k,
                                    (eee_i-1),
                                    (eee_partner_j%switches_per_level[eee_i-1]),
                                    (eee_partner_j/switches_per_level[eee_i-1]));
                        }

                    } else { // -- 2nd level switch conf
                    eee_switches[eee_i][eee_j].in_links[eee_k].partner_link =
                                &(eee_switches[(eee_i-1)][eee_partner_j].out_links[0]);
                        if (EEE_DEBUG) {
                            printf("eee_switches[%d][%d].in_links[%d].partner_link"
                                    "= &(eee_switches[%d][%d].out_links[%d]);\n"
                                    ,eee_i,eee_j,eee_k,
                                    (eee_i-1),
                                    eee_partner_j,
                                    0);
                        }
                    }
                    eee_partner_j++;
                }
            }
    }
    if (EEE_DEBUG) printf("INPUT LINKS configuration Over\n");

    // Configuring OUTPUT links
    // The below logic is to "TAG" the partner INs with OUT links of switches

    if (EEE_DEBUG) printf("Configuring OUTPUT LINKS\n");
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
            int eee_partner_j; eee_partner_j = 0;

            for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
                int eee_k;
                for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_out_links ; eee_k++) {
                    //INIT_EEE_VAR -- REMEMBER TO INIT IN LINKS AS WELL!!!!!!!!!!!!!!!!!
                    eee_switches[eee_i][eee_j].out_links[eee_k].last_inlink_off_time=-1;
                    eee_switches[eee_i][eee_j].out_links[eee_k].last_outlink_off_time=-1;
                    eee_switches[eee_i][eee_j].out_links[eee_k].inlink_next_free_time=0;
                    eee_switches[eee_i][eee_j].out_links[eee_k].outlink_next_free_time=0;
                    eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_inlink_on_time=0;
                    eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_outlink_on_time=0;
                    eee_switches[eee_i][eee_j].out_links[eee_k].link_id = eee_k;
                    eee_switches[eee_i][eee_j].out_links[eee_k].switch_id = eee_j;
                    eee_switches[eee_i][eee_j].out_links[eee_k].level = eee_i;

                    if ( eee_i == (N_levels-1)) { // Last level

                        printf("Error:: CANNOT BE HERE\n");
                        exit(EXIT_FAILURE);

                    } else if (eee_i != (N_levels-2)) { // all expect 2nd last level
                            if (EEE_DEBUG) {
                                printf("eee_switches[%d][%d].out_links[%d].partner_link"
                                    "= &(eee_switches[%d][%d].in_links[%d]);\n"
                                    ,eee_i,eee_j,eee_k,
                                    (eee_i+1),
                                    eee_partner_j/(eee_switches[(eee_i+1)][0].N_in_links),
                                    eee_partner_j%(eee_switches[(eee_i+1)][0].N_in_links));
                            }

                              eee_switches[eee_i][eee_j].out_links[eee_k].partner_link =
                                &(eee_switches[(eee_i+1)]
                                    [eee_partner_j/(eee_switches[(eee_i+1)][0].N_in_links)]
                                    .in_links[eee_partner_j
                                    %(eee_switches[(eee_i+1)][0].N_in_links)]);
                    } else { // 2nd last level -- contains 2 out links to last level
                            if (EEE_DEBUG) {
                                printf("eee_switches[%d][%d].out_links[%d].partner_link"
                                "= &(eee_switches[%d][%d].in_links[%d]);\n"
                                ,eee_i,eee_j,eee_k,
                                (eee_i+1),
                                eee_partner_j%eee_switches[eee_i][0].N_out_links,
                                eee_partner_j/eee_switches[(eee_i)][0].N_out_links
                                );
                            }

                            eee_switches[eee_i][eee_j].out_links[eee_k].partner_link =
                            &(eee_switches[(eee_i+1)]
                                [eee_partner_j%eee_switches[eee_i][0].N_out_links]
                                    .in_links[eee_partner_j/eee_switches[(eee_i)][0].N_out_links]);

                    }

                    eee_partner_j++;
                }
            }
    }
    /*      THE BELOW CODE MAKES SURE THAT THE PARTNER_NODE INSIDE EACH LINK INSIDE
            A SWITCH IS PROPERLY CONNECTED WITH ITS CORRESPONDING PARTNER LINK
            --- ALL LINKS/NICs IN EEE HAVE TO BE IN SYNC WITH THEIR PARTNER LINKS
            --- THIS IS TO MAKE SURE THEIR STATES ARE ALIGNED
    */

//    printf("Exit Code!!!\n\n\n"); exit(EXIT_FAILURE);

    if(EEE_DEBUG) printf("Testing Configuration\n");
    int eee_config_test;
    eee_config_test = 0;
    {
    int eee_i,eee_j,eee_k;
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
            for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_out_links ; eee_k++) {
                if(EEE_DEBUG) {
                printf("--%d--",eee_switches[eee_i][eee_j].N_out_links);
                printf("OUT:linkparams:Level,Switch,Link:%d,%d,%d-PP:%d %d %d->>%d,%d,%d\n",
                eee_switches[eee_i][eee_j].out_links[eee_k].level,
                eee_switches[eee_i][eee_j].out_links[eee_k].switch_id,
                eee_switches[eee_i][eee_j].out_links[eee_k].link_id,
                eee_switches[eee_i][eee_j].out_links[eee_k].partner_link->level,
                eee_switches[eee_i][eee_j].out_links[eee_k].partner_link->switch_id,
                eee_switches[eee_i][eee_j].out_links[eee_k].partner_link->link_id,
                eee_switches[eee_i][eee_j].out_links[eee_k]
                                            .partner_link->partner_link->level,
                eee_switches[eee_i][eee_j].out_links[eee_k]
                                            .partner_link->partner_link->switch_id,
                eee_switches[eee_i][eee_j].out_links[eee_k]
                                            .partner_link->partner_link->link_id
                ); }
                    if(&(eee_switches[eee_i][eee_j].out_links[eee_k])
                        == eee_switches[eee_i][eee_j].out_links[eee_k]
                                            .partner_link->partner_link) {
                        eee_config_test = 1;
                    } else {
                        printf("Test Unsuccessful!!!!!!!\n"); exit(EXIT_FAILURE);
                    }
            }
        }
    }

    if(EEE_DEBUG) printf("Link Configuration Test for Out Links Passed\n");

    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
            for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_in_links ; eee_k++) {
                if ( (eee_i == 0) && (eee_switches[eee_i][eee_j].in_links[eee_k]
                                                                .partner_link == NULL)) {
                    if(EEE_DEBUG){
                    printf("--%d--IN:linkparams:Level,Switch,Link:%d,%d,%d::NO PARTNER\n",
                    eee_switches[eee_i][eee_j].N_in_links,
                    eee_switches[eee_i][eee_j].in_links[eee_k].level,
                    eee_switches[eee_i][eee_j].in_links[eee_k].switch_id,
                    eee_switches[eee_i][eee_j].in_links[eee_k].link_id);
                    }
                } else {
                if(EEE_DEBUG){
                printf("--%d--",eee_switches[eee_i][eee_j].N_in_links);
                printf("IN:linkparams:Level,Switch,Link:%d,%d,%d-PP:%d %d %d->>%d,%d,%d\n",
                eee_switches[eee_i][eee_j].in_links[eee_k].level,
                eee_switches[eee_i][eee_j].in_links[eee_k].switch_id,
                eee_switches[eee_i][eee_j].in_links[eee_k].link_id,
                eee_switches[eee_i][eee_j].in_links[eee_k].partner_link->level,
                eee_switches[eee_i][eee_j].in_links[eee_k].partner_link->switch_id,
                eee_switches[eee_i][eee_j].in_links[eee_k].partner_link->link_id,
                eee_switches[eee_i][eee_j].in_links[eee_k].partner_link->partner_link->level,
                eee_switches[eee_i][eee_j].in_links[eee_k]
                                            .partner_link->partner_link->switch_id,
                eee_switches[eee_i][eee_j].in_links[eee_k]
                                            .partner_link->partner_link->link_id
                );
                }
                if(&(eee_switches[eee_i][eee_j].in_links[eee_k])
                    == eee_switches[eee_i][eee_j].in_links[eee_k]
                                        .partner_link->partner_link) {
                    eee_config_test = 1;
                } else {
                    printf("Test Unsuccessful!!!!!!!\n"); exit(EXIT_FAILURE);
                }
                }
            }
        }
    }
    }

    if (eee_config_test){ if(EEE_DEBUG) printf("LINK CONFIGURATION TEST Passed\n");}
    else {printf("Test Unsuccessful!\n"); exit(EXIT_FAILURE);}

    if (EEE_DEBUG) printf("OUTPUT LINKS configuration Over\n");

    /* End of C3 Code */

if(EEE_DEBUG) printf("Karthikeyan Switch Configuration Over\n");
return;

}

void
eee_process_stats()
{
    printf("\nPRINTING EEE STATS\n");
    int eee_i, eee_j, eee_k;
    printf("\nINLINKS\n");
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
        double avg_in,avg_out,sum_in,sum_out;
        int count;
        avg_in = 0;
        sum_in = 0;
        avg_out = 0;
        sum_out = 0;
        count = 0;
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
            for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_in_links ; eee_k++) {
/*                printf("L:%d::Sw:%d:Link:%d:::out_in:%f::out_out:%f\n"
                    ,eee_i
                    ,eee_j
                    ,eee_k
                    ,eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_inlink_on_time/1e6
                    ,eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_outlink_on_time/1e6
                    );
*/                count++;
                sum_in+=eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_inlink_on_time;
                sum_out+=eee_switches[eee_i][eee_j].in_links[eee_k].stats_total_outlink_on_time;
            }
        }
        avg_in = sum_in/count;
        avg_out = sum_out/count;
        printf("Level:%d\nIN_IN time %.6f\nLevel:%d\nIN_OUT time %.6f\n",eee_i,(avg_in/1e6),eee_i,(avg_out/1e6));
    }

    printf("\nOUTLINKS\n");
    for ( eee_i = 0; eee_i < N_levels; eee_i++) {
        double avg_in,avg_out,sum_in,sum_out;
        int count;
        avg_in = 0;
        sum_in = 0;
        avg_out = 0;
        sum_out = 0;
        count = 0;
        for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
            for ( eee_k = 0; eee_k < eee_switches[eee_i][eee_j].N_out_links ; eee_k++) {
/*                printf("L:%d::Sw:%d:Link:%d:::out_in:%f::out_out:%f\n"
                    ,eee_i
                    ,eee_j
                    ,eee_k
                    ,eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_inlink_on_time/1e6
                    ,eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_outlink_on_time/1e6
                    );
*/                count++;
                sum_in+=eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_inlink_on_time;
                sum_out+=eee_switches[eee_i][eee_j].out_links[eee_k].stats_total_outlink_on_time;
            }
        }
        avg_in = sum_in/count;
        avg_out = sum_out/count;
        printf("Level:%d\nOUT_IN time %.6f\nLevel:%d\nOUT_OUT time %.6f\n",eee_i,(avg_in/1e6),eee_i,(avg_out/1e6));
    }

}


void
eee_memory_dealloc()
{

   free(T_eee_lat);
   free(T_eee_bw);
   free(T_w);
   free(T_s);
   free(T_pdt);
   int eee_i,eee_j;
   for ( eee_i = 0; eee_i < N_levels; eee_i++) {
       for ( eee_j = 0; eee_j < switches_per_level[eee_i]; eee_j++) {
           free(eee_switches[eee_i][eee_j].in_links);
           free(eee_switches[eee_i][eee_j].out_links);
       }
   }
   for ( eee_i = 0; eee_i < N_levels; eee_i++) {
       free(eee_switches[eee_i]);
   }
   free(eee_switches);
   free(switches_per_level);

}

//-----------------------------------------------------------------------------------------------------
//---------------------END OF KARTHIKEYAN EEE CODE-----------------------------------------------------
