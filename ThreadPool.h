//
// Created by 唐仁初 on 2022/1/22.
//

/*
高级线程池通常具备以下几个模块：

- 共享任务队列：用户将需要被运行的任务投入到共享任务队列中等待被调用。

- 期望队列：期望队列与共享任务队列中的任务绑定，用户从此处获取运行结果。

- 线程池：集中构造、析构和管理的一组线程，在线程空闲时使用 yield 或者尝试任务窃取。

- 本地任务队列：线程池中的线程将共享任务队列拷贝到本地，避免因任务量过小导致乒乓缓存。

- 任务窃取：当线程无任务可以运行时，可以尝试从其他线程的本地任务队列中窃取。
*/

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <functional>
#include <thread>
#include <mutex>
#include <utility>
#include <vector>
#include <future>
#include <queue>


using Res = std::string;    /// 任务的返回类型
using Functor = std::function<Res()>;   /// 任务类型

class ThreadPool;

class ThreadTask {
private:
    Functor functor;
    Res result;
    std::mutex mtx;
    std::condition_variable cond;
    bool finished;
public:
    explicit ThreadTask(Functor func);

    ThreadTask(ThreadTask &&original) noexcept;

    void runTask();

    Res getResult();

    ThreadTask(const ThreadTask &) = delete;

    auto operator=(const ThreadTask &) = delete;
};

using TaskPtr = std::shared_ptr<ThreadTask>;
using TaskPack = std::pair<int,TaskPtr>;

class Thread {
private:
    std::thread thread;
    std::vector<TaskPtr> tasks;
    bool running;
    ThreadPool *owner;

public:
    bool getSomeTasks();

    explicit Thread(ThreadPool *threadPool);

    ~Thread() noexcept;

    Thread(const Thread &) = delete;

    auto operator=(const Thread &) = delete;
};

/// 从结果类中返回结果，记得及时释放资源
class TaskResult {
private:
    std::shared_ptr<ThreadTask> task;
public:
    explicit TaskResult(std::shared_ptr<ThreadTask> task) : task(std::move(task)) {}

    Res getResult() {
        return task->getResult();
    }
};

/// 用户将一个仿函数加入线程池中，线程池返回一个结果类
class ThreadPool {
private:
    std::condition_variable cond;
    std::mutex func_mtx;
    std::priority_queue<TaskPack,std::vector<TaskPack>,std::less<>> tasks;
    std::vector<std::unique_ptr<Thread>> threads;

    friend Thread::Thread(ThreadPool *threadPool);

    friend bool Thread::getSomeTasks();

    void waitForTask();

    std::vector<TaskPtr> assignTask();

public:
    explicit ThreadPool(int thread_nums);

    /// 优先级越高，越容易被调度
    std::shared_ptr<TaskResult> addTask(Functor functor,int priority = 0);

    ~ThreadPool() = default;

    ThreadPool(const ThreadPool &) = delete;

    auto operator=(const ThreadPool &) = delete;
};


#endif //THREADPOOL_THREADPOOL_H
