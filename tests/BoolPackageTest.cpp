// (c) 2024 Acid7Beast. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>
#include <limits>

#include <Stackable/Package.h>
#include <Stackable/Container.h>

namespace {
	struct PackTestTag
	{
		using Units = float;

		enum class ResourceId : uint8_t
		{
			Stealth,
			Guarding,
			Count,
		};

		//static constexpr bool CheckResourceFlow = true;
	};
	
	using StateResourceModel = ::Vessel::ResourceModel<PackTestTag>;
	using ResourceId = StateResourceModel::ResourceId;
	using Units = StateResourceModel::Units;
	using Package = ::Vessel::Package<StateResourceModel>;
	using Container = ::Vessel::Container<StateResourceModel>;

	// REGISTER_RESOURCE(State, bool, Stealth, Guarding);
	REGISTER_RESOURCE_LITERAL(StateResourceModel, Stealth);
	REGISTER_RESOURCE_LITERAL(StateResourceModel, Guarding);

	constexpr Units kInactiveState = false;
	constexpr Units kActiveState = true;

	static const Package::ResourceTable kContainerCapacities
	{
		{ ResourceId::Stealth, true },
		{ ResourceId::Guarding, true },
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
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kInactiveState);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kActiveState);
		}

		// Check 0% fullness state.
		void CheckEmptyState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kActiveState);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kInactiveState);
		}

		// Private state.
	private:
		const Package& mPackage;
	};

	class BoolPackageFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Package consumerPackage{ kContainerCapacities };
		Package providerPackage{ kContainerCapacities };
		PackageChecker consumerChecker{ consumerPackage };
		PackageChecker providerChecker{ providerPackage };
	};

	TEST_F(BoolPackageFixture, ConstructorTest) {
		// Resources are full on creation.
		consumerChecker.CheckEmptyState(ResourceId::Stealth);
		consumerChecker.CheckEmptyState(ResourceId::Guarding);

		providerChecker.CheckEmptyState(ResourceId::Stealth);
		providerChecker.CheckEmptyState(ResourceId::Guarding);
	}

	TEST_F(BoolPackageFixture, StoleResourceTest) {
		providerPackage.LoadState(kContainerCapacities);
		providerChecker.CheckFullState(ResourceId::Stealth);
		providerChecker.CheckFullState(ResourceId::Guarding);
		consumerChecker.CheckEmptyState(ResourceId::Stealth);
		consumerChecker.CheckEmptyState(ResourceId::Guarding);

		consumerPackage = providerPackage;

		providerChecker.CheckEmptyState(ResourceId::Stealth);
		providerChecker.CheckEmptyState(ResourceId::Guarding);
		consumerChecker.CheckFullState(ResourceId::Stealth);
		consumerChecker.CheckFullState(ResourceId::Guarding);

		providerPackage = std::move(consumerPackage);
		consumerChecker.CheckEmptyState(ResourceId::Stealth);
		consumerChecker.CheckEmptyState(ResourceId::Guarding);
		providerChecker.CheckFullState(ResourceId::Stealth);
		providerChecker.CheckFullState(ResourceId::Guarding);
	}

	TEST_F(BoolPackageFixture, StateLoadTest) {
		// Check state loading.
		consumerPackage.LoadState(kContainerCapacities);
		consumerChecker.CheckFullState(ResourceId::Stealth);
		consumerChecker.CheckFullState(ResourceId::Guarding);
	}
	TEST_F(BoolPackageFixture, SaveStateTest) {
		// Try to save.
		Package::ResourceTable testSafeStateTable;
		consumerPackage.SaveState(testSafeStateTable);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Stealth), kInactiveState);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Guarding), kInactiveState);
	}

	TEST_F(BoolPackageFixture, TransferStateTest) {
		// Check state loading.
		providerPackage.LoadState(kContainerCapacities);
		providerChecker.CheckFullState(ResourceId::Stealth);
		providerChecker.CheckFullState(ResourceId::Guarding);

		// Try to save.
		Package::ResourceTable testSateStateTable;
		consumerPackage.SaveState(testSateStateTable);

		// Try to load.
		providerPackage.LoadState(testSateStateTable);
	}

	TEST_F(BoolPackageFixture, TransferTest) {
		// Empty consumer.
		consumerChecker.CheckEmptyState(ResourceId::Stealth);
		consumerChecker.CheckEmptyState(ResourceId::Guarding);

		providerPackage << Container{ ResourceId::Stealth, kActiveState };
		providerPackage << Container{ ResourceId::Guarding, kActiveState };
		providerChecker.CheckFullState(ResourceId::Stealth);
		providerChecker.CheckFullState(ResourceId::Guarding);

		// Try and check resource transfer.
		providerPackage >> consumerPackage;
		consumerChecker.CheckFullState(ResourceId::Stealth);
		consumerChecker.CheckFullState(ResourceId::Guarding);
		providerChecker.CheckEmptyState(ResourceId::Stealth);
		providerChecker.CheckEmptyState(ResourceId::Guarding);
	}

	TEST_F(BoolPackageFixture, LiteralTest) {
		consumerPackage << 1Stealth << 1Guarding;

		consumerChecker.CheckFullState(ResourceId::Stealth);
		consumerChecker.CheckFullState(ResourceId::Guarding);
	}
} // namespace