// (c) 2024 Acid7Beast. Use with wisdom.
#include <gtest/gtest.h>
#include <format>
#include <ostream>
#include <numeric>

#include <Polymorphic/Drum.h>
#include <Polymorphic/Queue.h>

namespace
{
	constexpr size_t kCapacityCount = 6u;
	constexpr std::string_view kIncendiaryType = "Incendiary";
	constexpr std::string_view kExpansiveType = "Expansive";

	// Basice type of the test. Usually, a player meets in a game some kind of ammos.
	class Ammo
	{
	public:
		virtual ~Ammo() = default;

	public:
		virtual std::string_view GetType() const = 0;
	};

	// This type will ignite on contact.
	class Incendiary final : public Ammo
	{
	public:
		std::string_view GetType() const override { return kIncendiaryType; }
	};

	// This type will do more damage.
	class Expansive final : public Ammo
	{
	public:
		std::string_view GetType() const override { return kExpansiveType; }
	};

	using BeltInterface = ::Flow::BeltInterface<Ammo>;
	using Drum = ::Flow::Drum<Ammo>;
	using Queue = ::Flow::Queue<Ammo>;
	using ReferenceWrapper = ::Flow::Drum<Ammo>::ReferenceWrapper;
	using OptionalRefWrapper = ::Flow::Drum<Ammo>::OptionalRefWrapper;

	class BeltChecker final
	{
		// Life circle.
	public:
		BeltChecker(const BeltInterface& belt)
			: mBelt{ belt }
		{
		}

		// Public interface.
	public:
		// Template to iterate and check every item with custom checker.
		template <typename Callable>
		void IterateItems(const std::vector<OptionalRefWrapper>& items, Callable&& callback) const
		{
			for (size_t index = 0; index < items.size(); ++index)
			{
				callback(index, items[index]);
			}
		}

		// Check non-empty slots.
		void CheckOccupiedSlots(const std::set<size_t>& indices) const
		{
			std::vector<OptionalRefWrapper> items = mBelt.GetSlotItems();

			IterateItems(items, [&](size_t index, const OptionalRefWrapper& item) {
				EXPECT_EQ(item.has_value(), indices.contains(index));
				});
		}

		// Check types of inherited class.
		void CheckItemTypes(const std::vector<std::string>& types) const
		{
			std::vector<OptionalRefWrapper> items = mBelt.GetSlotItems();

			IterateItems(items, [&](size_t index, const OptionalRefWrapper& item) {
				if (index >= types.size())
				{
					EXPECT_TRUE(false);
					return;
				}

				if (item.has_value() && !types[index].empty())
				{
					EXPECT_EQ(item.value().get().GetType(), types[index]);
				}
				});
		}

		// Check common state of the belt.
		void CheckState(size_t itemsCount, size_t capacityCount) const
		{
			EXPECT_EQ(itemsCount, mBelt.GetItemCount());
			EXPECT_EQ(capacityCount, mBelt.GetSlotCount());
		}

		// Private properties.
	private:
		const BeltInterface& mBelt;
	};

	class BeltFixture : public ::testing::Test
	{
		// Inheritable state.
	protected:
		Drum drum{ kCapacityCount };
		Queue queue{ kCapacityCount };
		BeltChecker drumChecker{ drum };
		BeltChecker queueChecker{ queue };
		Incendiary incendiaryRef{};
		Expansive expansiveRef{};
	};

	TEST_F(BeltFixture, ConstructorTest)
	{
		drumChecker.CheckOccupiedSlots({});
		drumChecker.CheckState(0u, kCapacityCount);

		queueChecker.CheckOccupiedSlots({});
		queueChecker.CheckState(0u, kCapacityCount);
	}

	TEST_F(BeltFixture, ExchangeTest)
	{
		// Push item with feeder by exchange it with empty slot.
		{
			OptionalRefWrapper expectedItem = drum.ExchangeFeederSlot(incendiaryRef);
			drumChecker.CheckOccupiedSlots({ 0u });
			EXPECT_FALSE(expectedItem.has_value());
		}
		// Pull item from the feeder exchanging it with nothing.
		{
			OptionalRefWrapper expectedItem = drum.ExchangeFeederSlot();
			drumChecker.CheckOccupiedSlots({});
			EXPECT_TRUE(expectedItem.has_value());
		}
	}

	TEST_F(BeltFixture, DrumTurnTest)
	{
		// Push some type of item to the drum to imitate a revolver.
		drum.ExchangeFeederSlot(incendiaryRef);
		drumChecker.CheckOccupiedSlots({ 0u });
		drumChecker.CheckState(1u, kCapacityCount);
		drumChecker.CheckItemTypes({
			kIncendiaryType.data(),
			{},
			{},
			{},
			{},
			{},
			});

		// The drum made a turn.
		drum.NextBeltSlot();
		drumChecker.CheckOccupiedSlots({ kCapacityCount - 1 });
		drumChecker.CheckState(1u, kCapacityCount);
		drumChecker.CheckItemTypes({
			{},
			{},
			{},
			{},
			{},
			kIncendiaryType.data(),
			});

		// Push another type of item to the drum.
		drum.ExchangeFeederSlot(expansiveRef);
		drumChecker.CheckOccupiedSlots({ 0u, kCapacityCount - 1 });
		drumChecker.CheckState(2u, kCapacityCount);
		drumChecker.CheckItemTypes({
			kExpansiveType.data(),
			{},
			{},
			{},
			{},
			kIncendiaryType.data(),
			});

		// The drum made a one more turn.
		drum.NextBeltSlot();
		drumChecker.CheckOccupiedSlots({ kCapacityCount - 2, kCapacityCount - 1 });
		drumChecker.CheckState(2u, kCapacityCount);
		drumChecker.CheckItemTypes({
			{},
			{},
			{},
			{},
			kIncendiaryType.data(),
			kExpansiveType.data(),
			});

		// Turn the drum until an item on the feeder appears.
		while (drum.IsEmptySlot())
		{
			drum.NextBeltSlot();
		}
		drumChecker.CheckOccupiedSlots({ 0u, 1u });
		drumChecker.CheckState(2u, kCapacityCount);
		drumChecker.CheckItemTypes({
			kIncendiaryType.data(),
			kExpansiveType.data(),
			{},
			{},
			{},
			{},
			});

		// Turn the drum until are no items on the feeder.
		while (!drum.IsEmptySlot())
		{
			drum.NextBeltSlot();
		}
		drumChecker.CheckOccupiedSlots({ 4u, 5u });
		drumChecker.CheckState(2u, kCapacityCount);
		drumChecker.CheckItemTypes({
			{},
			{},
			{},
			{},
			kIncendiaryType.data(),
			kExpansiveType.data(),
			});

		// Push one more item to the drum.
		drum.ExchangeFeederSlot(incendiaryRef);
		drumChecker.CheckOccupiedSlots({ 0u, 4u, 5u });
		drumChecker.CheckState(3u, kCapacityCount);
		drumChecker.CheckItemTypes({
			kIncendiaryType.data(),
			{},
			{},
			{},
			kIncendiaryType.data(),
			kExpansiveType.data(),
			});

		// The drum made a double turn.
		drum.NextBeltSlot(2u);
		drumChecker.CheckOccupiedSlots({ 2u, 3u, 4u });
		drumChecker.CheckState(3u, kCapacityCount);
		drumChecker.CheckItemTypes({
			{},
			{},
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kIncendiaryType.data(),
			{},
			});


		// Push an item into receiver.
		drum.ExchangeReceiverSlot(expansiveRef);
		drumChecker.CheckOccupiedSlots({ 1u, 2u, 3u, 4u });
		drumChecker.CheckState(kCapacityCount - 2, kCapacityCount);
		drumChecker.CheckItemTypes({
			{},
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kIncendiaryType.data(),
			kExpansiveType.data(),
			{},
			});

		// Push an item into receiver.
		drum.ExchangeReceiverSlot(expansiveRef);
		drumChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u });
		drumChecker.CheckState(kCapacityCount - 1, kCapacityCount);
		drumChecker.CheckItemTypes({
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			{},
			});

		// Push an item into receiver.
		drum.ExchangeReceiverSlot(expansiveRef);
		drumChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u, 5u });
		drumChecker.CheckState(kCapacityCount, kCapacityCount);
		drumChecker.CheckItemTypes({
			kExpansiveType.data(),
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			kIncendiaryType.data(),
			});

		// Try push an item into receiver.
		drum.ExchangeReceiverSlot(expansiveRef);
		drumChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u, 5u });
		drumChecker.CheckState(kCapacityCount, kCapacityCount);
		drumChecker.CheckItemTypes({
			kExpansiveType.data(),
			kIncendiaryType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			kExpansiveType.data(),
			});
	}

	TEST_F(BeltFixture, QueueExchangeTest)
	{
		// Push some type of item to the queue.
		queue.ExchangeFeederSlot(incendiaryRef);
		queueChecker.CheckOccupiedSlots({ 0u });
		queueChecker.CheckState(1u, kCapacityCount);
		queueChecker.CheckItemTypes({
			kIncendiaryType.data(),
			{},
			{},
			{},
			{},
			{},
			});

		// Push some type of item to the queue.
		queue.ExchangeFeederSlot(expansiveRef);
		queueChecker.CheckOccupiedSlots({ 0u, 1u });
		queueChecker.CheckState(2u, kCapacityCount);
		queueChecker.CheckItemTypes({
			kExpansiveType.data(),
			kIncendiaryType.data(),
			{},
			{},
			{},
			{},
			});

		// Push some type of item to the queue.
		queue.ExchangeReceiverSlot(expansiveRef);
		queueChecker.CheckOccupiedSlots({ 0u, 1u, 2u });
		queueChecker.CheckState(3u, kCapacityCount);
		queueChecker.CheckItemTypes({
			kExpansiveType.data(),
			kIncendiaryType.data(),
			kExpansiveType.data(),
			{},
			{},
			{},
			});

		// Check overflow.
		{
			const size_t freeSlots = queue.GetSlotCount() - queue.GetItemCount();
			size_t extraItems = 0u;
			for (size_t iter = 0u; iter < kCapacityCount; ++iter)
			{
				OptionalRefWrapper pushedItem = queue.ExchangeReceiverSlot(incendiaryRef);
				if (pushedItem.has_value()) {
					++extraItems;
				}
			}
			EXPECT_EQ(extraItems, kCapacityCount - freeSlots);
			queueChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u, 5u });
			queueChecker.CheckState(kCapacityCount, kCapacityCount);
			queueChecker.CheckItemTypes({
				kExpansiveType.data(),
				kIncendiaryType.data(),
				kExpansiveType.data(),
				kIncendiaryType.data(),
				kIncendiaryType.data(),
				kIncendiaryType.data(),
				});
		}
	}

	TEST_F(BeltFixture, ComboTest)
	{
		for (size_t iter = 0u; iter < kCapacityCount; ++iter)
		{
			queue.ExchangeReceiverSlot(incendiaryRef);
		}
		queueChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u, 5u });
		queueChecker.CheckState(kCapacityCount, kCapacityCount);
		queueChecker.CheckItemTypes({
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			});

		while (drum.GetItemCount() < drum.GetSlotCount())
		{
			drum << queue;
		}

		queueChecker.CheckOccupiedSlots({});
		queueChecker.CheckState(0u, kCapacityCount);

		drumChecker.CheckOccupiedSlots({ 0u, 1u, 2u, 3u, 4u, 5u });
		drumChecker.CheckState(kCapacityCount, kCapacityCount);
		drumChecker.CheckItemTypes({
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			kIncendiaryType.data(),
			});
	}
} // namespace