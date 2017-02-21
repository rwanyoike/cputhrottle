#include "manip.h"

#include <boost/lexical_cast.hpp>

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/task_info.h>
#include <mach/thread_act.h>
#include <mach/vm_map.h>
#include <signal.h>

Process::Manipulator::Manipulator()
{
}

Process::Manipulator::~Manipulator()
{
    pidSetType::iterator i = _attachedPids.begin();

    while(i != _attachedPids.end())
    {
        try
        {
            i = detachIter(i);
        }
        catch(ManipulatorException &e)
        {            
	  break; // bail
        }
    }
}

void Process::Manipulator::attach(pid_t pid_)
{
    kern_return_t res;
    task_t task;

    if((res = task_for_pid(mach_task_self(), pid_, &task)) != KERN_SUCCESS)
    {
        throw ManipulatorException("Error on task_for_pid of pid " +
                                   boost::lexical_cast<std::string>(pid_) +
                                   ", res = " + 
                                   boost::lexical_cast<std::string>(res));
    }

    if(task_suspend(task))
    {
        throw ManipulatorException("Error on task_suspend of pid " +
                                   boost::lexical_cast<std::string>(pid_));
    }

    _attachedPids.insert(pidSetType::value_type(pid_, task));
}

void Process::Manipulator::detach(pid_t pid_)
{
    pidSetType::iterator i = _attachedPids.find(pid_);

    if(i == _attachedPids.end())
    {
        throw ManipulatorException("Not attached to pid " + boost::lexical_cast<std::string>(pid_));
    }

    detachIter(i);
}

Process::Manipulator::pidSetType::iterator 
Process::Manipulator::detachIter(const pidSetType::iterator &iter_)
{
    resumeIter(iter_);

    pidSetType::iterator iterCopy(iter_);
    _attachedPids.erase(iterCopy++);

    return iterCopy;
}

void Process::Manipulator::singleStep(pid_t pid_)
{
    throw ManipulatorException("Not implemented yet.");
}

void Process::Manipulator::continueRunning(pid_t pid_)
{
    throw ManipulatorException("Not implemented yet.");
}

void Process::Manipulator::suspend(pid_t pid_)
{
    pidSetType::iterator i = _attachedPids.find(pid_);

    if(i == _attachedPids.end())
    {
        throw ManipulatorException("Not attached to pid " + boost::lexical_cast<std::string>(pid_));
    }

    suspendIter(i);
}

void Process::Manipulator::resume(pid_t pid_)
{
    pidSetType::iterator i = _attachedPids.find(pid_);

    if(i == _attachedPids.end())
    {
        throw ManipulatorException("Not attached to pid " + boost::lexical_cast<std::string>(pid_));
    }

    resumeIter(i);
}

void Process::Manipulator::sample(pid_t pid_,
				  double *user_time, double *system_time, double *percent)
{
    pidSetType::iterator iter = _attachedPids.find(pid_);

    if(iter == _attachedPids.end())
    {
        throw ManipulatorException("Not attached to pid " + boost::lexical_cast<std::string>(pid_));
    }

    kill(pid_, SIGSTOP);
    
    kern_return_t error;
    struct task_basic_info t_info;
    thread_array_t th_array;	
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT, th_count;
    size_t i;
    double my_user_time = 0, my_system_time = 0, my_percent = 0;

    if ((error = task_info(iter->second, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count)) != KERN_SUCCESS)
      {
        throw ManipulatorException("Error on task_info of pid " + boost::lexical_cast<std::string>(pid_));
      }

    if ((error = task_threads(iter->second, &th_array, &th_count)) != KERN_SUCCESS)
      {
        throw ManipulatorException("Error on task_threads of pid " + boost::lexical_cast<std::string>(pid_));
      }

    // sum time for live threads
    for (i = 0; i < th_count; i++) 
    {
      double th_user_time, th_system_time, th_percent;

      //      if ((error = AGGetMachThreadCPUUsage(th_array[i], &th_user_time, &th_system_time, &th_percent)) != KERN_SUCCESS)
      //	break;

      struct thread_basic_info th_info;
      mach_msg_type_number_t th_info_count = THREAD_BASIC_INFO_COUNT;
      if ((error = thread_info(th_array[i], THREAD_BASIC_INFO, 
			       (thread_info_t)&th_info, &th_info_count)) != KERN_SUCCESS)
	{
	  throw ManipulatorException("Error on thread_info of pid " + boost::lexical_cast<std::string>(pid_));
	}

      th_user_time = th_info.user_time.seconds + th_info.user_time.microseconds / 1e6;
      th_system_time = th_info.system_time.seconds + th_info.system_time.microseconds / 1e6;
      th_percent = (double)th_info.cpu_usage / TH_USAGE_SCALE;


      my_user_time += th_user_time;
      my_system_time += th_system_time;
      my_percent += th_percent;
    }

    // destroy thread array
    for (i = 0; i < th_count; i++)
    {
      mach_port_deallocate(mach_task_self(), th_array[i]);
    }
    vm_deallocate(mach_task_self(), (vm_address_t)th_array, sizeof(thread_t) * th_count);

    // check last error	
    if (error != KERN_SUCCESS)
      {
	throw ManipulatorException("Error in collecting cpu sample of pid " + boost::lexical_cast<std::string>(pid_));
      }

    // add time for dead threads
    my_user_time += t_info.user_time.seconds + t_info.user_time.microseconds / 1e6;
    my_system_time += t_info.system_time.seconds + t_info.system_time.microseconds / 1e6;
    if (user_time != NULL) *user_time = my_user_time;
    if (system_time != NULL) *system_time = my_system_time;
    if (percent != NULL) *percent = my_percent;

    kill(pid_, SIGCONT);

}

void
Process::Manipulator::resumeIter(const pidSetType::iterator &iter_)
{
    if(task_resume(iter_->second))
    {
        throw ManipulatorException("Error on task_resume of pid " +
                                   boost::lexical_cast<std::string>(iter_->first));
    }
}

void
Process::Manipulator::suspendIter(const pidSetType::iterator &iter_)
{
    if(task_suspend(iter_->second))
    {
        throw ManipulatorException("Error on task_suspend of pid " +
                                   boost::lexical_cast<std::string>(iter_->first));
    }
}
