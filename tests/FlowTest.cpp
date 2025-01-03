// (c) 2024 Acid7Beast. Version 1.0.0. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>

#include <Flow/Container.h>
#include <Flow/Flow.h>
#include <Flow/Tag.h>
#include <Flow/ProvideLimiter.h>
#include <Flow/ConsumeLimiter.h>

namespace Flow
{
	enum class TestResource : uint8_t
	{
		Test,
	};

	struct TestTag {};

	// Specialize for Test.
	template <>
	struct TagSelector<TestTag> {
		using Units = float;
		using Resource = TestResource;
		using Pack = std::unordered_map<Resource, Units>;
	};
}

namespace {
	using Tag = ::Flow::TestTag;
	using Container = ::Flow::Container<Tag>;
	using ProvideLimiter = ::Flow::ProvideLimiter<Tag>;
	using ConsumeLimiter = ::Flow::ConsumeLimiter<Tag>;
	using Units = Container::Units;
	using Resource = Container::Resource;

	constexpr Units kEmptyAmountKg = 0.f;
	constexpr Units kCapacityAmountKg = 255.f;
	constexpr Units kHalfCapacityAmountKg = kCapacityAmountKg * 0.5f;

	class ResourceContainerChecker
	{
		// Life circle.
	public:
		ResourceContainerChecker(const Container& container)
			: _container{ container }
		{
		}

		// Public interface.
	public:
		// Check 100% fullness state.
		void CheckFullState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestResources().at(Resource::Test), kEmptyAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableResources().at(Resource::Test), kCapacityAmountKg);
		}

		// Check 50% fullness state.
		void CheckHalfState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestResources().at(Resource::Test), kHalfCapacityAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableResources().at(Resource::Test), kHalfCapacityAmountKg);
		}

		// Check 0% fullness state.
		void CheckEmptyState() const
		{
			EXPECT_FLOAT_EQ(_container.GetRequestResources().at(Resource::Test), kCapacityAmountKg);
			EXPECT_FLOAT_EQ(_container.GetAvailableResources().at(Resource::Test), kEmptyAmountKg);
		}

		// Private state.
	private:
		const Container& _container;
	};

	class ResourceContainerFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Container::Properties properties{ kCapacityAmountKg, Resource::Test };
		Container::State emptyState{ kEmptyAmountKg };
		Container::State halfState{ kHalfCapacityAmountKg };
		Container::State fullState{ kCapacityAmountKg };
		Container::State testState;
		Container provider{ properties };
		Container consumer{ properties };
		ResourceContainerChecker providerChecker{ provider };
		ResourceContainerChecker consumerChecker{ consumer };
	};

	TEST_F(ResourceContainerFixture, ConstructorTest) {
		// Resources are full on creation.
		providerChecker.CheckFullState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ResourceContainerFixture, LoadStateTest) {
		// Make the consumer empty.
		consumer.LoadState(emptyState);
		consumerChecker.CheckEmptyState();

		// Make the consumer full.
		consumer.LoadState(fullState);
		consumerChecker.CheckFullState();

		// Make the consumer half.
		consumer.LoadState(halfState);
		consumerChecker.CheckHalfState();

		// Make the consumer empty again.
		consumer.LoadState(emptyState);
		consumerChecker.CheckEmptyState();
	}

	TEST_F(ResourceContainerFixture, SaveStateTest) {
		// Check moving empty state to another container.
		consumer.LoadState(emptyState);
		consumer.SaveState(testState);
		provider.LoadState(testState);
		consumerChecker.CheckEmptyState();
		providerChecker.CheckEmptyState();

		// Check moving full state to another container.
		consumer.LoadState(fullState);
		consumer.SaveState(testState);
		provider.LoadState(testState);
		consumerChecker.CheckFullState();
		providerChecker.CheckFullState();

		// Check moving half state to another container.
		consumer.LoadState(halfState);
		consumer.SaveState(testState);
		provider.LoadState(testState);
		consumerChecker.CheckHalfState();
		providerChecker.CheckHalfState();

		// Check moving empty state to another container again.
		consumer.LoadState(emptyState);
		consumer.SaveState(testState);
		provider.LoadState(testState);
		consumerChecker.CheckEmptyState();
		providerChecker.CheckEmptyState();
	}

	TEST_F(ResourceContainerFixture, ProvideOperatorTestIn) {
		// Check resource transit to consumer with `<<`.
		consumer.LoadState(emptyState);
		consumer << provider << provider;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ResourceContainerFixture, ProvideOperatorTestOut) {
		// Check resource transit to consumer with `>>`.
		consumer.LoadState(emptyState);
		provider >> consumer >> consumer;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ResourceContainerFixture, ProvideLimiterTest) {
		// Check resource transit through limiter with `>>`.
		const float frameDelta = 1.f;

		consumer.LoadState(emptyState);
		ProvideLimiter(provider, kHalfCapacityAmountKg, frameDelta) >> consumer;
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		consumer << ProvideLimiter(provider, kHalfCapacityAmountKg, frameDelta);
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}

	TEST_F(ResourceContainerFixture, ConsumeLimiterTest) {
		// Check resource transit through limiter with `>>`.
		const float frameDelta = 1.f;

		consumer.LoadState(emptyState);
		provider >> ConsumeLimiter(consumer, kHalfCapacityAmountKg, frameDelta);
		providerChecker.CheckHalfState();
		consumerChecker.CheckHalfState();

		ConsumeLimiter(consumer, kHalfCapacityAmountKg, frameDelta) << provider;
		providerChecker.CheckEmptyState();
		consumerChecker.CheckFullState();
	}
} // namespace