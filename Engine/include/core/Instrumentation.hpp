#pragma once

#include <string_view>

#define DISARRAY_PROFILE_SCOPE(scope) Disarray::Instrumentation::Profiler(scope);

namespace Disarray::Instrumentation {

class Profiler {
public:
	Profiler(std::string_view scope_name);
	~Profiler();

private:
	std::string_view name;
	double start_time;
};

} // namespace Disarray::Instrumentation
