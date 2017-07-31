#include "ThreadPool.h"
#include "Task.h"

ThreadPool::ThreadPool(int N)
  : Workers(N)
  , Stopped(false)
  {
    // Initialise pool by creating N worker threads
    for (int I = 0; I < N; ++I)
      Pool.push_back(std::thread(&ThreadPool::Work, this));
  }

void ThreadPool::AddTask(Task *T)
{
  std::unique_lock<std::mutex> Lock(TaskQMutex);
  this->TaskQ.emplace(T);
  Cond.notify_one();
}

void ThreadPool::Work()
{
  for(;;) {
    Task* T = 0;
    {
      std::unique_lock<std::mutex> Lock(TaskQMutex);
      Cond.wait(Lock, [this] { return !TaskQ.empty() || Stopped; });
      if (Stopped) {
        return;
      }

      T = TaskQ.front();
      TaskQ.pop();
    }
    T->Run();
  }
}

void ThreadPool::ShutDown()
{
  {
    std::unique_lock<std::mutex> Lock(TaskQMutex);
    Stopped = true;
    Cond.notify_all();
  }

  for (std::thread& Thread : Pool) {
    Thread.join();
  }
}

ThreadPool::~ThreadPool()
{
  if (!Stopped)
    ShutDown();
}
