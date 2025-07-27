// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "ResourceModel.h"

#include <optional>
#include <algorithm>

namespace Vessel
{
	template<typename Model>
	class Limiter;

	template<typename Model>
	class Package;

	template<typename Model>
	class Container;

	template<typename Model>
	class Transfer final
	{
		// Public nested types.
	public:
		using Units = Model::Units;

		// Public static interface.
	public:
		static void Exchange(Package<Model>& providerPackage, Package<Model>& consumerPackage);
		static void Fill(Package<Model>& package, Container<Model>&& container);

		// Private constants.
	public:
		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Private interface.
	private:
		// Supply the consumer requested needs from the provider.
		static std::optional<Units> TransferUnits(Model::Units provider, Model::Units consumer);
	};

	template<typename Model>
	void Transfer<Model>::Exchange(Package<Model>& providerPackage, Package<Model>& consumerPackage)
	{
		for (typename Model::ResourceId resourceId : consumerPackage.GetManagedResourceIds())
		{
			typename Model::Units availableUnits = providerPackage.GetAvailableUnits(resourceId);
			typename Model::Units requiredUnits = consumerPackage.GetRequestedUnits(resourceId);

			std::optional<typename Model::Units> compromise = Transfer<Model>::template TransferUnits(availableUnits, requiredUnits);
			if (!compromise.has_value())
			{
				continue;
			}

			providerPackage.DecreaseUnits(resourceId, compromise.value_or(Model::kZeroUnits));
			consumerPackage.IncreaseUnits(resourceId, compromise.value_or(Model::kZeroUnits));
		}
	}

	template<typename Model>
	inline void Transfer<Model>::Fill(Package<Model>& package, Container<Model>&& container)
	{
		auto [id, amount] = std::move(std::move(container).Extract());
		package.IncreaseUnits(id, amount);
	}

	template<typename Model>
	inline std::optional<typename Model::Units> Transfer<Model>::TransferUnits(Units supplyUnits, Units demandUnits)
	{
		constexpr Units kEpsilonUnits = std::numeric_limits<Units>::epsilon();

		const Units compromise = std::clamp(demandUnits, kZeroUnits, supplyUnits);
		if (compromise <= kEpsilonUnits)
		{
			return std::nullopt;
		}

		return compromise;
	}

	template <typename Model>
	Package<Model>& operator<<(Package<Model>& package, Container<Model>&& container) {
		Transfer<Model>::Fill(package, std::move(container));
		return package;
	}

	template<typename Model>
	Package<Model>& operator>>(Package<Model>& providerPackage, Package<Model>& consumerPackage)
	{
		Transfer<Model>::Exchange(providerPackage, consumerPackage);
		return consumerPackage;
	}

	template<typename Model>
	decltype(auto) operator>>(Package<Model>& providerPackage, Package<Model>&& consumerPackage)
	{
		Transfer<Model>::Exchange(providerPackage, consumerPackage);
		return std::forward(consumerPackage);
	}

	template<typename Model>
	Package<Model>& operator>>(Package<Model>&& providerPackage, Package<Model>& consumerPackage)
	{
		Transfer<Model>::Exchange(providerPackage, consumerPackage);
		return std::forward(consumerPackage);
	}

	template<typename Model>
	decltype(auto) operator>>(Package<Model>&& providerPackage, Package<Model>&& consumerPackage)
	{
		Transfer<Model>::Exchange(providerPackage, consumerPackage);
		return std::forward(consumerPackage);
	}
} // Vessel
