// Real benchmark on YOUR CPU: sum a matrix by rows and by columns.
// Build in Release mode for meaningful times.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>

long long sumRows(const std::vector<int>& m, std::size_t n) {
    long long sum = 0;
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            sum += m[i * n + j];  // neighbors in memory
        }
    }
    return sum;
}

long long sumColumns(const std::vector<int>& m, std::size_t n) {
    long long sum = 0;
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < n; ++i) {
            sum += m[i * n + j];  // jumps n * 4 bytes every time
        }
    }
    return sum;
}

// Run `work` several times and return the fastest time in milliseconds.
template <typename Work>
double bestTimeMs(Work work, int repeats) {
    double best = 1e300;
    for (int r = 0; r < repeats; ++r) {
        const auto start = std::chrono::steady_clock::now();
        work();
        const auto end = std::chrono::steady_clock::now();
        best = std::min(best, std::chrono::duration<double, std::milli>(end - start).count());
    }
    return best;
}

int main() {
    const std::size_t n = 4096;           // 4096 x 4096 ints = 64 MB
    const std::vector<int> matrix(n * n, 1);

    long long rowSum = 0;
    long long colSum = 0;

    const double rowsMs = bestTimeMs([&] { rowSum = sumRows(matrix, n); }, 3);
    const double colsMs = bestTimeMs([&] { colSum = sumColumns(matrix, n); }, 3);

    std::cout << "Sum of a 4096 x 4096 int matrix (64 MB), best of 3 runs\n";
    std::cout << "  by rows:    " << rowsMs << " ms\n";
    std::cout << "  by columns: " << colsMs << " ms\n";
    std::cout << "  columns are " << colsMs / rowsMs << "x slower\n";
    std::cout << "  check: " << rowSum << " == " << colSum << '\n';  // use the results

    return 0;
}