//
// Created by 唐仁初 on 2022/1/22.
//

#include "ThreadPool.h"

ThreadTask::ThreadTask(Functor func) : functor(std::move(func)), result(), finished(false) {}

ThreadTask::ThreadTask(ThreadTask &&original) noexcept
        : functor(std::move(original.functor)),
          result(original.result),
          finished(original.finished) {}

Res ThreadTask::getResult() {
    std::unique_lock<std::mutex> lk(mtx);
    cond.wait(lk, [this]() { return finished; });
    return result;
}

void ThreadTask::runTask() {
    std::unique_lock<std::mutex> lk(mtx);
    result = functor();
    finished = true;
    cond.notify_one();
}


ThreadPool::ThreadPool(int thread_nums) {
    while (thread_nums--) threads.emplace_back(new Thread(this));
}

std::vector<std::shared_ptr<ThreadTask>> ThreadPool::assignTask() {
    std::unique_lock<std::mutex> lk(func_mtx);
    if (tasks.empty())
        return {};
    std::vector<std::shared_ptr<ThreadTask>> res;

    for (int i = 0; i < 3; i++) {
        if (tasks.empty())
            return res;
        res.emplace_back(tasks.top().second);
        tasks.pop();
    }
    return res;
}

void ThreadPool::waitForTask() {
    std::unique_lock<std::mutex> lk(func_mtx);
    cond.wait(lk, []() { return true; });
}

std::shared_ptr<TaskResult> ThreadPool::addTask(Functor functor, int priority) {
    auto task = std::make_shared<ThreadTask>(std::move(functor));
    {
        std::unique_lock<std::mutex> lk(func_mtx);
        tasks.emplace(priority, task);
        cond.notify_all();
    }
    return std::make_shared<TaskResult>(task);
}

bool Thread::getSomeTasks() {
    auto assigned_tasks = owner->assignTask();
    if (assigned_tasks.empty())
        return false;
    std::swap(tasks, assigned_tasks);
    return true;
}

Thread::Thread(ThreadPool *threadPool) : running(true), owner(threadPool) {
    thread = std::thread([this]() {
        while (running) {
            if (tasks.empty()) {
                if (!getSomeTasks()) {
                    owner->waitForTask();
                }
            } else {
                tasks.back()->runTask();
                tasks.pop_back();
            }
        }
    });
}

Thread::~Thread() noexcept {
    running = false;
    thread.join();
}