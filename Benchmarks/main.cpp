#include <benchmark/benchmark.h>

#include "cases/ModelLoader.hpp"
#include "cases/PipelineCompiler.hpp"

// Register the function as a benchmark
BENCHMARK(benchmark_model_loader);
BENCHMARK(benchmark_pipeline_compiler);
