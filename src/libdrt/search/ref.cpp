#include "ref.h"

std::ostream & operator<<(std::ostream & o, const ref_src_t source) {
    switch(source) {
        case ref_src_t::AIRCRAFT:
            o << "aircraft";
            return o;
        case ref_src_t::BLACKLIST:
            o << "blacklist";
            return o;
        case ref_src_t::FILE:
            o << "file";
            return o;
        case ref_src_t::PLUGIN:
            o << "plugin";
            return o;
        case ref_src_t::USER_MSG:
            o << "message";
            return o;
        case ref_src_t::X_PLANE:
            o << "X-Plane";
            return o;
        case ref_src_t::LUA:
            o << "Lua";
            return o;
        default:
            o << "other";
            return o;
    }
}
