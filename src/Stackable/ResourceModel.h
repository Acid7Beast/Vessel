// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include <type_traits>
#include <limits>

namespace Vessel
{
	namespace
	{
		template <typename, typename = void>
		struct HasEnumCount : std::false_type {};

		template <typename T>
		struct HasEnumCount<T, std::void_t<decltype(T::Count)>> : std::true_type {};

		template <typename T>
		constexpr bool HasEnumCountValue = HasEnumCount<T>::value;

		template <typename T>
		constexpr bool IsValidUnitsValue = std::is_arithmetic_v<T>;

		template <typename T, template <typename> class Template>
		struct IsSpecializationOf : std::false_type {};

		template <typename U, template <typename> class Template>
		struct IsSpecializationOf<Template<U>, Template> : std::true_type {};

		template <typename>
		struct IsLiteralRegistered : std::false_type {};

		template <typename T, typename = void>
		struct HasCheckResourceFlow : std::false_type {};

		template <typename T>
		struct HasCheckResourceFlow<T, std::enable_if_t<
			std::is_same_v<decltype(T::CheckResourceFlow), const bool>&&
			T::CheckResourceFlow
			>> : std::true_type {};
	}

	template <class Tag>
	struct ResourceModel
	{
		// Public nested types.
	public:
		using ResourceId = Tag::ResourceId;
		using Units = Tag::Units;

		// Public constants.
	public:
		// Resource count defines how many resources can be managed by this package.
		static constexpr uint8_t kResourceCount = static_cast<uint8_t>(ResourceId::Count);

		// Zero units constant defines empty amouht of resource.
		static constexpr Units kZeroUnits = static_cast<Units>(0);

		// Zero units constant defines empty amouht of resource.
		static constexpr Units kMaxCapacity = std::numeric_limits<Units>::max();

		// Should debugger check resource utilization?
		static constexpr bool kCheckResourceFlow = HasCheckResourceFlow<Tag>::value;

		// CT checks.
	private:
		static_assert(std::is_enum_v<ResourceId>,
			"Tag::ResourceId must be an enum type.");

		static_assert(HasEnumCountValue<ResourceId>,
			"Tag must have a static member 'Count' to be used as a TagSelector.");

		static_assert(std::is_same<std::underlying_type_t<ResourceId>, uint8_t>(),
			"ResourceId must be uint8_t type.");

		static_assert(static_cast<std::underlying_type_t<ResourceId>>(ResourceId::Count) > 0,
			"ResourceId::Count must be an integral constant above 0.");

		static_assert(IsValidUnitsValue<Tag::Units>,
			"Units must be an arithmetic type (int, float, bool, etc).");
	};
} // Vessel

#define REGISTER_RESOURCE(Name, Type, ...) \
struct Name##Tag { \
    enum class Name##ResourceId : uint8_t \
    { \
        __VA_ARGS__, \
        Count, \
    }; \
    using Units = Type; \
}; \
using Name##ResourceModel = ::Vessel::ResourceModel<Name##Tag>