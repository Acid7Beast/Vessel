// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "ResourceModel.h"

namespace Vessel
{
	enum class Transferesult : bool
	{
		Unchanged = false,
		Changed = true,
	};

	template<typename ResourceModel>
	class ProvideLimiter;

	template<typename ResourceModel>
	class ConsumeLimiter;

	template<typename ResourceModel>
	class Vessel final
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;
		using ResourceId = ResourceModel::ResourceId;

		// Public static interface.
	public:
		// Supply the consumer requested needs from the provider.
		static Transferesult Transfer(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer);

		// Private friend.
	private:
		friend ProvideLimiter<ResourceModel>;
		friend ConsumeLimiter<ResourceModel>;

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
	inline Transferesult Vessel<ResourceModel>::Transfer(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer)
	{
		constexpr Units kEpsilonUnits = std::numeric_limits<Units>::epsilon();

		if (provider.GetProvidableId() != consumer.GetConsumableId())
		{
			return Transferesult::Unchanged;
		}

		const Units demandUnits = consumer.GetRequestUnits();
		const Units supplyUnits = provider.GetAvailableUnits();

		const Units compromise = std::clamp(demandUnits, kZeroUnits, supplyUnits);
		if (compromise < kEpsilonUnits)
		{
			return Transferesult::Unchanged;
		}

		consumer.IncreaseUnits(compromise);
		provider.ReduceUnits(compromise);

		return Transferesult::Changed;
	}

	template<typename ResourceModel>
	inline void Vessel<ResourceModel>::IncreaseUnits(Consumer<ResourceModel>& consumer, Consumer<ResourceModel>::Units& supply)
	{
		consumer.IncreaseUnits(supply);
	}

	template<typename ResourceModel>
	inline void Vessel<ResourceModel>::ReduceUnits(Provider<ResourceModel>& provider, Provider<ResourceModel>::Units& request)
	{
		provider.ReduceUnits(request);
	}

	template<typename ResourceModel>
	Consumer<ResourceModel>& operator<<(Consumer<ResourceModel>& consumer, Provider<ResourceModel>& provider)
	{
		Vessel<ResourceModel>::Transfer(provider, consumer);

		return consumer;
	}

	template<typename ResourceModel>
	Provider<ResourceModel>& operator>>(Provider<ResourceModel>& provider, Consumer<ResourceModel>& consumer)
	{
		Vessel<ResourceModel>::Transfer(provider, consumer);

		return provider;
	}
} // Vessel
