#pragma once

#include <benchmark/benchmark.h>

#include <filesystem>
#include <stdexcept>

#include "Disarray.hpp"
#include "core/Log.hpp"
#include "graphics/Device.hpp"

inline void benchmark_pipeline_compiler(benchmark::State& state)
{
	using namespace Disarray;
	Logging::Logger::initialise_logger("info");

	for (auto value : state) { }
}
