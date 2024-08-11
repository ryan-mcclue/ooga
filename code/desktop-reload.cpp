// SPDX-License-Identifier: zlib-acknowledgement
#if !TEST_BUILD
#define PROFILER 1
#endif

#include "desktop.h"

GLOBAL State *g_state = NULL;

#include "desktop-assets.cpp"

EXPORT void 
code_preload(State *state)
{
  profiler_init();
  
  assets_preload(state);
}

EXPORT void 
code_postload(State *state)
{}

EXPORT void
code_profiler_end_and_print(State *state)
{
  profiler_end_and_print();
}

EXPORT void 
code_update(State *state)
{ 
  PROFILE_FUNCTION() {
  g_state = state;

  if (!state->is_initialised)
  {
    state->is_initialised = true;
    state->player_pos = {0, 0};
    state->camera.zoom = 1.f;
  }

  f32 dt = GetFrameTime();
  u32 rw = GetRenderWidth();
  u32 rh = GetRenderHeight();

  if (IsKeyPressed(KEY_F)) 
  {
    if (IsWindowMaximized()) RestoreWindow();
    else MaximizeWindow();
  }

  Vector2 player_dp = ZERO_STRUCT;
  f32 player_v = 150.f;
  if (IsKeyDown(KEY_UP)) player_dp.y -= 1;
  if (IsKeyDown(KEY_DOWN)) player_dp.y += 1;
  if (IsKeyDown(KEY_LEFT)) player_dp.x -= 1;
  if (IsKeyDown(KEY_RIGHT)) player_dp.x += 1;
  if (IsKeyDown(KEY_LEFT_SHIFT)) player_v += 200.f;
  player_dp = Vector2Normalize(player_dp);
  state->player_pos += (player_dp * player_v * dt);

  BeginDrawing();
  ClearBackground(RAYWHITE);
  BeginMode2D(state->camera);

  // NOTE(Ryan): Rendering at 1920; Sprites done on 240
  f32 texture_scale = 8.0f;
  DrawTextureEx(assets_get_texture(str8_lit("assets/portal.png")), 
                state->player_pos, 0, texture_scale, WHITE);

  EndMode2D();
  EndDrawing();
  }
}

PROFILER_END_OF_COMPILATION_UNIT
