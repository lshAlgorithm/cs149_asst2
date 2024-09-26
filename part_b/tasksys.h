#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"

#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_set>
#include <iostream>

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
};

/*
    Core data structure, recording all the details of a mission. 
    Subtask just points to it.
*/
class TaskState {
    public:
        TaskState();
        TaskState(TaskID task_id, IRunnable* runnable, int num_total_task, const std::vector<TaskID>& deps);
        ~TaskState();
        std::mutex* mutex_; // lock done_, task_distribute_, runnable, num_total_task_
        TaskID task_id_;
        int done_;
        int num_total_task_;
        int task_distribute_;
        IRunnable* runnable_;

        // new feature
        std::vector<int> task_dep_;
};

class TaskQueue {
    public:
        TaskQueue();
        ~TaskQueue();

        std::mutex* queMutex_;
        std::queue<TaskState>* que_;
        std::condition_variable* haveOne_;
};

class Subtask {
    public:
        Subtask(TaskState* task, int id);
        ~Subtask();

        int subtaskID_;
        TaskState* whole_task_;    // Directly point to the task itself, pay attention to mutex
};

class SubtaskBuffer {
    public:
        SubtaskBuffer();
        ~SubtaskBuffer();

        std::mutex* readyMutex_;
        std::condition_variable* empty_;
        std::mutex* emptyMutex_;
        std::queue<Subtask> buffer_;
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
        void Run();
        void sleepThread();
    private:
        bool killed_;
        int num_threads_;
        TaskQueue* waiting_que_;
        SubtaskBuffer* ready_que_;
        std::unordered_set<TaskID>* finish_set_;
        std::thread* threadPool_;
        std::condition_variable* nextMission_;
};

#endif
