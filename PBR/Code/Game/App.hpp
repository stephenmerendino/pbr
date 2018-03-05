#pragma once

#include "Engine/Core/Rgba.hpp"

class Font;

class App
{
public:
	bool    m_is_quitting;
	bool    m_has_focus;
    bool    m_force_show_mouse;
	float   m_delta_seconds;
	double  m_last_frame_time_seconds;
	float   m_view_fov;

	Font*   m_app_font;
    Rgba    m_clear_color;

public:
	App();
	~App();

	void init();
	void run();
    void shutdown();

public:
	void set_is_quitting(bool quit) { m_is_quitting = quit; }
	bool is_running() const { return !m_is_quitting; }
	bool is_quitting() const { return m_is_quitting; }

	void register_key_down(unsigned char keycode);
	void register_key_up(unsigned char keycode);

	void register_mouse_wheel_delta(float delta);

	void on_gained_focus();
	void on_lost_focus();
	bool has_focus() const { return m_has_focus; }

public:
	void run_frame();

	void start_frame();
    void update_show_mouse();
	void update(float ds);
	void render();
	void end_frame();

	void tick_time();
	void burn_extra_time();
};