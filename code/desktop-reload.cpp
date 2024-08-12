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

INTERNAL Entity *
entity_alloc(void)
{
  for (u32 i = 0; i < ARRAY_COUNT(g_state->entities); i += 1)
  {
    Entity *e = &g_state->entities[i];
    if (!e->is_active)
    {
      e->is_active = true;
      return e;
    }
  }

  return NULL;
}

INTERNAL void
entity_free(Entity *e)
{
  MEMORY_ZERO(e, sizeof(Entity));
}

INTERNAL Entity *
entity_create_player(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_PLAYER;
  return e;
}

INTERNAL Entity *
entity_create_rock(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_ROCK;
  return e;
}

INTERNAL Entity *
entity_create_tree(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_TREE;
  return e;
}

EXPORT void 
code_update(State *state)
{ 
  PROFILE_FUNCTION() {
  g_state = state;

  f32 dt = GetFrameTime();
  u32 rw = GetRenderWidth();
  u32 rh = GetRenderHeight();

  if (!state->is_initialised)
  {
    state->is_initialised = true;
    state->camera.zoom = 1.f;

    state->player = entity_create_player();
    state->player->pos = {rw/2, rh/2};

    u32 rand_seed = 1337;
    for (u32 i = 0; i < 10; i += 1)
    {
      Entity *e = entity_create_rock();
      e->pos = {rw/4 + i * f32_rand_range(&rand_seed, 1, 100), rh/4};
      e = entity_create_tree();
      e->pos = {rw/2 + i * f32_rand_range(&rand_seed, 1, 100), rh/2};
    }
  }

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
  state->player->pos += (player_dp * player_v * dt);

  Vector2 cur_camera = state->camera.target;
  Vector2 target_camera = state->player->pos;
  state->camera.target += (target_camera - cur_camera) * f32_exp_out_slow(dt);

  // TODO: add player sprite w/h to calculation
  state->camera.offset = V2(rw/2, rh/2) * state->camera.zoom;

  BeginDrawing();
  ClearBackground(RAYWHITE);
  BeginMode2D(state->camera);

  // NOTE(Ryan): Rendering at 1920; Sprites done on 240
  f32 texture_scale = 8.0f;
  for (u32 i = 0; i < ARRAY_COUNT(state->entities); i += 1)
  {
    Entity *e = &g_state->entities[i];
    String8 e_texture_str = ZERO_STRUCT;
    switch (e->type)
    {
      case ENTITY_TYPE_PLAYER:
      {
        e_texture_str = str8_lit("assets/player.png");
      } break;
      case ENTITY_TYPE_ROCK:
      {
        e_texture_str = str8_lit("assets/rock.png");
      } break;
      case ENTITY_TYPE_TREE:
      {
        e_texture_str = str8_lit("assets/tree.png");
      } break;
      default: {}
    }
    DrawTextureEx(assets_get_texture(e_texture_str), e->pos, 0, texture_scale, BLACK);
  }

  f32 closest = f32_inf();
  f32 draw_radius = 10.f * (texture_scale * 0.5f);
  Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), state->camera);
  Entity *mouse_entity = NULL;
  for (u32 i = 0; i < ARRAY_COUNT(state->entities); i += 1)
  {
    Entity *e = &g_state->entities[i];
    // Vector2 centred_e = 
    // e->radius
    f32 length = Vector2LengthSqr(e->pos - mouse);
    if (length < closest && length < SQUARE(draw_radius))
    {
      mouse_entity = e;
      closest = length;
    }
  }
  DBG_V2(mouse);
  DBG_F32(closest);
  if (mouse_entity != NULL) 
  {
    DBG_V2(mouse_entity->pos);
    DrawCircle(mouse_entity->pos.x, mouse_entity->pos.y, draw_radius, RED);
  }

  g_at_y = 0.f;
  EndMode2D();
  EndDrawing();
  }
}

PROFILER_END_OF_COMPILATION_UNIT
