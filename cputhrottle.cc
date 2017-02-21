// Program to throttle CPU usage of process specified by PID.
//
// William Nolan 2008/05/26

#include "manip.h"

#include <errno.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>

#include <boost/lexical_cast.hpp>

#include <iostream>

#include <math.h>

#include <time.h>

#include <vector>

Process::Manipulator manip;
bool done = false;

void sleepNanoseconds(double amt_)
{
  timespec tv;

  double integral, fractional;
  fractional = modf(amt_, &integral);
  tv.tv_sec = integral;
  tv.tv_nsec = fractional * 1000000000.0;

  nanosleep(&tv, 0);
}

void control_c(int sig)
{
  signal(SIGINT, control_c);
  done = true;
  std::cerr << "Caught Ctrl-C, detaching..." << std::endl;
}

int main(int argc_, char **argv_)
{
  if(argc_ < 3)
  {
    std::cerr << "Please supply PID to throttle and CPU percentage(0..100)." 
	      << std::endl;
    return -1;
  }

  signal(SIGINT, control_c);
  
  pid_t tracePid = boost::lexical_cast<pid_t>(argv_[1]);
  double throttle = boost::lexical_cast<double>(argv_[2]) / 100.0;

  manip.attach(tracePid);
  
  const double period = 0.1; // max amount for us to sleep
  const double minSleepAmt = 0.02; // minimum sleep amount is 20 ms
  double sleepAmt = minSleepAmt;
  double user_time, system_time, percent;

  manip.resume(tracePid);
  manip.sample(tracePid, &user_time, &system_time, &percent); // initial sample
  
  try 
  {
    while(!done)
    {
      manip.sample(tracePid, &user_time, &system_time, &percent);

//          std::cout << "Percent = " << percent << ", Throttle = " << throttle
//      	      << std::endl;
    
      double error = percent - throttle;
      if(sleepAmt == 0.0 && error > 0.0)
      {
	sleepAmt = minSleepAmt;
      }
      sleepAmt += error * minSleepAmt;
 //     std::cout << "Adj sleepamt = " << sleepAmt << std::endl;
      if(sleepAmt < minSleepAmt)
      {
	sleepAmt = 0;
      }

      if(sleepAmt > period)
      {
	sleepAmt = period;
      }

//std::cout << "Considering Sleep for " << sleepAmt << ", error = " << error << std::endl;
      if(sleepAmt >= minSleepAmt)
      {
//	      std::cout << "Sleep for " << sleepAmt << std::endl;
	manip.suspend(tracePid);
	sleepNanoseconds(sleepAmt);
	manip.resume(tracePid);
      }

      sleepNanoseconds(period - sleepAmt);
    }
  } 
  catch(...) 
  {}

  return 0;
}
