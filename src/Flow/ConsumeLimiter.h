// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Flow.h"

#include <unordered_map>

namespace Flow
{
	template <typename Tag>
	class ConsumeLimiter final : public Consumer<Tag>
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
			Consumer<Tag>& originConsumer;
		};

		struct Cache
		{
			Pack request;
			bool isDirty = true;
		};

		// Life circle.
	public:
		inline ConsumeLimiter(Consumer<Tag>& originConsumer, Units bandwidth, float requestLimit = 1.f);

		// Public interface.
	public:
		// Change outcoming bandwith.
		inline void ChangeBandwidth(Units newValue);

		// Temporary set request limit until resource will increased.
		inline void PendRequestLimit(float newValue = 1.f);

		// Private virtual interface substitution.
	private:
		// Consumer::GetRequestResources
		inline virtual const Pack& GetRequestResources([[maybe_unused]] Tag = {}) const override;

		// Consumer::IncreaseResource
		inline virtual void IncreaseResource(Pack& resourceSupply, [[maybe_unused]] Tag = {}) override;

		// Private state.
	private:
		State _state;

		// Private cache.
	private:
		mutable Cache _cache;
	};

	template<typename Tag>
	inline ConsumeLimiter<Tag>::ConsumeLimiter(Consumer<Tag>& originConsumer, Units bandwidth, float requestLimit)
		: Consumer<Tag>{}
		, _state{ bandwidth, requestLimit, originConsumer }
	{
	}

	template<typename Tag>
	inline void ConsumeLimiter<Tag>::ChangeBandwidth(Units newValue)
	{
		_state.bandwidth = newValue;
		_cache.isDirty = true;
	}

	template<typename Tag>
	inline void ConsumeLimiter<Tag>::PendRequestLimit(float newValue)
	{
		_state.requestLimit = newValue;
		_cache.isDirty = true;
	}

	template <typename Tag>
	inline const ConsumeLimiter<Tag>::Pack& ConsumeLimiter<Tag>::GetRequestResources(Tag tag) const
	{
		if (_cache.isDirty)
		{
			_cache.request = _state.originConsumer.GetRequestResources();
			Units totalLimit = _state.bandwidth * _state.requestLimit;

			for (auto iter = _cache.request.begin(); iter != _cache.request.end(); ++iter)
			{
				Units limit = std::min(iter->second, totalLimit);
				iter->second = limit;
				totalLimit -= limit;
			}

			_cache.isDirty = false;
		}

		return _cache.request;
	}

	template <typename Tag>
	inline void ConsumeLimiter<Tag>::IncreaseResource(ConsumeLimiter<Tag>::Pack& resourceSupply, Tag tag)
	{
		Flow<Tag>::IncreaseResource(_state.originConsumer, resourceSupply);

		_cache.isDirty = true;
	}
} // Flow