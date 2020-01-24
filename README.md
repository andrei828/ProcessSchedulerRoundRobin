# Process scheduler with weighted users (Round-Robin)


[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger) ![Build Status](https://img.shields.io/github/repo-size/andrei828/ProcessSchedulerRoundRobin?color=magenta) ![Build Status](https://img.shields.io/github/last-commit/andrei828/ProcessSchedulerRoundRobin)

This program enables you to run multiple processes executed by the CPU using the **Round-Robin algorithm**. Currently the repo has pre-loaded an example where a random number of processes are generated. This design uses **Inter Process Communication (IPC)** to schedule the processes on the CPU.


#  Compilation and Execution
- ####  Clone this repository
    ```sh
    $ git clone https://github.com/andrei828/ProcessSchedulerRoundRobin.git
    $ cd ProcessSchedulerRoundRobin
    ```
- #### Compile files with gcc
    ```sh
    $ gcc dummy.c -o dum
    $ gcc graphic.c -o run
    ```
- #### Execute binary and preview in terminal the processes
    ```sh
    $ ./run
    ```

##### The repo contains:
  - A ```graphic.c``` file that deals with the terminal interface.
  - A ```dummy.c``` file that should be used however the user prefers.
  - A ```dependencies.c``` file that has the core functions and data structures for the program. 


> The tasks ran but the dummy processes should be configured in the ```dummy.c``` file.

# Here is an example of the terminal interface
- ### The list of processes generated
    ![Process list](https://github.com/andrei828/ProcessSchedulerRoundRobin/blob/master/images/ProcessList.png)

- ### The processes pending execution in Activity Monitor
    ![Activity Monitor](https://github.com/andrei828/ProcessSchedulerRoundRobin/blob/master/images/ActivityMonitor.png)

- ### The Gantt diagram after execution
    ![Gantt Diagram](https://github.com/andrei828/ProcessSchedulerRoundRobin/blob/master/images/GanttDiagram.png)
