#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/GameConfig.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/Console.hpp"
#include "Engine/Core/log.h"
#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Profile/profiler.h"
#include "Engine/Net/Object/net_object_system.hpp"

COMMAND(quit, "Quits the app")
{
    g_app->set_is_quitting(true);
}

App::App()
	:m_has_focus(true)
	,m_is_quitting(false)
    ,m_force_show_mouse(true)
	,m_delta_seconds(0.0f)
	,m_last_frame_time_seconds(0.0f)
	,m_app_font(nullptr)
	,m_view_fov(0.0f)
{
}

App::~App()
{
}

void App::init()
{
	PROFILE_LOG_SCOPE("App::Init");

	// load up config options
	ConfigGetRgba(&m_clear_color, CONFIG_CLEAR_COLOR_NAME);
	ConfigGetFloat(&m_view_fov, CONFIG_FOV_NAME);

	m_app_font = g_theRenderer->m_device->CreateFontFromFile(APP_FONT);

	g_game = new Game();
}

void App::run()
{
	while(is_running()){
		run_frame();
	}
}

void App::shutdown()
{
	SAFE_DELETE(m_app_font);
	SAFE_DELETE(g_game);
}

void App::register_key_down(unsigned char keycode)
{
	console_register_non_char_key_down(keycode);

	if(!is_console_open()){
		g_theInputSystem->RegisterKeyDown(keycode);
	}
}

void App::register_key_up(unsigned char keycode)
{
	console_register_non_char_key_up(keycode);

	if (!is_console_open()){
		g_theInputSystem->RegisterKeyUp(keycode);
	}
}

void App::register_mouse_wheel_delta(float delta)
{
	if (!is_console_open()){
		g_theInputSystem->RegisterMouseWheelDelta(delta);
	}
}

void App::on_gained_focus()
{
	m_has_focus = true;
	g_theInputSystem->ShowMouseCursor(false);
}

void App::on_lost_focus()
{
	m_has_focus = false;
	g_theInputSystem->ShowMouseCursor(true);
}

void App::run_frame()
{
    PROFILE_SCOPE_FUNCTION();

	start_frame();

	if(!g_theRenderer->m_output->IsOpen()){
		set_is_quitting(true);
		return;
	}

	update(m_delta_seconds);
	render();

	end_frame();
}

void App::start_frame()
{
	tick_time();

    update_show_mouse();
    engine_tick(m_delta_seconds);
}

void App::update_show_mouse()
{
    bool should_show = (m_force_show_mouse || is_console_open());
	g_theInputSystem->ShowMouseCursor(should_show);

	// Only update mouse move delta if app has focus and dev console is not open
	if(m_has_focus && !should_show){
		g_theInputSystem->UpdateMouseMoveDelta();
	}
}

void App::update(float deltaSeconds)
{
    PROFILE_SCOPE_FUNCTION();

	if(g_theInputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)){
		set_is_quitting(true);
	}

    if(g_theInputSystem->WasKeyJustPressed('M')){
        m_force_show_mouse = !m_force_show_mouse;
        if(!m_force_show_mouse){
            g_theInputSystem->SetCursorScreenPos(g_theInputSystem->GetScreenCenter());
        }
    }

	g_game->update(deltaSeconds);
}

void App::render()
{
    engine_render();
    g_game->render();
}

void App::end_frame()
{
	g_theRenderer->Present();

	if(g_theInputSystem){
		g_theInputSystem->EndFrame();
	}

	burn_extra_time();
}

void App::tick_time()
{
	double timeNowSeconds = get_current_time_seconds();

	m_delta_seconds = (float)(timeNowSeconds - m_last_frame_time_seconds);
	m_delta_seconds = Min(m_delta_seconds, 0.1f); // Cap delta seconds to a tenth of a second

	m_last_frame_time_seconds = timeNowSeconds;
}

void App::burn_extra_time()
{
    PROFILE_SCOPE_FUNCTION();
	bool limitFPS;
	ConfigGetBool(&limitFPS, CONFIG_LIMIT_FPS_NAME);

	if(!limitFPS){
		return;
	}

	int fps;
	ConfigGetInt(&fps, CONFIG_FPS_NAME);

	double minFrameDurationSeconds = 1.0 / (double)fps;
	double frameElapsedTimeSeconds = get_current_time_seconds() - m_last_frame_time_seconds;

	double frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	if(frameTimeLeft > 0.002f){
		sleep(0.001f);
	}

	frameElapsedTimeSeconds = get_current_time_seconds() - m_last_frame_time_seconds;
	frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	while(frameTimeLeft > 0.0){
		frameElapsedTimeSeconds = get_current_time_seconds() - m_last_frame_time_seconds;
		frameTimeLeft = minFrameDurationSeconds - frameElapsedTimeSeconds;
	}
}