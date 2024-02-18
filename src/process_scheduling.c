#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    //If input parameters are incorrect output is false
    if(ready_queue == NULL || result == NULL || dyn_array_empty(ready_queue)){
        return false;
    }

    // Variables for time analysis
    float total_waiting_time = 0;
    float total_turnaround_time = 0;
    unsigned long total_run_time = 0;

    // Itterate over the entire size of the queue and proccess in a FIFO order
    for(size_t i = 0; i < dyn_array_size(ready_queue); i++){
        ProcessControlBlock_t* pcb = dyn_array_at(ready_queue, 1);

        //Calculate the waiting time for this proccess and add it to the total waiting time
        float waiting_time = total_run_time - pcb->arrival;
        total_waiting_time += waiting_time;

        // calculate this PCB's turnaround time prior to proccessing the command
        float turnaround_time = waiting_time + pcb->remaining_burst_time;

        // Perform the execution of the command in the PCB
        while(pcb->remaining_burst_time > 0){
            total_run_time++;
            virtual_cpu(pcb);
        }

        // update total turnaround
        total_turnaround_time += turnaround_time;
    }
    // calculate the average times for the result
    result->average_waiting_time = total_waiting_time / dyn_array_size(ready_queue);
    result->average_turnaround_time = total_turnaround_time / dyn_array_size(ready_queue);
    result->total_run_time = total_run_time;

    return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    UNUSED(quantum);
    return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    UNUSED(input_file);
    return NULL;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;
}
