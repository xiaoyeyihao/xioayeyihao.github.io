#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <functional>
#include <sys/time.h>

#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
using namespace std;
#define TNOWMS    getNowMs()
class Threadpool
{


protected:
    struct TaskFunc
    {
        TaskFunc(uint64_t expireTime):_expireTime(expireTime){}

        std::function<void()> _func;

        int64_t  _expireTime = 0;
    };
    typedef shared_ptr<struct TaskFunc> TaskFuncPtr;

private:
    /* data */
public:
    Threadpool(/* args */);
    virtual ~Threadpool();
    bool init(size_t num);//初始化线程个数
    bool  start();
    void run()
    {
        int64_t start_time = getnow

    }
    void stop();
    bool get(TaskFuncPtr &task) 
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if(_task.empty()) {
            _condition.wait(lock,[this] {

                return _bTerminate || !_task.empty();

            }
            );
        }

        if(_bTerminate)return false;
        
        if(!_task.empty()) {
            task = std::move(_task.front());
            _task.pop();
            return true;
        }
        return false;
        

    }
    bool isTerminate() {return _bTerminate;}
    bool waitForAllDone(int millsecond = -1);
protected:
    queue <TaskFuncPtr>       _task;
    std::vector   <std::thread *> _threads;
    std::mutex                _mutex;
    std::condition_variable   _condition;
    size_t                    _threadNum;
    bool                      _bTerminate;
    std::atomic<int>          _atomic{  0 };
     
};



#endif // ZERO_THREADPOOL_H