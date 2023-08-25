#include <ApprovalTests.hpp>
#include <Disarray.hpp>
#include <gtest/gtest.h>

#include "scene/Deserialiser.hpp"
#include "scene/Serialiser.hpp"

class QueueFamilyIndexMock : public Disarray::QueueFamilyIndex {
public:
	void force_recreation() override { }
	~QueueFamilyIndexMock() override = default;
	void recreate(bool b, const Disarray::Extent& extent) override { }
};

class PhysicalDeviceMock : public Disarray::PhysicalDevice {
public:
	void force_recreation() override { }
	void recreate(bool b, const Disarray::Extent& extent) override { }
	~PhysicalDeviceMock() override = default;
	Disarray::QueueFamilyIndex& get_queue_family_indexes() override { return qfi; }
	const Disarray::QueueFamilyIndex& get_queue_family_indexes() const override { return qfi; }

	QueueFamilyIndexMock qfi;
};

class DeviceMock : public Disarray::Device {
public:
	Disarray::PhysicalDevice& get_physical_device() override { return pd; }
	const Disarray::PhysicalDevice& get_physical_device() const override { return pd; }

	PhysicalDeviceMock pd {};
};

static auto device_mock = DeviceMock();

static auto json_to_string(const auto& json)
{
	std::stringstream stream;
	stream << std::setw(2) << json;
	return stream.str();
}

static auto verify_serialisation(const auto& serialiser) { ApprovalTests::Approvals::verify(json_to_string(serialiser.get_as_json())); }

TEST(SceneSerialisation, ForwardPass)
{
	Disarray::Scene s(device_mock, "Test");
	s.create("TEST1");
	s.create("TEST2");
	s.create("TEST3");
	Disarray::SceneSerialiser serialiser { s };
	verify_serialisation(serialiser);
}

TEST(SceneSerialisation, RoundTrip)
{
	Disarray::Scene s(device_mock, "Test");
	s.create("TEST1");
	s.create("TEST2");
	s.create("TEST3");
	Disarray::SceneSerialiser serialiser { s };
	auto out = serialiser.get_as_json();

	std::stringstream stream;
	stream << out;
	Disarray::Scene new_scene(device_mock, "Test");
	Disarray::SceneDeserialiser scene_deserialiser { new_scene, device_mock, stream };
	Disarray::SceneSerialiser serialiser_new { new_scene };
	auto out_new = serialiser_new.get_as_json();

	EXPECT_EQ(out, out_new);
}
