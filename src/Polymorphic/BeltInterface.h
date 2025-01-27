// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <optional>
#include <memory>
#include <vector>

namespace Vessel
{
	template<class BasicType>
	class SupplyChain;

	template<class BasicType>
	class BeltInterface
	{
		// Public nested types.
	public:
		using ReferenceWrapper = std::reference_wrapper<const BasicType>;
		using OptionalRefWrapper = std::optional<ReferenceWrapper>;

		// Life circle.
	public:
		virtual ~BeltInterface() = default;

	public:
		// Public virtual interface.
	public:
		// Load and init the state of slot items.
		virtual void SetSlotItems(std::vector<OptionalRefWrapper> slotItems) {};

		// Exchange the item with the feeder slot of the belt.
		virtual OptionalRefWrapper ExchangeFeederSlot(OptionalRefWrapper item = {}) = 0;

		// Exchange the item with the receiver slot of the belt.
		virtual OptionalRefWrapper ExchangeReceiverSlot(OptionalRefWrapper item = {}) = 0;

		// Move the belt to next slot.
		virtual void NextBeltSlot(size_t offset = 1u) {}

		// Is here free slot on belt end.
		virtual bool IsEmptySlot(size_t offset = 0u) const = 0;

		// Get non-empty items count.
		virtual size_t GetItemCount() const = 0;

		// Get slot capacity count.
		virtual size_t GetSlotCount() const = 0;

		// Get receiver slot offset for feeder.
		virtual size_t GetReceiverSlotOffset() const = 0;

		// Get slot items for saving or displaying.
		virtual std::vector<OptionalRefWrapper> GetSlotItems() const = 0;

		// Inheritable friend types.
	protected:
		friend SupplyChain<BasicType>;
	};

	template<class BasicType>
	class Exchanger
	{
		// Public nested types.
	public:
		using ReferenceWrapper = std::reference_wrapper<const BasicType>;
		using OptionalRefWrapper = std::optional<ReferenceWrapper>;

		// Static public interface.
	public:
		static OptionalRefWrapper PullItem(BeltInterface<BasicType>& feeder)
		{
			if (feeder.GetItemCount() == 0)
			{
				return {};
			}

			while (feeder.IsEmptySlot())
			{
				feeder.NextBeltSlot();
			}

			return feeder.ExchangeFeederSlot({});
		}

		static OptionalRefWrapper PushItem(BeltInterface<BasicType>& receiver, OptionalRefWrapper item)
		{
			if (receiver.GetSlotCount() == receiver.GetItemCount())
			{
				return item;
			}

			while (!receiver.IsEmptySlot(receiver.GetReceiverSlotOffset()))
			{
				receiver.NextBeltSlot();
			}

			return receiver.ExchangeReceiverSlot(item);
		}

		static void Exchange(BeltInterface<BasicType>& receiver, BeltInterface<BasicType>& feeder)
		{
			OptionalRefWrapper item = PullItem(feeder);

			if (!item.has_value())
			{
				return;
			}

			PushItem(receiver, item);
		}
	};


	template<class BasicType>
	BeltInterface<BasicType>& operator<<(BeltInterface<BasicType>& receiver, BeltInterface<BasicType>& feeder)
	{
		Exchanger<BasicType>::Exchange(receiver, feeder);

		return receiver;
	}

	template<class BasicType>
	BeltInterface<BasicType>& operator>>(BeltInterface<BasicType>& feeder, BeltInterface<BasicType>& receiver)
	{
		Exchanger<BasicType>::Exchange(receiver, feeder);

		return feeder;
	}
} // Vessel