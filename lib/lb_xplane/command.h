#pragma once

#include <exception>
#include <string>

#include "XPLMUtilities.h"

class CommandrefError : public std::exception {
    std::string message;
    CommandrefError(std::string message) : message(message) {}
public:
    static CommandrefError CommandrefMissing(const std::string ref_name) {
        return CommandrefError("Failed to find commandref \"" + ref_name + "\"");
    }
    
    virtual const char* what() const noexcept {
        return message.c_str();
    }
};

class CommandHeld;

class Commandref {
    XPLMCommandRef ref;
    friend class CommandHeld;
    std::string command_name;   // only necessary for debug purposes.
protected:
    void commandEnd() const { XPLMCommandEnd(ref); }    //shouldn't ever directly call this
    void commandBegin() const { XPLMCommandBegin(ref); }
public:
    Commandref(const std::string name) : command_name(name) {
        ref = XPLMFindCommand(name.c_str());
        if(nullptr == ref) {
            throw CommandrefError::CommandrefMissing(name);
        }
    }
    
    void commandOnce() const {
        XPLMCommandOnce(ref);
    }
    
    void commandSeveral(int count) const {
        for(int i = 0; i < count; i++) {
            XPLMCommandOnce(ref);
        }
    }
};

class CommandHeld {
    const Commandref * ref;
public:
    CommandHeld(const Commandref & ref) : ref(&ref) {
        ref.commandBegin();
    }
    CommandHeld(const CommandHeld & ) = delete;
    CommandHeld(CommandHeld && ch) { ref = ch.ref; ch.ref = nullptr; }
    CommandHeld & operator=(CommandHeld && ch) { ref = ch.ref; ch.ref = nullptr; return *this; }
    CommandHeld & operator=(const CommandHeld & ch) = delete;
    ~CommandHeld() {
        if(nullptr != ref) {
            ref->commandEnd();
        }
    }
    const Commandref * getCommandref() const { return ref; }
};
