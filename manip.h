#ifndef __LIBPROC_MANIP_H_
#define __LIBPROC_MANIP_H_

#include <boost/shared_ptr.hpp>
#include <exception>
#include <map>
#include <string>

#include <mach/task.h>

//#include "regs.h"

namespace Process
{
    class ManipulatorException : public std::exception
    {
    public:
        ManipulatorException(const std::string &msg_) : _msg(msg_) {}
        virtual ~ManipulatorException() throw() {}
        virtual const char* what() const throw()
        { 
            return _msg.c_str(); 
        }
        
    private:
        std::string _msg;
    };

    class Manipulator
    {
    public:
        Manipulator();
        ~Manipulator();

        void attach(pid_t pid_); // throw ManipulatorException
        void detach(pid_t pid_); // throw ManipulatorException

        void singleStep(pid_t pid_); // throw ManipulatorException
        void continueRunning(pid_t pid_); // throw ManipulatorException

        void suspend(pid_t pid_); // throw ManipulatorException
        void resume(pid_t pid_); // throw ManipulatorException

        void sample(pid_t pid_, 
		    double *user_time, double *system_time, 
		    double *percent); // throw ManipulatorException

      //        boost::shared_ptr<Registers> getRegs(); // throw ManipulatorException, RegistersException

    private:

        typedef std::map<pid_t, task_t> pidSetType;
        pidSetType _attachedPids;

        pidSetType::iterator detachIter(const pidSetType::iterator &iter_); // throw ManipulatorException
        void suspendIter(const pidSetType::iterator &iter_); // throw ManipulatorException
        void resumeIter(const pidSetType::iterator &iter_); // throw ManipulatorException
    };
}

#endif
