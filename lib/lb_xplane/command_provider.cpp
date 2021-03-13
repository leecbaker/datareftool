#include "command_provider.h"

#include "logging.h"

CommandProvider::CommandProvider(std::string name, std::string description, std::function<bool(void)> begin_func, std::function<bool(void)> continue_func, std::function<bool(void)> end_func)
: begin_func(std::move(begin_func))
, continue_func(std::move(continue_func))
, end_func(std::move(end_func))
{
    cr = XPLMCreateCommand(name.c_str(), description.c_str());
    
    XPLMRegisterCommandHandler(cr, CommandProvider::handler, 0, static_cast<void *>(this));
}

CommandProvider::~CommandProvider() {
    XPLMUnregisterCommandHandler(cr, CommandProvider::handler, 0, static_cast<void *>(this));
}


int CommandProvider::handler(XPLMCommandRef /* inCommand */, XPLMCommandPhase phase, void * inRefcon) {

    CommandProvider * pthis = static_cast<CommandProvider *>(inRefcon);
    switch(phase) {
        case xplm_CommandBegin:
            if(bool(pthis->begin_func)) {
                return pthis->begin_func() ? 1 : 0;
            }
            break;
        case xplm_CommandContinue:
            if(bool(pthis->continue_func)) {
                return pthis->continue_func() ? 1 : 0;
            }
            break;
        case xplm_CommandEnd:
            if(bool(pthis->end_func)) {
                return pthis->end_func() ? 1 : 0;
            }
            break;
    }

    return 0;
}
