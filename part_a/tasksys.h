#ifndef _TASKSYS_H
#define _TASKSYS_H

#define LSH

#include "itasksys.h"

#include <condition_variable>
#include <mutex>
#include <thread>


/*
 * Wrapper class around a counter, a condition variable, and a mutex.
 */
class ThreadState {
    public:
        std::condition_variable* condition_variable_;
        std::mutex* mutex_;
        int counter_;
        int num_waiting_threads_;
        ThreadState(int num_waiting_threads) {
            condition_variable_ = new std::condition_variable();
            mutex_ = new std::mutex();
            counter_ = 0;
            num_waiting_threads_ = num_waiting_threads;
        }
        ~ThreadState() {
            delete condition_variable_;
            delete mutex_;
        }
};

class TaskState {
    public:
        TaskState();
        ~TaskState();
        int done_;
        int num_total_task_;
        int task_distribute_;
        IRunnable* runnable_;
        std::mutex* mutex_;
        std::condition_variable* finishAll_;
        std::condition_variable* nextMission_;
        std::mutex* idleMutex_;
        std::mutex* finishMutex_;
};

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
    private:
        int num_threads;
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */

class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        // void threadRun(IRunnable* runnable, int num_total_tasks, std::mutex* mutex, int* finished_tasks);
    private:
        int num_threads;
        std::thread* threadPool;
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        void spinThread();
    private:
        bool killed_;
        int num_threads_;
        TaskState* task_stat_;
        std::thread* threadPool_;
};

#endif
