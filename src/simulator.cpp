/*
  Author: Calvin Mak
  Course: CSCI 442: Operating Systems
  Assignment: Project 1
  Date: 9/28/2018
  Desc: Create a Simulated Task Schedulating program
*/
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <locale>
#include <cmath>
#include <queue>
#include <map>
#include <bits/stdc++.h>

#include "Process.h"
#include "Event.h"
#include "Output.h"

using namespace std;

/*
  Notes:
    - Response Time is the time difference from the start (release) time and arrival time of a job.
    - Average response time is calculated over a period of time for the same job.
    - Turnaround Time  (TRT) (TAT) is the period for completing a process cycle. i.e Completion time - Arrival Time
    - Waiting time is TAT (TRT) - Burst Time
    CPU Efficiency = useful CPU time / total CPU time or cpuTime / Total Time. or service time / elapsed time
    CPU utilization = dispatch time + service time/elapsed time.
*/

/* Other Notes
  Output:
    - Track each type
      - Count total number of eacd thread;
      - Calculate the average response time
      - calculate the average turnaround time.
    - Total Time required to execute all threads to Completion
    - Time spent executing user processes (service time) CPU
    - Time Spent performing I/O (Sum of all I/O bursts)
    - Time spent doing process and thread switches (dispatching overhead)
    - The amount of time the CPU was idle
    - CPU utilization
    - CPU efficiency
  Flag -t:
    - Track by Process, by thread.
    - Print Arrival Time
    - Print CPU (service) time
    - Print I/O time
    - Print TurnAroundTime
    - Print Finish time
  Flag -v:
    - Track by Time period
    - Print Every State Changing Event
    - Print Every Scheduling Decision

*/

class Compare {                                           //Priority Queue Priority to Early Arrival
public:
  bool operator() (Thread &a, Thread &b) {
    return a.arrivalTime > b.arrivalTime;
  }
};

class Compare2 {
public:
  bool operator() (Thread &a, Thread &b) {                //Priority Queue Priority to Early endTime
    return a.endTime > b.endTime;
  }
};

class CompareType {
public:
  bool operator() (Thread &a, Thread &b) {                //Priority Queue Priority to Type
    return a.threadType > b.threadType;
  }
};



void help() {
  cout << "To Run this program, you must first \"cd\" to the current directory this program is located in." << endl;
  cout << "Then type the command 'make'." << endl;
  cout << "Use the command ./simulator [flags] simulation_file.txt where [flags] are flags supported by this program provided below and simulation_file.txt is your input file." << endl;
  cout << "[flags]:\n Supported flags include:" << endl;
  cout << " -t, or --per_thread:\n    Outputs additional per-thread statistics for arrival time, service (CPU) time, etc." << endl;
  cout << " -v, or --verbose:\n     Outputs information about every state-changing event and scheduling decision(s)." << endl;
  cout << " -a, or --algorithm:\n     The scheduling algorithm to use as provided:\n FCFS (First Come First Serve)\n RR (Round Robin)\n PRIORITY (Process-Priority Scheduling)\n CUSTOM (Custom Algorithm)" << endl << "just abbreviations are fine" << endl;
  cout << " -h or --help:\n      Displays this help menu." << endl;
  cout << "If the given algorithm is invalid, or does the follow guidelines stated above, the program will default to FCFS scheduling." << endl;
  //cout << "Full List of Acceptable Algorithm Inputs (Capitalization is optional):\n FCFS\n First Come First Serve\n Priority\n Process Priority\n Process-Priority Scheduling\n CUSTOM"  << endl;
  cout << "A combination of these flags are accepted as well such as -tv; -vta; -htav; etc" << endl;
  exit(0);
}

void verbo(priority_queue<Event> &ev) {                                                  //Verbose Thread
  cout << endl;
  while(!ev.empty()) {
    Event e = ev.top();
    cout << "At time " << e.threadArr << ":" << endl;
    cout << "\t" << e.event << endl;
    cout << "\tThread " << e.threadID << " in process " << e.processID << " ";
    switch(e.threadType) {
      case(0):
              cout << "[SYSTEM]" << endl;
              break;
      case(1):
              cout << "[INTERACTIVE]" << endl;
              break;
      case(2):
              cout << "[NORMAL]" << endl;
              break;
      case(3):
              cout << "[BATCH]" << endl;
              break;
    }
    cout << "\t" << e.desc << endl;
    ev.pop();
  }
}


void threads(vector<Process> &p) {                                                  //Per Thread Flag
  for(int i = 0; i < (int)p.size(); i++) {                                           //Goes through each process
    cout << "Process " << p.at(i).id;
    switch(p[i].type) {                                                         //Determines the Process Type
      case(0):
              cout << " [SYSTEM]:" << endl;
              break;
      case(1):
              cout << " [INTERACTIVE]:" << endl;
              break;
      case(2):
              cout << " [NORMAL]:" << endl;
              break;
      case(3):
              cout << " [BATCH]:" << endl;
              break;
    }
    for(int j = 0; j < (int)p[i].t.size(); j++) {                                        //Goes through each thread
      Thread threa = p[i].t.at(j);
      int arrTime = threa.arrivalTime;
      int cpuTime = 0;
      int IOTime = 0;
      int endTime = threa.endTime;
      cout << "\tThread " << j << " ARR: " << arrTime << " CPU: ";
      for (int k = 0; k < threa.burstCount; k++) {
        Bursts be = threa.b[k];
        cpuTime = cpuTime + be.cpuBurst;
        if(be.IOBurst > 0)
          IOTime = IOTime + be.IOBurst;
      }
      cout << cpuTime << " I/O: " << IOTime << " TRT: " << endTime - arrTime << " END: " << endTime << endl;
    }
    cout << endl;
  }
}


void unpack(priority_queue<Thread, vector<Thread>, Compare> &q) {
  while(!q.empty()) {
    Thread t = q.top();
    cout << "State: " << t.state << endl;
    cout << "Thread: " << t.threadID << endl;
    cout << "ArrivalTime: " << t.arrivalTime << endl;
    cout << "EndTime: " << t.endTime << endl;
    cout << "Process: " << t.ProcessId << endl;
    q.pop();
  }
  return;
}

void updateQueue(Thread &t, priority_queue<Thread, vector<Thread>, Compare> &sys,priority_queue<Thread, vector<Thread>, Compare> &interactive,priority_queue<Thread, vector<Thread>, Compare> &normal,priority_queue<Thread, vector<Thread>, Compare> &batch ) {
  switch(t.threadType) {                                             //Refresh
    case(0):
      sys.pop();
      sys.push(t);
      break;
    case(1):
      interactive.pop();
      interactive.push(t);
      break;
    case(2):
      normal.pop();
      normal.push(t);
      break;
    case(3):
      batch.pop();
      batch.push(t);
      break;
  }
  return;
}

priority_queue<Thread, vector<Thread>, Compare> &getQueue(Thread &t, priority_queue<Thread, vector<Thread>, Compare> &sys,priority_queue<Thread, vector<Thread>, Compare> &interactive,priority_queue<Thread, vector<Thread>, Compare> &normal,priority_queue<Thread, vector<Thread>, Compare> &batch ) {
  switch(t.threadType) {                                          //Initializes it in the proper queue
    case(0):
      return sys;
      break;
    case(1):
      return interactive;
      break;
    case(2):
      return normal;
      break;
    case(3):
      return batch;
      break;
    default:
      return batch;
  }
}

/*void combineThreads(vector<Process> &p, vector<Thread> &allThreads) {
  for (int i = 0; i < (int)p.size(); i++) {
    vector<Thread> t = p[i].t;
    allThreads.reserve(t.size() + allThreads.size());
    allThreads.insert(allThreads.end(), t.begin(), t.end());
  }
}*/

/*int minArrival(vector<Thread> t, vector<size_t> &iterator) {
  int min;
  bool check = false;
  int starter = 0;
  while (check == false) {
    min = t[starter].arrivalTime;
    if (iterator.empty()) {
      check = true;
    } else if (find(iterator.begin(),iterator.end(), starter) == iterator.end()){
      check = true;
    } else { starter++; }

  }
  int it = starter;
  for (int i = 0; i < (int)t.size(); i++) {
    if (find(iterator.begin(),iterator.end(), i) == iterator.end()) {
      if (t[i].arrivalTime < min) {
        min = t[i].arrivalTime;
        it = i;
      }
    }
  }
  iterator.push_back(it);
  return min;
}*/

/*vector<size_t> sortMinArrival(vector<Thread> &t, vector<size_t> iterator) {
  vector<size_t> dits;                      //indicies
  vector<size_t> newIterator;
  int min;
  bool check = false;
  int starter = 0;
  while(dits.size() != t.size()) {
    while (check == false) {
      min = t[starter].arrivalTime;
      if (dits.empty()) {
        check = true;
      } else if (find(dits.begin(),dits.end(), starter) == dits.end()){
        check = true;
      } else { starter++; }

    }
    int it = starter;
    for (int i = 0; i < (int)t.size(); i++) {
      if (find(dits.begin(),dits.end(), i) == dits.end()) {
        if (t[i].arrivalTime < min) {
          min = t[i].arrivalTime;
          it = i;
        }
      }
    }
    dits.push_back(it);
    newIterator.push_back(iterator[it]);
  }
  return newIterator;
}*/
string lower(string a){
  locale loc;
  string b = "";
  for (string::size_type i = 0; i < a.length(); ++i) {
    b = b + tolower(a[i], loc);
  }
  return b;
}

void Task_Schedule_Simulator(string textFile, bool thread, bool verbose, string algorithm) {
  //Initial Datavalues/structures
  int totalTime = 0;
  int cpuTime = 0;                      //Bursts and overhead
  int IOTime = 0;                       //Bursts and overhead
  int dispatchTime = 0;                 //Overhead time
  int idleTime = 0;
  int numProcesses = 0;                 //Total number of processes
  int threadOverhead = 0;               //Thread Switch Overhead Values
  int processOverhead = 0;              //Process Switch overhead values
  map<int, Output> processing; //Maps out My output threads.
  //Initializing map Values
  //System = 0
  //Interactive = 1
  //Normal = 2
  //Batch = 3
  processing.insert (pair<int, Output>(0,{0,0,0}));
  processing.insert (pair<int, Output>(1,{0,0,0}));
  processing.insert (pair<int, Output>(2,{0,0,0}));
  processing.insert (pair<int, Output>(3,{0,0,0}));



  vector<Process> p;                //Array of Processes
  priority_queue<Event> nextEvent;           //Next Event Queue
  int queueSize = 0;
  ifstream inputFile;
  inputFile.open(textFile);

  while(!inputFile) {
    inputFile.clear();
    cerr << "Error opening file " << textFile << endl;
    cout << "Please input the name of the input file: ";
    cin >> textFile;
    cout << endl;
    inputFile.open(textFile);
  }

  string input;
  //if statements to ensure no null/trailing whitespace values.
  if (inputFile >> input)  {                                               //Retrieves processCount
    numProcesses =  stoi(input);
  }
  if (inputFile >> input)  {                                               //Retrieves thread switch overhead time
    threadOverhead =  stoi(input);
  }
  if (inputFile >> input)  {                                               //Retrieves process switch overhead time
    processOverhead =  stoi(input);
  }
 //Each set of inputs Grouped in if statements to ensure no null/trailing whitespace.
  for (int i = 0; i < numProcesses; i++) {
    Process Pro;
    if (inputFile >> input)  {                                               //Retrieves process id, type, number of threads, and times and burst.
      Pro.id = stoi(input);
    }
    if (inputFile >> input)  {                                               //Retrieves Process Type
      Pro.type = stoi(input);
      if (Pro.type < 0) {
        cout << "Invalid input type is less than 0.\n";
        exit(1);
      } else if (Pro.type > 3) {
        cout << "Invalid input type is greater than 3.\n";
        exit(1);
      }
    }
    if (inputFile >> input)  {                                               //Retrieves Process ThreadCount
      Pro.threadCount = stoi(input);
      queueSize += Pro.threadCount;                                          //Total size of Arrival of thread queue
    }
    Thread thre;
    for (int j = 0; j < Pro.threadCount; j++) {
      thre.ProcessId  = Pro.id;
      thre.threadType = Pro.type;
      thre.received = false;
      thre.threadID = j;
      thre.state = "NEW";
      if (inputFile >> input)  {                                               //Retrieves thread arrival time and burst count.
        thre.arrivalTime = stoi(input);
        thre.endTime = thre.arrivalTime;
        //cout << "Arrived at " << stoi(input) << endl;
      }
      if (inputFile >> input)  {                                               //Retrieves burstCount.
        thre.burstCount = stoi(input);
      }

      for (int k = 0; k < thre.burstCount; k++) {
        Bursts urst;
        //cout << "Thread: " << thre.threadID << endl;
        if (inputFile >> input)  {                                               //Retrieves CPU Burst
          urst.cpuBurst = stoi(input);
          //cout << urst.cpuBurst;
          cpuTime += urst.cpuBurst;
        }
        if (k != thre.burstCount -1) {                                        //Last value will not exist, don't want to read the next line
          if (inputFile >> input)  {                                           //Retrieves Thread Burst (may be null)
            urst.IOBurst = stoi(input);
            IOTime += urst.IOBurst;
          }
        } else {urst.IOBurst = -1; }
        thre.b.push_back(urst);                                                //Add Bursts to burst array in Thread
        urst = {};
      }
      Pro.t.push_back(thre);
      thre = {};
    }
    processing.at(Pro.type).count = Pro.threadCount;
    p.push_back(Pro);                                                           //Add to Process Array
    Pro = {};
  }
  inputFile.close();                                                            //Close the input file

  vector<Process> p2 = p;

  algorithm = lower(algorithm);
  if(algorithm == "rr" || algorithm == "roundrobin") {
   //Round Robin Algorithm
   int quan = 3;
   priority_queue<Thread, vector<Thread>, Compare> readyQueue;                //Sets up my ready priority queue
   priority_queue<Thread, vector<Thread>, Compare2> eventQueue;               //Declares my event priority queue
   for(size_t i = 0; i < p.size(); i++) {      //Puts my arrival events in the event queues
     Process pro = p[i];
     for(size_t j = 0; j < pro.t.size();j++) {  //For each thread in each process...
       Thread ti = pro.t[j];
       Event ev = {"THREAD_ARRIVED" , ti.threadID, ti.arrivalTime, ti.threadType, ti.ProcessId, "Transitioned from NEW to READY"};
       eventQueue.push(ti);                                                    //Inserts threads into my event queue
       nextEvent.push(ev);                                                    //Inserts events into the Verbose queue
     }
   }
   //Base Case
   Thread currentThread = eventQueue.top();
   currentThread.state = "READY";
   readyQueue.push(currentThread);
   currentThread.ProcessId = -1;
   eventQueue.pop();
   bool dispatch = false;
   while(!readyQueue.empty()) {
     Thread ti = {};
     if (!readyQueue.empty()) {                                              //Make sure the counter is chronological
      ti = readyQueue.top();
      if(ti.threadID == currentThread.threadID) {
         if(ti.state == "READY" && dispatch == false) {                  //No thread is running and/or does not exist
           Event e = {"DISPATCHER_INVOKED", ti.threadID, currentThread.endTime, ti.threadType, ti.ProcessId, "Selected from " + to_string(readyQueue.size()) + " threads; will run to completion of burst or quantum"};
           nextEvent.push(e);
           if(currentThread.ProcessId != ti.ProcessId) {                       //Process Dispatch
             ti.endTime = currentThread.endTime + processOverhead;
             dispatchTime += processOverhead;                                  //Adds to total Dispatch time
             Event e = {"PROCESS_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
             nextEvent.push(e);                                                //Add to Verbose
           } else {                                                            //Same Process different Thread, Thread dispatch
             ti.endTime = currentThread.endTime + threadOverhead;
             dispatchTime += threadOverhead;                                   //Adds to total Dispatch time
             Event e = {"THREAD_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
             nextEvent.push(e);                                                //Add to Verbose
           }
           if(!ti.received) {
               ti.received = true;
               processing.at(ti.threadType).ResTime += (ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime);
           }
           currentThread = ti;                                                 //Copy the currentThread as a reference
           dispatch = true;
           readyQueue.pop();
           readyQueue.push(ti);                                              //Replace the same thread.
       } else if(ti.state == "READY" && dispatch == true && ti.endTime == currentThread.endTime) {
           ti.state = "RUNNING";
           if (quan >= ti.b[0].cpuBurst) {
             ti.endTime += ti.b[0].cpuBurst;
             ti.b[0].cpuBurst -= quan;
           } else {
             ti.endTime += quan;
             ti.b[0].cpuBurst -= quan;
           }
           readyQueue.pop();
           readyQueue.push(ti);                                                //Insert into eventQueue
           currentThread = ti;
       } else if(ti.state == "RUNNING") {                                  //Thread is running
        if (ti.b[0].cpuBurst > 0) {
          Event e = {"THREAD_PREEMPTED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING TO READ"};
          nextEvent.push(e);
          ti.state = "READY";
        } else {
          if(ti.b[0].IOBurst > -1) {                                        //There is an IO Burst
             Event e = {"CPU_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to BLOCKED"};
             ti.state = "BLOCKED";                                           //Transition to BLOCKED
             currentThread.endTime = ti.endTime + (quan - (quan + ti.b[0].cpuBurst)); //Keeps in time with the time slice
             ti.b[0].cpuBurst = 0;
             ti.endTime = ti.endTime + ti.b[0].IOBurst;                      //Update EndTime
             nextEvent.push(e);                                              //Add to Verbose
             eventQueue.push(ti);                                            //Insert into EventQueue
          } else {
             ti.state = "EXIT";
             int TRT =  ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime;
             processing[ti.threadType].TRT += TRT;
             Event e = {"THREAD_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to EXIT"};
             nextEvent.push(e);                                                //Add to verbose queue
             p2[ti.ProcessId].t.at(ti.threadID).endTime = ti.endTime;
             currentThread.endTime = ti.endTime + (quan - (quan + ti.b[0].cpuBurst)); //Keeps in time with the time slice
           }
        }
        readyQueue.pop();
        if(!readyQueue.empty()) {
         ti = readyQueue.top();
         currentThread.threadID = ti.threadID;
        }
        dispatch = false;
       }
      } else if(ti.endTime < currentThread.endTime){
         ti.endTime = currentThread.endTime;
         readyQueue.pop();
         readyQueue.push(ti);
       }
       if(!eventQueue.empty()) {
         while(eventQueue.top().endTime < currentThread.endTime || readyQueue.empty()) {
           ti = eventQueue.top();
           if(ti.state == "NEW") {                                           //Threads have Arrived
             ti.state = "READY";
             readyQueue.push(ti);
             eventQueue.pop();
           } else if(ti.state == "BLOCKED") {
             ti.state = "READY";
             Event e = {"IO_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from BLOCKED to READY"};
             ti.b.erase(ti.b.begin());                                           //Erases Bursts at index 0.
             ti.arrivalTime = ti.endTime;
             if(readyQueue.empty()) {
               idleTime += (ti.endTime - currentThread.endTime);
               currentThread.threadID = ti.threadID;
               currentThread.endTime = ti.endTime;
             }
             nextEvent.push(e);                                                  //Add to Verbose
             readyQueue.push(ti);                                                //Add ready event to queue
             eventQueue.pop();
           }
           if(eventQueue.empty()) { break; }
         }
       }
     }
   }
 } else if(algorithm == "priority") {
   priority_queue<Thread, vector<Thread>, Compare> sys;
   priority_queue<Thread, vector<Thread>, Compare> interactive;
   priority_queue<Thread, vector<Thread>, Compare> normal;
   priority_queue<Thread, vector<Thread>, Compare> batch;
   priority_queue<Thread, vector<Thread>, Compare2> eventQueue;              //Declares my event priority queue

   for(size_t i = 0; i < p.size(); i++) {      //Puts my arrival events in the event queues
     Process pro = p[i];
     for(size_t j = 0; j < pro.t.size();j++) {  //For each thread in each process...
       Thread ti = pro.t[j];
       Event ev = {"THREAD_ARRIVED" , ti.threadID, ti.arrivalTime, ti.threadType, ti.ProcessId, "Transitioned from NEW to READY"};
       eventQueue.push(ti);                                                    //Inserts threads into my event queue
       nextEvent.push(ev);                                                    //Inserts events into the Verbose queue
     }
   }

    //Base Case
    Thread currentThread =  eventQueue.top();                                    //Tracks current Processing Thread
    currentThread.state = "READY";                                              //Sets base case to ready
    getQueue(currentThread,sys,interactive,normal,batch).push(currentThread);
    currentThread.ProcessId = -1;                                               //Takes Initial Process Switch into account
    eventQueue.pop();
    bool dispatch = false;

    while(!sys.empty() || !interactive.empty() || !normal.empty() || !batch.empty()) {
      Thread ti = {};
      if (!sys.empty() || !interactive.empty() || !normal.empty() || !batch.empty()) {    //Verifies if readyQueues are empty or not.
        if (!sys.empty() && !dispatch) {
          ti = sys.top();
        } else if(!interactive.empty() && !dispatch) {
          ti = interactive.top();
        } else if(!normal.empty() && !dispatch) {
          ti = normal.top();
        } else if(batch.empty() && !dispatch) {
          ti = batch.top();
        } else if(dispatch) {
          ti = getQueue(currentThread,sys,interactive,normal,batch).top();
        }
        if(ti.threadID == currentThread.threadID) {
          if(ti.state == "READY" && dispatch == false) {                  //No thread is running and/or does not exist
            Event e = {"DISPATCHER_INVOKED", ti.threadID, currentThread.endTime, ti.threadType, ti.ProcessId, "Selected from " + to_string(getQueue(ti, sys, interactive, normal, batch).size()) + " threads; will run to completion of burst"};
            nextEvent.push(e);
            if(currentThread.ProcessId != ti.ProcessId) {                       //Process Dispatch
              ti.endTime = currentThread.endTime + processOverhead;
              dispatchTime += processOverhead;                                  //Adds to total Dispatch time
              Event e = {"PROCESS_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
              nextEvent.push(e);                                                //Add to Verbose
            } else {                                                            //Same Process different Thread, Thread dispatch
              ti.endTime = currentThread.endTime + threadOverhead;
              dispatchTime += threadOverhead;                                   //Adds to total Dispatch time
              Event e = {"THREAD_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
              nextEvent.push(e);                                                //Add to Verbose
            }
            if(!ti.received) {
                ti.received = true;
                processing.at(ti.threadType).ResTime += (ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime);
            }
            currentThread = ti;                                                 //Copy the currentThread as a reference
            dispatch = true;
            updateQueue(ti, sys, interactive, normal, batch);
        } else if(ti.state == "READY" && dispatch == true && ti.endTime == currentThread.endTime) {
            ti.state = "RUNNING";
            ti.endTime += ti.b[0].cpuBurst;
            updateQueue(ti, sys, interactive, normal, batch);                   //Inserts in appropriate queue
            currentThread = ti;
        } else if(ti.state == "RUNNING") {                                  //Thread is running
          if (ti.b[0].cpuBurst > 0) {
            Event e = {"THREAD_PREEMPTED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING TO READ"};
            nextEvent.push(e);
            ti.state = "READY";
          } else {
            if(ti.b[0].IOBurst > -1) {                                        //There is an IO Burst
              Event e = {"CPU_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to BLOCKED"};
              ti.state = "BLOCKED";                                           //Transition to BLOCKED
              ti.endTime = ti.endTime + ti.b[0].IOBurst;                      //Update EndTime
              nextEvent.push(e);                                              //Add to Verbose
              eventQueue.push(ti);                                            //Insert into EventQueue
            } else {
              ti.state = "EXIT";
              int TRT =  ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime;
              processing[ti.threadType].TRT += TRT;
              Event e = {"THREAD_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to EXIT"};
              nextEvent.push(e);                                                //Add to verbose queue
              p2[ti.ProcessId].t.at(ti.threadID).endTime = ti.endTime;
            }
          }
          getQueue(ti, sys, interactive, normal, batch).pop();
          if (!sys.empty()) {                                                   //Looks for next available Thread
            ti = sys.top();
            currentThread.threadID = ti.threadID;
          } else if(!interactive.empty()) {
            ti = interactive.top();
            currentThread.threadID = ti.threadID;
          } else if(!normal.empty()) {
            ti = normal.top();
            currentThread.threadID = ti.threadID;
          } else if(!batch.empty()) {
            ti = batch.top();
            currentThread.threadID = ti.threadID;
          }
          dispatch = false;
          }
        } else if(ti.endTime < currentThread.endTime){
          ti.endTime = currentThread.endTime;
          updateQueue(ti, sys, interactive, normal, batch);                   //Inserts in appropriate queue
        }
        if(!eventQueue.empty()) {
          while(eventQueue.top().endTime < currentThread.endTime || getQueue(ti, sys, interactive, normal, batch).empty()) {
            ti = eventQueue.top();
            if(ti.state == "NEW") {                                           //Threads have Arrived
              ti.state = "READY";
              getQueue(ti, sys, interactive, normal, batch).push(ti);
              eventQueue.pop();
            } else if(ti.state == "BLOCKED") {
              ti.state = "READY";
              Event e = {"IO_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from BLOCKED to READY"};
              ti.b.erase(ti.b.begin());                                           //Erases Bursts at index 0.
              ti.arrivalTime = ti.endTime;
              if(getQueue(ti, sys, interactive, normal, batch).empty() && !dispatch && currentThread.state != "RUNNING") {
                idleTime += (ti.endTime - currentThread.endTime);
                currentThread.endTime = ti.endTime;
                if (!sys.empty()) {
                  ti = sys.top();
                } else if(!interactive.empty()) {
                  ti = interactive.top();
                } else if(!normal.empty()) {
                  ti = normal.top();
                } else if(batch.empty()) {
                  ti = batch.top();
                } else {

                }
                currentThread.threadType = ti.threadType;
                currentThread.threadID = ti.threadID;
              }
              nextEvent.push(e);                                                  //Add to Verbose
              getQueue(ti, sys, interactive, normal, batch).push(ti);                                                //Add ready event to queue
              eventQueue.pop();
            }
            if(eventQueue.empty()) { break; }
          }
        }
      }
    }
  } else if (algorithm == "custom"){
    //CUSTOM
    int quan = (double)((double)cpuTime/(double)(p[0].t.at(0).burstCount)/4 * 0.8);  //Changes quantum based on average values
    priority_queue<Thread, vector<Thread>, Compare> sys;
    priority_queue<Thread, vector<Thread>, Compare> interactive;
    priority_queue<Thread, vector<Thread>, Compare> normal;
    priority_queue<Thread, vector<Thread>, Compare> batch;
    priority_queue<Thread, vector<Thread>, Compare2> eventQueue;              //Declares my event priority queue

    for(size_t i = 0; i < p.size(); i++) {      //Puts my arrival events in the event queues
      Process pro = p[i];
      for(size_t j = 0; j < pro.t.size();j++) {  //For each thread in each process...
        Thread ti = pro.t[j];
        Event ev = {"THREAD_ARRIVED" , ti.threadID, ti.arrivalTime, ti.threadType, ti.ProcessId, "Transitioned from NEW to READY"};
        eventQueue.push(ti);                                                    //Inserts threads into my event queue
        nextEvent.push(ev);                                                    //Inserts events into the Verbose queue
      }
    }

     //Base Case
     Thread currentThread =  eventQueue.top();                                    //Tracks current Processing Thread
     currentThread.state = "READY";                                              //Sets base case to ready
     getQueue(currentThread,sys,interactive,normal,batch).push(currentThread);
     currentThread.ProcessId = -1;                                               //Takes Initial Process Switch into account
     eventQueue.pop();
     bool dispatch = false;

     while(!sys.empty() || !interactive.empty() || !normal.empty() || !batch.empty()) {
       Thread ti = {};
       if (!sys.empty() || !interactive.empty() || !normal.empty() || !batch.empty()) {    //Verifies if readyQueues are empty or not.
         if (!sys.empty() && !dispatch) {
           ti = sys.top();
         } else if(!interactive.empty() && !dispatch) {
           ti = interactive.top();
         } else if(!normal.empty() && !dispatch) {
           ti = normal.top();
         } else if(batch.empty() && !dispatch) {
           ti = batch.top();
         } else if(dispatch) {
           ti = getQueue(currentThread,sys,interactive,normal,batch).top();
         }
         if(ti.threadID == currentThread.threadID) {
           if(ti.state == "READY" && dispatch == false) {                  //No thread is running and/or does not exist
             Event e = {"DISPATCHER_INVOKED", ti.threadID, currentThread.endTime, ti.threadType, ti.ProcessId, "Selected from " + to_string(getQueue(ti, sys, interactive, normal, batch).size()) + " threads; will run to completion of burst"};
             nextEvent.push(e);
             if(currentThread.ProcessId != ti.ProcessId) {                       //Process Dispatch
               ti.endTime = currentThread.endTime + processOverhead;
               dispatchTime += processOverhead;                                  //Adds to total Dispatch time
               Event e = {"PROCESS_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
               nextEvent.push(e);                                                //Add to Verbose
             } else {                                                            //Same Process different Thread, Thread dispatch
               ti.endTime = currentThread.endTime + threadOverhead;
               dispatchTime += threadOverhead;                                   //Adds to total Dispatch time
               Event e = {"THREAD_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
               nextEvent.push(e);                                                //Add to Verbose
             }
             if(!ti.received) {
                 ti.received = true;
                 processing.at(ti.threadType).ResTime += (ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime);
             }
             currentThread = ti;                                                 //Copy the currentThread as a reference
             dispatch = true;
             updateQueue(ti, sys, interactive, normal, batch);
         } else if(ti.state == "READY" && dispatch == true && ti.endTime == currentThread.endTime) {
           ti.state = "RUNNING";
           if (quan >= ti.b[0].cpuBurst) {
             ti.endTime += ti.b[0].cpuBurst;
             ti.b[0].cpuBurst -= quan;
           } else {
             ti.endTime += quan;
             ti.b[0].cpuBurst -= quan;
           }
             updateQueue(ti, sys, interactive, normal, batch);                   //Inserts in appropriate queue
             currentThread = ti;
         } else if(ti.state == "RUNNING") {                                  //Thread is running
           if (ti.b[0].cpuBurst > 0) {
             Event e = {"THREAD_PREEMPTED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING TO READ"};
             nextEvent.push(e);
             ti.state = "READY";
           } else {                                   //Thread is running
              if(ti.b[0].IOBurst > -1) {                                        //There is an IO Burst
                 Event e = {"CPU_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to BLOCKED"};
                 ti.state = "BLOCKED";                                           //Transition to BLOCKED
                 currentThread.endTime = ti.endTime + (quan - (quan + ti.b[0].cpuBurst)); //Keeps in time with the time slice
                 ti.b[0].cpuBurst = 0;
                 ti.endTime = ti.endTime + ti.b[0].IOBurst;                      //Update EndTime
                 nextEvent.push(e);                                              //Add to Verbose
                 eventQueue.push(ti);                                            //Insert into EventQueue
              } else {
                 ti.state = "EXIT";
                 int TRT =  ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime;
                 processing[ti.threadType].TRT += TRT;
                 Event e = {"THREAD_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to EXIT"};
                 nextEvent.push(e);                                                //Add to verbose queue
                 p2[ti.ProcessId].t.at(ti.threadID).endTime = ti.endTime;
                 currentThread.endTime = ti.endTime + (quan - (quan + ti.b[0].cpuBurst)); //Keeps in time with the time slice
               }
            }
            getQueue(ti, sys, interactive, normal, batch).pop();
            if (!sys.empty()) {                                                   //Looks for next available Thread
              ti = sys.top();
              currentThread.threadID = ti.threadID;
            } else if(!interactive.empty()) {
              ti = interactive.top();
              currentThread.threadID = ti.threadID;
            } else if(!normal.empty()) {
              ti = normal.top();
              currentThread.threadID = ti.threadID;
            } else if(!batch.empty()) {
              ti = batch.top();
              currentThread.threadID = ti.threadID;
            }
            dispatch = false;
           }
         } else if((ti.endTime - ti.arrivalTime) > (currentThread.endTime - currentThread.arrivalTime)) {
           ti.threadType--;
           if(ti.endTime < currentThread.endTime) {
             ti.endTime = currentThread.endTime;
           }
           updateQueue(ti, sys, interactive, normal, batch);                   //Inserts in appropriate queue
         } else if (ti.endTime < currentThread.endTime){
           ti.endTime = currentThread.endTime;
           updateQueue(ti, sys, interactive, normal, batch);                   //Inserts in appropriate queue
         }
         if(!eventQueue.empty()) {
           while(eventQueue.top().endTime < currentThread.endTime || getQueue(ti, sys, interactive, normal, batch).empty()) {
             ti = eventQueue.top();
             if(ti.state == "NEW") {                                           //Threads have Arrived
               ti.state = "READY";
               getQueue(ti, sys, interactive, normal, batch).push(ti);
               eventQueue.pop();
             } else if(ti.state == "BLOCKED") {
               ti.state = "READY";
               Event e = {"IO_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from BLOCKED to READY"};
               ti.b.erase(ti.b.begin());                                           //Erases Bursts at index 0.
               ti.arrivalTime = ti.endTime;
               if(getQueue(ti, sys, interactive, normal, batch).empty() && !dispatch && currentThread.state != "RUNNING") {
                 idleTime += (ti.endTime - currentThread.endTime);
                 currentThread.endTime = ti.endTime;
                 if (!sys.empty()) {
                   ti = sys.top();
                 } else if(!interactive.empty()) {
                   ti = interactive.top();
                 } else if(!normal.empty()) {
                   ti = normal.top();
                 } else if(batch.empty()) {
                   ti = batch.top();
                 } else {

                 }
                 currentThread.threadType = ti.threadType;
                 currentThread.threadID = ti.threadID;
               }
               nextEvent.push(e);                                                  //Add to Verbose
               getQueue(ti, sys, interactive, normal, batch).push(ti);                                                //Add ready event to queue
               eventQueue.pop();
             }
             if(eventQueue.empty()) { break; }
           }
         }

         /*if(!sys.empty()) {                                                     //Demotion
           ti = sys.top();
           if((ti.b[0].cpuBurst) > (2 * quan) && !((ti.endTime - ti.arrivalTime) > (3 * quan))) {
             ti.threadType--;
             interactive.push(ti);
             sys.pop();
           }
         } else if(!interactive.empty()) {
           ti = interactive.top();
           if((ti.b[0].cpuBurst) > (2 * quan)) {
             ti.threadType--;
             normal.push(ti);
             interactive.pop();
           }
         } else if(!normal.empty()) {
           ti = normal.top();
           if((ti.b[0].cpuBurst) > (2 * quan)) {
             ti.threadType--;
             batch.push(ti);
             normal.pop();
           }
         }
         if (!batch.empty()) {                                                  //Aging
           ti = batch.top();
           if((ti.endTime - ti.arrivalTime) > quan) {
             ti.threadType++;
             normal.push(ti);
             batch.pop();
           }
         } else if (!normal.empty()) {                                                  //Aging
           ti = normal.top();
           if((ti.endTime - ti.arrivalTime) > quan) {
             ti.threadType++;
             interactive.push(ti);
             normal.pop();
           }
         } if (!interactive.empty()) {                                                  //Aging
           ti = interactive.top();
           if((ti.endTime - ti.arrivalTime) > quan) {
             ti.threadType++;
             sys.push(ti);
             interactive.pop();
           }
         }*/
       }
     }
   } else {
      if (algorithm != "fcfs"/* || algorithm != "first come first serve"*/) {
        cout << "Your algorithm provided cannot be determined or is invalid.\n Switching to default algorithm: FCFS." << endl;

      }
      priority_queue<Thread, vector<Thread>, Compare> readyQueue;               //Sets up my ready priority queue
      priority_queue<Thread, vector<Thread>, Compare2> eventQueue;              //Declares my event priority queue.

      for(size_t i = 0; i < p.size(); i++) {      //Puts my arrival events in the event queues
        Process pro = p[i];
        for(size_t j = 0; j < pro.t.size();j++) {  //For each thread in each process...
          Thread ti = pro.t[j];
          Event ev = {"THREAD_ARRIVED" , ti.threadID, ti.arrivalTime, ti.threadType, ti.ProcessId, "Transitioned from NEW to READY"};
          eventQueue.push(ti);                                                    //Inserts threads into my event queue
          nextEvent.push(ev);                                                    //Inserts events into the Verbose queue
        }
      }

      /*
        Processes the following events:
          ReadyQueue:
            DISPATCHER_INVOKED
            THREAD_PREEMPTED
            PROCESS_DISPATCH_COMPLETED
            THREAD_DISPATCH_COMPLETED
            CPU_BURST_COMPLETED
            THREAD_COMPLETED
          EventQueue:
            THREAD_ARRIVED
            IO_BURST_COMPLETED
      */
      //fcfs

      //Base Case
      Thread currentThread = eventQueue.top();
      currentThread.state = "READY";
      readyQueue.push(currentThread);
      currentThread.ProcessId = -1;
      eventQueue.pop();
      bool dispatch = false;

      while(!readyQueue.empty()) {
        Thread ti = {};
        if (!readyQueue.empty()) {                                              //Make sure the counter is chronological
          ti = readyQueue.top();
          if(ti.threadID == currentThread.threadID) {
            if(ti.state == "READY" && dispatch == false) {                  //No thread is running and/or does not exist
              Event e = {"DISPATCHER_INVOKED", ti.threadID, currentThread.endTime, ti.threadType, ti.ProcessId, "Selected from " + to_string(readyQueue.size()) + " threads; will run to completion of burst"};
              nextEvent.push(e);
              if(currentThread.ProcessId != ti.ProcessId) {                       //Process Dispatch
                ti.endTime = currentThread.endTime + processOverhead;
                dispatchTime += processOverhead;                                  //Adds to total Dispatch time
                Event e = {"PROCESS_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
                nextEvent.push(e);                                                //Add to Verbose
              } else {                                                            //Same Process different Thread, Thread dispatch
                ti.endTime = currentThread.endTime + threadOverhead;
                dispatchTime += threadOverhead;                                   //Adds to total Dispatch time
                Event e = {"THREAD_DISPATCH_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from READY to RUNNING"};
                nextEvent.push(e);                                                //Add to Verbose
              }
              if(!ti.received) {
                  ti.received = true;
                  processing.at(ti.threadType).ResTime += (ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime);
              }
              currentThread = ti;                                                 //Copy the currentThread as a reference
              dispatch = true;
              readyQueue.pop();
              readyQueue.push(ti);                                              //Replace the same thread.
          } else if(ti.state == "READY" && dispatch == true && ti.endTime == currentThread.endTime) {
              ti.state = "RUNNING";
              ti.endTime += ti.b[0].cpuBurst;
              readyQueue.pop();
              readyQueue.push(ti);                                                //Insert into eventQueue
              currentThread = ti;
          } else if(ti.state == "RUNNING") {                                  //Thread is running
              if(ti.b[0].IOBurst > -1) {                                        //There is an IO Burst
                Event e = {"CPU_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to BLOCKED"};
                ti.state = "BLOCKED";                                           //Transition to BLOCKED
                ti.endTime = ti.endTime + ti.b[0].IOBurst;                      //Update EndTime
                nextEvent.push(e);                                              //Add to Verbose
                eventQueue.push(ti);                                            //Insert into EventQueue
              } else {
                ti.state = "EXIT";
                int TRT =  ti.endTime - p2[ti.ProcessId].t.at(ti.threadID).arrivalTime;
                processing[ti.threadType].TRT += TRT;
                Event e = {"THREAD_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from RUNNING to EXIT"};
                nextEvent.push(e);                                                //Add to verbose queue
                p2[ti.ProcessId].t.at(ti.threadID).endTime = ti.endTime;
              }
              readyQueue.pop();
              if(!readyQueue.empty()) {
                ti = readyQueue.top();
                currentThread.threadID = ti.threadID;
              }
              dispatch = false;
            }
          } else if(ti.endTime < currentThread.endTime){
            ti.endTime = currentThread.endTime;
            readyQueue.pop();
            readyQueue.push(ti);
          }
          if(!eventQueue.empty()) {
            while(eventQueue.top().endTime < currentThread.endTime || readyQueue.empty()) {
              ti = eventQueue.top();
              if(ti.state == "NEW") {                                           //Threads have Arrived
                ti.state = "READY";
                readyQueue.push(ti);
                eventQueue.pop();
              } else if(ti.state == "BLOCKED") {
                ti.state = "READY";
                Event e = {"IO_BURST_COMPLETED", ti.threadID, ti.endTime, ti.threadType, ti.ProcessId, "Transitioned from BLOCKED to READY"};
                ti.b.erase(ti.b.begin());                                           //Erases Bursts at index 0.
                ti.arrivalTime = ti.endTime;
                if(readyQueue.empty()) {
                  idleTime += (ti.endTime - currentThread.endTime);
                  currentThread.threadID = ti.threadID;
                  currentThread.endTime = ti.endTime;
                }
                nextEvent.push(e);                                                  //Add to Verbose
                readyQueue.push(ti);                                                //Add ready event to queue
                eventQueue.pop();
              }
              if(eventQueue.empty()) { break; }
            }
          }
        }
      }
    }

  if(thread) {
    threads(p2);
  }
  if(verbose) {
    verbo(nextEvent);
  }

  //The Outputs
  cout << "SIMULATION COMPLETED!" << endl;
  for(int i = 0; i < 4; i++) {
    Output o = processing.at(i);
    switch(i) {
      case(0):
              cout << "SYSTEM ";
              break;
      case(1):
              cout << "INTERACTIVE ";
              break;
      case(2):
              cout << "NORMAL ";
              break;
      case(3):
              cout << "BATCH ";
              break;
    }
    cout << "THREADS:" << endl;
    double ResTime = 0.00;
    double TRT = 0.00;
    if (o.count > 0.00) {
      ResTime = ((double)o.ResTime/(double)o.count);
      TRT = ((double)o.TRT/(double)o.count);
    }
    printf("\t Total count: %34d\n",o.count);
    printf("\t Avg Response Time: %28.2f\n", ResTime);
    printf("\t Avg Turnaround Time: %26.2f\n", TRT);
  }
  double CPUUTil = 0.00;
  double CPUEff = 0.00;
  totalTime = cpuTime + dispatchTime + idleTime;                                //Calculate totalTime
  //if (totalTime > 0) {
  CPUUTil = (double)(((double)(cpuTime + dispatchTime)/(double)(totalTime))*100);   //Calculate CPUUtilization
  CPUEff = (double)(((double)(cpuTime)/(double)(totalTime))*100);                   //Calculate CPU Efficiency
  //}
  printf("Total elapsed time: %36d\n", totalTime);
  printf("Total service time: %36d\n", cpuTime);
  printf("Total I/O time: %40d\n", IOTime);
  printf("Total dispatch time: %35d\n", dispatchTime);
  printf("Total idle time: %39d\n\n", idleTime);
  printf("CPU utilization: %39.2f\n", CPUUTil);
  printf("CPU efficiency: %40.2f\n", CPUEff);
  return;
}

int main(int argc, char* const argv[]) {
  cout << endl;
  //cout << "WORK";
  if ((string)argv[0] == "./simulator") {                            //First argument is simulator
    //Flags
    if ((string)argv[1] == "-h" || (string)argv[1] == "--help"){     //Help flag
      help();
      return 0;
    }
    //Settings Boolean                                 //Default Settings
    bool thread = false;                               //per-thread Statistics settings
    bool verbose = false;                              //Verbose settings
    bool flagsExists = false;                            //Are there any flags
    string algorithm = "FCFS";                         //Default algorithm
    string file = "";
    //Flag Processing
    int flag_length = ((string)argv[1]).length();        //Converts the flags string into a char array
    char flags[flag_length + 1];
    strcpy(flags, ((string)argv[1]).c_str());

    if ((string)argv[1] == "--per_thread") {                                  //output file
        thread = true;
    }
    if ((string)argv[1] == "--verbose"){
        verbose = true;
    }
    if ((string)argv[1] == "--algorithm" || (string)argv[1] == "-a") {
        algorithm = (string)argv[2];
        file = (string)argv[3];
    }
    //Iterates over flag characters
    if (flags[0] == '-' && flags[1] != '-') {
      flagsExists = true;
      for (int i = 1; i < flag_length; i++) {
        switch(flags[i]) {
          case 't':
            thread = true;
            file = (string)argv[2];
            break;
            case 'v':
            verbose = true;
            file = (string)argv[2];
            break;
            case 'a':
            algorithm = (string)argv[2];
            break;
            case 'h':
            help();
            break;
          }
        }
    } else {flagsExists = false;}
    if(algorithm > "") {file = (string)argv[3];}
    if(!thread && !verbose && ((string)argv[1] != "--algorithm") && flagsExists == false) {
      file = (string)argv[1];
    }
    //cout << file << endl;
    Task_Schedule_Simulator(file, thread, verbose, algorithm);
  }
  return 0;
}
