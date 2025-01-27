// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "PackageInterface.h"

#include <unordered_map>

namespace Vessel
{
	template<typename Tag>
	class Package final : public PackageInterface<Tag>
	{
		// Public nested types.
	public:
		using Units = TagSelector<Tag>::Units;
		using ResourceId = TagSelector<Tag>::ResourceId;
		using Container = Container<Tag>;
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

	template<typename Tag>
	inline Package<Tag>::Package(const Package<Tag>::ContainerPropertiesTable& containerProperties)
		: PackageInterface<Tag>{}
		, mContainerProperties{ containerProperties }
	{
		ResetState();
	}

	template<typename Tag>
	inline void Package<Tag>::LoadState(const Package<Tag>::ContainerStateTable& containerStates)
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

	template<typename Tag>
	inline void Package<Tag>::SaveState(Package<Tag>::ContainerStateTable& containerStates) const
	{
		for (const auto& [resourceId, container] : mContainerItems)
		{
			containerStates[resourceId] = container.GetAmount();
		}
	}

	template<typename Tag>
	inline void Package<Tag>::ResetState()
	{
		mContainerItems.clear();
		for (auto& [resourceId, properties] : mContainerProperties)
		{
			mContainerItems.emplace(resourceId, Container{ properties });
		}
	}

	template<typename Tag>
	inline std::vector<typename Package<Tag>::ResourceId> Package<Tag>::GetManagedResourceIds() const
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

	template<typename Tag>
	inline Package<Tag>::Container& Package<Tag>::GetContainer(ResourceId resourceId)
	{
		static Container kEmptyItem{ kZeroUnits };

		if (!mContainerItems.contains(resourceId))
		{
			return kEmptyItem;
		}

		return mContainerItems.at(resourceId);
	}

	template<typename Tag>
	inline const Package<Tag>::Container& Package<Tag>::GetContainer(ResourceId resourceId) const
	{
		static Container kEmptyItem{ kZeroUnits };

		if (!mContainerItems.contains(resourceId))
		{
			return kEmptyItem;
		}

		return mContainerItems.at(resourceId);
	}
} // Vessel