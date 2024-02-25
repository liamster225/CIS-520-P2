#include <fcntl.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <pthread.h>
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function managling
extern "C" 
{
#include <dyn_array.h>
}


#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment 
{
    public:
        virtual void SetUp() 
        {
            score = 0;
            total = 210;
        }

        virtual void TearDown()
        {
            ::testing::Test::RecordProperty("points_given", score);
            ::testing::Test::RecordProperty("points_total", total);
            std::cout << "SCORE: " << score << '/' << total << std::endl;
        }
};

// first_come_first_serve TEST 1: Ensure the function returns false when either ready_queue or result is NULL
TEST(first_come_first_serve, NullPointers) {
    dyn_array_t* ready_queue = NULL; //Create A null ready queue
    ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t)); // Allocate memory for result
    ASSERT_EQ(false, first_come_first_serve(ready_queue, result));
    free(result);
}



TEST(first_come_first_serve, NullResult) {
    dyn_array_t* ready_queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL); // Create a queue for ProcessControlBlock_t elements
    
    // Allocate memory for pcb1
    ProcessControlBlock_t* pcb1 = (ProcessControlBlock_t*)malloc(sizeof(ProcessControlBlock_t));
    if (pcb1 == NULL) {
        // Handle allocation failure if needed
        // For simplicity, exiting the test case here
        dyn_array_destroy(ready_queue);
        return;
    }
    
    // Initialize pcb1 members
    pcb1->remaining_burst_time = 5;
    pcb1->arrival = 0;
    pcb1->priority = 0; 
    pcb1->started = false; 

    // Push pcb1 onto the queue
    dyn_array_push_back(ready_queue, pcb1);

    // Create null result
    ScheduleResult_t* result = NULL;

    // Call the function under test
    ASSERT_EQ(false, first_come_first_serve(ready_queue, result));

    // Free the dynamically allocated memory for pcb1
    free(pcb1);

    // Destroy the queue
    dyn_array_destroy(ready_queue);

}



// first_come_first_serve TEST 2: Ensure the function returns false when the ready_queue is empty
TEST(first_come_first_serve, EmptyQueue) {
    dyn_array_t* ready_queue =  dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);;
    ScheduleResult_t result;
    ASSERT_EQ(false, first_come_first_serve(ready_queue, &result));
    
    free(ready_queue);
}


// Test case for first_come_first_serve
TEST(first_come_first_serve, NonEmptyQueue) {
    dyn_array_t* ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    if (ready_queue == NULL) {
        FAIL() << "Failed to create dyn_array";
    }

    // Initialize and push PCBs
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 7, .priority = 0, .arrival = 0, .started = false };
    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // Create result variable
    ScheduleResult_t result;
    ASSERT_TRUE(first_come_first_serve(ready_queue, &result));

    // Test the calculated statistics
    ASSERT_NEAR(4.33, result.average_waiting_time, 0.01);
    ASSERT_NEAR(12.33, result.average_turnaround_time, 0.01);
    ASSERT_EQ(28u, result.total_run_time);

    dyn_array_destroy(ready_queue);
}

// Test case for first_come_first_serve
TEST(first_come_first_serve, FirstProcessedFirst) {
    dyn_array_t* ready_queue = dyn_array_create(2, sizeof(ProcessControlBlock_t), NULL);
    if (ready_queue == NULL) {
        FAIL() << "Failed to create dyn_array";
    }

    // Initialize PCBs
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 0, .arrival = 0, .started = false };

    // Push PCBs onto the array
    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);

    // Create result variable
    ScheduleResult_t result;
    ASSERT_TRUE(first_come_first_serve(ready_queue, &result));

    // After processing, the first process should have remaining burst time 0
    ASSERT_EQ(0u, pcb1.remaining_burst_time);

    dyn_array_destroy(ready_queue);
}

// load_process_control_block TEST 1: Test case to ensure NULL is returned when the input file is not found
TEST(load_process_control_blocks, FileNotFound) {
    const char* input_file = "nonexistent_file.bin";
    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_EQ(nullptr, result);
}

// load_process_control_block TEST 2: Test case to ensure NULL is returned when the file is empty
TEST(load_process_control_blocks, EmptyFile) {
    // Create an empty file
    const char* input_file = "empty_file.bin";
    FILE* file = fopen(input_file, "wb");
    fclose(file);

    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_EQ(nullptr, result);
}


TEST(load_process_control_blocks, CorrectNumberOfPCBsLoaded) {
    // Create a binary file with PCBs
    const char* input_file = "pcb_data.bin";
    FILE* file = fopen(input_file, "wb");
    
    // Dynamically allocate memory for PCBs
    ProcessControlBlock_t* pcbs = (ProcessControlBlock_t*)malloc(3 * sizeof(ProcessControlBlock_t));
    if (pcbs == nullptr) {
        // Handle allocation failure
        fclose(file);
        FAIL() << "Failed to allocate memory for PCBs";
    }
    
    // Initialize PCB properties separately
    pcbs[0].remaining_burst_time = 5;
    pcbs[0].arrival = 0;
    pcbs[1].remaining_burst_time = 3;
    pcbs[1].arrival = 0;
    pcbs[2].remaining_burst_time = 7;
    pcbs[2].arrival = 0;

    // Write PCBs to the file
    fwrite(pcbs, sizeof(ProcessControlBlock_t), 3, file);
    fclose(file);

    // Load PCBs from the file
    dyn_array_t* result = load_process_control_blocks(input_file);
    
    // Assertion checks
    ASSERT_NE(nullptr, result);
    //Used the idea of the static cast from previous method that chat gpt suggested
    ASSERT_EQ(static_cast<size_t>(3), dyn_array_size(result));

    // Free dynamically allocated memory
    free(pcbs);
    free(result);
}



TEST(load_process_control_blocks, CorrectPCBDataLoaded) {
    // Create a binary file with PCBs
    const char* input_file = "pcb_data.bin";
    FILE* file = fopen(input_file, "wb");

    // Allocate memory for the PCBs
    ProcessControlBlock_t* pcbs = (ProcessControlBlock_t*)malloc(3 * sizeof(ProcessControlBlock_t));
    if (pcbs == nullptr) {
        // Handle allocation failure
        fclose(file);
        FAIL() << "Memory allocation failed";
    }

    // Initialize properties of each PCB individually
    pcbs[0].remaining_burst_time = 5;
    pcbs[0].arrival = 0;
    pcbs[1].remaining_burst_time = 3;
    pcbs[1].arrival = 1;
    pcbs[2].remaining_burst_time = 7;
    pcbs[2].arrival = 2;

    // Write PCBs to the file
    fwrite(pcbs, sizeof(ProcessControlBlock_t), 3, file);
    fclose(file);

    // Load PCBs from the file
    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(static_cast<size_t>(3), dyn_array_size(result));

    // Verify the loaded PCB data
    for (size_t i = 0; i < 3; ++i) {
        ProcessControlBlock_t* loaded_pcb = (ProcessControlBlock_t*)dyn_array_at(result, i);
        // Cast to int to match the type of pcbs' members
        ASSERT_EQ(static_cast<int>(pcbs[i].remaining_burst_time), static_cast<int>(loaded_pcb->remaining_burst_time));
        ASSERT_EQ(static_cast<int>(pcbs[i].arrival), static_cast<int>(loaded_pcb->arrival));
    }

    // Free dynamically allocated memory
    free(pcbs);
    free(result);
}


//Tests past this point generated by ChatGPT for additional testing, 
//given the load_process_control_blocks as a reference test case
TEST(load_process_control_blocks, NonExistingFile) {
    const char* input_file = "non_existing_file.bin";

    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_EQ(nullptr, result);
}

TEST(load_process_control_blocks, InvalidFileFormat) {
    const char* input_file = "invalid_format.bin";
    FILE* file = fopen(input_file, "wb");
    // Write some invalid data to the file
    fwrite("Invalid data", sizeof(char), 13, file);
    fclose(file);

    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_EQ(nullptr, result);
}

TEST(load_process_control_blocks, LargeFile) {
    const char* input_file = "large_file.bin";
    FILE* file = fopen(input_file, "wb");

    // Generate large amount of PCB data
    const size_t num_pcbs = 1000000;
    ProcessControlBlock_t* pcbs = (ProcessControlBlock_t*)malloc(num_pcbs * sizeof(ProcessControlBlock_t));
    if (pcbs == nullptr) {
        fclose(file);
        FAIL() << "Memory allocation failed";
    }

    for (size_t i = 0; i < num_pcbs; ++i) {
        pcbs[i].remaining_burst_time = static_cast<int>(i % 10); // Varying burst times
        pcbs[i].arrival = static_cast<int>(i);
    }

    // Write PCBs to the file
    fwrite(pcbs, sizeof(ProcessControlBlock_t), num_pcbs, file);
    fclose(file);

    // Load PCBs from the file
    dyn_array_t* result = load_process_control_blocks(input_file);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(num_pcbs, dyn_array_size(result));

    // Verify the loaded PCB data
    for (size_t i = 0; i < num_pcbs; ++i) {
        ProcessControlBlock_t* loaded_pcb = (ProcessControlBlock_t*)dyn_array_at(result, i);
        ASSERT_EQ(pcbs[i].remaining_burst_time, loaded_pcb->remaining_burst_time);
        ASSERT_EQ(pcbs[i].arrival, loaded_pcb->arrival);
    }

    // Free dynamically allocated memory
    free(pcbs);
    dyn_array_destroy(result);
}

//Null ready queue for shortest job first
TEST(shortest_remaining_time_first, NullPointers) {
    dyn_array_t* ready_queue = NULL; //Create A null ready queue
    ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t)); // Allocate memory for result
    ASSERT_EQ(false, shortest_remaining_time_first(ready_queue, result));
    free(result);
}




// first_come_first_serve TEST 2: Ensure the function returns false when the ready_queue is empty
TEST(shortest_remaining_time_first, EmptyQueue) {
    dyn_array_t* ready_queue =  dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);;
    ScheduleResult_t result;
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    dyn_array_push_back(ready_queue,&pcb1);
    ASSERT_EQ(false, shortest_remaining_time_first(ready_queue, &result));
    
    free(ready_queue);
}

// Test case for a non-empty ready queue with multiple processes
TEST(shortest_remaining_time_first, NonEmptyQueue) {
    dyn_array_t* ready_queue = dyn_array_create(5, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    // Creating and adding processes to the ready queue
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 1, .arrival = 1, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 7, .priority = 2, .arrival = 2, .started = false };

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // Running the shortest_remaining_time_first algorithm
    ASSERT_EQ(true, shortest_remaining_time_first(ready_queue, &result));

    // Validating the results
    ASSERT_EQ(5.0f, result.average_waiting_time); // Waiting time is sum of run times = 0+3+8 = 11 / 3 = 3.666...
    ASSERT_FLOAT_EQ(5.6666665f, result.average_turnaround_time); // Turnaround time is sum of burst times = 5+3+7 = 15 / 3 = 5.0
    ASSERT_EQ(15ul, result.total_run_time); // Total run time is sum of burst times = 5+3+7 = 15

    free(ready_queue);
}

// Test case for processes with varying burst times
TEST(shortest_remaining_time_first, VaryingBurstTimes) {
    dyn_array_t* ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    // Creating and adding processes to the ready queue with varying burst times
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 8, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 5, .priority = 2, .arrival = 0, .started = false };

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // Running the shortest_remaining_time_first algorithm
    ASSERT_EQ(true, shortest_remaining_time_first(ready_queue, &result));

    // Validating the results
    ASSERT_EQ(5.0f, result.average_waiting_time); // Waiting time is sum of run times = 0+8+3 = 11 / 3 = 3.666...
    ASSERT_FLOAT_EQ(5.0f, result.average_turnaround_time); // Turnaround time is sum of burst times = 8+2+5 = 15 / 3 = 5.0
    ASSERT_EQ(15ul, result.total_run_time); // Total run time is sum of burst times = 8+2+5 = 15

    free(ready_queue);
}

//Null ready queue for shortest job first
TEST(shortest_job_first, NullPointers) {
    dyn_array_t* ready_queue = NULL; //Create A null ready queue
    ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t)); // Allocate memory for result
    ASSERT_EQ(false, shortest_job_first(ready_queue, result));
    free(result);
}




// first_come_first_serve TEST 2: Ensure the function returns false when the ready_queue is empty
TEST(shortest_job_first, EmptyQueue) {
    dyn_array_t* ready_queue =  dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);;
    ScheduleResult_t result;
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    dyn_array_push_back(ready_queue,&pcb1);
    ASSERT_EQ(false, shortest_job_first(ready_queue, &result));
    
    free(ready_queue);
}

// Test case for a non-empty ready queue with multiple processes
TEST(shortest_job_first, NonEmptyQueue) {
    dyn_array_t* ready_queue = dyn_array_create(5, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    // Creating and adding processes to the ready queue
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 6, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 3, .priority = 1, .arrival = 1, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 9, .priority = 2, .arrival = 2, .started = false };

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // Running the shortest_job_first algorithm
    ASSERT_EQ(true, shortest_job_first(ready_queue, &result));

    // Validating the results
    ASSERT_EQ(4.0f, result.average_waiting_time); // Waiting time is sum of run times = 0+3+9 = 13 / 3 = 4.0
    ASSERT_FLOAT_EQ(6.0f, result.average_turnaround_time); // Turnaround time is sum of burst times = 5+3+7 = 15 / 3 = 5.0
    ASSERT_EQ(18ul, result.total_run_time); // Total run time is sum of burst times = 5+3+7 = 15

    free(ready_queue);
}

// Test case for processes with varying burst times
TEST(shortest_job_first, VaryingBurstTimes) {
    dyn_array_t* ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    // Creating and adding processes to the ready queue with varying burst times
    ProcessControlBlock_t pcb1 = { .remaining_burst_time = 8, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb2 = { .remaining_burst_time = 2, .priority = 1, .arrival = 0, .started = false };
    ProcessControlBlock_t pcb3 = { .remaining_burst_time = 5, .priority = 2, .arrival = 0, .started = false };

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // Running the shortest_remaining_time_first algorithm
    ASSERT_EQ(true, shortest_remaining_time_first(ready_queue, &result));

    // Validating the results
    ASSERT_EQ(5.0f, result.average_waiting_time); // Waiting time is sum of run times = 0+8+3 = 11 / 3 = 3.666...
    ASSERT_FLOAT_EQ(5.0f, result.average_turnaround_time); // Turnaround time is sum of burst times = 8+2+5 = 15 / 3 = 5.0
    ASSERT_EQ(15ul, result.total_run_time); // Total run time is sum of burst times = 8+2+5 = 15

    free(ready_queue);
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}
