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
        ProcessControlBlock_t* pcb = dyn_array_at(ready_queue, i);

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

// Runs the Shortest Remaining Time First Process Scheduling algorithm over the incoming ready_queue
// \param ready queue a dyn_array of type ProcessControlBlock_t that contain be up to N elements
// \param result used for shortest job first stat tracking \ref ScheduleResult_t
// \return true if function ran successful else false for an error
bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
   if(ready_queue == NULL || result == NULL || dyn_array_size(ready_queue) == 0)
    {
        return false;
    }

    // Variables to calculate statistics
    float waiting_time = 0;
    float turnaround_time = 0;
    unsigned long run_time = 0;
    unsigned long curr_run_time = 0;

    dyn_array_sort(ready_queue, compare_burst_time);
  
    // For loop to go through the ready_queue
    for (size_t i = 0; i < dyn_array_size(ready_queue); ++i) {
        // Use dyn_array_at the current index to get the current PCB
        ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);

        // Make sure that the PCB is not null
        if (pcb == NULL) {
            return false; // PCB is null, therefore unsuccessful
        }

        // Calculate waiting time for the current process
        waiting_time += run_time;

        // Simulate processing on virtual CPU until the process is executed
        while (pcb->remaining_burst_time > 0) {
            virtual_cpu(pcb);
            ++curr_run_time;
        }

        // Calculate turnaround time for the current process
        turnaround_time += curr_run_time;

        run_time += curr_run_time;

        curr_run_time = 0;
    }
    
    // Set the average waiting time
    result->average_waiting_time = waiting_time / dyn_array_size(ready_queue);

    // Set the average turnaround time
    result->average_turnaround_time = turnaround_time / dyn_array_size(ready_queue);

    // Set the total run time
    result->total_run_time = run_time;

    return true;   
}
