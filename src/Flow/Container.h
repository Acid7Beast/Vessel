// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <algorithm>

#include "Tag.h"
#include "Consumer.h"
#include "Provider.h"

namespace Flow
{
	template<typename Tag>
	class Container final : public Consumer<Tag>, public Provider<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;

		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Life circle.
	public:
		inline Container(Units capacity);

		inline Container(const Container& other);
		inline Container(const Container&& other);
		inline Container& operator=(Container& other);
		inline void operator=(const Container& other);

		inline ~Container() = default;

		// Public interface.
	public:
		// Deserialize state of this resource container for a save.
		inline void SetAmount(Units amount);

		// Serialize state of this resource container from a save. 
		inline Units GetAmount() const;

		// Reset state of this resource container to default values.
		inline void ResetState();

		// Get capacity of this container.
		inline Units GetCapacity() const { return mCapacity; }

		// Check container for any resources.
		inline bool IsEmpty() const { return mAmount > kZeroUnits; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetRequestUnits
		inline Units GetRequestUnits(Tag tag = {}) const override { return mCapacity - mAmount; }

		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(Tag tag = {}) const override { return mAmount; }

		// Private virtual interface substitution.
	private:
		// Consumer::SupplyResource
		inline void IncreaseUnits(Units resourceSupply) override;

		// Provider::ReduceResource
		inline void ReduceUnits(Units resourceRequest) override;

		// Private state.
	private:
		Units mAmount;

		// Private properties.
	private:
		const Units mCapacity;
	};

	template<typename Tag>
	inline Container<Tag>::Container(Container<Tag>::Units capacity)
		: mCapacity{ capacity }
		, mAmount{ capacity }
	{
	}

	template<typename Tag>
	inline Container<Tag>::Container(const Container<Tag>& other)
		: mCapacity{ other.mCapacity }
	{		
	}

	template<typename Tag>
	inline Container<Tag>::Container(const Container&& other)
		: mCapacity{ other.mCapacity }
		, mAmount{ other.mAmount }
	{
	}

	template<typename Tag>
	inline Container<Tag>& Container<Tag>::operator=(Container<Tag>& other)
	{
		mCapacity = other.mCapacity;
		mAmount = std::min(mAmount, mCapacity);

		Exchanger<Tag>::Exchange(other, *this);
	}

	template<typename Tag>
	inline void Container<Tag>::operator=(const Container<Tag>& other)
	{
		mCapacity = other.mCapacity;
	}

	template<typename Tag>
	inline void Container<Tag>::SetAmount(Units amount)
	{
		mAmount = amount;
	}

	template<typename Tag>
	inline Container<Tag>::Units Container<Tag>::GetAmount() const
	{
		return mAmount;
	}

	template<typename Tag>
	inline void Container<Tag>::ResetState()
	{
		mAmount = kZeroUnits;
	}

	template<typename Tag>
	inline void Container<Tag>::IncreaseUnits(Units resourceSupply)
	{
		mAmount = std::min(mAmount + resourceSupply, mCapacity);
	}

	template<typename Tag>
	inline void Container<Tag>::ReduceUnits(Units resourceRequest)
	{
		mAmount = std::max(mAmount - resourceRequest, kZeroUnits);
	}
} // Flow