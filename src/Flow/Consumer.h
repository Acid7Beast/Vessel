// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

namespace Flow
{
	enum class ExchangeResult : bool;

	template<typename Tag>
	class Provider;

	template<typename Tag>
	class Flow;

	template<typename Tag>
	class Consumer
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using ResourceId = TagSelector<Tag>::ResourceId;

		// Life circle.
	public:
		virtual ~Consumer() = default;

		ExchangeResult Consume(Provider<Tag>& provider);

		// Public virtual interface.
	public:
		// Get consumable resource identifiers to work with.
		virtual ResourceId GetConsumableId(Tag tag = {}) const = 0;

		// Requested resource amount needed to fulfill all the needs of this consumer.
		virtual Units GetRequestUnits(Tag tag = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Satisfy request of this consumer with some amount of the resource.
		virtual void IncreaseUnits(Units resourceSupply) = 0;

		// Private types.
	private:
		friend class Flow<Tag>;
	};

	template<typename Tag>
	inline ExchangeResult Consumer<Tag>::Consume(Provider<Tag>& provider)
	{
		return Flow<Tag>::Exchange(provider, *this);
	}
} // Flow