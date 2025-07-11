// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "ResourceModel.h"

namespace Vessel
{
	enum class ExchangeResult : bool;

	template<typename ResourceModel>
	class Consumer;

	template<typename ResourceModel>
	class Exchanger;

	template<typename ResourceModel>
	class Provider
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;

		// Life circle.
	public:
		virtual ~Provider() = default;

		// Public interface.
	public:
		ExchangeResult Provide(Consumer<ResourceModel>& consumer);

		// Public virtual interface.
	public:
		// Get available resource amount to check possibility to satisfy a request.
		virtual Units GetAvailableUnits(ResourceModel model = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Call after satisfying every request to maintain real amount of the resource.
		virtual void ReduceUnits(Units resourceRequest) = 0;

		// Private types.
	private:
		friend class Exchanger<ResourceModel>;
	};

	template<typename ResourceModel>
	inline ExchangeResult Provider<ResourceModel>::Provide(Consumer<ResourceModel>& consumer)
	{
		return Exchanger<ResourceModel>::Exchange(*this, consumer);
	}
} // Vessel