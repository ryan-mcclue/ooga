// SPDX-License-Identifier: zlib-acknowledgement
#if !TEST_BUILD
#define PROFILER 1
#endif

#include "desktop.h"

GLOBAL State *g_state = NULL;

#include "desktop-assets.cpp"

// TODO: merge these into an introspected struct for UI tweaking
#define TILE_WIDTH 120
#define TILE_HEIGHT TILE_WIDTH
#define TILE_SIZE V2(TILE_WIDTH, TILE_HEIGHT) 
#define ROCK_HEALTH 3
#define TREE_HEALTH 3

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

INTERNAL Vector2
world_to_tile_pos(Vector2 world)
{
  Vector2 tile = world;
  tile.x /= (TILE_WIDTH * g_state->camera.zoom);
  tile.y /= (TILE_HEIGHT * g_state->camera.zoom);
  return tile;
}

INTERNAL Vector2
tile_to_world_pos(Vector2 tile)
{
  Vector2 world = tile;
  world.x *= (TILE_WIDTH * g_state->camera.zoom);
  world.y *= (TILE_HEIGHT * g_state->camera.zoom);
  return world;
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
  e->health = ROCK_HEALTH;
  return e;
}

INTERNAL Entity *
entity_create_tree(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_TREE;
  e->health = TREE_HEALTH;
  return e;
}

INTERNAL Entity *
entity_create_item_pinewood(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_ITEM_PINEWOOD;
  e->is_item = true;
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
    state->player->pos = world_to_tile_pos({rw/2, rh/2});

    u32 rand_seed = 1337;
    for (u32 i = 0; i < 10; i += 1)
    {
      Entity *e = entity_create_rock();
      e->pos = {f32_rand_range(&rand_seed, 0, 20), f32_rand_range(&rand_seed, 0, 20)};
      e = entity_create_tree();
      e->pos = {f32_rand_range(&rand_seed, 0, 20), f32_rand_range(&rand_seed, 0, 20)};
    }

    state->inventory_items[ENTITY_TYPE_ITEM_PINEWOOD].amount = 5;
  }

  // TODO: have input consumption to establish a hierarchical nature
  // e.g: consume = inputs[code] &= ~(KEY_PRESSED);
  if (IsKeyPressed(KEY_F)) 
  {
    if (IsWindowMaximized()) RestoreWindow();
    else MaximizeWindow();
  }

  Vector2 player_dp = ZERO_STRUCT;
  f32 player_v = 8.f;
  if (IsKeyDown(KEY_UP)) player_dp.y -= 1;
  if (IsKeyDown(KEY_DOWN)) player_dp.y += 1;
  if (IsKeyDown(KEY_LEFT)) player_dp.x -= 1;
  if (IsKeyDown(KEY_RIGHT)) player_dp.x += 1;
  if (IsKeyDown(KEY_LEFT_SHIFT)) player_v *= 2.f;
  player_dp = Vector2Normalize(player_dp);
  state->player->pos += (player_dp * player_v * dt);

  Vector2 cur_camera = state->camera.target;
  Vector2 target_camera = tile_to_world_pos(state->player->pos);
  state->camera.target += (target_camera - cur_camera) * f32_exp_out_slow(dt);

  // TODO: add player sprite w/h to calculation
  state->camera.offset = V2(rw/2, rh/2) * state->camera.zoom;

  BeginDrawing();
  ClearBackground(RAYWHITE);
  BeginMode2D(state->camera);

  Vector2 player_tile = state->camera.target / TILE_SIZE;
  for (u32 i = 0; i < 16*16; i += 1)
  {
    u32 x = i % 16;
    u32 y = i / 16;
    Color c = (x + y) & 1 ? GREEN : BROWN;
    Rectangle tile_rec = {(player_tile.x - 8 + x) * TILE_WIDTH, 
                          (player_tile.y - 8 + y) * TILE_HEIGHT, 
                          TILE_WIDTH, TILE_HEIGHT};
    DrawRectangleRec(tile_rec, c);
    //if (CheckCollisionPointRec(mouse_world, tile_rec))
  }

  // NOTE(Ryan): Rendering at 1920; Sprites done on 240
  f32 entity_scale = 8.0f;
  for (u32 i = 0; i < ARRAY_COUNT(state->entities); i += 1)
  {
    Entity *e = &g_state->entities[i];
    if (!e->is_active) continue;

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
        e_texture_str = str8_lit("assets/adasdasdas.png");
      } break;
      case ENTITY_TYPE_ITEM_PINEWOOD:
      {
        e_texture_str = str8_lit("assets/item-pinewood.png");
      } break;
      default: {}
    }
    Texture e_texture = assets_get_texture(e_texture_str);
    Vector2 e_world_pos = tile_to_world_pos(e->pos);
    if (e->type == ENTITY_TYPE_ITEM_PINEWOOD)
    {
      e_world_pos.y += (entity_scale * 5 * f32_sin_in_out(GetTime()));
    }

    // TODO: wrapper for DrawTexture() that if receives ZERO_TEXTURE draws rectangle
    DrawTextureEx(e_texture, e_world_pos, 0, entity_scale, BLACK);

    Vector2 texture_size = V2(e_texture.width, e_texture.height) * entity_scale;
    Rectangle e_hitbox = {e_world_pos.x, e_world_pos.y, texture_size.x, texture_size.y};
    DrawRectangleLinesEx(e_hitbox, 2.0f, MAGENTA);
    Hitbox *h = MEM_ARENA_PUSH_STRUCT(g_state->frame_arena, Hitbox);
    h->r = e_hitbox;
    h->e = e;
    SLL_STACK_PUSH(g_state->e_hitbox_stack, h);
  }

  f32 closest = f32_inf();
  Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), state->camera);
  Entity *e_hovering_entity = NULL;
  Rectangle e_rect = ZERO_STRUCT;
  for (Hitbox *e_hitbox = state->e_hitbox_stack; 
       e_hitbox != NULL; 
       e_hitbox = e_hitbox->next)
  {
    Rectangle r = e_hitbox->r;
    f32 radius = MAX(r.width*.5f, r.height*.5f);
    Vector2 centre = {r.x + r.width*.5f, r.y + r.height*.5f};
    f32 length = Vector2LengthSqr(centre - mouse);
    if (length < closest && length <= SQUARE(radius) && !e_hitbox->e->is_item)
    {
      e_hovering_entity = e_hitbox->e;
      closest = length;
      e_rect = {r.x + r.width*.5f, r.y + r.height*.5f, radius, radius};
    }
  }
  DrawCircle(e_rect.x, e_rect.y, e_rect.width, {122, 33, 11, 180});

  if (e_hovering_entity != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
  {
    e_hovering_entity->health -= 1;
    if (e_hovering_entity->health <= 0)
    {
      switch (e_hovering_entity->type)
      {
        case ENTITY_TYPE_TREE:
        {
          Entity *e = entity_create_item_pinewood();
          e->pos = e_hovering_entity->pos;
        } break;
      }

      entity_free(e_hovering_entity);
    }
  }

  // IMPORTANT: using frame arena, so don't have to explicitly clear memory
  // TODO: have frame arena be cleared to 0, so don't have explicit 'clearers'
  state->e_hitbox_stack = NULL;
  g_dbg_at_y = 0.f;
  EndMode2D();
  EndDrawing();
  }
}

PROFILER_END_OF_COMPILATION_UNIT
