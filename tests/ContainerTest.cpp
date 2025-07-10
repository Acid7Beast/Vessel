// (c) 2024 Acid7Beast. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>

#include <Stackable/Container.h>
#include <Stackable/ProvideLimiter.h>
#include <Stackable/ConsumeLimiter.h>

namespace {
	enum class TestResource : uint8_t
	{
		Test1,
		Test2,
		Count,
	};

	struct ContainerTestTag {
		using ResourceId = TestResource;
		using Units = float;
	};

	using KgResourceModel = ::Vessel::ResourceModel<ContainerTestTag>;
	using Container = ::Vessel::Container<KgResourceModel>;
	using ProvideLimiter = ::Vessel::ProvideLimiter<KgResourceModel>;
	using ConsumeLimiter = ::Vessel::ConsumeLimiter<KgResourceModel>;
	using Units = KgResourceModel::Units;

	constexpr Units kEmptyAmountKg = 0.f;
	constexpr Units kCapacityAmountKg = 255.f;
	constexpr Units kHalfCapacityAmountKg = kCapacityAmountKg * 0.5f;

	class ContainerChecker
	{
		// Life circle.
	public:
		ContainerChecker(const Container& container)
			: _container{ container }
		{
		}

		// Public interface.
	public:
		// Check 100% fullness state.
		void CheckFullState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestUnits(), kEmptyAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableUnits(), kCapacityAmountKg);
		}

		// Check 50% fullness state.
		void CheckHalfState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestUnits(), kHalfCapacityAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableUnits(), kHalfCapacityAmountKg);
		}

		// Check 0% fullness state.
		void CheckEmptyState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestUnits(), kCapacityAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableUnits(), kEmptyAmountKg);
		}

		// Private state.
	private:
		const Container& _container;
	};

	class ContainerFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Container provider{ kCapacityAmountKg };
		Container consumer{ kCapacityAmountKg };
		ContainerChecker providerChecker{ provider };
		ContainerChecker consumerChecker{ consumer };
	};

	TEST_F(ContainerFixture, ConstructorTest) {
		// Resources are full on creation.
		providerChecker.CheckFullState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ContainerFixture, LoadStateTest) {
		// Make the consumer empty.
		consumer.SetAmount(kEmptyAmountKg);
		consumerChecker.CheckEmptyState();

		// Make the consumer full.
		consumer.SetAmount(kCapacityAmountKg);
		consumerChecker.CheckFullState();

		// Make the consumer half.
		consumer.SetAmount(kHalfCapacityAmountKg);
		consumerChecker.CheckHalfState();

		// Make the consumer empty again.
		consumer.SetAmount(kEmptyAmountKg);
		consumerChecker.CheckEmptyState();
	}

	TEST_F(ContainerFixture, SaveStateTest) {
		// Check moving empty state to another container.
		consumer.SetAmount(kEmptyAmountKg);
		provider.SetAmount(consumer.GetAmount());
		consumerChecker.CheckEmptyState();
		providerChecker.CheckEmptyState();

		// Check moving full state to another container.
		consumer.SetAmount(kCapacityAmountKg);
		provider.SetAmount(consumer.GetAmount());
		consumerChecker.CheckFullState();
		providerChecker.CheckFullState();

		// Check moving half state to another container.
		consumer.SetAmount(kHalfCapacityAmountKg);
		provider.SetAmount(consumer.GetAmount());
		consumerChecker.CheckHalfState();
		providerChecker.CheckHalfState();

		// Check moving empty state to another container again.
		consumer.SetAmount(kEmptyAmountKg);
		provider.SetAmount(consumer.GetAmount());
		consumerChecker.CheckEmptyState();
		providerChecker.CheckEmptyState();
	}

	TEST_F(ContainerFixture, ProvideOperatorTestIn) {
		// Check resource transit to consumer with `<<`.
		consumer.SetAmount(kEmptyAmountKg);
		consumer << provider << provider;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ContainerFixture, ProvideOperatorTestOut) {
		// Check resource transit to consumer with `>>`.
		consumer.SetAmount(kEmptyAmountKg);
		provider >> consumer >> consumer;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}
	
	TEST_F(ContainerFixture, ProvideLimiterTestIn) {
		// Check resource transit through limiter with `<<`.
		consumer.SetAmount(kEmptyAmountKg);
		consumer << ProvideLimiter(provider, kHalfCapacityAmountKg);
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		consumer << ProvideLimiter(provider, kHalfCapacityAmountKg);
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ContainerFixture, ProvideLimiterTestOut) {
		// Check resource transit through limiter with `>>`.
		consumer.SetAmount(kEmptyAmountKg);
		ProvideLimiter(provider, kHalfCapacityAmountKg) >> consumer;
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		ProvideLimiter(provider, kHalfCapacityAmountKg) >> consumer;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ContainerFixture, ConsumeLimiterTestIn) {
		// Check resource transit through limiter with `<<`.
		consumer.SetAmount(kEmptyAmountKg);
		ConsumeLimiter(consumer, kHalfCapacityAmountKg) << provider;
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		ConsumeLimiter(consumer, kHalfCapacityAmountKg) << provider;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ContainerFixture, ConsumeLimiterTestOut) {
		// Check resource transit through limiter with `>>`.
		consumer.SetAmount(kEmptyAmountKg);
		provider >> ConsumeLimiter(consumer, kHalfCapacityAmountKg);
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		provider >> ConsumeLimiter(consumer, kHalfCapacityAmountKg);
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}
} // namespace