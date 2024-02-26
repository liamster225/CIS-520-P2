#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

int main(int argc, char **argv) 
{
    if (argc < 3) 
    {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pcb_file = argv[1];
    const char *algorithm = argv[2];
    size_t quantum = 0;

    if (argc == 4) 
    {
        quantum = atoi(argv[3]);
    }

    dyn_array_t *ready_queue = load_process_control_blocks(pcb_file);

    if (ready_queue == NULL) 
    {
        printf("Failed to load process control blocks.\n");
        return EXIT_FAILURE;
    }

    ScheduleResult_t result;

    if (strcmp(algorithm, FCFS) == 0) 
    {
        if (!first_come_first_serve(ready_queue, &result)) 
        {
            printf("Failed to execute FCFS algorithm.\n");
            dyn_array_destroy(ready_queue);
            return EXIT_FAILURE;
        }
    } 
    else if (strcmp(algorithm, SJF) == 0) 
    {
        if (!shortest_job_first(ready_queue, &result)) 
        {
            printf("Failed to execute SJF algorithm.\n");
            dyn_array_destroy(ready_queue);
            return EXIT_FAILURE;
        }
    } 
    else if (strcmp(algorithm, RR) == 0) 
    {
        if (!round_robin(ready_queue, &result, quantum)) 
        {
            printf("Failed to execute RR algorithm.\n");
            dyn_array_destroy(ready_queue);
            return EXIT_FAILURE;
        }
    } 
    else if (strcmp(algorithm, P) == 0) 
    {
        if (!priority(ready_queue, &result)) 
        {
            printf("Failed to execute Priority algorithm.\n");
            dyn_array_destroy(ready_queue);
            return EXIT_FAILURE;
        }
    } 
    else 
    {
        printf("Invalid scheduling algorithm.\n");
        dyn_array_destroy(ready_queue);
        return EXIT_FAILURE;
    }

    printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
    printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
    printf("Total Clock Time: %lu\n", result.total_run_time);

    dyn_array_destroy(ready_queue);

    return EXIT_SUCCESS;
}
