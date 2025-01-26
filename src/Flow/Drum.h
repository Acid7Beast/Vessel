// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "BeltInterface.h"

#include <numeric>
#include <deque>

namespace Flow
{
	template<class BasicType>
	class Drum final : public BeltInterface<BasicType>
	{
		// Public nested types.
	public:
		using ReferenceWrapper = BeltInterface<BasicType>::ReferenceWrapper;
		using OptionalRefWrapper = BeltInterface<BasicType>::OptionalRefWrapper;

		// Life circle.
	public:
		inline Drum(size_t capacity = 1u, std::optional<size_t> receiverOffset = {});

		// Public virtual interface substitution.
	public:
		// BeltInterface::ExchangeFeederSlot
		inline OptionalRefWrapper ExchangeFeederSlot(OptionalRefWrapper item = {}) override;

		// BeltInterface::ExchangeReceiverSlot
		inline OptionalRefWrapper ExchangeReceiverSlot(OptionalRefWrapper item = {}) override;

		// BeltInterface::NextBeltSlot
		inline void NextBeltSlot(size_t offset = 1u) override;

		// BeltInterface::IsEmptySlot
		inline bool IsEmptySlot(size_t offset = 0u) const override;

		// BeltInterface::GetItemCount
		inline size_t GetItemCount() const override;

		// BeltInterface::GetSlotCount
		inline size_t GetSlotCount() const override { return mCapacity; }

		// BeltInterface::GetReceiverSlotOffset
		inline size_t GetReceiverSlotOffset() const override { return mReceiverOffset; }

		// Private interface.
	private:
		// Exchange the item with the certain slot of the belt.
		inline OptionalRefWrapper ExchangeSlotAtIndex(size_t index, OptionalRefWrapper item = {});

		// Translate index to cyclic buffer.
		inline size_t TranslateIndex(size_t offset) const;

		// Exchange the item with the certain slot of the belt.
		inline std::vector<OptionalRefWrapper> GetSlotItems() const;

		// Private state.
	private:
		size_t mIndex = 0u;
		std::vector<OptionalRefWrapper> mCyclicBelt;

		// Private properties.
	private:
		const size_t mCapacity = 1u;
		const size_t mReceiverOffset = mCapacity;
	};

	template<class BasicType>
	inline Drum<BasicType>::Drum(size_t capacity, std::optional<size_t> receiverOffset)
		: mCapacity{ std::max(capacity, size_t{ 1 }) }
		, mReceiverOffset{ std::min(receiverOffset.value_or(mCapacity - 1), mCapacity - 1) }
	{
		mCyclicBelt.resize(mCapacity);
	}

	template<class BasicType>
	inline Drum<BasicType>::OptionalRefWrapper Drum<BasicType>::ExchangeFeederSlot(OptionalRefWrapper item)
	{
		const bool pushEmpty = !item.has_value();
		OptionalRefWrapper result = ExchangeSlotAtIndex(mIndex, item);

		const bool pullNonEmpty = result.has_value();
		if (pullNonEmpty && pushEmpty)
		{
			NextBeltSlot();
		}

		return result;
	}

	template<class BasicType>
	inline Drum<BasicType>::OptionalRefWrapper Drum<BasicType>::ExchangeReceiverSlot(OptionalRefWrapper item)
	{
		const bool pushNonEmpty = item.has_value();
		OptionalRefWrapper result = ExchangeSlotAtIndex(TranslateIndex(mReceiverOffset), item);

		const bool pullEmpty = !result.has_value();
		if (pullEmpty && pushNonEmpty)
		{
			NextBeltSlot();
		}

		return result;
	}

	template<class BasicType>
	inline void Drum<BasicType>::NextBeltSlot(size_t offset)
	{
		mIndex = TranslateIndex(offset);
	}

	template<class BasicType>
	inline bool Drum<BasicType>::IsEmptySlot(size_t offset) const
	{
		return !mCyclicBelt[TranslateIndex(offset)].has_value();
	}

	template<class BasicType>
	inline size_t Drum<BasicType>::GetItemCount() const
	{
		return std::accumulate(mCyclicBelt.cbegin(), mCyclicBelt.cend(), 0u, [](size_t sum, OptionalRefWrapper wrapper) -> size_t
			{
				if (!wrapper.has_value())
				{
					return sum;
				}

				return sum + 1u;
			});
	}

	template<class BasicType>
	inline Drum<BasicType>::OptionalRefWrapper Drum<BasicType>::ExchangeSlotAtIndex(size_t index, Drum<BasicType>::OptionalRefWrapper item)
	{
		item.swap(mCyclicBelt[index]);

		return item;
	}

	template<class BasicType>
	inline size_t Drum<BasicType>::TranslateIndex(size_t offset) const
	{
		return (mIndex + offset) % mCapacity;
	}

	template<class BasicType>
	inline std::vector<typename Drum<BasicType>::OptionalRefWrapper> Drum<BasicType>::GetSlotItems() const
	{
		std::vector<OptionalRefWrapper> result;
		result.reserve(mCapacity);

		for (size_t index = 0u; index < mCapacity; ++index)
		{
			result.emplace_back(mCyclicBelt[TranslateIndex(index)]);
		}

		return result;
	}
} // Flow