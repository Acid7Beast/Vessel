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
			Steel,
			Wood,
			Count,
		};

		//static constexpr bool CheckResourceFlow = true;
	};
	
	using KgResourceModel = ::Vessel::ResourceModel<PackTestTag>;
	using ResourceId = KgResourceModel::ResourceId;
	using Units = KgResourceModel::Units;
	using Package = ::Vessel::Package<KgResourceModel>;
	using Container = ::Vessel::Container<KgResourceModel>;

	// REGISTER_RESOURCE(Kg, float, Steel, Wood);
	REGISTER_RESOURCE_LITERAL(KgResourceModel, Steel);
	REGISTER_RESOURCE_LITERAL(KgResourceModel, Wood);

	constexpr Units kEmptyAmountKg = 0.f;
	constexpr Units kCapacityAmountKg = 500.f;
	constexpr Units kHalfCapacityAmountKg = kCapacityAmountKg * 0.5f;

	static const Package::ResourceTable kContainerCapacities
	{
		{ ResourceId::Steel, kCapacityAmountKg },
		{ ResourceId::Wood, kCapacityAmountKg },
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
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kEmptyAmountKg);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kCapacityAmountKg);
		}

		// Check 50% fullness state.
		void CheckHalfState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kHalfCapacityAmountKg);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kHalfCapacityAmountKg);
		}

		// Check 0% fullness state.
		void CheckEmptyState(ResourceId resourceId) const
		{
			EXPECT_FLOAT_EQ(mPackage.GetRequestedUnits(resourceId), kCapacityAmountKg);
			EXPECT_FLOAT_EQ(mPackage.GetAvailableUnits(resourceId), kEmptyAmountKg);
		}

		// Private state.
	private:
		const Package& mPackage;
	};

	class FloatPackageFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Package consumerPackage{ kContainerCapacities };
		Package providerPackage{ kContainerCapacities };
		PackageChecker consumerChecker{ consumerPackage };
		PackageChecker providerChecker{ providerPackage };
	};

	TEST_F(FloatPackageFixture, ConstructorTest) {
		// Resources are full on creation.
		consumerChecker.CheckEmptyState(ResourceId::Steel);
		consumerChecker.CheckEmptyState(ResourceId::Wood);

		providerChecker.CheckEmptyState(ResourceId::Steel);
		providerChecker.CheckEmptyState(ResourceId::Wood);
	}

	TEST_F(FloatPackageFixture, StoleResourceTest) {
		providerPackage.LoadState(kContainerCapacities);
		providerChecker.CheckFullState(ResourceId::Steel);
		providerChecker.CheckFullState(ResourceId::Wood);
		consumerChecker.CheckEmptyState(ResourceId::Steel);
		consumerChecker.CheckEmptyState(ResourceId::Wood);

		consumerPackage = providerPackage;

		providerChecker.CheckEmptyState(ResourceId::Steel);
		providerChecker.CheckEmptyState(ResourceId::Wood);
		consumerChecker.CheckFullState(ResourceId::Steel);
		consumerChecker.CheckFullState(ResourceId::Wood);

		providerPackage = std::move(consumerPackage);
		consumerChecker.CheckEmptyState(ResourceId::Steel);
		consumerChecker.CheckEmptyState(ResourceId::Wood);
		providerChecker.CheckFullState(ResourceId::Steel);
		providerChecker.CheckFullState(ResourceId::Wood);
	}

	TEST_F(FloatPackageFixture, StateLoadTest) {
		// Check state loading.
		consumerPackage.LoadState(kContainerCapacities);
		consumerChecker.CheckFullState(ResourceId::Steel);
		consumerChecker.CheckFullState(ResourceId::Wood);
	}
	TEST_F(FloatPackageFixture, SaveStateTest) {
		// Try to save.
		Package::ResourceTable testSafeStateTable;
		consumerPackage.SaveState(testSafeStateTable);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Steel), kEmptyAmountKg);
		EXPECT_EQ(testSafeStateTable.at(ResourceId::Wood), kEmptyAmountKg);
	}

	TEST_F(FloatPackageFixture, TransferStateTest) {
		// Half of capacity containers.
		const Package::ResourceTable halfStateTable
		{
			{ResourceId::Steel, kHalfCapacityAmountKg },
			{ResourceId::Wood, kHalfCapacityAmountKg },
		};

		// Check state loading.
		consumerPackage.LoadState(halfStateTable);
		providerPackage.LoadState(kContainerCapacities);
		consumerChecker.CheckHalfState(ResourceId::Steel);
		consumerChecker.CheckHalfState(ResourceId::Wood);
		providerChecker.CheckFullState(ResourceId::Steel);
		providerChecker.CheckFullState(ResourceId::Wood);

		// Try to save.
		Package::ResourceTable testSateStateTable;
		consumerPackage.SaveState(testSateStateTable);
		consumerChecker.CheckHalfState(ResourceId::Steel);
		consumerChecker.CheckHalfState(ResourceId::Wood);

		// Try to load.
		providerPackage.LoadState(testSateStateTable);
		providerChecker.CheckHalfState(ResourceId::Steel);
		providerChecker.CheckHalfState(ResourceId::Wood);
	}

	TEST_F(FloatPackageFixture, TransferTest) {
		// Empty consumer.
		consumerChecker.CheckEmptyState(ResourceId::Steel);
		consumerChecker.CheckEmptyState(ResourceId::Wood);

		providerPackage << Container{ ResourceId::Steel, kCapacityAmountKg };
		providerPackage << Container{ ResourceId::Wood, kCapacityAmountKg };
		providerChecker.CheckFullState(ResourceId::Steel);
		providerChecker.CheckFullState(ResourceId::Wood);

		// Try and check resource transfer.
		providerPackage >> consumerPackage;
		consumerChecker.CheckFullState(ResourceId::Steel);
		consumerChecker.CheckFullState(ResourceId::Wood);
		providerChecker.CheckEmptyState(ResourceId::Steel);
		providerChecker.CheckEmptyState(ResourceId::Wood);
	}

	TEST_F(FloatPackageFixture, LiteralTest) {
		consumerPackage << 500Steel << 500Wood;

		consumerChecker.CheckFullState(ResourceId::Steel);
		consumerChecker.CheckFullState(ResourceId::Wood);

		const Package::ResourceTable halfStateTable
		{
			250Steel,
			250Wood,
		};

		consumerPackage.LoadState(halfStateTable);
		consumerChecker.CheckHalfState(ResourceId::Steel);
		consumerChecker.CheckHalfState(ResourceId::Wood);
	}
} // namespace