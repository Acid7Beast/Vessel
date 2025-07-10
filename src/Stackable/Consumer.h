// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

namespace Vessel
{
	enum class ExchangeResult : bool;

	template<typename ResourceModel>
	class Provider;

	template<typename ResourceModel>
	class Exchanger;

	template<typename ResourceModel>
	class Consumer
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

		// Life circle.
	public:
		virtual ~Consumer() = default;

		// Public interface.
	public:
		// Consume resource from the provider.
		ExchangeResult Consume(Provider<ResourceModel>& provider);

		// Public virtual interface.
	public:
		// Requested resource amount needed to fulfill all the needs of this consumer.
		virtual Units GetRequestUnits(ResourceModel model = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Satisfy request of this consumer with some amount of the resource.
		virtual void IncreaseUnits(Units resourceSupply) = 0;

		// Private types.
	private:
		friend class Exchanger<ResourceModel>;
	};

	template<typename ResourceModel>
	inline ExchangeResult Consumer<ResourceModel>::Consume(Provider<ResourceModel>& provider)
	{
		return Exchanger<ResourceModel>::Exchange(provider, *this);
	}
} // Vessel