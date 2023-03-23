#include <benchmark/benchmark.h>

#include "omp.h"
#include "get_matrix.h"

constexpr auto THREAD_NUM = 4;

static void BM_Matrix_DiffSizeSameThreadNum(benchmark::State& state) {
    omp_set_num_threads(THREAD_NUM);

    for (auto _ : state)
    {
        auto size = state.range(0);
        auto firstMatrix = util::GetRandomMatrix(size, size);
        auto secondMatrix = util::GetRandomMatrix(size, size);

        benchmark::DoNotOptimize(firstMatrix * secondMatrix);
    }
}

BENCHMARK(BM_Matrix_DiffSizeSameThreadNum)->RangeMultiplier(2)->Range(2 << 5, 2 << 12);

constexpr auto MATRIX_SIZE = 2e3;

static void BM_Matrix_SameSizeDiffThreadNum(benchmark::State& state) {
    auto firstMatrix = util::GetRandomMatrix(MATRIX_SIZE, MATRIX_SIZE);
    auto secondMatrix = util::GetRandomMatrix(MATRIX_SIZE, MATRIX_SIZE);

    for (auto _ : state)
    {
        omp_set_num_threads(state.range(0));
        benchmark::DoNotOptimize(firstMatrix * secondMatrix);
    }
}

BENCHMARK(BM_Matrix_SameSizeDiffThreadNum)->DenseRange(1, 12, 1);

BENCHMARK_MAIN();