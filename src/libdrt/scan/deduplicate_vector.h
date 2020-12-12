#pragma once

#include <algorithm>
#include <vector>

template <class T>
void inline deduplicate_vector(std::vector<T> & v) {
    std::sort(v.begin(), v.end());
    auto last_unique = std::unique(v.begin(), v.end());
    v.erase(last_unique, v.end());
}