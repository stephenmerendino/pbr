#pragma once

#include "Engine/Math/Vector2.hpp"

enum UIPositioningMode
{
	UI_POSITION_ABSOLUTE,
	NUM_UI_POSITIONING_MODES
};

enum UIFillMode
{
	UI_FILL_STRETCH,
	UI_FILL_CROP,
	NUM_UI_FILL_MODES
};

struct ui_metric_t
{
	ui_metric_t(const Vector2& ratio = Vector2::ZERO, const Vector2& units = Vector2::ZERO)
		:ratio(ratio)
		,units(units)
	{}

	Vector2 ratio;
	Vector2 units;
};