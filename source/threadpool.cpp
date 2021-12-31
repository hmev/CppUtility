#include "threadpool.h"

namespace utility {

    ThreadPool::ThreadPool(int concurrency) {
        _concurrency = concurrency;
        adjust(concurrency);
    }

    ThreadPool::~ThreadPool() {
        _valid = false;
        _condvar.notify_all();

        for (auto&& thread: _pool) {
            if (thread.joinable()){
                thread.join();
            }
        }
    }

    int ThreadPool::GetConcurrency() {
        return std::thread::hardware_concurrency();
    }

    void ThreadPool::adjust(int capacity) {
        int newthreadnum = capacity - _pool.size();

        if (newthreadnum > 0) {
            addThreads(newthreadnum);
        }
    }

    void ThreadPool::addThread() {
        addThreads(1);
    }

    void ThreadPool::addThreads(int nThread) {
        for (int i = 0; i < nThread; i++) {
            _pool.emplace_back([this]() {
                std::unique_lock<std::mutex> ul(_mutex);
                _condvar.wait(ul, []{return true;});
                while (true) {
                    std::unique_lock<std::mutex> ul(_mutex);
                    _condvar.wait(ul, [this]{!_valid || !_taskQueue.empty();});

                    _taskQueue.getTask()();
                }
            });
        }

        _condvar.notify_all();
    }

    void ThreadPool::runTask(Task&& task) {
        _taskQueue.addTask(task);
        waitAll();
    }

    void ThreadPool::runTasks(const std::vector<Task>& tasks) {
        _taskQueue.addTasks(tasks);
        waitAll();
    }

    void ThreadPool::waitAll() {
        std::unique_lock<std::mutex> ul(_mutex);
        _condvar.wait(ul, [this]{ return _taskQueue.empty(); });
    }

    static ThreadPool impl(ThreadPool::GetConcurrency());

    // static 
    ThreadPool& threadpool() {
        return impl;
    }   
}