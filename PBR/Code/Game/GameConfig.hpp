#pragma once

#include "Engine/Core/Rgba.hpp"

static const char*	APP_FONT						= "consolas32.fnt";
//static const char*	APP_FONT						= "calibri32.fnt";

// Config file file name and key names
//static const char*	CONFIG_FILE								= "Data/Config.dat";
static const char*	CONFIG_FILE								= "Data/Config.dat.autosave";
static const char*	CONFIG_AUTOSAVE_OPTION_NAME				= "autosaveConfig";
static const char*	CONFIG_AUTOSAVE_FILEPATH_OPTION_NAME	= "config_autosave_filepath";

static const char*	CONFIG_WINDOW_TITLE_NAME		= "windowTitle";
static const char*	CONFIG_RESOLUTION_WIDTH_NAME	= "resolutionWidth";
static const char*	CONFIG_RESOLUTION_HEIGHT_NAME	= "resolutionHeight";
static const char*	CONFIG_CLEAR_COLOR_NAME			= "clearColor";
static const char*	CONFIG_LIMIT_FPS_NAME			= "limitFPS";
static const char*	CONFIG_FPS_NAME					= "fps";
static const char*	CONFIG_FOV_NAME					= "fov";

static const int DEFAULT_HOST_PORT = 12345;
static const int DEFAULT_MAX_PLAYERS = 32;
static const int DEFAULT_MAX_NAME_LENGTH = 64;
static const int DEFAULT_GAME_FIELD_WIDTH = 128;
static const int DEFAULT_GAME_FIELD_HEIGHT = 128;

static const float DEV_SLOW_TIME_SCALE = 0.1f;

static const float NET_TICK_RATE = 20.0f;

static const float SHIP_RESPAWN_TIME_SECONDS = 1.0f;
static const float SHIP_TURN_SPEED = 270.0f;
static const float SHIP_THRUST_POWER = 750.0f;
static const float SHIP_SPEED_CAP = 5.0f;
static const float SHIP_RADIUS = 0.40f;
static const float SHIP_RELOAD_RATE_SECONDS = 0.1f;
static const float SHIP_VELOCITY_FRICTION = 0.975f;
static const float SHIP_CLIENT_VELOCITY_CORRECTION_LERP = 0.10f;
static const float SHIP_CLIENT_POSITION_CORRECTION_LERP = 0.10f;
static const float SHIP_CLIENT_SOFT_POSITION_LOWER_CORRECTION_BOUNDS = 0.20f;
static const float SHIP_CLIENT_SOFT_POSITION_UPPER_CORRECTION_BOUNDS = 1.00f;
static const float SHIP_CLIENT_HARD_POSITION_LOWER_CORRECTION_BOUNDS = 0.10f;
static const float SHIP_CLIENT_HARD_POSITION_UPPER_CORRECTION_BOUNDS = 0.50f;
static const int SHIP_HEALTH = 5;

static const float BULLET_SPEED = 10.0f;
static const float BULLET_LIFETIME_SECONDS = 5.0f;
static const float BULLET_RADIUS = 0.1f;

static const float MIN_ASTEROID_RADIUS = 0.5f;
static const float MAX_ASTEROID_RADIUS = 1.5f;
static const float ASTEROID_SPAWN_SECONDS = 0.5f;
static const float ASTEROID_LIFE_SECONDS = 40.0f;