#pragma once

#include <deque>
#include <numeric>
#include <cmath>

template <typename T>
T calcSTDEV(const std::deque<T>& data) {
    if (data.empty()) return 0.0;
    const T sum = std::accumulate(data.begin(), data.end(), 0.0);
    const T mean = sum / static_cast<T>(data.size());

    const T sq_sum = std::inner_product(
        data.begin(),
        data.end(),
        data.begin(),
        0.0,
        [](T a, T b) { return a + b; },
        [mean](T a, T b) { return (a - mean) * (b - mean); });

    return std::sqrt(sq_sum / static_cast<T>(data.size()));
}
