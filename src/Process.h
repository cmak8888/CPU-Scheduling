/*
  Author: Calvin Mak
  Course: CSCI 442: Operating Systems
  Assignment: Project 1
  Date: 9/28/2018
  Desc: Setups Struct Process
*/

#include "Thread.h"

using namespace std;

struct Process {
  int id;
  int type;     //SYSTEM = 0, INTERACTIVE = 1, NORMAL = 2, BATCH = 3
  int threadCount;
  vector<Thread> t;
};
