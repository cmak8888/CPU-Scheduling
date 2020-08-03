/*
  Author: Calvin Mak
  Course: CSCI 442: Operating Systems
  Assignment: Project 1
  Date: 9/28/2018
  Desc: Setup Struct Process
*/
using namespace std;
/*
  Your simulation must support the following event types:
    - THREAD_ARRIVED: A thread has been created in the system.
    - THREAD_DISPATCH_COMPLETED: A thread switch has completed, allowing a new thread to start exe-
cuting on the CPU.
    - PROCESS_DISPATCH_COMPLETED: A process switch has completed, allowing a new thread in a dif-
ferent process to start executing on the CPU.
    - CPU_BURST_COMPLETED: A thread has finished one of its CPU bursts and has initiated an I/O request.
    - IO_BURST_COMPLETED: A thread has finished one of its I/O bursts and is once again ready to be
executed.
    - THREAD_COMPLETED: A thread has finished the last of its CPU bursts.
    - THREAD_PREEMPTED: A thread has been preempted during execution of one of its CPU bursts.
    - DISPATCHER_INVOKED: The OS dispatcher routine has been invoked to determine the next thread to
be run on the CPU
*/

struct Event {
  string event;     //What is the Event
  int threadID;     //Thread Index
  int threadArr;    //Thread Arrival Time
  int threadType;   //System, Interactive, Normal, BATCH
  int processID;
  string desc;      //Description/verbose
  bool operator<(const Event &e) const {    //For Priority Queue ordering
    return threadArr > e.threadArr;
  }
};
