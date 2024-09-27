1. Standard I/O is not thread-safe
```cpp
std::mutex cerr_mutex;
void safeCerr(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cerr_mutex);
    std::cerr << msg;
}
```
2. About `notify` in Conditional Variable
    * notify_one() 被多次调用：
        每次调用 notify_one() 将唤醒一个正在等待该条件变量的线程（如果有）。如果当前没有线程在等待，那么调用 notify_one() 不会产生立即可见的效果，直到有线程开始等待。一旦有线程开始等待，它将被唤醒，无论之前有多少次 notify_one() 被调用。
        如果有多个线程在等待同一个条件变量，每次 notify_one() 只会唤醒其中一个线程。具体唤醒哪个线程通常由底层操作系统或运行时库的调度策略决定。
    * notify_all() 被调用：
        调用 notify_all() 将会唤醒所有正在等待该条件变量的线程。即使之后有新的线程开始等待，它们也不会被之前的 notify_all() 影响，除非再次调用 notify_all() 或足够次数的 notify_one()。
3. Before wake up, make sure the process before wait is done. Using `lock.lock(); lock.unlock();`
4. Process, thread, OpenMP, MPI:
   * OpenMP use more cores(if not hyperthreading) to spawn threads from a process in one node. 
   * MPI(Message Passing Interface) uses cores to create processes total to the demand. It launch one process per core, no matter it is on one node or many. But one node need less cost in communication.
     * I think MPI cares only about the process rather than node, use `MPI_rev`, etc. to communicate between processes.
  > Therefore, you'd better let `(process in MPI) * (threads in OMP) == core number`.