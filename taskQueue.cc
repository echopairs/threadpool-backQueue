#include "taskQueue.h"


TaskQueueManager::TaskQueueManager()
{
    pthread_mutex_init(&mutex_,NULL);
}

TaskQueueManager::~TaskQueueManager()
{
    pthread_mutex_lock(&mutex_);
    for (size_t i = 0; i < datas_.size(); i++)
    {
        DataObj *tmp = NULL;
        tmp = datas_.front();
        datas_.pop();
        if (NULL != tmp)
        {
            delete tmp;
            tmp = NULL;
        }
    }
}

// push data to queue
void TaskQueueManager::push(DataObj *t)
{
    pthread_mutex_lock(&mutex_);
    datas_.push(t);
    pthread_mutex_unlock(&mutex_);
}

// pop DataObj from queue
DataObj* TaskQueueManager::pop()
{
    pthread_mutex_lock(&mutex_);
    DataObj *tmp = NULL;
    if (!datas_.empty())
    {
        tmp = datas_.front();
        datas_.pop();
    }
    pthread_mutex_unlock(&mutex_);
    return tmp;
}


// DataProcessManager function

DataProcessManager::DataProcessManager(int thread_num, TaskQueueManager* itk):
                                        workThreadNum_(thread_num),
                                        Itk_(itk)
                                        {}
DataProcessManager::~DataProcessManager()
{
    
}

// start process,(create thread and work)
void DataProcessManager::start()
{
    pthread_t thrManger;
    pthread_create(&thrManger, NULL, managerThreadFunc, (void*)this);
}

// mangerThreadFunc: create/destory/check/update workThread 

void * DataProcessManager::managerThreadFunc(void *arg)
{
    DataProcessManager *processer = (DataProcessManager*)arg;
    // get work threadNum
    size_t workThreadNum = processer->workThreadNum_;  

    // workThread map, key:pthreadId, Value:thread OBJ
    std::map<pthread_t,WorkPthread*>KpidVPthreadMap; 
    int err = 0;

    // init workThread
    err = processer->initWorkThreadMap(KpidVPthreadMap, workThreadNum);
    if(err)
    {
        cout << "initWorkThreadMap failed" << endl;
        cout << strerror(errno);
        //return;
    }

    // do loop, 1.get workThreadNum, now the workThreadNum not change
    // 2. update workThreadMap with new workThreadNum
    // 3. check workThreadStatus, if dead, try create again

    while(true)
    {
        // int workThreadNum = arg->workThreadNum;
        int err = 0;
        err = processer->checkProcessThreadStatus(KpidVPthreadMap);
        if(!err)
        {
            // no thread kill
        }
    }
    return 0;
}



int DataProcessManager::initWorkThreadMap(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap, size_t thread_num)
{
    for (size_t i = 0; i < thread_num; i++)
    {
        int err = 0;
        err = addWorkPthreadToMap(KpidVPthreadMap);
        if(err)
        {
            cout << "addWorkPthreadToMap failed" << endl;
            return -2;
        }
    }
    return 0;
}


int DataProcessManager::addWorkPthreadToMap(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap)
{
    try
    {
        WorkPthread *processerObj = new WorkPthread;
        // work thread is running
        processerObj->running_ = 1;
        // get data interface
        processerObj->taskQueueMgr_ = Itk_;

        int err = 0;
        err = createProcessThread(processerObj);
        if(err)
        {
            cout << "addWorkPthreadToMap::createProcessThread failed" << endl;
            return -2;
        }
        KpidVPthreadMap[processerObj->pthreadId_] = processerObj;
        return 0;
    }
    catch(...)
    {
        cout << "addWorkPthreadToMap failed" << endl;
    }
    return 0;
}


// cheack all thread status

int DataProcessManager::checkProcessThreadStatus(std::map<pthread_t,WorkPthread*>&KpidVPthreadMap)
{
    std::map<pthread_t, WorkPthread*>::iterator  mapItr;

    for (mapItr = KpidVPthreadMap.begin(); mapItr != KpidVPthreadMap.end(); ++mapItr)
    {
        pthread_t id;
        int err = 0;
        id = (*mapItr).first;
        err = judgePthreadStatus(id);
        if (err)
        {
            createProcessThread((*mapItr).second);
            KpidVPthreadMap[(*mapItr).second->pthreadId_] = (*mapItr).second;
            KpidVPthreadMap.erase(mapItr);  
        }
    }
    return 0;
}

// create Process Thread

int DataProcessManager::createProcessThread(WorkPthread *StrPthread)
{
    int err = 0;
    err = pthread_create(&(StrPthread->pthreadId_), NULL, processThreadFunc, (void*)StrPthread);

    if (err)
    {
        cout << "createProcessThread failed" << endl;
        return -2;
    }
    usleep(1000);
    return 0;
}



// judge all thread status

int DataProcessManager::judgePthreadStatus(pthread_t tid)
{
    int pthread_kill_err = 0;
    pthread_kill_err = pthread_kill(tid, 0);

    if (ESRCH == pthread_kill_err)
    {
        cout << "judgePthreadStatu: thread already kill or exit" << endl;
        return -2;
    }
    else if(pthread_kill_err == EINVAL)
    {
        cout << "judgePthreadSttu: send signal illegal" << endl;
        return -2;
    }
    return 0;
}

// data workThread function
void *DataProcessManager::processThreadFunc(void *arg)
{
    WorkPthread *pthread = (WorkPthread*)arg;
    TaskQueueManager *taskQueueMgr = pthread->taskQueueMgr_;

    // To Do 
    // do some initial work, e.g: yuv handle
    sleep(1);
    DataObj *obj;
    try
    {
        while(pthread->running_)
        {
            // get data
            // analysis
            // call callback function to processer data after analysis
            obj = taskQueueMgr->pop();
            if (NULL == obj)
            {
                cout << "the obj is null" << endl;
                usleep(100);
                continue;
            }

            // TODO process

            // TODO After process
            obj->callBack(obj->data_);

            if (obj)
            {
                // release obj member ,then release obj
                // TO DO relase obj's member first
                delete obj;
                obj = NULL;
            } 
        }
    }
    catch(...)
    {
        if(obj)
        {
            delete obj;
            obj = NULL;
        }   
    }
    return NULL;
}
