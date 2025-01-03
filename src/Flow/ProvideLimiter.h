// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Flow.h"

#include <unordered_map>

namespace Flow
{
	template <typename Tag>
	class ProvideLimiter final : public Provider<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using Resource = TagSelector<Tag>::Resource;
		using Pack = TagSelector<Tag>::Pack;

		struct State
		{
			Units bandwidth;
			float requestLimit;
			Provider<Tag>& originProvider;
		};

		struct Cache
		{
			Pack supply;
			bool isDirty = true;
		};

		// Life circle.
	public:
		inline ProvideLimiter(Provider<Tag>& originProvider, Units bandwidth, float requestLimit = 1.f);

		// Public interface.
	public:
		// Change outcoming bandwith.
		inline void ChangeBandwidth(Units newValue);

		// Temporary set request limit until resource will reduced.
		inline void PendRequestLimit(float newValue = 1.f);

		// Public virtual interface substitution.
	public:
		// Provider::GetAvailableResources
		inline virtual const Pack& GetAvailableResources([[maybe_unused]] Tag = {}) const override;

		// Provider::ReduceResource
		inline virtual void ReduceResource(Pack& resourceRequest, [[maybe_unused]] Tag = {}) override;

		// Private state.
	private:
		State _state;

		// Private cache.
	private:
		mutable Cache _cache;
	};

	template<typename Tag>
	inline ProvideLimiter<Tag>::ProvideLimiter(Provider<Tag>& originProvider, ProvideLimiter<Tag>::Units bandwidth, float requestLimit)
		: Provider<Tag>{}
		, _state{ bandwidth, requestLimit, originProvider }
	{
	}

	template<typename Tag>
	inline void ProvideLimiter<Tag>::ChangeBandwidth(Units newValue)
	{
		_state.bandwidth = newValue;
		_cache.isDirty = true;
	}

	template<typename Tag>
	inline void ProvideLimiter<Tag>::PendRequestLimit(float newValue)
	{
		_state.requestLimit = newValue;
		_cache.isDirty = true;
	}

	template<typename Tag>
	inline const ProvideLimiter<Tag>::Pack& ProvideLimiter<Tag>::GetAvailableResources(Tag tag) const
	{
		if (_cache.isDirty)
		{
			_cache.supply = _state.originProvider.GetAvailableResources();
			Units totalLimit = _state.bandwidth * _state.requestLimit;

			for (auto iter = _cache.supply.begin(); iter != _cache.supply.end(); ++iter)
			{
				Units limit = std::min(iter->second, totalLimit);
				iter->second = limit;
				totalLimit -= limit;
			}

			_cache.isDirty = false;
		}

		return _cache.supply;
	}

	template <typename Tag>
	inline void ProvideLimiter<Tag>::ReduceResource(ProvideLimiter<Tag>::Pack& resourceRequest, Tag tag)
	{
		Flow<Tag>::ReduceResource(_state.originProvider, resourceRequest);

		_cache.isDirty = true;
	}
} // Flow