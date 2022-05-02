#include "Threadpool.h"
Threadpool::Threadpool(/* args */):_threadNum(1),_bTerminate(false)
{
}

Threadpool::~Threadpool()
{
    //stop();
}
int gettimeofday(struct timeval &tv)
{
#if WIN32
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm. tm_isdst  = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;

    return 0;
#else
    return ::gettimeofday(&tv, 0);
#endif
}
void Threadpool::run() {
    int64_t start_time = getNowMs();

    while(!isTerminate()) {
        TaskFuncPtr task;
        bool ok = get(task); 
        if(ok) {
            ++ _atomic;
            try
            {
                if(task->_expireTime != 0 && task->_expireTime < TNOWMS) {

                }
                else
                {
                    task->_func();
                }
            }
            catch(...)
            {
                std::cerr << e.what() << '\n';
            }
            --atomic;

            std::unique_lock<std::mutex> lock(_mutex);
            if(_atomic == 0 && _task.empty()) {
                cout << "nead time :"<< getNowMs() - start << "ms\n";
                _condition.notify_all(); // 这里只是为了通知waitForAllDone


            }

            


        }




    }

}

// 等待所有的任务执行完毕，包括任务队列的，但是这里是可以带超时的，如果超时了也还没有执行完毕，那我就不等了。
bool Threadpool::waitForAllDone(int millsecond) {
    std::unique_lock<std::mutex> lock(_mutex);
    if(_task.empty()) {
        return true;
    }
    if(millsecond < 0) {
        _condition.wait(lock,[this]
            {
                return _task.empty();
                
            };
        )
    }else{

        return _condition.wait_for(lock,std::chrono::microseconds(millsecond),[this]
            {
                return _task.empty();
            }
        
        );
    }
}

int64_t getNowMs() 
{
    struct timeval tv;
    gettimeofday(tv);
    return tv.tv_sec *1000 +tv.tv_usec / 1000;

}

bool Threadpool::init(size_t num) {
    if(!_threads.empty()) {
        return false;
    }
    _threadNum = num;
    return true;
}
bool Threadpool::start() {
    std::unique_lock<std::mutex> lock(_mutex);
    if(!_threads.empty()) {
        return false;
    }
    for(size_t i = 0; i < _threadNum;i++) {
        _threads.push_back(new thread(&Threadpool::run,this));
    }
    return true;

}
void Threadpool::stop() {
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _bTerminate = true;
    _condition.notify_all();
  }

  for(size_t i = 0;i < _threads.size();i ++) {

      if(_threads[i]->joinable())
      {
          _threads[i]->join();
      }
      delete _threads[i];
      _threads[i] = NULL;
    }
    std::unique_lock<std::mutex> lock(_mutex);
    _threads.clear();
}




