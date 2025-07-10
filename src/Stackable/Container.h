// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <algorithm>

#include "Tag.h"
#include "Consumer.h"
#include "Provider.h"

namespace Vessel
{
	template<typename ResourceModel>
	class Container final : public Consumer<ResourceModel>, public Provider<ResourceModel>
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

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
		// Deserialize state of this resource container from a save.
		inline void SetAmount(Units amount);

		// Serialize state of this resource container to a save. 
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
		inline Units GetRequestUnits(ResourceModel model = {}) const override { return mCapacity - mAmount; }

		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(ResourceModel model = {}) const override { return mAmount; }

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

	template<typename ResourceModel>
	inline Container<ResourceModel>::Container(Container<ResourceModel>::Units capacity)
		: mCapacity{ capacity }
		, mAmount{ capacity }
	{
	}

	template<typename ResourceModel>
	inline Container<ResourceModel>::Container(const Container<ResourceModel>& other)
		: mCapacity{ other.mCapacity }
	{		
	}

	template<typename ResourceModel>
	inline Container<ResourceModel>::Container(const Container&& other)
		: mCapacity{ other.mCapacity }
		, mAmount{ other.mAmount }
	{
	}

	template<typename ResourceModel>
	inline Container<ResourceModel>& Container<ResourceModel>::operator=(Container<ResourceModel>& other)
	{
		mCapacity = other.mCapacity;
		mAmount = std::min(mAmount, mCapacity);

		Exchanger<ResourceModel>::Exchange(other, *this);
	}

	template<typename ResourceModel>
	inline void Container<ResourceModel>::operator=(const Container<ResourceModel>& other)
	{
		mCapacity = other.mCapacity;
	}

	template<typename ResourceModel>
	inline void Container<ResourceModel>::SetAmount(Units amount)
	{
		mAmount = amount;
	}

	template<typename ResourceModel>
	inline Container<ResourceModel>::Units Container<ResourceModel>::GetAmount() const
	{
		return mAmount;
	}

	template<typename ResourceModel>
	inline void Container<ResourceModel>::ResetState()
	{
		mAmount = kZeroUnits;
	}

	template<typename ResourceModel>
	inline void Container<ResourceModel>::IncreaseUnits(Units resourceSupply)
	{
		mAmount = std::min(mAmount + resourceSupply, mCapacity);
	}

	template<typename ResourceModel>
	inline void Container<ResourceModel>::ReduceUnits(Units resourceRequest)
	{
		mAmount = std::max(mAmount - resourceRequest, kZeroUnits);
	}
} // Vessel