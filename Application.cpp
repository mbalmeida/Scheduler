#include "PersistanceLayer.h"
#include "Scheduler.h"
#include "Task.h"
#include <boost/program_options.hpp>
#include <cstdio>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

/// Measure current physical memory used by this process
class ProcessPhysicalMemory : public Task {
public:
  ProcessPhysicalMemory(PersistanceLayer* PL)
  {
    Task::PL = PL;
  }

  virtual void Run() {
    // Get VmRSS (Resident Set Size) that contains the number of pages
    // currently resident in physical memory excluding pages that were
    // swapped out. If we want that information as well we need to add VmSwap
    // that can be obtained by parsing /proc/$PID/status.

    // There are two ways of getting VmRSS on Linux:
    // 1. Parse /proc/$PID/status
    // 2. Parse /proc/$PID/statm and multiply by the page size

    // Solution 2 was chosen because the statm file is the simplest to parse.
    long PageSize = sysconf(_SC_PAGESIZE);
    PageSize /= 1024;
    FILE* Statm = 0;
    if ((Statm = fopen("/proc/self/statm", "r")) == 0) {
      PhysicalMemoryInKBytes = 0;
      return;
    }
    unsigned long VmRss;
    int Res = fscanf(Statm, "%*d%lu", &VmRss);
    fclose(Statm);
    if (Res != 1)
      return;

    PhysicalMemoryInKBytes = VmRss * PageSize;
    Persist();
  }

  virtual void Persist() {
    Task::PL->UpdatePhysicalMemoryTable(PhysicalMemoryInKBytes);
  }

private:
  unsigned long PhysicalMemoryInKBytes;
};

class TCPConnection : public Task {
public:
  TCPConnection(const std::string& H,
                const std::string& P,
                PersistanceLayer* PL)
  {
    Host = H;
    Port = P;
    Task::PL = PL;
  }

  virtual void Run() {
    const char* Host = this->Host.c_str();
    const char* Service = Port.c_str();
    struct addrinfo *Result = 0;
    struct addrinfo Hints;
    memset(&Hints, 0, sizeof(struct addrinfo));
    Hints.ai_family = AF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_flags = 0;
    Hints.ai_protocol = IPPROTO_TCP;
    int AddrInfoR = getaddrinfo(Host, Service, &Hints, &Result);
    if (AddrInfoR == -1) {
      std::cout << "Failed to obtain AddrInfo" << std::endl;
      return;
    }

    struct addrinfo* Current;
    int S;
    auto Before = std::chrono::steady_clock::now();
    for (Current = Result; Current != NULL; Current = Current->ai_next)
    {
      S = socket(Current->ai_family, Current->ai_socktype,
           Current->ai_protocol);
      if (S == -1)
        continue;

      if (connect(S, Current->ai_addr, Current->ai_addrlen) != -1)
        break;
      close(S);
    }
    auto After = std::chrono::steady_clock::now();
    ConnectionTime = After - Before;

    if (Current == NULL) {
      std::cout << "Couldn't establish a connection" << std::endl;
      return;
    }

    freeaddrinfo(Result);
    close(S);
    Persist();
  }

  virtual void Persist() {
    Task::PL->UpdateTCPConnectionTable(Host, ConnectionTime.count());
  }

private:
  std::chrono::duration<double> ConnectionTime;
  std::string Host;
  std::string Port;
};

class Application {
public:
  Application(int argc, char** argv)
  {
    Descs.add_options()
      ("help,h", "Print this help")
      ;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(Descs).run(),
          ProgramOptions);
    boost::program_options::notify(ProgramOptions);
    DBLoc = ".appdb";
  }

  bool Run()
  {
    if (ProgramOptions.count("help")) {
      std::cout << Descs << std::endl;
      return 0;
    }

    PersistanceLayer PL;
    if (!PL.Connect(DBLoc)) {
      std::cout << "Can't access DB: " << PL.GetLastError() << "\n";
      return 1;
    }

    // Initialise the DB before starting any of the tasks.
    PL.CreateTCPConnectionsTable();
    PL.CreatePhysicalMemoryTable();

    // Initialise the Scheduler
    PeriodicScheduler Sched;
    // Create and add a task that measures the physical memory
    // used by this application.
    Task* PMTask = new ProcessPhysicalMemory(&PL);
    Sched.AddTask(PMTask, std::chrono::seconds(1));

    Task* GoogleTCP = new TCPConnection("google.com", "80", &PL);
    Sched.AddTask(GoogleTCP, std::chrono::seconds(1));
    Task* FacebookTCP = new TCPConnection("facebook.com", "80", &PL);
    Sched.AddTask(FacebookTCP, std::chrono::seconds(1));
    Task* YahooTCP = new TCPConnection("yahoo.com", "80", &PL);
    Sched.AddTask(YahooTCP, std::chrono::seconds(5));
    Task* HNTCP = new TCPConnection("news.ycombinator.com", "80", &PL);
    Sched.AddTask(HNTCP, std::chrono::seconds(5));
    Task* ThousandEyesTCP = new TCPConnection("thousandeyes.com", "80", &PL);
    Sched.AddTask(ThousandEyesTCP, std::chrono::seconds(10));
    // Start the Scheduler
    Sched.Start();

    std::string Line;
    while(true) {
      std::cout << "> ";
      std::getline(std::cin, Line);
      if (Line == "exit")
        break;
    }
    // Stop the Scheduler after receiving EOF
    Sched.Stop();

    return 0;
  }

private:
  boost::program_options::variables_map ProgramOptions;
  boost::program_options::options_description Descs;
  std::string PathToTCPConfig;
  std::string DBLoc;
};

int main(int argc, char** argv)
{
  Application App(argc, argv);
  return App.Run();
}

