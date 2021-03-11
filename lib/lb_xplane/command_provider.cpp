#include "command_provider.h"

CommandProvider::CommandProvider(std::string name, std::string description, std::function<bool(void)> begin_func, std::function<bool(void)> continue_func, std::function<bool(void)> end_func)
: begin_func(std::move(begin_func))
, continue_func(std::move(continue_func))
, end_func(std::move(end_func))
{
    cr = XPLMCreateCommand(name.c_str(), description.c_str());
    
    if(bool(this->begin_func)) {
        XPLMRegisterCommandHandler(cr, CommandProvider::begin_handler, 1, static_cast<void *>(this));
    }

    if(bool(this->continue_func)) {
        XPLMRegisterCommandHandler(cr, CommandProvider::continue_handler, 1, static_cast<void *>(this));
    }

    if(bool(this->end_func)) {
        XPLMRegisterCommandHandler(cr, CommandProvider::end_handler, 1, static_cast<void *>(this));
    }
}

CommandProvider::~CommandProvider() {
    if(bool(begin_func)) {
        XPLMUnregisterCommandHandler(cr, CommandProvider::begin_handler, 1, static_cast<void *>(this));
    }

    if(bool(continue_func)) {
        XPLMUnregisterCommandHandler(cr, CommandProvider::continue_handler, 1, static_cast<void *>(this));
    }

    if(bool(end_func)) {
        XPLMUnregisterCommandHandler(cr, CommandProvider::end_handler, 1, static_cast<void *>(this));
    }
}


int CommandProvider::begin_handler(XPLMCommandRef /* inCommand */, XPLMCommandPhase /* inPhase */, void * inRefcon) {
    CommandProvider * pthis = static_cast<CommandProvider *>(inRefcon);
    return pthis->begin_func() ? 1 : 0;
}

int CommandProvider::continue_handler(XPLMCommandRef /* inCommand */, XPLMCommandPhase /* inPhase */, void * inRefcon) {
    CommandProvider * pthis = static_cast<CommandProvider *>(inRefcon);
    return pthis->continue_func() ? 1 : 0;
}

int CommandProvider::end_handler(XPLMCommandRef /* inCommand */, XPLMCommandPhase /* inPhase */, void * inRefcon ) {
    CommandProvider * pthis = static_cast<CommandProvider *>(inRefcon);
    return pthis->end_func() ? 1 : 0;
}
