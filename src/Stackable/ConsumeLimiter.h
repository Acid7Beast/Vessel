// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Exchanger.h"

namespace Vessel
{
	template <typename Tag>
	class ConsumeLimiter final : public Consumer<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;

		struct State
		{
			Units buffer;
			float bandwidth;
			Consumer<Tag>& originConsumer;
		};

		// Life circle.
	public:
		inline ConsumeLimiter(Consumer<Tag>& originConsumer, Units buffer, float bandwidth = 1.f);

		// Public interface.
	public:
		// Change exchange buffer.
		inline void SetUnitsBuffer(Units newValue) { mState.buffer = newValue; }

		// Set bandwidth for any cases.
		inline void SetUnitsBandwidth(float newValue = 1.f) { mState.bandwidth = newValue; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetRequestUnits
		inline Units GetRequestUnits(Tag tag = {}) const override;

		// Private virtual interface substitution.
	private:
		// Consumer::IncreaseUnits
		inline void IncreaseUnits(Units resourceRequest) override { Exchanger<Tag>::IncreaseUnits(mState.originConsumer, resourceRequest); }

		// Private state.
	private:
		State mState;
	};

	template<typename Tag>
	inline ConsumeLimiter<Tag>::ConsumeLimiter(Consumer<Tag>& originConsumer, ConsumeLimiter::Units buffer, float bandwidth)
		: Consumer<Tag>{}
		, mState{ bandwidth, buffer, originConsumer }
	{
	}

	template<typename Tag>
	inline ConsumeLimiter<Tag>::Units ConsumeLimiter<Tag>::GetRequestUnits(Tag tag) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.originConsumer.GetRequestUnits();

		return std::min(availableUnits, possibleUnits);
	}

	template<typename Tag>
	Provider<Tag>& operator>>(Provider<Tag>& provider, ConsumeLimiter<Tag>&& consumer)
	{
		Exchanger<Tag>::Exchange(provider, consumer);

		return provider;
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(ConsumeLimiter<Tag>&& consumer, Provider<Tag>& provider)
	{
		Exchanger<Tag>::Exchange(provider, consumer);

		return consumer;
	}
} // Vessel