#include "core/Instrumentation.hpp"

#include "core/Clock.hpp"
#include "core/Log.hpp"

namespace Disarray::Instrumentation {

Profiler::Profiler(std::string_view scope_name)
	: name(scope_name)
	, start_time(Clock::nanos<double>())
{
}

Profiler::~Profiler()
{
	start_time = Clock::nanos<double>() - start_time;
	Log::info("Profile", "Scope {} took {}ms.", name, 1000.0 * start_time);
}

} // namespace Disarray::Instrumentation
