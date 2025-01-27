// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Exchanger.h"

namespace Vessel
{
	template<typename Tag>
	class ProvideLimiter final : public Provider<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;

		struct State
		{
			Units buffer;
			float bandwidth;
			Provider<Tag>& originProvider;
		};

		// Life circle.
	public:
		inline ProvideLimiter(Provider<Tag>& originProvider, Units buffer, float bandwidth = 1.f);

		// Public interface.
	public:
		// Change exchange buffer.
		inline void SetUnitsBuffer(Units newValue) { mState.buffer = newValue; }

		// Set bandwidth for any cases.
		inline void SetUnitsBandwidth(float newValue = 1.f) { mState.bandwidth = newValue; }

		// Public virtual interface substitution.
	public:
		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(Tag tag = {}) const override;

		// Public virtual interface substitution.
	private:
		// Provider::ReduceUnits
		inline void ReduceUnits(Units resourceRequest) override { Exchanger<Tag>::ReduceUnits(mState.originProvider, resourceRequest); }

		// Private state.
	private:
		State mState;
	};

	template<typename Tag>
	inline ProvideLimiter<Tag>::ProvideLimiter(Provider<Tag>& originProvider, ProvideLimiter::Units buffer, float bandwidth)
		: Provider<Tag>{}
		, mState{ bandwidth, buffer, originProvider }
	{
	}

	template<typename Tag>
	inline ProvideLimiter<Tag>::Units ProvideLimiter<Tag>::GetAvailableUnits(Tag tag) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.originProvider.GetAvailableUnits();

		return std::min(availableUnits, possibleUnits);
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(Consumer<Tag>& consumer, ProvideLimiter<Tag>&& provider)
	{
		Exchanger<Tag>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename Tag>
	Provider<Tag>& operator>>(ProvideLimiter<Tag>&& provider, Consumer<Tag>& consumer)
	{
		Exchanger<Tag>::Exchange(provider, consumer);

		return provider;
	}
} // Vessel