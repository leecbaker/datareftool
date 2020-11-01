#pragma once

#include <exception>

// This is a backport of std::visit for OS X < 10.14. The necessary support for bad_variant_access is missing
// in earlier versions of the OS, so we'll essentially just re-implement the function but with a different
// exception.
//
// This implementation is 90% copied from libc++ as included in X-Code 11 under the MIT license.


namespace lb {

#if defined(__APPLE__)
    template <class _Visitor, class... _Vs>
    inline
    constexpr decltype(auto) visit(_Visitor&& __visitor, _Vs&&... __vs) {
    using std::__variant_detail::__visitation::__variant;
    bool __results[] = {__vs.valueless_by_exception()...};
    for (bool __result : __results) {
        if (__result) {
            throw std::out_of_range("Visiting valueless variant");
        }
    }
    return __variant::__visit_value(_VSTD::forward<_Visitor>(__visitor),
                                    _VSTD::forward<_Vs>(__vs)...);
    }
#else
    using std::visit;
#endif
}