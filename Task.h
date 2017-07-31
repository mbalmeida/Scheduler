#ifndef TASK_H
#define TASK_H

#include <unistd.h>

// Foward declarations.
class PersistanceLayer;

/// Simple Task interface
/// Run() :
/// Persist(): Use information gathered by Run() and *store it*.
class Task {
public:
  /// Implement task operation
  virtual void Run() = 0;

  /// Use information gathered by Run() and *store it*.
  virtual void Persist() = 0;

  virtual ~Task() {}

  void operator()() {
    this->Run();
  }

protected:
  PersistanceLayer* PL;
};

#endif
