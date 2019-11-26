#include "threadpool.hpp"

namespace plane_render {

ThreadPool::ThreadPool(size_t n_threads) :
    task_mutices_(n_threads)
{
    for (size_t i = 0; i < n_threads; i++)
    {
        threads_.emplace_back(std::bind(&ThreadPool::ThreadFunction, this, i));
    }
}

void ThreadPool::AddTask(const FunctionType&& task, bool join)
{
    tasks_->emplace_back(std::move(task), false);
    FinishAddTasks(join);
}

void ThreadPool::AddTasks(const std::vector<FunctionType>& tasks, bool join)
{
    for (auto& t : tasks)
    {
        tasks_->emplace_back(std::move(t), false);
    }
    FinishAddTasks(join);
}

void ThreadPool::FinishAddTasks(bool join)
{
    new_tasks_.notify_all();
    if (join)
        Join();
}

void ThreadPool::Join()
{   
    if (tasks_->empty())
        return;

    while (true)
    {
        std::unique_lock<std::mutex> lock_cv(cv_join_mutex_);
        cv_join_.wait(lock_cv);

        if (tasks_->size() == 0)
            return;
    }
}

void ThreadPool::ThreadFunction(size_t id)
{
    while (true)
    {
        // Ждем пробуждения (и сразу освобождаем мьютекс)
        if (tasks_->empty())
        {
            std::unique_lock<std::mutex> cv_lock(task_mutices_[id]);
            new_tasks_.wait(cv_lock);
        }

        auto accessor = tasks_.GetAccessor();
        if (accessor->size() == 0)
            continue; // Пока нет заданий, случайное пробуждение

        auto tasks_it = accessor->begin();
        for (; tasks_it != accessor->end(); tasks_it++)
        {
            if (!tasks_it->second)
            {
                tasks_it->second = true;
                break;
            }
        }
        if (tasks_it == accessor->end())
            continue; // Все задания разобраны, ждем новых
        accessor.Release(); // Снимаем блокировку

        // Выполняем функцию
        tasks_it->first();

        // Удаляем выполненное задание
        auto accessor2 = tasks_.GetAccessor(); // Нужен новый акцессор
        accessor2->erase(tasks_it);
    
        // Разрешаем ждущему продолжить, если все выполнили
        if (accessor2->size() == 0) // Все выполнили: отпускаем очередь и продолжаем
        {
            accessor2.Release();
            cv_join_.notify_all();
        }
        else // Просто отпускаем очередь
        {
            accessor2.Release();
        }
    }
}

} // namespace plane_render
