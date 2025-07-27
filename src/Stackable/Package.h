// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "ResourceModel.h"
#include "Transfer.h"

#include <unordered_map>
#include <array>

namespace Vessel
{
	/**
	* Package represents a container of resources that can be Transferd between other packages.
	*
	* Requirements:
	* - Package specialization required specialization of ResourceModel.
	* - Package can be constructed only with a table of capacities, which defines the maximum amount of units for each resource.
	*
	* Aliases:
	* - Units - type of units in the package.
	* - ResourceId - type of resource enum identifier in the package.
	* - Transfer - type of Transfer for this package.
	* - ResourceTable - type of table that defines the current amount for each resource.
	*
	* Constants:
	* - kResourceCount - constant that defines how many resources can be managed by this package.
	* - kAvailableBytes - constant that defines available bytes for storing amount of resources in cache line, excluding reference to capacity table.
	* - kUseSBO - constant that defines whether to use small buffer optimization (SBO) for this package.
	*/
	template<typename Model>
	class Package final
	{
		// Public nested types.
	public:
		using Units = Model::Units;
		using ResourceId = Model::ResourceId;
		using Transfer = Transfer<Model>;
		using Container = Container<Model>;

		template<size_t Count> using ArrayType = std::array<Units, Count>;
		using ResourceTable = std::unordered_map<ResourceId, Units>;

		// Public life cycle.
	public:
		// Construct empty package with a table of capacities using SBO optimization.
		inline Package(const ResourceTable& containerProperties);

		// Copy table of capacities and stole resource from another package.
		inline Package(Package& other) noexcept;

		// Stole resources from another package as many as fit to capacities.
		inline Package& operator=(Package& other) noexcept;

		// Copy table of capacities and stole resources from other package on constructing.
		inline Package(Package&& other) noexcept;

		// Stole resources from other package on assignment as many as fit to capacities.
		inline Package& operator=(Package&& other) noexcept;

		// Destroy containers using SBO optimization.
		inline ~Package();

		// Public interface.
	public:
		// Deserialize state of this resource package for a save.
		inline void LoadState(const ResourceTable& containerStates);

		// Serialize state of this resource package from a save. 
		inline void SaveState(ResourceTable& containerStates) const;

		// Reset state of this resource package to default values.
		inline void ResetState();

		// Count required units to fill the package.
		inline Units GetRequestedUnits(ResourceId resourceId) const;

		// Count available units in the package.
		inline Units GetAvailableUnits(ResourceId resourceId) const;

		// Get list of managed resources.
		inline std::vector<ResourceId> GetManagedResourceIds() const;

		// Fried classes.
	public:
		friend class Transfer;

		// Private constants.
	private:
		// Available bytes for storing amount of resources in cache line, excluding reference to capacity table.
		static constexpr size_t kAvailableBytes = 64 - sizeof(void*);

		// Use small buffer optimization (SBO) if resource count is smaller than ResourceTable.
		static constexpr bool kUseSBO = (Model::kResourceCount <= (kAvailableBytes / sizeof(Units)));

		// CT checks.
	private:
		static_assert(IsSpecializationOf<Model, ResourceModel>::value, "Package<T>: Model must be specialization of ResourceModel<Tag>.");

		// Private interface.
	private:
		// Access resource by SBO flag.
		Units& AccessResource(ResourceId id);
		Units AccessResource(ResourceId id) const;

		// Increase amount of units in the package.
		inline void IncreaseUnits(ResourceId resourceId, Units amount);

		// Decrease amount of units in the package.
		inline void DecreaseUnits(ResourceId resourceId, Units amount);

		// Private state.
	private:
		union {
			ArrayType<Model::kResourceCount> mArray;
			ResourceTable mMap;
		};

		// Private properties.
	private:
		const ResourceTable& mContainerProperties;
	};

	template<typename Model>
	inline Package<Model>::Package(const Package<Model>::ResourceTable& containerProperties)
		: mContainerProperties{ containerProperties }
	{
		if constexpr (kUseSBO)
		{
			std::construct_at(&mArray);
		}
		else
		{
			std::construct_at(&mMap);
		}

		ResetState();
	}

	template<typename Model>
	inline Package<Model>::Package(Package& other) noexcept
		: mContainerProperties{ other.mContainerProperties }
	{
		if constexpr (kUseSBO)
		{
			std::construct_at(&mArray);
		}
		else
		{
			std::construct_at(&mMap);
		}

		ResetState();
		Transfer::Exchange(other, *this);
	}

	template<typename Model>
	inline Package<Model>& Package<Model>::operator=(Package& other) noexcept
	{
		Transfer::Exchange(other, *this);

		return *this;
	}

	template<typename Model>
	inline Package<Model>::Package(Package&& other) noexcept
		: mContainerProperties{ other.mContainerProperties }
	{
		if constexpr (kUseSBO)
		{
			std::construct_at(&mArray);
		}
		else
		{
			std::construct_at(&mMap);
		}

		if constexpr (Model::kCheckResourceFlow)
		{
			for (const auto& [resourceId, capacity] : mContainerProperties)
			{
				assert(capacity > Model::kZeroUnits, "Capacity must be greater than zero.");
				assert(capacity <= Model::kMaxCapacity, "Capacity must be less than or equal to maximum capacity.");
			}
		}

		ResetState();
		Transfer::Exchange(other, *this);
	}

	template<typename Model>
	inline Package<Model>& Package<Model>::operator=(Package&& other) noexcept
	{
		Transfer::Exchange(other, *this);

		return *this;
	}

	template<typename Model>
	inline Package<Model>::~Package()
	{
		if constexpr (kUseSBO)
		{
			std::destroy_at(&mArray);
		}
		else
		{
			std::destroy_at(&mMap);
		}

		if constexpr (Model::kCheckResourceFlow)
		{
			bool check = Model::kCheckResourceFlow;
			std::cout << check;
			for (auto& [resourceId, _] : mContainerProperties)
			{
				Units amount = AccessResource(resourceId);
				assert(amount == Model::kZeroUnits, "Resource wasn't utilized.");
			}
		}
	}

	template<typename Model>
	inline void Package<Model>::LoadState(const Package<Model>::ResourceTable& containerStates)
	{
		ResetState();

		// Load new state.
		for (auto& [resourceId, capacity] : mContainerProperties)
		{
			Units& amount = AccessResource(resourceId);
			amount = std::min(amount, capacity);

			auto stateIter = containerStates.find(resourceId);
			if (stateIter != containerStates.cend())
			{
				amount = std::min(stateIter->second, capacity);
			}
		}
	}

	template<typename Model>
	inline void Package<Model>::SaveState(Package<Model>::ResourceTable& containerStates) const
	{
		for (auto [resourceId, _] : mContainerProperties)
		{
			containerStates.emplace(resourceId, AccessResource(resourceId));
		}
	}

	template<typename Model>
	inline void Package<Model>::ResetState()
	{
		if constexpr (kUseSBO)
		{
			mArray.fill(Model::kZeroUnits);
		}
		else
		{
			mMap.clear();
		}
	}

	template<typename Model>
	inline Package<Model>::Units Package<Model>::GetRequestedUnits(ResourceId resourceId) const
	{
		auto iterProperties = mContainerProperties.find(resourceId);
		if (iterProperties == mContainerProperties.end())
		{
			return Model::kZeroUnits;
		}

		const Units capacity = iterProperties->second;
		const Units amount = AccessResource(resourceId);

		return capacity - amount;
	}

	template<typename Model>
	inline Package<Model>::Units Package<Model>::GetAvailableUnits(ResourceId resourceId) const
	{
		auto iterProperties = mContainerProperties.find(resourceId);
		if (iterProperties == mContainerProperties.end())
		{
			return Model::kZeroUnits;
		}

		return AccessResource(resourceId);
	}

	template<typename Model>
	inline std::vector<typename Package<Model>::ResourceId> Package<Model>::GetManagedResourceIds() const
	{
		std::vector<ResourceId> result;
		result.reserve(mContainerProperties.size());

		std::transform(
			mContainerProperties.cbegin(),
			mContainerProperties.cend(),
			std::back_inserter(result),
			[](const auto& pair) -> ResourceId {
				return pair.first;
			}
		);

		return result;
	}

	template<typename Model>
	inline Package<Model>::Units& Package<Model>::AccessResource(ResourceId id)
	{
		if constexpr (kUseSBO)
		{
			return mArray[static_cast<size_t>(id)];
		}
		else
		{
			return mMap[id];
		}
	}

	template<typename Model>
	inline Package<Model>::Units Package<Model>::AccessResource(ResourceId id) const
	{
		if constexpr (kUseSBO)
		{
			return mArray[static_cast<size_t>(id)];
		}
		else
		{
			auto iter = mMap.find(id);
			if (iter == mMap.cend())
			{
				return Model::kZeroUnits;
			}

			return iter->second;
		}
	}

	template<typename Model>
	inline void Package<Model>::IncreaseUnits(ResourceId resourceId, Units amount)
	{
		auto iterProperties = mContainerProperties.find(resourceId);
		if (iterProperties == mContainerProperties.end())
		{
			return;
		}

		Units& currentAmount = AccessResource(resourceId);
		currentAmount = std::min(currentAmount + amount, iterProperties->second);
	}

	template<typename Model>
	inline void Package<Model>::DecreaseUnits(ResourceId resourceId, Units amount)
	{
		if (!mContainerProperties.contains(resourceId))
		{
			return;
		}

		Units& currentAmount = AccessResource(resourceId);
		currentAmount = std::max(currentAmount - amount, Model::kZeroUnits);
	}
} // Vessel