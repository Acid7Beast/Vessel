// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

#include "Provider.h"
#include "Consumer.h"

namespace Vessel
{
	enum class ExchangeResult : bool
	{
		Unchanged = false,
		Changed = true,
	};

	template<typename Tag>
	class ProvideLimiter;

	template<typename Tag>
	class ConsumeLimiter;

	template<typename Tag>
	class Vessel final
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using ResourceId = TagSelector<Tag>::ResourceId;

		// Public static interface.
	public:
		// Supply the consumer requested needs from the provider.
		static ExchangeResult Exchange(Provider<Tag>& provider, Consumer<Tag>& consumer);

		// Private friend.
	private:
		friend ProvideLimiter<Tag>;
		friend ConsumeLimiter<Tag>;

		// Private constants.
	public:
		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Private static interface.
	private:
		// Increase resource.
		static void IncreaseUnits(Consumer<Tag>& consumer, Consumer<Tag>::Units& supply);

		// Reduce resource.
		static void ReduceUnits(Provider<Tag>& provider, Provider<Tag>::Units& request);
	};

	template<typename Tag>
	inline ExchangeResult Vessel<Tag>::Exchange(Provider<Tag>& provider, Consumer<Tag>& consumer)
	{
		constexpr Units kEpsilonUnits = std::numeric_limits<Units>::epsilon();

		if (provider.GetProvidableId() != consumer.GetConsumableId())
		{
			return ExchangeResult::Unchanged;
		}

		const Units demandUnits = consumer.GetRequestUnits();
		const Units supplyUnits = provider.GetAvailableUnits();

		const Units compromise = std::clamp(demandUnits, kZeroUnits, supplyUnits);
		if (compromise < kEpsilonUnits)
		{
			return ExchangeResult::Unchanged;
		}

		consumer.IncreaseUnits(compromise);
		provider.ReduceUnits(compromise);

		return ExchangeResult::Changed;
	}

	template<typename Tag>
	inline void Vessel<Tag>::IncreaseUnits(Consumer<Tag>& consumer, Consumer<Tag>::Units& supply)
	{
		consumer.IncreaseUnits(supply);
	}

	template<typename Tag>
	inline void Vessel<Tag>::ReduceUnits(Provider<Tag>& provider, Provider<Tag>::Units& request)
	{
		provider.ReduceUnits(request);
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(Consumer<Tag>& consumer, Provider<Tag>& provider)
	{
		Vessel<Tag>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename Tag>
	Provider<Tag>& operator>>(Provider<Tag>& provider, Consumer<Tag>& consumer)
	{
		Vessel<Tag>::Exchange(provider, consumer);

		return provider;
	}
} // Vessel
