/*********************************************************************
  Copyright (C), 1990-2016, HangZhou RED HEAT Tech. Co., Ltd.
  FileName: taskQueue.h
  Author  : pairs & 736418319@qq.com
  Version : 1.0
  Date    : 2016/04/25
  Description:
  Function List:
  History:
      <author>    <time>    <version>    <desc>
        pairs     16/04/25      1.0     build this moudle
*********************************************************************/

#ifndef  __TASK_QUEUE_H__
#define  __TASK_QUEUE_H__

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <queue>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
using namespace std;

// forward declaration 
class TaskQueueManager;

// data element

typedef struct __DataObj
{
    // now data want to process is a int 
    int data_;
    // call back function, tell how to process data, now is int
    void (*callBack)(int i);
}DataObj;


typedef struct __WorkPthread
{
    // thread id
    pthread_t pthreadId_;
    // control thread
    int running_;
    // interface to get date element form queue
    TaskQueueManager *taskQueueMgr_;
}WorkPthread;

// class for mgr queue, provide two interface pop and push
class TaskQueueManager
{
    public:
      TaskQueueManager();
      ~TaskQueueManager();
      DataObj *pop();
      void push(DataObj *);
    private:
      pthread_mutex_t mutex_;
      std::queue<DataObj*>datas_;
};


class DataProcessManager
{
public:
    DataProcessManager(int , TaskQueueManager*);
    int initlize();
    void start();
    ~DataProcessManager();
private:
  int judgePthreadStatus(pthread_t tid);
  int createProcessThread(WorkPthread *);

  // operate workThread map
  int addWorkPthreadToMap(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap);
  int initWorkThreadMap(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap, size_t thread_num);
  int checkProcessThreadStatus(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap);

  // thread function
  
  static void* managerThreadFunc(void *arg);
  static void* processThreadFunc(void *arg);

  size_t workThreadNum_;
  TaskQueueManager *Itk_;

};

#endif
