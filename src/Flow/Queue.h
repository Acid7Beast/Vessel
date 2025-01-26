// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "BeltInterface.h"

#include <numeric>
#include <deque>
#include <ranges>

namespace Flow
{
	template<class BasicType>
	class Queue final : public BeltInterface<BasicType>
	{
		// Public nested types.
	public:
		using ReferenceWrapper = BeltInterface<BasicType>::ReferenceWrapper;
		using OptionalRefWrapper = BeltInterface<BasicType>::OptionalRefWrapper;

		// Life circle.
	public:
		inline Queue(size_t capacity = 1u);

		// Public virtual interface substitution.
	public:
		// BeltInterface::ExchangeFeederSlot
		inline OptionalRefWrapper ExchangeFeederSlot(OptionalRefWrapper item = {}) override;

		// BeltInterface::ExchangeReceiverSlot
		inline OptionalRefWrapper ExchangeReceiverSlot(OptionalRefWrapper item = {}) override;

		// BeltInterface::IsEmptySlot
		inline bool IsEmptySlot(size_t offset = 0u) const override;

		// BeltInterface::GetItemCount
		inline size_t GetItemCount() const override;

		// BeltInterface::GetSlotCount
		inline size_t GetSlotCount() const override { return mCapacity; }

		// BeltInterface::GetReceiverSlotOffset
		inline size_t GetReceiverSlotOffset() const override { return mCapacity - 1; }

		// BeltInterface::GetSlotItems
		inline std::vector<OptionalRefWrapper> GetSlotItems() const override;

		// Private state.
	private:
		std::deque<OptionalRefWrapper> mBelt;

		// Private properties.
	private:
		const size_t mCapacity = 1u;
	};

	template<class BasicType>
	inline Queue<BasicType>::Queue(size_t capacity)
		: mCapacity{ capacity }
	{
	}

	template<class BasicType>
	inline Queue<BasicType>::OptionalRefWrapper Queue<BasicType>::ExchangeFeederSlot(OptionalRefWrapper item)
	{
		const bool hasFreeSlots = mBelt.size() < mCapacity;
		if (item.has_value() && hasFreeSlots)
		{
			mBelt.push_front(item);
			return {};
		}

		mBelt.front().swap(item);

		if (!mBelt.front().has_value())
		{
			mBelt.pop_front();
		}

		return item;
	}

	template<class BasicType>
	inline Queue<BasicType>::OptionalRefWrapper Queue<BasicType>::ExchangeReceiverSlot(OptionalRefWrapper item)
	{
		const bool hasFreeSlots = mBelt.size() < mCapacity;
		if (item.has_value() && hasFreeSlots)
		{
			mBelt.push_back(item);
			return {};
		}

		mBelt.back().swap(item);

		return item;
	}

	template<class BasicType>
	inline bool Queue<BasicType>::IsEmptySlot(size_t offset) const
	{
		return !mBelt[offset % mCapacity].has_value();
	}

	template<class BasicType>
	inline size_t Queue<BasicType>::GetItemCount() const
	{
		return std::accumulate(mBelt.cbegin(), mBelt.cend(), 0u, [](size_t sum, OptionalRefWrapper wrapper) -> size_t
			{
				if (!wrapper.has_value())
				{
					return sum;
				}

				return sum + 1u;
			});
	}

	template<class BasicType>
	inline std::vector<typename Queue<BasicType>::OptionalRefWrapper> Queue<BasicType>::GetSlotItems() const
	{
		std::vector<OptionalRefWrapper> result(mCapacity);

		for (size_t index = 0u; index < mBelt.size(); ++index)
		{
			result[index] = mBelt[index];
		}

		return result;
	}
} // Flow