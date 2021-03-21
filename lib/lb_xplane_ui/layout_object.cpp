#include "layout_object.h"


// Send a key press to the object that will handle it. Start first with the object
// with keyboard focus, and then work our way up the tree until
// we find somebody that does want to handle it.
bool LayoutObject::dispatchKeyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    if(keyPress(key, flags, virtual_key)) {
        return true;
    }

    if(nullptr != parent) {
        return parent->dispatchKeyPress(key, flags, virtual_key);
    }

    return false;
}