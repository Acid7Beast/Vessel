// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Container.h"
#include "Exchanger.h"

#include <vector>

namespace Vessel
{
	template<typename ResourceModel>
	class PackageInterface
	{
		// Public nested types.
	public:
		using ResourceId = ResourceModel::ResourceId;
		using Container = Container<ResourceModel>;

		// Life circle.
	public:
		virtual ~PackageInterface() = default;

		// Public virtual interface.
	public:
		// Get list of managed resources.
		virtual std::vector<ResourceId> GetManagedResourceIds() const = 0;

		// Get readonly with resources by resource id.
		virtual Container& GetContainer(ResourceId resourceId) = 0;
		virtual const Container& GetContainer(ResourceId resourceId) const = 0;
	};

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& Exchange(PackageInterface<ResourceModel>& providerPackage, PackageInterface<ResourceModel>& consumerPackage)
	{
		for (typename Package<ResourceModel>::ResourceId resourceId : consumerPackage.GetManagedResourceIds())
		{
			Container<ResourceModel>& providerContainer = providerPackage.GetContainer(resourceId);
			Container<ResourceModel>& consumerContainer = consumerPackage.GetContainer(resourceId);

			Exchanger<ResourceModel>::Exchange(providerContainer, consumerContainer);
		}

		return providerPackage;
	}

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator>>(PackageInterface<ResourceModel>& providerPackage, PackageInterface<ResourceModel>& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator<<(PackageInterface<ResourceModel>& consumerPackage, PackageInterface<ResourceModel>& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator>>(PackageInterface<ResourceModel>&& providerPackage, PackageInterface<ResourceModel>& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}
	

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator>>(PackageInterface<ResourceModel>& providerPackage, PackageInterface<ResourceModel>&& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator<<(PackageInterface<ResourceModel>&& consumerPackage, PackageInterface<ResourceModel>& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename ResourceModel>
	PackageInterface<ResourceModel>& operator<<(PackageInterface<ResourceModel>& consumerPackage, PackageInterface<ResourceModel>&& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}
} // Vessel