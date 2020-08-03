/*
  Author: Calvin Mak
  Course: CSCI 442: Operating Systems
  Assignment: Project 1
  Date: 9/28/2018
  Desc: Sets up Thread Struct
*/


using namespace std;

struct Bursts {
  int cpuBurst;
  int IOBurst;              //If ioBurst == null, terminate thread (thread ends)
};
