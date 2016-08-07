#include <signal.h>
#include <errno.h>

#include "signal-handler.h"

bool SignalHandler::got_signal = false;

// here we define our own version of signal() as a wrapper to a POSIX-compliant
// sigaction(). the complicated function prototype can be explained as follows:
//  - the return type of signal() is a function pointer to a function that 
//    takes an int arg and returns void. i.e. a signal function returns a 
//    pointer to the handler function.
//  - the signal() function takes an int and a pointer to an handler function 
//    as argument. 
void (*signal(int signo, void (*func)(int)))(int) {

    // the sigaction struct basically describes how to handle a signal. we 
    // explain this as we set each attribute.
    struct sigaction act, oact;

    // the sa_handler member holds a pointer to the handler function. it is 
    // of type void (*func)(int), i.e. functions that take one int as arg and 
    // return void
    act.sa_handler = func;
    // the sa_mask member allows us to set a list of signals to be blocked 
    // by the handler. i.e. the signals specified in this mask will be caught 
    // by the handler function but not delivered to the process setting the 
    // handler. we just set it to an empty set, meaning no signals will be 
    // blocked.
    sigemptyset(&act.sa_mask);
    // usually, special flags are off...
    act.sa_flags = 0;
    // ... however, in special cases, we may which to make up for missing 
    // features in some UNIX releases. 
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;   // SunOS 4.x
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;     // SVR4, 4.4BSD
#endif
    }

    // finally call the sigaction() function. the old signal hanlder is 
    // saved in oact (if any) and returned by signal(). this is useful in case 
    // we want to restore the older signal handler.
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;

    return oact.sa_handler;
}

bool SignalHandler::is_signal() {

    return got_signal;
}

void SignalHandler::disarm_signal() {

    got_signal = false;
}

void SignalHandler::signal_handler(int signo) {

    got_signal = true;
}

void SignalHandler::set_signal_handler(int signo) {

    if (signal((int) signo, SignalHandler::signal_handler) == SIG_ERR) {

        throw SignalException("SignalHandler::set_signal_handler() : [ERROR] "\
            "error setting up signal handler.");
    }
}