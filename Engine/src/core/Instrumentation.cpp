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
	const auto end_time = Clock::nanos<double>() - start_time;
	Log::to_file("Profile", "Scope {} took {}ms.", name, 1000.0 * end_time);
}

} // namespace Disarray::Instrumentation
