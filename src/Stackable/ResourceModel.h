// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <type_traits>

namespace Vessel
{
	template <typename, typename = void>
	struct HasEnumCount : std::false_type {};

	template <typename T>
	struct HasEnumCount<T, std::void_t<decltype(T::Count)>> : std::true_type {};

	template <typename T>
	constexpr bool HasEnumCountValue = HasEnumCount<T>::value;

	template <typename T>
	constexpr bool IsValidUnitsValue = std::is_arithmetic_v<T>;

	template <class Tag>
	struct ResourceModel
	{
		static_assert(std::is_enum_v<Tag::ResourceId>, 
			"Tag::ResourceId must be an enum type.");

		static_assert(HasEnumCountValue<Tag::ResourceId>,
			"Tag must have a static member 'Count' to be used as a TagSelector.");

		static_assert(std::is_same<std::underlying_type_t<Tag::ResourceId>, uint8_t>(),
			"ResourceId must be uint8_t type.");

		static_assert(static_cast<std::underlying_type_t<Tag::ResourceId>>(Tag::ResourceId::Count) > 0,
			"ResourceId::Count must be an integral constant above 0.");

		static_assert(IsValidUnitsValue<Tag::Units>,
			"Units must be an arithmetic type (int, float, bool, etc).");

		using ResourceId = Tag::ResourceId;
		using Units = Tag::Units;

		static constexpr uint8_t Count = static_cast<uint8_t>(ResourceId::Count);
	};
} // Vessel