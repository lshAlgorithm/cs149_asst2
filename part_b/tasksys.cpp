#include "tasksys.h"


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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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
}

TaskState::TaskState(TaskID task_id, IRunnable* runnable, int num_total_task, const std::vector<TaskID>& deps):
    task_id_(task_id),
    num_total_task_(num_total_task),
    runnable_(runnable),
    done_(0),
    task_distribute_(0)
{
    std::cerr << "create taskstate\t";
    this->mutex_ = new std::mutex();
    this->task_dep_ = deps;
}

TaskState::~TaskState() {
    delete mutex_;
}

Subtask::Subtask(TaskState* task, int id) {
    whole_task_ = task;
    subtaskID_ = id;
}

Subtask::~Subtask() {}

SubtaskBuffer::SubtaskBuffer() {
    readyMutex_ = new std::mutex;
    empty_ = new std::condition_variable;
    emptyMutex_ = new std::mutex;
}
SubtaskBuffer::~SubtaskBuffer() {
    delete readyMutex_;
    delete empty_;
    delete emptyMutex_;
}

TaskQueue::TaskQueue() {
    std::cerr << "create taskqueue\t";
    queMutex_ = new std::mutex;
    que_ = new std::queue<TaskState>;
    haveOne_ = new std::condition_variable;
    // oneMutex_ = new std::mutex;
}

TaskQueue::~TaskQueue() {
    std::cerr << "Delete taskqueue\t";
    delete que_;
    delete queMutex_;
    delete haveOne_;
    // delete oneMutex_;
}

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    threadPool_ = new std::thread[num_threads];
    waiting_que_ = new TaskQueue();
    ready_que_ = new SubtaskBuffer();
    finish_set_ = new std::unordered_set<TaskID>;
    nextMission_ = new std::condition_variable();

    threadPool_[0] = std::thread(&TaskSystemParallelThreadPoolSleeping::Run, this);
    for (int i = 1; i < num_threads; i++) {
        // Use a class function in a thread
        threadPool_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepThread, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    
    killed_ = true;
    nextMission_->notify_all();
    waiting_que_->haveOne_->notify_all();
    for (int i = 0; i < num_threads_; i++) {
        threadPool_[i].join();
    }
    delete finish_set_;
    delete nextMission_;
    delete []threadPool_;
    delete ready_que_;
    delete waiting_que_;
}

// Q: Any semaphore that can be utilized here?
void TaskSystemParallelThreadPoolSleeping::sleepThread() {
    // std::cerr << "sleep thread begin\t";
    try {
        while (true) {
            if (killed_) 
                break;
            
            // I encounter a bug here, saying operation not permitted. GPT says`在进行系统调用时，正确处理返回的错误码，并在必要时转换为异常抛出，而不是让错误码直接导致程序终止。`
            // How to do it?

            std::unique_lock<std::mutex> lk(*(ready_que_->readyMutex_));

            if (ready_que_->buffer_.empty()) {
                ready_que_->empty_->notify_all();
                // Sleep...
                nextMission_->wait(lk);
            }

            if (killed_) break;

            Subtask &t = ready_que_->buffer_.front();
            ready_que_->buffer_.pop();
            lk.unlock();

            t.whole_task_->mutex_->lock();
            t.whole_task_->runnable_->runTask(t.subtaskID_, t.whole_task_->num_total_task_);
            t.whole_task_->done_++;
            if (t.whole_task_->done_ == t.whole_task_->num_total_task_) {
                finish_set_->insert(t.whole_task_->task_id_);
            }
            t.whole_task_->mutex_->unlock();
        }
    } catch (const std::exception&e) {
        std::cerr << "Exception!: " << e.what() << '\t';
    }
}

// Override the serial run, to be compatible with the serial tests.
void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}


// Check if there is anything mission in *queue* can be pushed to the *buffer*
void TaskSystemParallelThreadPoolSleeping::Run() {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // initMutex_->lock();
    bool fl;

    while (true) {
        if (killed_) 
            break;
        
        TaskQueue &w = *waiting_que_;
        std::unique_lock<std::mutex> lk(*(w.queMutex_));
        if (w.que_->empty()) {
            // sleep...
            lk.unlock();
            w.haveOne_->wait(lk);
            if(killed_) {
                lk.unlock();
                break;
            }
        }
        TaskState& newTask = w.que_->front();
        w.que_->pop();
        fl = true;
        for (auto c: newTask.task_dep_) {
            if (finish_set_->find(c) != finish_set_->end()) {
                fl = false;
                w.que_->push(newTask);
            }
        }
        lk.unlock();
        if (fl == false) {
            continue;
        }

        /* 
            There should only be one task_stat at all time...
            No! Every task has its own task_stat_, bound with a taskID.
        */

        SubtaskBuffer &r = *ready_que_;
        r.readyMutex_->lock();
        for (int i = 0; i < newTask.num_total_task_; i++) {
            Subtask subtask = Subtask(&newTask, i);
            r.buffer_.emplace(subtask);
        }
        r.readyMutex_->unlock();
        nextMission_->notify_all(); // wait and see...

        /* We don't need it now. */
        // task_stat_->idleMutex_->lock();  
        // task_stat_->idleMutex_->unlock();
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //
    static TaskID id = 0;

    waiting_que_->queMutex_->lock();

    TaskID new_task_id = id++;

    // You can make it a unique_ptr for better performance
    TaskState new_task = TaskState(new_task_id, runnable, num_total_tasks, deps);
    waiting_que_->que_->push(new_task);

    // wake up Run()
    if (waiting_que_->que_->size() == 1) {
        waiting_que_->haveOne_->notify_all();
    }

    waiting_que_->queMutex_->unlock();


    return new_task_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    
    // Plan: sleep until the threads to complete
    std::unique_lock<std::mutex> lk(*(ready_que_->emptyMutex_));
    if (ready_que_->buffer_.empty()) 
        return ;
    else {
        ready_que_->empty_->wait(lk);
        lk.unlock();
    }

    return;
}
