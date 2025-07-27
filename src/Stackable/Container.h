// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <algorithm>

#include "ResourceModel.h"

namespace Vessel
{
	/**
	* 
	*/
	template<typename Model>
	struct Container final
	{
		// Public nested types.
	public:
		using ResourceId = Model::ResourceId;
		using Units = Model::Units;

		// Public life cycle.
	public:
		// Implicit construct with pair.
		inline Container(std::pair<ResourceId, Units> count) noexcept;

		// Construct with explicit ResourceId and Units.
		inline Container(ResourceId id, Units amount) noexcept;

		// Delete copy constructor.
		inline Container(const Container&) = delete;

		// Delete copy assign operator.
		inline Container operator=(const Container&) = delete;

		// Allow only move constructing.
		inline Container(Container&& other) noexcept;

		// Allow only move assignment.
		inline Container& operator=(Container&& other) noexcept;

		// Default destructor.
		inline ~Container() = default;

		// Public interface.
	public:
		// Implicit cast to pair.
		inline operator std::pair<ResourceId, Units>() const { return Extract(); }
		inline operator std::pair<const ResourceId, Units>() const { return Extract(); }

		// Private friends.
	private:
		friend Transfer<Model>;

		// CT checks.
	private:
		static_assert(IsSpecializationOf<Model, ResourceModel>::value, "Package<T>: Model must be specialization of ResourceModel<Tag>.");

		// Private interface.
	private:
		// Cast resource to pair.
		inline std::pair<ResourceId, Units> Extract() const { return { mId, mAmount }; }

		// Private state.
	private:
		ResourceId mId;
		Units mAmount;
	};

	template<typename Model>
	inline Container<Model>::Container(std::pair<ResourceId, Units> pair) noexcept
		: mId{ pair.first }
		, mAmount{ pair.second }
	{
	}

	template<typename Model>
	inline Container<Model>::Container(ResourceId id, Units amount) noexcept
		: mId{ id }
		, mAmount{ amount }
	{
	}

	template<typename Model>
	inline Container<Model>::Container(Container&& other) noexcept
	{
		mId = std::exchange(other.mId, mId);
		mAmount = std::exchange(other.mAmount, mAmount);
	}

	template<typename Model>
	inline Container<Model>& Container<Model>::operator=(Container&& other) noexcept
	{
		mId = std::exchange(other.mId, mId);
		mAmount = std::exchange(other.mAmount, mAmount);

		return *this;
	}
} // Vessel

#define REGISTER_RESOURCE_LITERAL(Model, Name)\
	static_assert(##Model##::ResourceId::##Name != ##Model##::ResourceId::Count, \
	"Literal Name must not be 'Count' - it is reserved in enum.");\
	[[maybe_unused]] static auto __resource_literal_guard_##Name = 0;\
	auto operator"" Name(unsigned long long amount){\
		return ::Vessel::Container<Model>{ Model::ResourceId::Name, static_cast<Model::Units>(amount) };\
	}\
	struct __##Name##_SemiconlonCloseRequirement{}