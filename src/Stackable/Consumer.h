// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

namespace Vessel
{
	enum class ExchangeResult : bool;

	template<typename Tag>
	class Provider;

	template<typename Tag>
	class Exchanger;

	template<typename Tag>
	class Consumer
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;

		// Life circle.
	public:
		virtual ~Consumer() = default;

		// Public interface.
	public:
		// Consume resource from the provider.
		ExchangeResult Consume(Provider<Tag>& provider);

		// Public virtual interface.
	public:
		// Requested resource amount needed to fulfill all the needs of this consumer.
		virtual Units GetRequestUnits(Tag tag = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Satisfy request of this consumer with some amount of the resource.
		virtual void IncreaseUnits(Units resourceSupply) = 0;

		// Private types.
	private:
		friend class Exchanger<Tag>;
	};

	template<typename Tag>
	inline ExchangeResult Consumer<Tag>::Consume(Provider<Tag>& provider)
	{
		return Exchanger<Tag>::Exchange(provider, *this);
	}
} // Vessel