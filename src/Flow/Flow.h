#pragma once

#include "Tag.h"

#include "Provider.h"
#include "Consumer.h"

#include <utility>

namespace Flow
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
	class Flow final
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using Resource = TagSelector<Tag>::Resource;
		using Pack = TagSelector<Tag>::Pack;

		// Public static interface.
	public:
		// Supply the consumer requested needs from the provider.
		static ExchangeResult Exchange(Provider<Tag>& provider, Consumer<Tag>& consumer);

		// Private friend.
	private:
		friend ProvideLimiter<Tag>;
		friend ConsumeLimiter<Tag>;

		// Private static interface.
	private:
		// Increase resource.
		static void IncreaseResource(Consumer<Tag>& consumer, Consumer<Tag>::Pack& supply);

		// Reduce resource.
		static void ReduceResource(Provider<Tag>& provider, Provider<Tag>::Pack& request);
	};

	template<typename Tag>
	inline ExchangeResult Flow<Tag>::Exchange(Provider<Tag>& provider, Consumer<Tag>& consumer)
	{
		constexpr Units kEpsilon = std::numeric_limits<Units>::epsilon();
		constexpr Units kZero = static_cast<Units>(0);

		const Pack demand = consumer.GetRequestResources();
		const Pack supply = provider.GetAvailableResources();

		auto hasResource =
			[](const std::pair<Resource, Units>& pair) -> bool {
			return (pair.second > kEpsilon);
			};

		auto demandIter = std::find_if(
			demand.begin(),
			demand.end(),
			hasResource
		);

		const bool emptyDemand = (demandIter == demand.end());
		if (emptyDemand)
		{
			return ExchangeResult::Unchanged;
		}

		auto supplyIter = std::find_if(
			supply.begin(),
			supply.end(),
			hasResource
		);

		const bool emptySupply = (supplyIter == supply.end());
		if (emptySupply)
		{
			return ExchangeResult::Unchanged;
		}

		Pack request;
		for (auto [resource, consumeAmount] : demand)
		{
			auto provideIter = supply.find(resource);
			if (provideIter == supply.end())
			{
				continue;
			}

			request[resource] = std::max(std::min(consumeAmount, provideIter->second), kZero);
		}

		auto requestIter = std::find_if(
			request.begin(),
			request.end(),
			hasResource
		);

		if (requestIter == request.end())
		{
			return ExchangeResult::Unchanged;
		}

		consumer.IncreaseResource(request);
		provider.ReduceResource(request);

		return ExchangeResult::Changed;
	}

	template<typename Tag>
	inline void Flow<Tag>::IncreaseResource(Consumer<Tag>& consumer, Consumer<Tag>::Pack& supply)
	{
		consumer.IncreaseResource(supply);
	}

	template<typename Tag>
	inline void Flow<Tag>::ReduceResource(Provider<Tag>& provider, Provider<Tag>::Pack& request)
	{
		provider.ReduceResource(request);
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(Consumer<Tag>& consumer, Provider<Tag>& provider)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename Tag>
	Provider<Tag>& operator>>(Provider<Tag>& provider, Consumer<Tag>& consumer)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return provider;
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(ConsumeLimiter<Tag>&& consumer, Provider<Tag>& provider)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename Tag>
	Provider<Tag>& operator>>(ProvideLimiter<Tag>&& provider, Consumer<Tag>& consumer)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return provider;
	}

	template<typename Tag>
	Consumer<Tag>& operator<<(Consumer<Tag>& consumer, ProvideLimiter<Tag>&& provider)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return consumer;
	}

	template<typename Tag>
	Provider<Tag>& operator>>(Provider<Tag>& provider, ConsumeLimiter<Tag>&& consumer)
	{
		Flow<Tag>::Exchange(provider, consumer);

		return provider;
	}
} // Flow
