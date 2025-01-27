// (c) 2024 Acid7Beast. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>

#include <Stackable/Package.h>
#include <Stackable/PackageAdapter.h>

namespace Vessel
{
	struct PackTestTag {};

	enum class TestResource : uint8_t
	{
		Test1,
		Test2,
	};

	// Specialize for Test.
	template <>
	struct TagSelector<PackTestTag> {
		using Units = float;
		using ResourceId = TestResource;
	};
}

namespace {
	using Tag = ::Vessel::PackTestTag;
	using ResourceId = ::Vessel::Package<Tag>::ResourceId;
	using Container = ::Vessel::Container<Tag>;
	using PackageInterface = ::Vessel::PackageInterface<Tag>;
	using Package = ::Vessel::Package<Tag>;
	using PackageAdapter = ::Vessel::PackageAdapter<Tag>;
	using Units = Package::Units;

	constexpr Units kEmptyAmountKg = 0.f;
	constexpr Units kCapacityAmountKg = 255.f;
	constexpr Units kHalfCapacityAmountKg = kCapacityAmountKg * 0.5f;

	static const std::unordered_map<ResourceId, Package::Units> kContainerProperties
	{
		{ ResourceId::Test1, kCapacityAmountKg},
		{ ResourceId::Test2, kCapacityAmountKg},
	};

	class PackageChecker
	{
		// Life circle.
	public:
		PackageChecker(const PackageInterface& package)
			: mPackage{ package }
		{
		}

		// Public interface.
	public:
		// Check 100% fullness state.
		void CheckFullState(ResourceId resourceId) const
		{
			const Container& container = mPackage.GetContainer(resourceId);

			EXPECT_FLOAT_EQ(container.GetRequestUnits(), kEmptyAmountKg);
			EXPECT_FLOAT_EQ(container.GetAvailableUnits(), kCapacityAmountKg);
		}

		// Check 50% fullness state.
		void CheckHalfState(ResourceId resourceId) const
		{
			const Container& container = mPackage.GetContainer(resourceId);

			EXPECT_FLOAT_EQ(container.GetRequestUnits(), kHalfCapacityAmountKg);
			EXPECT_FLOAT_EQ(container.GetAvailableUnits(), kHalfCapacityAmountKg);
		}

		// Check 0% fullness state.
		void CheckEmptyState(ResourceId resourceId) const
		{
			const Container& container = mPackage.GetContainer(resourceId);

			EXPECT_FLOAT_EQ(container.GetRequestUnits(), kCapacityAmountKg);
			EXPECT_FLOAT_EQ(container.GetAvailableUnits(), kEmptyAmountKg);
		}

		// Private state.
	private:
		const PackageInterface& mPackage;
	};

	class PackageFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Package consumerPackage{ kContainerProperties };
		Package providerPackage{ kContainerProperties };
		PackageChecker consumerChecker{ consumerPackage };
		PackageChecker providerChecker{ providerPackage };
	};

	TEST_F(PackageFixture, ConstructorTest) {
		// Resources are full on creation.
		consumerChecker.CheckFullState(ResourceId::Test1);
		consumerChecker.CheckFullState(ResourceId::Test2);

		providerChecker.CheckFullState(ResourceId::Test1);
		providerChecker.CheckFullState(ResourceId::Test2);
	}

	TEST_F(PackageFixture, StateLoadTest) {
		// Empty containers.
		const Package::ContainerStateTable emptyStateTable
		{
			{ResourceId::Test1, { kEmptyAmountKg }},
			{ResourceId::Test2, { kEmptyAmountKg }},
		};

		// Check state loading.
		consumerPackage.LoadState(emptyStateTable);
		consumerChecker.CheckEmptyState(ResourceId::Test1);
		consumerChecker.CheckEmptyState(ResourceId::Test2);
	}
	TEST_F(PackageFixture, SaveStateTest) {
		// Try to save.
		Package::ContainerStateTable testSafeStateTable;
		consumerPackage.SaveState(testSafeStateTable);

		EXPECT_EQ(testSafeStateTable.at(ResourceId::Test1), kCapacityAmountKg);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Test2), kCapacityAmountKg);
	}

	TEST_F(PackageFixture, ExchangeStateTest) {
		// Half of capacity containers.
		const Package::ContainerStateTable halfStateTable
		{
			{ResourceId::Test1, { kHalfCapacityAmountKg }},
			{ResourceId::Test2, { kHalfCapacityAmountKg }},
		};

		// Check state loading.
		consumerPackage.LoadState(halfStateTable);
		consumerChecker.CheckHalfState(ResourceId::Test1);
		consumerChecker.CheckHalfState(ResourceId::Test2);
		providerChecker.CheckFullState(ResourceId::Test1);
		providerChecker.CheckFullState(ResourceId::Test2);

		// Try to save.
		Package::ContainerStateTable testSateStateTable;
		consumerPackage.SaveState(testSateStateTable);
		consumerChecker.CheckHalfState(ResourceId::Test1);
		consumerChecker.CheckHalfState(ResourceId::Test2);

		// Try to load.
		providerPackage.LoadState(testSateStateTable);
		providerChecker.CheckHalfState(ResourceId::Test1);
		providerChecker.CheckHalfState(ResourceId::Test2);
	}

	TEST_F(PackageFixture, ExchangeTest) {
		// Empty consumer.
		consumerPackage.GetContainer(ResourceId::Test1).SetAmount(kEmptyAmountKg);
		consumerPackage.GetContainer(ResourceId::Test2).SetAmount(kEmptyAmountKg);
		consumerChecker.CheckEmptyState(ResourceId::Test1);
		consumerChecker.CheckEmptyState(ResourceId::Test2);

		// Try and check resource transfer.
		providerPackage >> consumerPackage;
		consumerChecker.CheckFullState(ResourceId::Test1);
		consumerChecker.CheckFullState(ResourceId::Test2);
		providerChecker.CheckEmptyState(ResourceId::Test1);
		providerChecker.CheckEmptyState(ResourceId::Test2);

		// Try and check one more resource transfer.
		providerPackage << consumerPackage;
		consumerChecker.CheckEmptyState(ResourceId::Test1);
		consumerChecker.CheckEmptyState(ResourceId::Test2);
		providerChecker.CheckFullState(ResourceId::Test1);
		providerChecker.CheckFullState(ResourceId::Test2);
	}

	TEST_F(PackageFixture, AdapterTest) {
		// Create a test container to adapt.
		Package::Container testContainer{ kCapacityAmountKg };
		testContainer.SetAmount(kEmptyAmountKg);
		{
			PackageAdapter testAdapter{ ResourceId::Test2, testContainer };
			PackageChecker checker{ testAdapter };
			checker.CheckEmptyState(ResourceId::Test2);
		}

		// Supply resources to the test container.
		providerPackage >> PackageAdapter(ResourceId::Test2, testContainer);
		providerChecker.CheckFullState(ResourceId::Test1);
		providerChecker.CheckEmptyState(ResourceId::Test2);
		{
			PackageAdapter testAdapter{ ResourceId::Test2, testContainer };
			PackageChecker checker{ testAdapter };
			checker.CheckFullState(ResourceId::Test2);
		}
	}
} // namespace