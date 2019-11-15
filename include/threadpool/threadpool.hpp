#pragma once

#include "threadpool/list_mt.hpp"

#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <functional>

namespace plane_render {

class ThreadPool
{
public:
    typedef std::function<void()> FunctionType;
    typedef std::pair<FunctionType, bool> TaskWrapper; // (таска; взята ли в работу)

public:
    ThreadPool(size_t n_threads);

    // Обе функции - с передачей владения
    void AddTask(const FunctionType&& task, bool join);
    void AddTasks(const std::vector<FunctionType>& tasks, bool join);

    void Join();

private:
    bool IsEmptyTasks();

    void FinishAddTasks(bool join);
    void ThreadFunction(size_t id);

private:
    ListMT<TaskWrapper> tasks_;

    std::vector<std::thread> threads_;
    std::vector<std::mutex> task_mutices_;
    std::condition_variable new_tasks_;

    std::mutex cv_join_mutex_;
    std::condition_variable cv_join_;
};

} // namespace plane_render