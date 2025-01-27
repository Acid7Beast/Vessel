// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "PackageInterface.h"

namespace Vessel
{
	template<typename Tag>
	class PackageAdapter final : public PackageInterface<Tag>
	{
		// Public nested types.
	public:
		using ResourceId = TagSelector<Tag>::ResourceId;
		using Container = Container<Tag>;

		// Life circle.
	public:
		inline PackageAdapter(ResourceId resourceId, Container& adaptee);

		// Public virtual interface substitution.
	public:
		// Get list of managed resources.
		inline std::vector<ResourceId> GetManagedResourceIds() const override;

		// Get readonly with resources by resource id.
		inline Container& GetContainer(ResourceId resourceId) override;
		inline const Container& GetContainer(ResourceId resourceId) const override;

		// Private state.
	private:
		ResourceId mResourceId;
		Container& mAdaptee;
	};

	template<typename Tag>
	inline PackageAdapter<Tag>::PackageAdapter(ResourceId resourceId, Container& adaptee)
		: PackageInterface<Tag>{}
		, mResourceId{ resourceId }
		, mAdaptee{ adaptee }
	{
	}

	template<typename Tag>
	inline std::vector<typename PackageAdapter<Tag>::ResourceId> PackageAdapter<Tag>::GetManagedResourceIds() const
	{
		return { mResourceId };
	}

	template<typename Tag>
	inline PackageAdapter<Tag>::Container& PackageAdapter<Tag>::GetContainer(ResourceId resourceId)
	{
		return mAdaptee;
	}

	template<typename Tag>
	inline const PackageAdapter<Tag>::Container& PackageAdapter<Tag>::GetContainer(ResourceId resourceId) const
	{
		return mAdaptee;
	}
} // Vessel