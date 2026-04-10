#include "Platform/GameMouseInput.h"

namespace platform
{
	namespace
	{
		int ClampInt(int value, int minimum, int maximum)
		{
			if (value < minimum)
			{
				return minimum;
			}

			if (value > maximum)
			{
				return maximum;
			}

			return value;
		}
	}

	GameMouseMetrics CreateGameMouseMetrics(int window_pixel_width, int window_pixel_height, float pointer_scale)
	{
		GameMouseMetrics metrics = {};
		metrics.window_pixel_width = window_pixel_width > 0 ? window_pixel_width : 0;
		metrics.window_pixel_height = window_pixel_height > 0 ? window_pixel_height : 0;
		metrics.pointer_scale = pointer_scale > 0.0f ? pointer_scale : 1.0f;
		metrics.max_mouse_x = static_cast<int>(static_cast<float>(metrics.window_pixel_width) / metrics.pointer_scale);
		metrics.max_mouse_y = static_cast<int>(static_cast<float>(metrics.window_pixel_height) / metrics.pointer_scale);
		if (metrics.max_mouse_x < 0)
		{
			metrics.max_mouse_x = 0;
		}
		if (metrics.max_mouse_y < 0)
		{
			metrics.max_mouse_y = 0;
		}
		return metrics;
	}

	void ResetGameMouseState(GameMouseState* state, int initial_x, int initial_y)
	{
		if (state == 0)
		{
			return;
		}

		state->x = initial_x;
		state->y = initial_y;
		state->back_x = initial_x;
		state->back_y = initial_y;
		state->left_button = false;
		state->left_button_pop = false;
		state->left_button_push = false;
		state->right_button = false;
		state->right_button_pop = false;
		state->right_button_push = false;
		state->left_button_double_click = false;
		state->middle_button = false;
		state->middle_button_pop = false;
		state->middle_button_push = false;
		state->wheel = 0;
		state->left_pop_x = initial_x;
		state->left_pop_y = initial_y;
	}

	void PrepareGameMouseStateForMessage(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->left_button_double_click = false;
		if (state->left_button_pop && (state->left_pop_x != state->x || state->left_pop_y != state->y))
		{
			state->left_button_pop = false;
		}
	}

	void ApplyInactiveWindowMouseState(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->left_button = false;
		state->left_button_pop = false;
		state->right_button = false;
		state->right_button_pop = false;
		state->right_button_push = false;
		state->left_button_double_click = false;
		state->middle_button = false;
		state->middle_button_pop = false;
		state->middle_button_push = false;
		state->wheel = 0;
	}

	void UpdateGameMousePositionFromWindowPixels(GameMouseState* state, const GameMouseMetrics& metrics, float window_pixel_x, float window_pixel_y)
	{
		if (state == 0)
		{
			return;
		}

		state->back_x = state->x;
		state->back_y = state->y;
		state->x = ClampInt(static_cast<int>(window_pixel_x / metrics.pointer_scale), 0, metrics.max_mouse_x);
		state->y = ClampInt(static_cast<int>(window_pixel_y / metrics.pointer_scale), 0, metrics.max_mouse_y);
	}

	void SetGameMouseLeftButtonDown(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->left_button_pop = false;
		if (!state->left_button)
		{
			state->left_button_push = true;
		}
		state->left_button = true;
	}

	void SetGameMouseLeftButtonUp(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->left_button_push = false;
		state->left_button_pop = true;
		state->left_button = false;
		state->left_pop_x = state->x;
		state->left_pop_y = state->y;
	}

	void SetGameMouseRightButtonDown(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->right_button_pop = false;
		if (!state->right_button)
		{
			state->right_button_push = true;
		}
		state->right_button = true;
	}

	void SetGameMouseRightButtonUp(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->right_button_push = false;
		if (state->right_button)
		{
			state->right_button_pop = true;
		}
		state->right_button = false;
	}

	void SetGameMouseMiddleButtonDown(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->middle_button_pop = false;
		if (!state->middle_button)
		{
			state->middle_button_push = true;
		}
		state->middle_button = true;
	}

	void SetGameMouseMiddleButtonUp(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->middle_button_push = false;
		if (state->middle_button)
		{
			state->middle_button_pop = true;
		}
		state->middle_button = false;
	}

	void SetGameMouseLeftDoubleClick(GameMouseState* state)
	{
		if (state == 0)
		{
			return;
		}

		state->left_button_double_click = true;
	}

	void SetGameMouseWheelDelta(GameMouseState* state, int wheel_delta)
	{
		if (state == 0)
		{
			return;
		}

		state->wheel = wheel_delta;
	}
}
