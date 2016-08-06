#include <signal.h>
#include <errno.h>

#include "signal-handler.h"

bool SignalHandler::got_sigint = false;

bool SignalHandler::got_exit_signal() {

    return got_sigint;
}

void SignalHandler::exit_signal_handler(int ignored) {

    got_sigint = true;
}

void SignalHandler::setup_signal_handlers() {

    if (signal((int) SIGINT, SignalHandler::exit_signal_handler) == SIG_ERR) {

        throw SignalException("SignalHandler::setup_signal_handlers() : [ERROR] "\
            "error setting up signal handlers !!!!!");
    }
}