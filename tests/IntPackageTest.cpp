// (c) 2024 Acid7Beast. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>

#include <Stackable/Package.h>
#include <Stackable/Container.h>

namespace {
	struct PackTestTag
	{
		using Units = int;

		enum class ResourceId : uint8_t
		{
			Health,
			Armor,
			Count,
		};

		//static constexpr bool CheckResourceFlow = true;
	};

	using PointResourceModel = ::Vessel::ResourceModel<PackTestTag>;
	using ResourceId = PointResourceModel::ResourceId;
	using Units = PointResourceModel::Units;
	using Package = ::Vessel::Package<PointResourceModel>;
	using Container = ::Vessel::Container<PointResourceModel>;

	// REGISTER_RESOURCE(Points, int, Health, Armor);
	REGISTER_RESOURCE_LITERAL(PointResourceModel, Health);
	REGISTER_RESOURCE_LITERAL(PointResourceModel, Armor);

	constexpr Units kEmptyPoints = 0;
	constexpr Units kCapacityAmountPoints = 100;
	constexpr Units kHalfCapacityAmountPoints = kCapacityAmountPoints / 2;

	static const Package::ResourceTable kContainerCapacities
	{
		{ ResourceId::Health, kCapacityAmountPoints },
		{ ResourceId::Armor, kCapacityAmountPoints },
	};

	class PackageChecker
	{
		// Life circle.
	public:
		PackageChecker(const Package& package)
			: mPackage{ package }
		{
		}

		// Public interface.
	public:
		// Check 100% fullness state.
		void CheckFullState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kEmptyPoints);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kCapacityAmountPoints);
		}

		// Check 50% fullness state.
		void CheckHalfState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kHalfCapacityAmountPoints);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kHalfCapacityAmountPoints);
		}

		// Check 0% fullness state.
		void CheckEmptyState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kCapacityAmountPoints);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kEmptyPoints);
		}

		// Private state.
	private:
		const Package& mPackage;
	};

	class IntPackageFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Package consumerPackage{ kContainerCapacities };
		Package providerPackage{ kContainerCapacities };
		PackageChecker consumerChecker{ consumerPackage };
		PackageChecker providerChecker{ providerPackage };
	};

	TEST_F(IntPackageFixture, ConstructorTest) {
		// Resources are full on creation.
		consumerChecker.CheckEmptyState(ResourceId::Health);
		consumerChecker.CheckEmptyState(ResourceId::Armor);

		providerChecker.CheckEmptyState(ResourceId::Health);
		providerChecker.CheckEmptyState(ResourceId::Armor);
	}

	TEST_F(IntPackageFixture, StoleResourceTest) {
		providerPackage.LoadState(kContainerCapacities);
		providerChecker.CheckFullState(ResourceId::Health);
		providerChecker.CheckFullState(ResourceId::Armor);
		consumerChecker.CheckEmptyState(ResourceId::Health);
		consumerChecker.CheckEmptyState(ResourceId::Armor);

		consumerPackage = providerPackage;

		providerChecker.CheckEmptyState(ResourceId::Health);
		providerChecker.CheckEmptyState(ResourceId::Armor);
		consumerChecker.CheckFullState(ResourceId::Health);
		consumerChecker.CheckFullState(ResourceId::Armor);

		providerPackage = std::move(consumerPackage);
		consumerChecker.CheckEmptyState(ResourceId::Health);
		consumerChecker.CheckEmptyState(ResourceId::Armor);
		providerChecker.CheckFullState(ResourceId::Health);
		providerChecker.CheckFullState(ResourceId::Armor);
	}

	TEST_F(IntPackageFixture, StateLoadTest) {
		// Check state loading.
		consumerPackage.LoadState(kContainerCapacities);
		consumerChecker.CheckFullState(ResourceId::Health);
		consumerChecker.CheckFullState(ResourceId::Armor);
	}
	TEST_F(IntPackageFixture, SaveStateTest) {
		// Try to save.
		Package::ResourceTable testSafeStateTable;
		consumerPackage.SaveState(testSafeStateTable);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Health), kEmptyPoints);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Armor), kEmptyPoints);
	}

	TEST_F(IntPackageFixture, TransferStateTest) {
		// Half of capacity containers.
		const Package::ResourceTable halfStateTable
		{
			{ ResourceId::Health, kHalfCapacityAmountPoints },
			{ ResourceId::Armor, kHalfCapacityAmountPoints },
		};

		// Check state loading.
		consumerPackage.LoadState(halfStateTable);
		providerPackage.LoadState(kContainerCapacities);
		consumerChecker.CheckHalfState(ResourceId::Health);
		consumerChecker.CheckHalfState(ResourceId::Armor);
		providerChecker.CheckFullState(ResourceId::Health);
		providerChecker.CheckFullState(ResourceId::Armor);

		// Try to save.
		Package::ResourceTable testSateStateTable;
		consumerPackage.SaveState(testSateStateTable);
		consumerChecker.CheckHalfState(ResourceId::Health);
		consumerChecker.CheckHalfState(ResourceId::Armor);

		// Try to load.
		providerPackage.LoadState(testSateStateTable);
		providerChecker.CheckHalfState(ResourceId::Health);
		providerChecker.CheckHalfState(ResourceId::Armor);
	}

	TEST_F(IntPackageFixture, TransferTest) {
		// Empty consumer.
		consumerChecker.CheckEmptyState(ResourceId::Health);
		consumerChecker.CheckEmptyState(ResourceId::Armor);

		providerPackage << Container{ ResourceId::Health, kCapacityAmountPoints };
		providerPackage << Container{ ResourceId::Armor, kCapacityAmountPoints };
		providerChecker.CheckFullState(ResourceId::Health);
		providerChecker.CheckFullState(ResourceId::Armor);

		// Try and check resource transfer.
		providerPackage >> consumerPackage;
		consumerChecker.CheckFullState(ResourceId::Health);
		consumerChecker.CheckFullState(ResourceId::Armor);
		providerChecker.CheckEmptyState(ResourceId::Health);
		providerChecker.CheckEmptyState(ResourceId::Armor);
	}

	TEST_F(IntPackageFixture, LiteralTest) {
		consumerPackage << 100Health << 100Armor;

		consumerChecker.CheckFullState(ResourceId::Health);
		consumerChecker.CheckFullState(ResourceId::Armor);

		const Package::ResourceTable halfStateTable
		{
			50Health,
			50Armor,
		};

		consumerPackage.LoadState(halfStateTable);
		consumerChecker.CheckHalfState(ResourceId::Health);
		consumerChecker.CheckHalfState(ResourceId::Armor);
	}
} // namespace