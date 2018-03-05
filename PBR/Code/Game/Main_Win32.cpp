#include "Engine/engine.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Display.hpp"
#include "Engine/Core/Console.hpp"

#include "Engine/Profile/mem_tracker.h"
#include "Engine/Profile/auto_profile_log_scope.h"

#include "Game/GameCommon.hpp"
#include "Game/GameConfig.hpp"
#include "Game/App.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <time.h>

bool WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	UNUSED(windowHandle);
	UNUSED(lParam);

	unsigned char asKey = (unsigned char)wParam;

	switch(wmMessageCode){
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
		{
			if(g_app){
				g_app->set_is_quitting(true);
			}
		} return false;

		case WM_CHAR:
		{
			char asCharKey = (char)wParam;
			console_register_char_key_down(asCharKey);
		} break;

		case WM_KEYDOWN:
		{
			if(g_app){
				g_app->register_key_down(asKey);
			}
		} break;

		case WM_KEYUP:
		{
			if(g_app){
				g_app->register_key_up(asKey);
			}
		} break;

		case WM_LBUTTONDOWN:
		{
			if(g_app){
				g_app->register_key_down(KEYCODE_LMB);
			}
		} break;

		case WM_RBUTTONDOWN:
		{
			if(g_app){
				g_app->register_key_down(KEYCODE_RMB);
			}
		} break;

		case WM_LBUTTONUP:
		{
			if(g_app){
				g_app->register_key_up(KEYCODE_LMB);
			}
		} break;

		case WM_RBUTTONUP:
		{
			if(g_app){
				g_app->register_key_up(KEYCODE_RMB);
			}
		} break;

		case WM_MOUSEWHEEL:
		{
			if(g_app){
				float mouseWheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
				g_app->register_mouse_wheel_delta(mouseWheelDelta);
			}
		} break;

		case WM_SETFOCUS:
		{
			if(g_app){
				g_app->on_gained_focus();
			}
		} break;

		case WM_KILLFOCUS:
		{
			if(g_app){
				g_app->on_lost_focus();
			}
		} break;
	}

	return true;
}

void Initialize(char* command_line)
{
	PROFILE_LOG_SCOPE("App Full Initialized");

	engine_startup_t startup_params;

	startup_params.config_filename = CONFIG_FILE;
	startup_params.command_line = command_line;
	startup_params.config_key_config_autosave = CONFIG_AUTOSAVE_OPTION_NAME;
	startup_params.config_key_config_autosave_filepath = CONFIG_AUTOSAVE_FILEPATH_OPTION_NAME;
	startup_params.config_key_window_width = CONFIG_RESOLUTION_WIDTH_NAME;
	startup_params.config_key_window_height = CONFIG_RESOLUTION_HEIGHT_NAME;
	startup_params.config_key_window_title = CONFIG_WINDOW_TITLE_NAME;
	startup_params.config_key_clear_color = CONFIG_CLEAR_COLOR_NAME;
	startup_params.windows_message_cb = WindowsMessageHandlingProcedure;

	engine_init(startup_params);
    
	g_app = new App();
    g_app->init();
}

void Shutdown()
{
    g_app->shutdown();
	SAFE_DELETE(g_app);

	engine_shutdown();
}

int CALLBACK WinMain(HINSTANCE app, HINSTANCE prev_app, LPSTR command_line, int show_command)
{
	UNUSED(app);
	UNUSED(prev_app);
	UNUSED(show_command);

	Initialize(command_line);
	g_app->run();
	Shutdown();

	return 0;
}