// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Exchanger.h"

namespace Vessel
{
	template <typename ResourceModel>
	class ConsumeLimiter final : public Consumer<ResourceModel>
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

		struct State
		{
			Units buffer;
			float bandwidth;
			Consumer<ResourceModel>& originConsumer;
		};

		// Life circle.
	public:
		inline ConsumeLimiter(Consumer<ResourceModel>& originConsumer, Units buffer, float bandwidth = 1.f);

		// Public interface.
	public:
		// Change exchange buffer.
		inline void SetUnitsBuffer(Units newValue) { mState.buffer = newValue; }

		// Set bandwidth for any cases.
		inline void SetUnitsBandwidth(float newValue = 1.f) { mState.bandwidth = newValue; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetRequestUnits
		inline Units GetRequestUnits(ResourceModel model = {}) const override;

		// Private virtual interface substitution.
	private:
		// Consumer::IncreaseUnits
		inline void IncreaseUnits(Units resourceRequest) override { Exchanger<ResourceModel>::IncreaseUnits(mState.originConsumer, resourceRequest); }

		// Private state.
	private:
		State mState;
	};

	template<typename ResourceModel>
	inline ConsumeLimiter<ResourceModel>::ConsumeLimiter(Consumer<ResourceModel>& originConsumer, ConsumeLimiter::Units buffer, float bandwidth)
		: Consumer<ResourceModel>{}
		, mState{ bandwidth, buffer, originConsumer }
	{
	}

	template<typename ResourceModel>
	inline ConsumeLimiter<ResourceModel>::Units ConsumeLimiter<ResourceModel>::GetRequestUnits(ResourceModel ResourceModel) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.originConsumer.GetRequestUnits();

		return std::min(availableUnits, possibleUnits);
	}

	template<typename ResourceModel>
	Provider<ResourceModel>& operator>>(Provider<ResourceModel>& provider, ConsumeLimiter<ResourceModel>&& consumer)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return provider;
	}

	template<typename ResourceModel>
	Consumer<ResourceModel>& operator<<(ConsumeLimiter<ResourceModel>&& consumer, Provider<ResourceModel>& provider)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return consumer;
	}
} // Vessel