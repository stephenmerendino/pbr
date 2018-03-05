#pragma once

class App;
class Game;

extern App*			g_app;
extern Game*		g_game;
extern bool         g_debug_draw_on;

enum GameState
{
    GAME_STATE_MAIN_MENU,
    GAME_STATE_HOST,
    GAME_STATE_JOIN,
    GAME_STATE_CLIENT,
    NUM_GAME_STATES
};