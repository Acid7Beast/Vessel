// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "ResourceModel.h"

#include "Provider.h"
#include "Consumer.h"

namespace Vessel
{
	enum class ExchangeResult : bool
	{
		Unchanged = false,
		Changed = true,
	};

	template<typename ResourceModel>
	class ProvideLimiter;

	template<typename ResourceModel>
	class ConsumeLimiter;

	template<typename ResourceModel>
	class Package;

	template<typename ResourceModel>
	class Exchanger final
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

		// Public static interface.
	public:
		// Supply the consumer requested needs from the provider.
		static ExchangeResult Exchange(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer);

		// Private friend.
	private:
		friend ProvideLimiter<ResourceModel>;
		friend ConsumeLimiter<ResourceModel>;
		friend Package<ResourceModel>;

		// Private constants.
	public:
		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Private static interface.
	private:
		// Increase resource.
		static void IncreaseUnits(Consumer<ResourceModel>& consumer, Consumer<ResourceModel>::Units& supply);

		// Reduce resource.
		static void ReduceUnits(Provider<ResourceModel>& provider, Provider<ResourceModel>::Units& request);
	};

	template<typename ResourceModel>
	inline ExchangeResult Exchanger<ResourceModel>::Exchange(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer)
	{
		constexpr Units kEpsilonUnits = std::numeric_limits<Units>::epsilon();

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

	template<typename ResourceModel>
	inline void Exchanger<ResourceModel>::IncreaseUnits(Consumer<ResourceModel>& consumer, Consumer<ResourceModel>::Units& supply)
	{
		consumer.IncreaseUnits(supply);
	}

	template<typename ResourceModel>
	inline void Exchanger<ResourceModel>::ReduceUnits(Provider<ResourceModel>& provider, Provider<ResourceModel>::Units& request)
	{
		provider.ReduceUnits(request);
	}

	template<typename ResourceModel>
	Consumer<ResourceModel>& operator<<(Consumer<ResourceModel>& consumer, Provider<ResourceModel>& provider)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename ResourceModel>
	Provider<ResourceModel>& operator>>(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer)
	{
		Exchanger<ResourceModel>::Exchange(provider, consumer);

		return provider;
	}
} // Vessel
