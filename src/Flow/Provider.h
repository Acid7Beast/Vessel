// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Tag.h"

namespace Flow
{
	enum class ExchangeResult : bool;

	template<typename Tag>
	class Consumer;

	template<typename Tag>
	class Flow;

	template<typename OriginResourceTag, typename ConvertedResourceTag>
	class ConversionWrapper;

	template<typename Tag>
	class Provider
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using Resource = TagSelector<Tag>::Resource;
		using Pack = TagSelector<Tag>::Pack;

		// Life circle.
	public:
		virtual ~Provider() = default;

		// Public interface.
	public:
		ExchangeResult Provide(Consumer<Tag>& consumer);

		// Public virtual interface.
	public:
		// Get available resource amount to check possibility to satisfy a request.
		virtual const Pack& GetAvailableResources([[maybe_unused]] Tag = {}) const = 0;

		// Inheritable virtual interface.
	protected:
		// Call after satisfying every request to maintain real amount of the resource.
		virtual void ReduceResource(Pack& resourceRequest, [[maybe_unused]] Tag = {}) = 0;

		// Private types.
	private:
		friend class Flow<Tag>;
	};

	template<typename Tag>
	inline ExchangeResult Provider<Tag>::Provide(Consumer<Tag>& consumer)
	{
		return Flow<Tag>::Exchange(*this, consumer);
	}
} // Flow