# Workload Scheduler Simulator

A C++ simulation framework that models job scheduling and resource allocation across multiple workers using different scheduling strategies.

## About

This project builds a discrete-time scheduling simulation engine where multiple scheduling algorithms can be tested under the same workload.

The simulator tracks job execution using time ticks, manages worker assignments, and compares scheduling strategies based on total completion time.

The goal is to analyze how different scheduling approaches affect efficiency, fairness, and resource utilization.

## System Design

### Scheduler Engine

The Scheduler Engine is responsible for:
- Managing job and worker states
- Running simulation tick by tick
- Assigning jobs to available workers
- Tracking execution and completion

### Scheduling Strategy

Scheduling algorithms are implemented using a common strategy interface, allowing different approaches to be plugged into the same simulation engine.

## Scheduling Strategies

### First Come First Serve (FCFS)

A queue-based scheduling approach where jobs are executed in the order they arrive.

### Shortest Job First (SJF)

A greedy scheduling approach that selects jobs with smaller remaining execution time first to improve completion efficiency.

### Priority Scheduling with Aging

A priority-based approach where waiting jobs gradually gain priority over time to reduce starvation and improve fairness.

### Highest Response Ratio Next (HRRN)

A scheduling approach that balances waiting time and execution time to provide fair job selection.

### Load Balancing

A resource allocation strategy that distributes jobs across workers by considering worker workload to improve resource utilization.

## Simulation Rules

Each job contains:
- Job ID
- Burst time
- Priority
- Remaining execution time
- Waiting time

Each worker contains:
- Worker ID
- Processing capacity
- Current assigned job
- Assigned workload tracking

Rules:
- A worker executes one job at a time
- A job continues execution until completion after assignment
- Waiting jobs update their waiting time during simulation
- All strategies are tested on the same workload for comparison

## Input Format

The input contains job information followed by worker information.

Jobs:
- Number of jobs
- Job ID
- Burst time
- Priority

Workers:
- Number of workers
- Worker ID
- Worker capacity

Example:

number_of_jobs

job_id burst_time priority

...

number_of_workers

worker_id capacity

...

## Output

The simulator provides:

- Total ticks required by each scheduling strategy
- Comparison of completion time between strategies
- Tick-by-tick execution trace showing:
  - Job assignments
  - Worker execution
  - Job completion events

Example:

=== COMPARISON ===

FCFS        : X ticks  
SJF         : X ticks  
Aging       : X ticks  
HRRN        : X ticks  
Load-Balance: X ticks

## Concepts Used

- C++ Object-Oriented Programming
- Inheritance and Polymorphism
- Strategy Design Pattern
- STL Data Structures and Algorithms
- Greedy Algorithms
- Priority Scheduling
- Fair Scheduling Techniques
- Resource Allocation
- Discrete Time Simulation
