/*
  Author: Calvin Mak
  Course: CSCI 442: Operating Systems
  Assignment: Project 1
  Date: 9/28/2018
  Desc: Sets up Thread Struct
*/

#include "Bursts.h"

using namespace std;

struct Thread {
  string state;           //NEW, READY, RUNNING, BLOCKED, EXIT
  int threadID;
  int arrivalTime;
  int endTime;            //Time Tracker
  int burstCount;         //Number of Bursts;
  int threadType;         //Same as ProcessType;
  int ProcessId;          //ProcessID that the thread belongs to.
  bool received;          //Checks for first response.
  vector<Bursts> b;       //Read-Only for FCFS
  /*Thread operator=(const Thread &t) {
    vector<Bursts> g;
    for (int i = 0; i < burstCount; i++) {
      g[i] = t.b[i];
    }
    Thread a = {
      t.state,
      t.threadID,
      t.arrivalTime,
      t.endTime,
      t.burstCount,
      t.threadType,
      t.ProcessId,
      t.received,
      g
    };
    return a;
  }*/
};
