#define APPROVALS_GOOGLETEST_EXISTING_MAIN
#include <ApprovalTests.hpp>
#include <ApprovalTests/reporters/WindowsReporters.h>
#include <gtest/gtest.h>
#include <memory>

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	// 2. Add this line to your main:
	ApprovalTests::initializeApprovalTestsForGoogleTests();
	// main.cpp:
	auto reporter = ApprovalTests::Approvals::useAsDefaultReporter(std::make_shared<ApprovalTests::Windows::VisualStudioCodeReporter>());

	return RUN_ALL_TESTS();
}
