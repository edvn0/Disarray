#pragma once

#include <benchmark/benchmark.h>

#include <filesystem>
#include <stdexcept>

#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"

inline void benchmark_model_loader(benchmark::State& state)
{
	using namespace Disarray;
	Logging::Logger::initialise_logger("info");
	AssimpModelLoader loader {};
	for (auto value : state) {
		try {
			auto loaded = loader.import("Assets/Models/sponza/sponza.obj");
		} catch (const CouldNotLoadModelException& exc) {
			Log::info("Benchmark", "{}", exc.what());
		}
	}
}
