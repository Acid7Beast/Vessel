// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "PackageInterface.h"

#include <unordered_map>

namespace Vessel
{
	template<typename ResourceModel>
	class Package final : public PackageInterface<ResourceModel>
	{
		// Public nested types.
	public:
		using Units = ResourceModel::Units;
		using ResourceId = ResourceModel::ResourceId;
		using Container = Container<ResourceModel>;
		using ContainerPropertiesTable = std::unordered_map<ResourceId, Units>;
		using ContainerStateTable = std::unordered_map<ResourceId, Units>;

		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Life circle.
	public:
		inline Package(const ContainerPropertiesTable& containerProperties);

		// Public interface.
	public:
		// Deserialize state of this resource package for a save.
		inline void LoadState(const ContainerStateTable& containerStates);

		// Serialize state of this resource package from a save. 
		inline void SaveState(ContainerStateTable& containerStates) const;

		// Reset state of this resource package to default values.
		inline void ResetState();

		// Public virtual interface substitution.
	public:
		// Get list of managed resources.
		inline std::vector<ResourceId> GetManagedResourceIds() const override;

		// Get readonly with resources by resource id.
		inline Container& GetContainer(ResourceId resourceId) override;
		inline const Container& GetContainer(ResourceId resourceId) const override;

		// Private state.
	private:
		std::unordered_map<ResourceId, Container> mContainerItems;

		// Private properties.
	private:
		const ContainerPropertiesTable& mContainerProperties;
	};

	template<typename ResourceModel>
	inline Package<ResourceModel>::Package(const Package<ResourceModel>::ContainerPropertiesTable& containerProperties)
		: PackageInterface<ResourceModel>{}
		, mContainerProperties{ containerProperties }
	{
		ResetState();
	}

	template<typename ResourceModel>
	inline void Package<ResourceModel>::LoadState(const Package<ResourceModel>::ContainerStateTable& containerStates)
	{
		mContainerItems.clear();
		for (auto& [resourceId, capacity] : mContainerProperties)
		{
			mContainerItems.emplace(resourceId, Container{ capacity });

			if (containerStates.contains(resourceId))
			{
				mContainerItems.at(resourceId).SetAmount(containerStates.at(resourceId));
			}
		}
	}

	template<typename ResourceModel>
	inline void Package<ResourceModel>::SaveState(Package<ResourceModel>::ContainerStateTable& containerStates) const
	{
		for (const auto& [resourceId, container] : mContainerItems)
		{
			containerStates[resourceId] = container.GetAmount();
		}
	}

	template<typename ResourceModel>
	inline void Package<ResourceModel>::ResetState()
	{
		mContainerItems.clear();
		for (auto& [resourceId, properties] : mContainerProperties)
		{
			mContainerItems.emplace(resourceId, Container{ properties });
		}
	}

	template<typename ResourceModel>
	inline std::vector<typename Package<ResourceModel>::ResourceId> Package<ResourceModel>::GetManagedResourceIds() const
	{
		std::vector<ResourceId> result;

		std::transform(
			mContainerItems.cbegin(),
			mContainerItems.cend(),
			std::back_inserter(result),
			[](const std::pair<ResourceId, Container>& pair) -> ResourceId {
				return pair.first;
			}
		);

		return result;
	}

	template<typename ResourceModel>
	inline Package<ResourceModel>::Container& Package<ResourceModel>::GetContainer(ResourceId resourceId)
	{
		static Container kEmptyItem{ kZeroUnits };

		if (!mContainerItems.contains(resourceId))
		{
			return kEmptyItem;
		}

		return mContainerItems.at(resourceId);
	}

	template<typename ResourceModel>
	inline const Package<ResourceModel>::Container& Package<ResourceModel>::GetContainer(ResourceId resourceId) const
	{
		static Container kEmptyItem{ kZeroUnits };

		if (!mContainerItems.contains(resourceId))
		{
			return kEmptyItem;
		}

		return mContainerItems.at(resourceId);
	}
} // Vessel