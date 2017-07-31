#include "Scheduler.h"
#include "Task.h"

void PeriodicScheduler::AddTask(Task* T,
                                std::chrono::duration<int> PeriodInSeconds) {
  Tasks.insert(new RunnableTask(T, std::chrono::steady_clock::now(),
                                PeriodInSeconds));
}

void PeriodicScheduler::_MainLoop() {
  while(!Stopped) {
    auto now = std::chrono::steady_clock::now();

    std::chrono::duration<double> SleepDuration(1);
    bool HasScheduledTasks = true;
    do {
      auto RTIt = Tasks.begin();
      if (RTIt != Tasks.end()) {
        RunnableTask* RT = *RTIt;
        if (now > RT->TimeToRun) {
          Tasks.erase(RTIt);
          ThreadP.AddTask(RT->T);
          Tasks.insert(new RunnableTask(RT->T, now + RT->PeriodInSeconds,
                                        RT->PeriodInSeconds));
        } else {
          SleepDuration = RT->TimeToRun - now;
          HasScheduledTasks = false;
        }
      }
      else
        HasScheduledTasks = false;

    } while(HasScheduledTasks);

    std::this_thread::sleep_for(SleepDuration);
  }
}

void PeriodicScheduler::Start() {
  std::thread T(&PeriodicScheduler::_MainLoop, this);
  T.detach();
}

void PeriodicScheduler::Stop() {
  if (Stopped)
    return;

  Stopped = true;
  ThreadP.ShutDown();
}
