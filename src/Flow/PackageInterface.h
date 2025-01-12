// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Container.h"
#include "Exchanger.h"

#include <vector>

namespace Flow
{
	template<typename Tag>
	class PackageInterface
	{
		// Public nested types.
	public:
		using ResourceId = TagSelector<Tag>::ResourceId;
		using Container = Container<Tag>;

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

	template<typename Tag>
	PackageInterface<Tag>& Exchange(PackageInterface<Tag>& providerPackage, PackageInterface<Tag>& consumerPackage)
	{
		for (typename Package<Tag>::ResourceId resourceId : consumerPackage.GetManagedResourceIds())
		{
			Container<Tag>& providerContainer = providerPackage.GetContainer(resourceId);
			Container<Tag>& consumerContainer = consumerPackage.GetContainer(resourceId);

			Exchanger<Tag>::Exchange(providerContainer, consumerContainer);
		}

		return providerPackage;
	}

	template<typename Tag>
	PackageInterface<Tag>& operator>>(PackageInterface<Tag>& providerPackage, PackageInterface<Tag>& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename Tag>
	PackageInterface<Tag>& operator<<(PackageInterface<Tag>& consumerPackage, PackageInterface<Tag>& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename Tag>
	PackageInterface<Tag>& operator>>(PackageInterface<Tag>&& providerPackage, PackageInterface<Tag>& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}
	

	template<typename Tag>
	PackageInterface<Tag>& operator>>(PackageInterface<Tag>& providerPackage, PackageInterface<Tag>&& consumerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename Tag>
	PackageInterface<Tag>& operator<<(PackageInterface<Tag>&& consumerPackage, PackageInterface<Tag>& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}

	template<typename Tag>
	PackageInterface<Tag>& operator<<(PackageInterface<Tag>& consumerPackage, PackageInterface<Tag>&& providerPackage)
	{
		return Exchange(providerPackage, consumerPackage);
	}
} // Flow