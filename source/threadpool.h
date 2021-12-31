#pragma once

#include <functional>
#include <queue>
#include <list>

#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <condition_variable>

#include "taskqueue.h"

namespace utility {
    
    class ThreadPool {
    public:
        using Task = std::function<void()>;

        template <typename T>
        using AsyncTask = std::packaged_task<T()>;

    public:

        ThreadPool(int capacity);
        ~ThreadPool();

        static int GetConcurrency();

        void adjust(int capacity);
        void addThread();
        void addThreads(int nThread);

        template <typename F, typename ...Ts>
        void runTask(const F& f, Ts... args) {
            runTask([&]{f(args...);});
        }
        void runTask(Task&& task);
        void runTasks(const std::vector<Task>& tasks);

        template <typename F, typename ...Ts>
        auto runAsyncTask(const F& f, Ts... args) -> std::future<decltype(f(args...))> {
            using retType = decltype(f(args...));
            return runAsyncTask(std::packaged_task<retType()>([&]{
                return f(args...);
            }));
        }

        template <typename T>
        auto runAsyncTask(const AsyncTask<T>&& asyncTask) {
            addTask([&asyncTask]{
                asyncTask();
            });

            return asyncTask.get_future();            
        }

        template <typename T>
        auto runAsyncTask(const std::vector<AsyncTask<T>>& asyncTasks) {
            std::vector<std::future<T>> rets;
            std::vector<Task> tasks;
            for (auto asyncTask : asyncTasks) {
                tasks.push_back([&asyncTask]{ asyncTask();});
                rets.push_back(asyncTask.get_future);
            }
            runTasks(tasks);
            return rets;
        }

        void waitAll();

    private:
        int _concurrency;
        bool _valid;

        std::mutex _mutex;
        std::condition_variable _condvar;
        std::list<std::thread> _pool;
        TaskQueue _taskQueue;
    };

    static ThreadPool& threadpool();

}