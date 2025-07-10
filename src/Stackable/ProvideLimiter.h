// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Exchanger.h"

namespace Vessel
{
	template<typename ResourceModel>
	class ProvideLimiter final : public Provider<ResourceModel>
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

		struct State
		{
			Units buffer;
			float bandwidth;
			Provider<ResourceModel>& originProvider;
		};

		// Life circle.
	public:
		inline ProvideLimiter(Provider<ResourceModel>& originProvider, Units buffer, float bandwidth = 1.f);

		// Public interface.
	public:
		// Change exchange buffer.
		inline void SetUnitsBuffer(Units newValue) { mState.buffer = newValue; }

		// Set bandwidth for any cases.
		inline void SetUnitsBandwidth(float newValue = 1.f) { mState.bandwidth = newValue; }

		// Public virtual interface substitution.
	public:
		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(ResourceModel model = {}) const override;

		// Public virtual interface substitution.
	private:
		// Provider::ReduceUnits
		inline void ReduceUnits(Units resourceRequest) override { Exchanger<ResourceModel>::ReduceUnits(mState.originProvider, resourceRequest); }

		// Private state.
	private:
		State mState;
	};

	template<typename ResourceModel>
	inline ProvideLimiter<ResourceModel>::ProvideLimiter(Provider<ResourceModel>& originProvider, ProvideLimiter::Units buffer, float bandwidth)
		: Provider<ResourceModel>{}
		, mState{ bandwidth, buffer, originProvider }
	{
	}

	template<typename ResourceModel>
	inline ProvideLimiter<ResourceModel>::Units ProvideLimiter<ResourceModel>::GetAvailableUnits(ResourceModel model) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.originProvider.GetAvailableUnits();

		return std::min(availableUnits, possibleUnits);
	}

	template<typename ResourceModel>
	Consumer<ResourceModel>& operator<<(Consumer<ResourceModel>& consumer, ProvideLimiter<ResourceModel>&& provider)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename ResourceModel>
	Provider<ResourceModel>& operator>>(ProvideLimiter<ResourceModel>&& provider, Consumer<ResourceModel>& consumer)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return provider;
	}
} // Vessel