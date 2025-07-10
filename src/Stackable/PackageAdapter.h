// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "PackageInterface.h"

namespace Vessel
{
	template<typename ResourceModel>
	class PackageAdapter final : public PackageInterface<ResourceModel>
	{
		// Public nested types.
	public:
		using ResourceId = ResourceModel::ResourceId;
		using Container = Container<ResourceModel>;

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

	template<typename ResourceModel>
	inline PackageAdapter<ResourceModel>::PackageAdapter(ResourceId resourceId, Container& adaptee)
		: PackageInterface<ResourceModel>{}
		, mResourceId{ resourceId }
		, mAdaptee{ adaptee }
	{
	}

	template<typename ResourceModel>
	inline std::vector<typename PackageAdapter<ResourceModel>::ResourceId> PackageAdapter<ResourceModel>::GetManagedResourceIds() const
	{
		return { mResourceId };
	}

	template<typename ResourceModel>
	inline PackageAdapter<ResourceModel>::Container& PackageAdapter<ResourceModel>::GetContainer(ResourceId resourceId)
	{
		return mAdaptee;
	}

	template<typename ResourceModel>
	inline const PackageAdapter<ResourceModel>::Container& PackageAdapter<ResourceModel>::GetContainer(ResourceId resourceId) const
	{
		return mAdaptee;
	}
} // Vessel