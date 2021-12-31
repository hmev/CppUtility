#pragma once

#include <functional>
#include <queue>

namespace utility {
    using uint = unsigned int;

    class TaskQueue {
    public:
        using Task = std::function<void()>;
        template <typename T>
        using AsyncTask = std::packaged_task<T()>;

    public:
        TaskQueue();
        ~TaskQueue();

        template <typename F, typename ...Ts>
        void addTask(const F& f, Ts... args){
            addTask(Task([&]{f(args...);}));
        }
        void addTask(Task&& task);

        template <typename F, typename ...Ts>
        void addTasks(F&& mapTask, uint taskNum, Ts... args) {
            for (auto i = 0; i < size; i++) {
                addTask(Task([&, i]{ mapTask(i)(args...); });
            }
        }
        void addTasks(const std::vector<Task>& tasks);

        Task getTask();

    public:
        bool empty() { return _queue.empty(); }

    private:
        std::queue<Task> _queue;
        std::mutex _mutex;
    };

}