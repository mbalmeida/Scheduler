#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// Forward declaration.
class Task;

/// Simple ThreadPool that simplifies and thread spawing and joining.
class ThreadPool {
public:
  /// Create ThreadPool with N workers.
  ThreadPool(int N);

  /// Dtor
  ~ThreadPool();

  /// Add/Enqueue task to be executed
  void AddTask(Task* T);

  /// Stop ThreadPool execution gracefully.
  /// Queued tasks that haven't been executed at the point this functions
  /// is called are discarded.
  void ShutDown();

private:
  /// Function executed by each thread.
  void Work();

  /// Number of worker threads.
  int Workers;

  /// Structure containing the Thread objects.
  std::vector<std::thread> Pool;

  /// Task queue.
  std::queue<Task*> TaskQ;

  /// Mutual exclusion mechanism for the TaskQ.
  std::mutex TaskQMutex;

  /// Conditional variable used to implement thread notification.
  std::condition_variable Cond;

  /// Saves the current running state of the ThreadPool.
  bool Stopped;
};

#endif
