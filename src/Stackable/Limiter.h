// (c) 2024 Acid7Beast. Use with wisdom.
#pragma once

#include "Transfer.h"

namespace Vessel
{
	template<typename Model>
	struct Container;

	template <typename Model>
	class Limiter final
	{
		// Public nested types.
	public:
		using Units = Model::Units;

		struct State
		{
			Units buffer;
			float bandwidth;
			Container<Model>& container;
		};

		// Life circle.
	public:
		inline Limiter(Container<Model>& container, Units buffer, float bandwidth = 1.f);

		// Public interface.
	public:
		// Change Transfer buffer.
		inline void SetUnitsBuffer(Units newValue) { mState.buffer = newValue; }

		// Set bandwidth for any cases.
		inline void SetUnitsBandwidth(float newValue = 1.f) { mState.bandwidth = newValue; }

		// Public virtual interface substitution.
	public:
		// Consumer::GetRequestedUnits
		inline Units GetRequestUnits(Model model = {}) const override;

		// Provider::GetAvailableUnits
		inline Units GetAvailableUnits(Model model = {}) const override;

		// Private virtual interface substitution.
	private:
		// Consumer::IncreaseUnits
		inline void IncreaseUnits(Units resourceRequest) override { Transfer<Model>::IncreaseUnits(mState.container, resourceRequest); }

		// Consumer::DecreaseUnits
		inline void DecreaseUnits(Units resourceRequest) override { Transfer<Model>::DecreaseUnits(mState.container, resourceRequest); }

		// Private state.
	private:
		State mState;
	};

	template<typename Model>
	inline Limiter<Model>::Limiter(Container<Model>& container, Limiter::Units buffer, float bandwidth)
		: mState{ bandwidth, buffer, container }
	{
	}

	template<typename Model>
	inline Limiter<Model>::Units Limiter<Model>::GetRequestUnits(Model model) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.container.GetRequestUnits();

		return std::min(availableUnits, possibleUnits);
	}
	
	template<typename Model>
	inline Limiter<Model>::Units Limiter<Model>::GetAvailableUnits(Model model) const
	{
		const Units possibleUnits = static_cast<Units>(mState.buffer * mState.bandwidth);
		const Units availableUnits = mState.container.GetAvailableUnits();

		return std::min(availableUnits, possibleUnits);
	}
} // Vessel