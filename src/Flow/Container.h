// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <algorithm>
#include <unordered_map>

#include "Tag.h"
#include "Consumer.h"
#include "Provider.h"

namespace Flow
{
	template<typename Tag>
	class Container : public Consumer<Tag>, public Provider<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using Resource = TagSelector<Tag>::Resource;
		using Pack = TagSelector<Tag>::Pack;

		struct State
		{
			Units amount = static_cast<Units>(0);
		};

		struct Cache
		{
			Pack request;
			Pack supply;
			bool isDirty = true;
		};

		struct Properties
		{
			Units capacity;
			Resource type;
		};

		// Life circle.
	public:
		inline Container(const Properties& properties);

		// Public interface.
	public:
		// Deserialize state of this resource container for saving.
		inline void LoadState(const State& state);

		// Serialize state of this resource container. 
		inline void SaveState(State& state) const;

		// Reset state to default.
		inline void ResetState();

		// Get container properties.
		inline const Properties& GetProperties() const { return _properties; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetAvailableAmount
		inline const Pack& GetRequestResources([[maybe_unused]] Tag = {}) const override;

		// Provider::GetAvailableAmount
		inline const Pack& GetAvailableResources([[maybe_unused]] Tag = {}) const override;

		// Private virtual interface substitution.
	private:
		// Consumer::SupplyResource
		inline void IncreaseResource(Pack& resourceSupply, [[maybe_unused]] Tag = {}) override;

		// Provider::ReduceResource
		inline void ReduceResource(Pack& resourceRequest, [[maybe_unused]] Tag = {}) override;

		// Inheritable interface.
	protected:
		// Make cache dirty.
		inline void SetDirty();

		// Private interface.
	private:
		// Try rebuild if the cache is dirty.
		inline void RebuildCacheIfRequired() const;

		// Private state.
	private:
		State _state;

		// Private cache.
	private:
		mutable Cache _cache;

		// Private properties.
	private:
		const Properties& _properties;
	};

	template<typename Tag>
	inline Container<Tag>::Container(const Properties& properties)
		: _properties{ properties }
		, _state{ _state.amount = properties.capacity }
	{
	}

	template<typename Tag>
	inline void Container<Tag>::LoadState(const State& state)
	{
		_state = state;

		_cache.isDirty = true;
	}

	template<typename Tag>
	inline void Container<Tag>::SaveState(State& state) const
	{
		state = _state;
	}

	template<typename Tag>
	inline void Container<Tag>::ResetState()
	{
		_state = {};
	}

	template<typename Tag>
	inline void Container<Tag>::RebuildCacheIfRequired() const
	{
		constexpr Units kZero = static_cast<Units>(0);

		if (!_cache.isDirty)
		{
			return;
		}

		_cache.supply[_properties.type] = std::max(_state.amount, kZero);
		_cache.request[_properties.type] = std::max(_properties.capacity - _state.amount, kZero);

		_cache.isDirty = false;
	}

	template<typename Tag>
	inline const Container<Tag>::Pack& Container<Tag>::GetRequestResources(Tag) const
	{
		RebuildCacheIfRequired();

		return { _cache.request };
	}


	template<typename Tag>
	inline const Container<Tag>::Pack& Container<Tag>::GetAvailableResources(Tag) const
	{
		RebuildCacheIfRequired();

		return { _cache.supply };
	}

	template<typename Tag>
	inline void Container<Tag>::IncreaseResource(Pack& resourceSupply, Tag)
	{
		_state.amount = std::min(_state.amount + resourceSupply[_properties.type], _properties.capacity);

		_cache.isDirty = true;
	}

	template<typename Tag>
	inline void Container<Tag>::ReduceResource(Pack& resourceRequest, Tag)
	{
		constexpr Units zero = static_cast<Units>(0);

		_state.amount = std::max(_state.amount - resourceRequest[_properties.type], zero);

		_cache.isDirty = true;
	}

	template<typename Tag>
	inline void Container<Tag>::SetDirty()
	{
		_cache.isDirty = true;
	}

} // Flow