// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

namespace Vessel
{
	enum class ExchangeResult : bool;

	template<typename Tag>
	class Consumer;

	template<typename Tag>
	class Exchanger;

	template<typename Tag>
	class Provider
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;

		// Life circle.
	public:
		virtual ~Provider() = default;

		// Public interface.
	public:
		ExchangeResult Provide(Consumer<Tag>& consumer);

		// Public virtual interface.
	public:
		// Get available resource amount to check possibility to satisfy a request.
		virtual Units GetAvailableUnits(Tag tag = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Call after satisfying every request to maintain real amount of the resource.
		virtual void ReduceUnits(Units resourceRequest) = 0;

		// Private types.
	private:
		friend class Exchanger<Tag>;
	};

	template<typename Tag>
	inline ExchangeResult Provider<Tag>::Provide(Consumer<Tag>& consumer)
	{
		return Exchanger<Tag>::Exchange(*this, consumer);
	}
} // Vessel