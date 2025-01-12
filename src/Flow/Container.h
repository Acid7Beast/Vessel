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

		struct State
		{
			Units amount = kZeroUnits;
		};

		struct Properties
		{
			Units capacity = kZeroUnits;
		};

		// Life circle.
	public:
		inline Container(const Properties& properties);

		inline Container(const Container& other);
		inline Container(const Container&& other);
		inline Container& operator=(Container& other);
		inline void operator=(const Container& other);

		inline ~Container() = default;

		// Public interface.
	public:
		// Deserialize state of this resource container for a save.
		inline void LoadState(const State& state);

		// Serialize state of this resource container from a save. 
		inline void SaveState(State& state) const;

		// Reset state of this resource container to default values.
		inline void ResetState();

		// Get capacity of this container.
		inline Units GetCapacity() const { return mProperties.capacity; }

		// Check container for any resources.
		inline bool IsEmpty() const { return mState.amount > kZeroUnits; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetRequestUnits
		inline Units GetRequestUnits(Tag tag = {}) const override { return mProperties.capacity - mState.amount; }

		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(Tag tag = {}) const override { return mState.amount; }

		// Private virtual interface substitution.
	private:
		// Consumer::SupplyResource
		inline void IncreaseUnits(Units resourceSupply) override;

		// Provider::ReduceResource
		inline void ReduceUnits(Units resourceRequest) override;

		// Private state.
	private:
		State mState;

		// Private properties.
	private:
		const Properties& mProperties;
	};

	template<typename Tag>
	inline Container<Tag>::Container(const Properties& properties)
		: mProperties{ properties }
		, mState{ mState.amount = properties.capacity }
	{
	}

	template<typename Tag>
	inline Container<Tag>::Container(const Container<Tag>& other)
		: mProperties{ other.mProperties }
	{		
	}

	template<typename Tag>
	inline Container<Tag>::Container(const Container&& other)
		: mProperties{ other.mProperties }
		, mState{ other.mState }
	{
	}

	template<typename Tag>
	inline Container<Tag>& Container<Tag>::operator=(Container<Tag>& other)
	{
		mProperties = other.mProperties;
		mState.amount = std::min(mState.amount, mProperties.capacity);

		Exchanger<Tag>::Exchange(other, *this);
	}

	template<typename Tag>
	inline void Container<Tag>::operator=(const Container<Tag>& other)
	{
		mProperties = other.mProperties;
	}

	template<typename Tag>
	inline void Container<Tag>::LoadState(const State& state)
	{
		mState = state;
	}

	template<typename Tag>
	inline void Container<Tag>::SaveState(State& state) const
	{
		state = mState;
	}

	template<typename Tag>
	inline void Container<Tag>::ResetState()
	{
		mState = {};
	}

	template<typename Tag>
	inline void Container<Tag>::IncreaseUnits(Units resourceSupply)
	{
		mState.amount = std::min(mState.amount + resourceSupply, mProperties.capacity);
	}

	template<typename Tag>
	inline void Container<Tag>::ReduceUnits(Units resourceRequest)
	{
		mState.amount = std::max(mState.amount - resourceRequest, kZeroUnits);
	}
} // Flow