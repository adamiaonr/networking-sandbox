#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <stdexcept>

using std::runtime_error;

class SignalException : public runtime_error {

    public:
        SignalException(const std::string& _message)
            : std::runtime_error(_message)
        {}
};

// this class provides an interface to setup a SIGINT signal catcher. this 
// allows an application to catch a 'CTRL+C' hit by the user, and (for example) 
// gracefully exit. this was based in an example available at 
// http://www.yolinux.com/TUTORIALS/C++Signals.html
class SignalHandler {

    public:

        SignalHandler() {}
        ~SignalHandler() {}

        // abstracts the signal() call, which associates the signal to catch 
        // (SIGINT in this case), with a method to handle the signal when caught 
        // caught (exit_signal_handler()). if the signal() call fails, this 
        // function raises a SignalException.
        void setup_signal_handlers();

        // returns a boolean value : TRUE if a SIGINT signal has been caught, 
        // FALSE otherwise.
        static bool got_exit_signal();

    private:

        static bool got_sigint;

        // the actual signal handler function. sets the got_sigint flag to 
        // TRUE. the 'ignored' argument is not used, but required by 
        // function prototype to match required handler.
        static void exit_signal_handler(int ignored);
};

#endif