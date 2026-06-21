# Workload Scheduler Simulator

A C++ simulation project that models job scheduling and resource allocation across multiple workers using different scheduling strategies.

## About

This project simulates how multiple jobs are assigned to workers and compares different scheduling approaches.

The simulator tracks job execution using time ticks and shows how different algorithms behave on the same workload.

## Scheduling Strategies

### Shortest Job First (SJF)

A greedy approach that selects jobs with smaller execution time first.

### Priority Scheduling with Aging

A priority-based approach where waiting jobs gradually gain priority to reduce starvation.

### Dynamic Programming Based Selection

Uses a knapsack-style DP approach to select jobs based on available capacity and job priority.

## Concepts Used

- C++ Object-Oriented Programming
- Inheritance and Polymorphism
- STL
- Greedy Algorithms
- Dynamic Programming
- Simulation

## Project Structure

```
main.cpp
input.txt
test_cases
```
