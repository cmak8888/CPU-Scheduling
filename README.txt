Name: Calvin Mak
Course: CSCI 442 Operating Systems
Project: Project 1: CPU Scheduling Simulator

Deliverable 1:
The current output displays similar to the "verbose" style flag.
It will output to the terminal via cout.
In the code, I had some implementations of code that have no effect whatsoever on the code nor the output, the only reason it exists is due to my errors of variables not being used.
These will be removed once I actually complete some algorithms and use the other variables acquired from input.


Deliverable 2:
I couldn't get the algorithm finished in time, letalone a day late. I spent the last fall break coming in to Alamode to work on it. I turned
in what I have so far. While I do have the outputs for the other flags ready and implemented, due to a runtime/infinite loop error at the moment, I simply have it print to terminal
the verbose up till it starts making an error. I have about a 1/3 of the verbose displaying mostly properly. I'll still come in for the next few days to re-work the algorithm.
I'll continue to work on the project and try to have everything set by the Final Deliverable, but I am still worried about my grade. The only thing I
have left to do is implementing the algorithms at this moment.
I had to do a lot of re-working since I had issues confusing c++ with various other programming languages I'm working with at this moment.

Final Deliverable:
Files Included:
  - Bursts.h
    A default struct holding the cpuBursts and IOBursts of THREADS
  - Event.h
    A default struct holding the Events to be unraveled in the verbose. It contains threadID, a description of the event and event state and processId
  - Output.h
    Output Map for each Thread type.
  - Process.h
    Basic Struct for Processes and all the threads within it.
  - Thread.h
    Basic Struct for Threads and all the bursts within it.
  - simulator.cpp
    The main c++ file that runs the simulator algorithm.
Features:
  My Program can support multiple flags and will display all flags unless -h or --help is called.
  The algorithm is case-insensitive.
Time:
  I spent at least 160 hours on this project. I had long nights and had a hard time figuring/understanding the project. Then I had an even harder time finding a good algorithm.
  72 hours once I got my feet in.

About my Custom CPU Scheduling Algorithm:
  My CPU Scheduling algorithm utilizes lots of elements. The Threads of the highest priority will always be handled first
  at the time unless unavailable. Then the next order of priority is handled in its stead down to the batch Thread types.
  I tried to maintain fairness in my algorithm by regulating the quantum time. The quantum time slice is calculated by the average total
  CPU bursts divided by the burstcount of the first thread divided by 4 for each thread type (0-3). Threads are preempted mid-cpuburst by the quantum
  counter. I tried to share processes throughout multiple queues for fairness. Starvation is possible but unlikely due to the fair chance each
  process gets in ters of burst time.
  I wouldn't say though that my algorithm is ideally fair. I didn't have a lot of time to perfect it to the degree I wanted to. I was hoping to finish
  setting up aging and demotion within the queues to improve efficiency of my process.

Additional Comments:
  The output outputs to the command prompt.
