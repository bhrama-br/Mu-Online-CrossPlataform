#pragma once

namespace platform
{
	struct GameMouseMetrics
	{
		int window_pixel_width;
		int window_pixel_height;
		float pointer_scale;
		int max_mouse_x;
		int max_mouse_y;
	};

	struct GameMouseState
	{
		int x;
		int y;
		int back_x;
		int back_y;
		bool left_button;
		bool left_button_pop;
		bool left_button_push;
		bool right_button;
		bool right_button_pop;
		bool right_button_push;
		bool left_button_double_click;
		bool middle_button;
		bool middle_button_pop;
		bool middle_button_push;
		int wheel;
		int left_pop_x;
		int left_pop_y;
	};

	GameMouseMetrics CreateGameMouseMetrics(int window_pixel_width, int window_pixel_height, float pointer_scale);
	void ResetGameMouseState(GameMouseState* state, int initial_x, int initial_y);
	void PrepareGameMouseStateForMessage(GameMouseState* state);
	void ApplyInactiveWindowMouseState(GameMouseState* state);
	void UpdateGameMousePositionFromWindowPixels(GameMouseState* state, const GameMouseMetrics& metrics, float window_pixel_x, float window_pixel_y);
	void SetGameMouseLeftButtonDown(GameMouseState* state);
	void SetGameMouseLeftButtonUp(GameMouseState* state);
	void SetGameMouseRightButtonDown(GameMouseState* state);
	void SetGameMouseRightButtonUp(GameMouseState* state);
	void SetGameMouseMiddleButtonDown(GameMouseState* state);
	void SetGameMouseMiddleButtonUp(GameMouseState* state);
	void SetGameMouseLeftDoubleClick(GameMouseState* state);
	void SetGameMouseWheelDelta(GameMouseState* state, int wheel_delta);
}
