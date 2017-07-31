#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <set>

#include "ThreadPool.h"

class Task;

/// Class that implements a periodic scheduler with a
/// very simple interface.
/// Tasks (Tyle Task) can be added and are executed every after every Period.
class PeriodicScheduler {
public:
  void AddTask(Task* T, std::chrono::duration<int> PeriodInSeconds);
  void Start();
  void Stop();
  void _MainLoop();

  /// FIXME: Make number of ThreadPool's workers a parameter.
  PeriodicScheduler()
    : Stopped(false)
    , ThreadP(8)
  {}

  ~PeriodicScheduler() {
    if (!Stopped)
      Stop();
  }

  typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

  /// Abstraction of a scheduled task i.e, it's the combination of a task
  /// and a TimePoint that represents when the task is scheduled to run.
  class RunnableTask {
  public:
    RunnableTask(Task* Task, TimePoint TP, std::chrono::duration<int> P)
      : T(Task)
      , TimeToRun(TP)
      , PeriodInSeconds(P)
    { }

    bool operator<(const RunnableTask& R) const
    {
      return this->TimeToRun < R.TimeToRun;
    }

    Task* T;
    std::chrono::time_point<std::chrono::steady_clock> TimeToRun;
    std::chrono::duration<int> PeriodInSeconds;
  };

  class RunnableTaskComp {
  public:
    bool operator()(const RunnableTask* a, const RunnableTask* b) const
    {
      return *a < *b;
    }
  };

private:
  /// Set of scheduled Tasks using a arbitrary comparator.
  /// Tasks in this set are ordered by the time a task is scheduled to run.
  /// In other words, the first task in the set is the next scheduled task.
  std::multiset<RunnableTask*, RunnableTaskComp> Tasks;

  /// Indicates running state of the Scheduler.
  bool Stopped;

  /// ThreadPool handle
  ThreadPool ThreadP;
};

#endif

