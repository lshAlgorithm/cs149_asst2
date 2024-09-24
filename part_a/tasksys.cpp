#include "tasksys.h"
#include <vector>
#include <iostream>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): 
    ITaskSystem(num_threads),
    num_threads(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //


/*
    Idiot implementation. Static assignment
*/
    std::vector<std::thread> t;
    for (int i = 0; i < num_total_tasks; i += num_threads) {
        for (int j = 0; j < num_threads && i + j < num_total_tasks; j++) {
            t.emplace_back([&, i, j]() {    // ensure i, j are captured when the lambda expression(thread) is *initialized*
                runnable->runTask(i + j, num_total_tasks);
            });
        }
        for (auto &c: t) {
            c.join();
        }
        t.clear();
    }
    // Q: Where and when is the thread literally launched?
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}


TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): 
    ITaskSystem(num_threads),
    num_threads(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    threadPool = new std::thread[num_threads];
    // thread_stat = new ThreadState(num_threads);
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    // delete thread_stat;
    delete []threadPool;
}




void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

/*
    Seems not meet the spinning demand, but I think it performs good.
*/
    int* finished_task = new int;
    *finished_task = 0;
    std::mutex* mutex_ = new std::mutex;
    for (int i = 0; i < num_threads; i++) {
        threadPool[i] = std::thread([&]() {
            mutex_->lock();
            while (*finished_task < num_total_tasks) {
                int to_do = *finished_task;
                *finished_task += 1;
                mutex_->unlock();

                runnable->runTask(to_do, num_total_tasks);

                mutex_->lock();
            }
            mutex_->unlock();
        });
    }

    for (int i = 0; i < num_threads; i++) {
        threadPool[i].join();
    }

    delete finished_task;
    delete mutex_;
}


TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */


TaskState::TaskState():
    done_(0) ,
    num_total_task_(0),
    task_distribute_(0)
{
    this->mutex_ = new std::mutex();
    this->finishAll_ = new std::condition_variable();
    this->finishMutex_ = new std::mutex();
}

TaskState::~TaskState() {
    delete mutex_;
    delete finishAll_;
    delete finishMutex_;
}

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): 
    ITaskSystem(num_threads) ,
    killed_(false),     // you should ensure that killed_ is read-only for threads
    num_threads_(num_threads)
    {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    task_stat_ = new TaskState();
    threadPool_ = new std::thread[num_threads];

    for (int i = 0; i < num_threads; i++) {
        // Use a class function in a thread
        threadPool_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::spinThread, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    killed_ = true;     // only main thread can modify the value.
    for (int i = 0; i < num_threads_; i++) {
        threadPool_[i].join();
    }
    delete []threadPool_;
    delete task_stat_;
}

#if(0)
void TaskSystemParallelThreadPoolSleeping::spinThread() {
    /*
        spinning here in search of new mission, or notify that the sleeping thread should be waken up.
    */
    int num_finish, total;
    while (true) {
        if (killed_) break;

        TaskState &t = *task_stat_;

        t.mutex_->lock();

#if(0)
        /*
         Fool and *wrong* implementation
        */
        while (t.num_total_task_ == -1) {
            t.mutex_->unlock();
            t.mutex_->lock();
        } // busy waiting.
#endif

        t.done_++;              // Without it, you will do more work.
        num_finish = t.done_;
        total = t.num_total_task_;
        t.mutex_->unlock();

        if (num_finish < total) {
            t.runnable_->runTask(num_finish, total);
        } else {
            // Sleep...
        }

        if (num_finish == total - 1) {
            // The mission has been *distrubuted* rather than finish.


            // PKU says it is necessary to force the run() thread (i.e. main thread) to sleep
            /*
                More details:
                    A lock acquirement ensures that the run() is not to wake up *prematurely*, that
                    is to say that the main process hasn't reached wait and release the mutex. If so,
                    it will be an chaos.
                    In essence, it serves as a synchronization point that ensures the main thread 
                    waits appropriately for the completion of all tasks.
            */
            t.finishMutex_->lock();
            t.finishMutex_->unlock();

            t.finishAll_->notify_all();
        }
    }
}
#else
void TaskSystemParallelThreadPoolSleeping::spinThread() {
    int num_finish, total;
    while (true) {
        if (killed_) 
            break;

        TaskState &t = *task_stat_;

        t.mutex_->lock();

        total = t.num_total_task_;
        num_finish = t.task_distribute_;

        if (t.task_distribute_ < total) 
            t.task_distribute_++; 

        t.mutex_->unlock();

        if (num_finish < total) {
            t.runnable_->runTask(num_finish, total);


            t.mutex_->lock();
            t.done_++;
            if (t.done_ == total) {
                t.mutex_->unlock();
                // The mission has been *finished* rather than distrubute.


                // PKU says it is necessary to force the run() thread (i.e. main thread) to sleep
                /*
                    More details:
                        A lock acquirement ensures that the run() is not to wake up *prematurely*, that
                        is to say that the main process hasn't reached wait and release the mutex. If so,
                        it will be an chaos.
                        In essence, it serves as a synchronization point that ensures the main thread 
                        waits appropriately for the completion of all tasks.
                */
                t.finishMutex_->lock();
                t.finishMutex_->unlock();

                t.finishAll_->notify_all();
            } else {
                t.mutex_->unlock();
            }

            
        } else {
            // Sleep...
        }

    }
}
#endif
void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // Sleep the idle main thread

    std::unique_lock<std::mutex> lk(*(task_stat_->finishMutex_));
    task_stat_->mutex_->lock();
    task_stat_->done_ = 0;
    task_stat_->task_distribute_ = 0;
    task_stat_->num_total_task_ = num_total_tasks;
    task_stat_->runnable_ = runnable;
    task_stat_->mutex_->unlock();


    task_stat_->finishAll_->wait(lk);
    lk.unlock();
    // Q: Which thread runs you? At least three threads work on this func. See in test.h:553
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
