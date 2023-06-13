#include "ankerl/unordered_dense.h"
#include "pyoptinterface/core.hpp"

template <typename K, typename V>
using map = ankerl::unordered_dense::map<K, V>;

template <typename V>
using set = ankerl::unordered_dense::set<V>;

auto main() -> int {
    auto m = map<int, int>();

    m.emplace(1, 2);
    m.emplace(3, 4);

    auto s = set<int>();

    s.emplace(100);
    return 0;
}