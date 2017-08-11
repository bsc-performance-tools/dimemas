#include <math.h>

#include <types.h>
#include <define.h>
#include <extern.h>
#include <cpu.h>
#include <node.h>
#include <task.h>

#include "eee_configuration.h"

/*****************************************************************************
* Karthikeyan: EEE: Code starts here
*****************************************************************************/

t_nano eee_network(struct t_thread *thread);

t_nano link_transmit(struct t_thread *thread);

t_nano nw_switch(struct t_thread *thread);

t_nano eee_calc_nw_bw_lat(struct t_thread *thread, int mess_size);

/* This function calculates transmission delay based on
    specified bandwidth and latency
    Note: Bandwidth is specified in Mbits/second
    Node: Latency is specified in micro-seconds
*/
t_nano eee_calc_nw_bw_lat(struct t_thread *thread, int mess_size)
{

    t_nano eee_nw_delay;
    eee_nw_delay = 0;

    struct t_action         *action;
    struct t_send           *mess;
    struct t_task           *task, *task_partner;
    struct t_node           *node_s, *node_r;

    action = thread->action;
    mess = &(action->desc.send);
    task = thread->task;
    task_partner = locate_task (task->Ptask, mess->dest);
    node_s = get_node_of_task (task);
    node_r = get_node_of_task (task_partner);

    int node_s_id, node_r_id;
    node_s_id = (node_s->nodeid - 1);
    node_r_id = (node_r->nodeid - 1);

    if((node_s_id < 0) || (node_r_id < 0))
    {
        printf("EEE ERROR::::NOT POSSIBLE for Node ID to be Negative\n");
        exit(EXIT_FAILURE);
    }

    double eee_bandwidth, eee_latency;
    int bw_lat_level, bw_infinite;


    if (thread->routing_dir == EEE_UP)
    {
        bw_lat_level = thread->current_level + 1;
        eee_bandwidth = T_eee_bw[bw_lat_level] * (1024 * 1024); // Converting to Bytes/second
        eee_bandwidth = eee_bandwidth / 1000000000; // Converting to Bytes/microsecond
        eee_latency = T_eee_lat[bw_lat_level];
        if (T_eee_bw[bw_lat_level] == 0) bw_infinite = 1;
        else bw_infinite = 0;
        if(!bw_infinite)
        {
            eee_nw_delay = eee_latency + mess_size / eee_bandwidth;
        } else eee_nw_delay = eee_latency ; // Infinite Bandwidth

    } else if (thread->routing_dir == EEE_DOWN) {

        bw_lat_level = thread->current_level;
        eee_bandwidth = T_eee_bw[bw_lat_level] * (1024*1024); // Converting to Bytes/second
        eee_bandwidth = eee_bandwidth / 1000000; // Converting to Bytes/microsecond
        eee_latency = T_eee_lat[bw_lat_level];
        if (T_eee_bw[bw_lat_level] == 0) bw_infinite = 1;
        else bw_infinite = 0;
        if(!bw_infinite) {
            eee_nw_delay = eee_latency + mess_size / eee_bandwidth;
        } else eee_nw_delay = eee_latency ; // Infinite Bandwidth

    } else {
        printf("EEE_ERROR::eee_calc_nw_bw_lat::Cant be here\n");
        exit(EXIT_FAILURE);
    }

    if(EEE_DEBUG) {
        PRINT_TIMER(current_time);
        printf("Returning Bw+Lat:%f::Transmission between %d->%d::MesSize:%d::Level:%d\n"
                                        ,eee_nw_delay,node_s_id,node_r_id
                                        ,mess_size,bw_lat_level);
    }

    return(eee_nw_delay);
}


// Function transmites the message tru a link
// Uses B/W and Latency
t_nano link_transmit(struct t_thread *thread) // Step 1
{

    t_nano eee_nw_delay, msg_payload_delay;
    eee_nw_delay = 0;
    msg_payload_delay = 0;

    t_boolean transmit_possible = FALSE;
    int org_current_level = thread->current_level;

    struct t_action     *action;
    struct t_send       *mess;
    struct t_task       *task, *task_partner;
    struct t_node       *node_s, *node_r;

    action = thread->action;
    mess = &(action->desc.send);
    task = thread->task;
    task_partner = locate_task (task->Ptask, mess->dest);
    node_s = get_node_of_task (task);
    node_r = get_node_of_task (task_partner);

    int node_s_id, node_r_id;
    node_s_id = (node_s->nodeid - 1); // from (0 - node_id-1)
    node_r_id = (node_r->nodeid - 1);
    if((node_s_id < 0) || (node_r_id < 0)) {
        printf("EEE ERROR::::NOT POSSIBLE for Node ID to be Negative\n");
        exit(EXIT_FAILURE);
    }

    int mess_size, fh_size; //Bytes
    mess_size = mess->mess_size;
    fh_size = eee_frame_header_size; // Global variable

    //Test Code to remove Virtual Cut-Tru
    // Below code combined with infinite upper levels can be used
    // to convert the multi level network to single level
    if (0) {
        printf("Warning! Sim Virtual Cut Tru Switched OFF!\n");
        mess_size = 0;
        fh_size = mess->mess_size;
    }

    if (!eee_enabled) {
        printf("Cannot be inside EEE code! EEE not enabled\n");
        exit(EXIT_FAILURE);
    }
    if (eee_frame_header_size < 0) {
        printf("eee_frame_header_size cannot be negative!\n");
        exit(EXIT_FAILURE);
    }
    if(EEE_DEBUG) {
        PRINT_TIMER(current_time);
        printf("At link_transmit()::Transmission between %d->%d\n",node_s_id,node_r_id);
    }

    //Finding TRANSMISSION LINK if curr_level == -1
    // if curr_level is -1 then it means this is the 1st transmission
    // so finding which switch and which link in the 0th level switch

    if(thread->current_level == -1) { //First transmission Has to be EEE_UP

        int no_of_links;
        no_of_links = N_nodes/switches_per_level[0];
        thread->eee_switchid = node_s_id/no_of_links;
        thread->eee_linkid = node_s_id%no_of_links;
        if ( EEE_DEBUG ) {
            printf("\nSTARTING EEE - FIRST TRANSMISSION:\n"
                "Current Level:%d::First Transmission is From-To:%d->%d\n"
                "Next Level %d, Switch Assigned:%d via Link assigned:%d\n"
                        ,thread->current_level,node_s_id,node_r_id
                        ,(thread->current_level+1) //1st transmission always UP
                        ,thread->eee_switchid,thread->eee_linkid);
        }

        if(node_s_id == node_r_id) {
                printf("Warning: Same node communication - Not expected - Not sure why!\n");
                thread->eee_send_done = TRUE;
                //exit(EXIT_FAILURE);
                return(0);
        }
        thread->messages_in_flight++;//Test Code!
        node_s->messages_in_flight++;
        node_r->messages_in_flight++;

    }

    // Congession code!
    if(thread->routing_dir == EEE_UP) {

        // if current level is -1 and UP then use the IN link of Switch 0
        // else use the OUT link of the current Level Switch
        // current Level can never be (Last Level) when routing dir == UP

        if(thread->current_level == -1) {
            if(eee_switches[thread->current_level+1][thread->eee_switchid]
                .in_links[thread->eee_linkid].inlink_next_free_time
                        <= current_time ) {
                t_nano stats_pdt_time;
                stats_pdt_time = 0;
                // Check if LINK is ON if its OFF switch ON 1st before transmission
                int link_ON, link_in_sleep ;
                t_nano link_in_sleep_time ;
                link_ON = FALSE;
                link_in_sleep = FALSE;
                if ( eee_switches[thread->current_level+1][thread->eee_switchid]
                     .in_links[thread->eee_linkid].last_inlink_off_time
                     < current_time ) {
                    link_ON = FALSE; // THIS MEANS LINK IS OFF OR MOVING TO SLEEP STATE
                    // Accounting for sleep time of link
                    stats_pdt_time = T_pdt[thread->current_level+1];
                    double sleep_diff;
                    sleep_diff = current_time
                                    - eee_switches[thread->current_level+1][thread->eee_switchid]
                                        .in_links[thread->eee_linkid].last_inlink_off_time;
                    if ( sleep_diff < T_s[thread->current_level+1] ) {
                        link_in_sleep = TRUE;
                        link_in_sleep_time = T_s[thread->current_level+1] - sleep_diff;
                    }
                } else {
                    link_ON = TRUE; // THE LINKS IS ON!
                    stats_pdt_time = current_time
                                        - eee_switches[thread->current_level+1][thread->eee_switchid]
                                        .in_links[thread->eee_linkid].inlink_next_free_time ;
                }
                transmit_possible = TRUE;
                if(EEE_DEBUG) {
                        PRINT_TIMER(current_time);
                        printf("::%d->%d::Transmission Possible!\n",node_s_id,node_r_id);
                }
                //DONE! -- ADD VIRTUAL CUT-TRU BELOW
                //Calculating transmission for Frame Header and Payload
                eee_nw_delay = (t_nano) eee_calc_nw_bw_lat(thread, fh_size);
                msg_payload_delay = (t_nano) eee_calc_nw_bw_lat(thread, mess_size);
                eee_switches[thread->current_level+1][thread->eee_switchid]
                        .in_links[thread->eee_linkid].inlink_next_free_time
                        = current_time + eee_nw_delay + msg_payload_delay ;
                // inlink_next_free_time is checked becuase
                // this is a incomming message to level 0 switch!
                if (!link_ON) { // LINK is OFF or moving to SLEEP mode
                    t_nano total_eee_delay;
                    total_eee_delay = (t_nano) T_w[thread->current_level+1];
                    if (link_in_sleep) total_eee_delay+=link_in_sleep_time;

                    eee_nw_delay += (t_nano) total_eee_delay ;
                    msg_payload_delay += (t_nano) total_eee_delay ;
                    eee_switches[thread->current_level+1][thread->eee_switchid]
                        .in_links[thread->eee_linkid].inlink_next_free_time
                        +=  total_eee_delay ;
                } else {
                    // Link is ON and nothing is added
                }
                // TODO Can I write some check for this EEE code?
                eee_switches[thread->current_level+1][thread->eee_switchid]
                    .in_links[thread->eee_linkid].last_inlink_off_time
                    = eee_switches[thread->current_level+1][thread->eee_switchid]
                        .in_links[thread->eee_linkid].inlink_next_free_time
                        + T_pdt[thread->current_level+1] ;

                /* STATS COLLECTION */

                eee_switches[thread->current_level+1][thread->eee_switchid]
                    .in_links[thread->eee_linkid].stats_total_inlink_on_time
                    += eee_nw_delay + msg_payload_delay + stats_pdt_time;

                /* end STATS COLLECTION */

            } else {
                if(EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::%d->%d::Transmission NOT Possible - Link currently in USE\n"
                                                                    ,node_s_id,node_r_id);
                }
                transmit_possible = FALSE;
                eee_nw_delay = eee_switches[thread->current_level+1][thread->eee_switchid]
                                .in_links[thread->eee_linkid].inlink_next_free_time
                               - current_time;
                if(EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::Call Message After: %f\n",eee_nw_delay);
                }
                if(eee_nw_delay == 0) {
                    PRINT_TIMER(current_time);
                    printf(":::::::Next Call cannot be Zero!\n");
                    exit(EXIT_FAILURE);
                }
            }

        } else if (thread->current_level == (N_levels-1)) {
            printf("EEE Error: Current Level cannot be Last when routing is UP\n");
            exit(EXIT_FAILURE);
        } else {
            if(eee_switches[thread->current_level][thread->eee_switchid]
                .out_links[thread->eee_linkid].outlink_next_free_time
                <= current_time ) {
                t_nano stats_pdt_time;
                stats_pdt_time = 0;
                int link_ON,link_in_sleep;
                t_nano link_in_sleep_time;
                link_ON = FALSE;
                link_in_sleep = FALSE;
                if ( eee_switches[thread->current_level][thread->eee_switchid]
                        .out_links[thread->eee_linkid].last_outlink_off_time
                        < current_time ) {
                    link_ON = FALSE;
                    stats_pdt_time = T_pdt[thread->current_level];
                    double sleep_diff;
                    sleep_diff = current_time
                                    - eee_switches[thread->current_level][thread->eee_switchid]
                                       .out_links[thread->eee_linkid].last_outlink_off_time;
                    if ( sleep_diff < T_s[thread->current_level] ) {
                        link_in_sleep = TRUE;
                        link_in_sleep_time = T_s[thread->current_level] - sleep_diff;
                    }
                } else {
                    link_ON = TRUE; // THE LINK is ON
                    stats_pdt_time = current_time
                                        - eee_switches[thread->current_level][thread->eee_switchid]
                                        .out_links[thread->eee_linkid].outlink_next_free_time ;
                }

                // TODO Write EEE Code!
                transmit_possible = TRUE;
                if(EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::%d->%d::Transmission Possible!\n",node_s_id,node_r_id);
                }
                //Calculating transmission for Frame Header and Payload
                eee_nw_delay = (t_nano) eee_calc_nw_bw_lat(thread, fh_size);
                msg_payload_delay = (t_nano) eee_calc_nw_bw_lat(thread, mess_size);
                eee_switches[thread->current_level][thread->eee_switchid]
                    .out_links[thread->eee_linkid].outlink_next_free_time
                    =  current_time + eee_nw_delay + msg_payload_delay ;

                if(!link_ON) { // LINK is OFF or moving to SLEEP mode
                    t_nano total_eee_delay;
                    total_eee_delay = (t_nano) T_w[thread->current_level];
                    if(link_in_sleep) total_eee_delay += link_in_sleep_time ;

                    eee_nw_delay += (t_nano) total_eee_delay ;
                    msg_payload_delay += (t_nano) total_eee_delay ;
                    eee_switches[thread->current_level][thread->eee_switchid]
                        .out_links[thread->eee_linkid].outlink_next_free_time
                        += total_eee_delay ;
                } else {
                    // Link is ON and nothing to add
                }
                // TODO Can I write a Check for this code!
                eee_switches[thread->current_level][thread->eee_switchid]
                    .out_links[thread->eee_linkid].last_outlink_off_time
                    =   eee_switches[thread->current_level][thread->eee_switchid]
                        .out_links[thread->eee_linkid].outlink_next_free_time
                        + T_pdt[thread->current_level] ;
                /* STATS COLLECTION */

                eee_switches[thread->current_level][thread->eee_switchid]
                    .out_links[thread->eee_linkid].stats_total_outlink_on_time
                    += eee_nw_delay + msg_payload_delay + stats_pdt_time ;
                //updating partner link
                eee_switches[thread->current_level][thread->eee_switchid]
                    .out_links[thread->eee_linkid].partner_link->stats_total_inlink_on_time
                = eee_switches[thread->current_level][thread->eee_switchid]
                    .out_links[thread->eee_linkid].stats_total_outlink_on_time ;

                /* end STATS COLLECTION */

                thread->eee_switchid
                    = eee_switches[thread->current_level][thread->eee_switchid]
                                            .out_links[thread->eee_linkid]
                                            .partner_link->switch_id;

            } else {
                if(EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::%d->%d::Transmission NOT Possible - Link currently in USE\n"
                                                                    ,node_s_id,node_r_id);
                }
                transmit_possible = FALSE;
                eee_nw_delay = eee_switches[thread->current_level][thread->eee_switchid]
                                                .out_links[thread->eee_linkid]
                                                .outlink_next_free_time
                                                - current_time;
            }
        }

    } else if (thread->routing_dir == EEE_DOWN) {

            if(eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].outlink_next_free_time
                        <= current_time ) {
                t_nano stats_pdt_time ;
                stats_pdt_time = 0;
                //TODO Check if Link is ON of OFF
                int link_ON,link_in_sleep;
                t_nano link_in_sleep_time;
                link_ON = FALSE;
                link_in_sleep = FALSE;

                if ( eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].last_outlink_off_time
                        < current_time ) {
                    link_ON = FALSE;
                    // if link is OFF then add full PDT to stats_on_time
                    stats_pdt_time = T_pdt[thread->current_level];
                    double sleep_diff;
                    sleep_diff = current_time
                                    - eee_switches[thread->current_level][thread->eee_switchid]
                                        .in_links[thread->eee_linkid].last_outlink_off_time ;
                    if (sleep_diff < T_s[thread->current_level] ) {
                        link_in_sleep = TRUE ;
                        link_in_sleep_time = T_s[thread->current_level] - sleep_diff;
                        stats_pdt_time = 0; // No time for pdt
                    }
                } else {
                    link_ON = TRUE;
                    stats_pdt_time = current_time
                                        - eee_switches[thread->current_level][thread->eee_switchid]
                                            .in_links[thread->eee_linkid].outlink_next_free_time ;
                }

                transmit_possible = TRUE;
                if (EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::%d->%d::Transmission Possible!\n",node_s_id,node_r_id);
                }
                //Calculating transmission for Frame Header and Payload
                eee_nw_delay = (t_nano) eee_calc_nw_bw_lat(thread, fh_size);
                msg_payload_delay = (t_nano) eee_calc_nw_bw_lat(thread, mess_size);
                eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].outlink_next_free_time
                        = current_time + eee_nw_delay + msg_payload_delay;
                if(thread->current_level == 0) {
                    // Final transmission will take fullmessage transmit time!
                    if(EEE_DEBUG) printf("Last Transmit with PAYLOAD delay\n");
                    eee_nw_delay = eee_nw_delay + msg_payload_delay ;
                }

                if(!link_ON) {
                    t_nano total_eee_delay ;
                    total_eee_delay = (t_nano) T_w[thread->current_level];
                    if (link_in_sleep) total_eee_delay += link_in_sleep_time;

                    eee_nw_delay += (t_nano) total_eee_delay ;
                    msg_payload_delay += (t_nano) total_eee_delay ;
                    eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].outlink_next_free_time
                        += total_eee_delay ;
                } else {
                    // link is ON so nothing needs to be added
                }

                eee_switches[thread->current_level][thread->eee_switchid]
                    .in_links[thread->eee_linkid].last_outlink_off_time
                    = eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].outlink_next_free_time
                        + T_pdt[thread->current_level] ;

                /* STATS COLLECTION */

                eee_switches[thread->current_level][thread->eee_switchid]
                    .in_links[thread->eee_linkid].stats_total_outlink_on_time
                    += eee_nw_delay + msg_payload_delay + stats_pdt_time ;
                //updating partner link
                if(thread->current_level != 0) {
                eee_switches[thread->current_level][thread->eee_switchid]
                    .in_links[thread->eee_linkid].partner_link->stats_total_inlink_on_time
                    = eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].stats_total_outlink_on_time ;
                }

                /* end STATS COLLECTION */

                //TODO Must change times of PARTNER LINKs as well!!!!
                if ( thread->current_level != 0) {
                    thread->eee_switchid
                        = eee_switches[thread->current_level][thread->eee_switchid]
                                                    .in_links[thread->eee_linkid]
                                                    .partner_link->switch_id;
                }

            } else {
                if (EEE_DEBUG) {
                    PRINT_TIMER(current_time);
                    printf("::%d->%d::Transmission NOT Possible - Link in USE!"
                                                            ,node_s_id,node_r_id);
                }
                transmit_possible = FALSE;
                eee_nw_delay = eee_switches[thread->current_level][thread->eee_switchid]
                                .in_links[thread->eee_linkid].outlink_next_free_time
                                - current_time;
            }
            //TODO CHECK ABOVE EEE+PDT CODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            //TODO HAVE NOT UPDATED THE PARTNER LINK FREE_TIME AND EEE_LAST_OFF_TIME
            //      DOES THAT MAKE A DIFFERENCE?????????????????????????????????????????????????????

    } else {
            printf("EEE Error: Routing Dir incorrect: Not supposed to be here\n");
            exit(EXIT_FAILURE);
    }

    // The transmission is always from one level to next
    //  - So level must change based on routing direction!
    if( transmit_possible == TRUE){
        if(thread->routing_dir == EEE_UP) {
            thread->current_level++;
        } else if (thread->routing_dir == EEE_DOWN) {
            thread->current_level--;
        }
    }

    if (EEE_DEBUG) {
        printf("Message in link transmission to NEXT switch\n");
    }


    // Actual Send Code!

    //DONE! transmit only the header as long as this is not the last transmission!
    //          -- if this is the last transmission, then transmit full message!
    //DONE! header only for virtual cut through & header+1st frame for store and fwd


    //TODO if the link is ON - transmit possible = TRUE else turn link ON

    if(thread->current_level == -1) { //Last Transmission
        if(transmit_possible) thread->eee_send_done = TRUE;
    }

    if(transmit_possible == TRUE) {
        // Transmission is possible
        if((thread->link_transmit_done == TRUE) || (thread->nw_switch_done == FALSE)) {
            if((org_current_level != -1) && (thread->routing_dir == EEE_UP)) {
                printf("Cannot Be HERE!!!!\n"); exit(EXIT_FAILURE);
            }
        }
        thread->link_transmit_done = TRUE;
        thread->nw_switch_done = FALSE;
    } else {
        // Transmission is NOT  possible - link busy or link is turning ON
        // REVERTING ALL IMP ACTIONS
        thread->link_transmit_done = FALSE;
        thread->nw_switch_done = TRUE;
        // if current_level is changed to -1, it means transmission is over
    //    if(thread->current_level == -1){ thread->eee_send_done = FALSE; }
    //    thread->current_level = org_current_level;
    }

    return(eee_nw_delay);

}

// Function is incharge of moving and routing message tru the switch
t_nano nw_switch(struct t_thread *thread) // Step 2
{
    t_nano eee_nw_delay;
    eee_nw_delay = 0;

    t_boolean hop_possible = FALSE;

    struct t_action     *action;
    struct t_send       *mess;
    struct t_task       *task, *task_partner;
    struct t_node       *node_s, *node_r;

    action = thread->action;
    mess = &(action->desc.send);
    task = thread->task;
    task_partner = locate_task (task->Ptask, mess->dest);
    node_s = get_node_of_task (task);
    node_r = get_node_of_task (task_partner);

    int node_s_id, node_r_id;
    node_s_id = (node_s->nodeid - 1); // from (0 - node_id-1)
    node_r_id = (node_r->nodeid - 1);
    if((node_s_id < 0) || (node_r_id < 0)) {
        printf("EEE ERROR::::NOT POSSIBLE for Node ID to be Negative\n");
        exit(EXIT_FAILURE);
    }

    // LOGIC for changing routing dir goes here
    int div_function,lev_loop;
    div_function = 1;//mul function
    for (lev_loop = 0; lev_loop <= thread->current_level; lev_loop++)
        div_function *= eee_switches[lev_loop][0].N_in_links;
        //returns number of nodes per switch in a level
    int node_s_div_nodes,node_r_div_nodes;
    node_s_div_nodes = node_s_id / div_function;
    node_r_div_nodes = node_r_id / div_function;
    if ( node_s_div_nodes == node_r_div_nodes ) {
        //Reverse Dir!!
        if(thread->routing_dir == EEE_UP) {
            thread->routing_dir = EEE_DOWN;
        } else {
            if(thread->current_level != (N_levels-1)) {
                printf("NW_SWITCH ERROR!!! Cant be here!\n");
                exit(EXIT_FAILURE);
            }
        }
    } else {} // Continue in same dir

    // End Change of Routing Dir Code!

    if (EEE_DEBUG) {
        printf("%d->%d::At Switch:: Preparing to HOP::Current Level:%d::Switch: %d\n"
                                ,node_s_id,node_r_id
                                ,thread->current_level, thread->eee_switchid);
    }

    /* COMMENTS!***********************************************************************
    // Allocating Link for Transmission
    // Below code will find a link for transmission depending on direction
    // Find a IN link during DOWN transmission
    // Find a OUT link during UP transmission (except last level only has IN links)
    // First level's IN links do not have partner links!
    // So -1 to 0 level will use the IN links of 0th level rather than the
    //      OUT links of -1 level (which does not exist
    ***********************************************************************************/

    // CURRENT SWITCH IS EXPECTED TO BE ASSIGNED WHEN WE COME TO THIS PART OF THE CODE
    // Below code changes the current switch to NEXT switch!

    // End of Link Allocation Code!

     if( thread->routing_dir == EEE_UP) {
         int temp_i;
         if(thread->current_level == (N_levels-1)) {
             printf("ERROR! Not supposed to be here\n");
             exit(EXIT_FAILURE);
         }
                 // Assigning NEXT link and switch
                 // Static route based on receiving node!!
                 // Assigning Static route into Fat-Tree
                 temp_i = node_r_id
                          % eee_switches[thread->current_level][thread->eee_switchid]
                          .N_out_links;
                 thread->eee_linkid
                        = eee_switches[thread->current_level][thread->eee_switchid]
                          .out_links[temp_i].link_id;
                 if(EEE_DEBUG) {
                     printf("Level:%d::Moving to Higher Level %d::Current switch: %d - hop via Link: %d\n"
                                            ,thread->current_level
                                            ,(thread->current_level+1)
                                            ,thread->eee_switchid
                                            ,thread->eee_linkid);
                 }
     } else if (thread->routing_dir == EEE_DOWN) {
         int next_level;
         // Allocate Link when the messages go down!
         next_level = thread->current_level - 1;
         if(next_level == -1) {
             int no_of_links;
             no_of_links = N_nodes/switches_per_level[0];
             thread->eee_linkid = ((int)node_r_id)%no_of_links;
         } else {
             int div_function,lev_loop;
             div_function = 1; // mul function
             for ( lev_loop = 0; lev_loop <= next_level; lev_loop++)
                 div_function *= eee_switches[lev_loop][0].N_in_links;
                 //returns number of nodes per switch in next level
             int node_r_mod_nodes;
             node_r_mod_nodes = node_r_id / div_function;
             thread->eee_linkid = node_r_mod_nodes % eee_switches[thread->current_level][thread->eee_switchid].N_in_links;
             // Check for correctness
             if ( eee_switches[thread->current_level][thread->eee_switchid]
                        .in_links[thread->eee_linkid].partner_link->switch_id
                        != node_r_mod_nodes) {
                    printf("%d->%d::Level:%d::Moving to Lower Level %d\n - Current switch: %d - hop via Link: %d\n"
                                   ,node_s_id,node_r_id
                                   ,thread->current_level
                                   ,(thread->current_level-1)
                                   ,thread->eee_switchid
                                   ,thread->eee_linkid);

                    printf("Error in nw_switch(), switch calc error! %d != %d\n"
                                   ,eee_switches[thread->current_level][thread->eee_switchid]
                                    .in_links[thread->eee_linkid].partner_link->switch_id
                                   ,node_r_mod_nodes);
                    exit(EXIT_FAILURE);
             }
         }
         if (EEE_DEBUG) {
             printf("Level:%d::Moving to Lower Level %d\n - Current switch: %d - hop via Link: %d\n"
                            ,thread->current_level
                            ,(thread->current_level-1)
                            ,thread->eee_switchid
                            ,thread->eee_linkid);
         }

     } else {
         printf("EEE_ERROR::Cant be here!\n");
         exit(EXIT_FAILURE);
     }

    if((thread->nw_switch_done == TRUE) || (thread->link_transmit_done == FALSE)) {
        printf("EEE_ERROR:Cannot Be Here!!!\n");
        exit(EXIT_FAILURE);
    }
    thread->nw_switch_done = TRUE;
    thread->link_transmit_done = FALSE;
    // EEE Switch assumes Zero Latency
    eee_nw_delay = (t_nano) T_eee_hop_lat[thread->current_level]; // HOP LATENCY
    if(EEE_DEBUG){
        PRINT_TIMER(current_time);
        printf("CURR LEVEL:::%d::::Adding HopLat:%f\n",thread->current_level,eee_nw_delay);
    }
    return(eee_nw_delay);
}

t_nano eee_network(struct t_thread *thread)
{

    t_nano eee_nw_delay;
    eee_nw_delay = 0;

    if (thread->eee_send_done != TRUE) {

        // Setting routing direction
        if(thread->current_level == -1) { // 1st level
            thread->routing_dir = EEE_UP;
        } else if(thread->current_level == (N_levels-1)) { // top most level
            thread->routing_dir = EEE_DOWN;
        } else { // do nothing
        }

        if(thread->link_transmit_done == FALSE) {
            eee_nw_delay = link_transmit(thread);
        } else if (thread->nw_switch_done == FALSE) {
            eee_nw_delay = nw_switch(thread);
        } else {
            printf("Error in fn eee-network: Not supposed to be here\n");
            exit(EXIT_FAILURE);
        }

    } else {
        printf("Inside network but sending was over!\n");
        exit(EXIT_FAILURE);
    }

    return (eee_nw_delay);

}


/******************************************************************************
* End of EEE Code
******************************************************************************/
