#include "taskqueue.h"

#include <mutex>

namespace utility {

    TaskQueue::TaskQueue() {

    }

    TaskQueue::~TaskQueue() {

    }

    void TaskQueue::addTask(Task&& task) {
        // TODO.
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(task);
    }

    void TaskQueue::addTasks(const std::vector<Task>& tasklist) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto task : tasklist)
            addTask(task);   
    }

    TaskQueue::Task TaskQueue::getTask() {
        std::lock_guard<std::mutex> lock(_mutex);
        auto task = std::move(_queue.front());
        _queue.pop();

        return task;
    }

}