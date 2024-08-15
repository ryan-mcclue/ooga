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
#define PLAYER_PICKUP_RADIUS 40

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

INTERNAL Texture
get_texture_from_entity_type(ENTITY_TYPE type)
{
  String8 texture_string = ZERO_STRUCT;
  switch (type)
  {
    default:
    {
      return g_state->assets.default_texture;
    } break;
    case ENTITY_TYPE_PLAYER:
    {
      texture_string = str8_lit("assets/player.png");
    } break;
    case ENTITY_TYPE_ROCK:
    {
      texture_string = str8_lit("assets/rock.png");
    } break;
    case ENTITY_TYPE_TREE:
    {
      texture_string = str8_lit("assets/tree.png");
    } break;
    case ENTITY_TYPE_ITEM_PINEWOOD:
    {
      texture_string = str8_lit("assets/item-pinewood.png");
    } break;
  }
  return assets_get_texture(texture_string);
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

    state->hitbox_arena = mem_arena_allocate(MB(64), MB(64));

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

    //state->inventory_items[ENTITY_TYPE_ITEM_PINEWOOD].amount = 5;
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

  // :process entity hitboxes
  f32 closest_hitbox_lengthsq = f32_inf();
  Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), state->camera);
  Entity *e_hovering = NULL;
  Rectangle e_hovering_rect = ZERO_STRUCT;
  Vector2 player_world = tile_to_world_pos(state->player->pos);
  for (Hitbox *h = state->hitbox_stack; h != NULL; h = h->next)
  {
    Rectangle r = h->r;
    Entity *e = h->e;
    f32 h_radius = MAX(r.width*.5f, r.height*.5f);
    Vector2 h_centre = {r.x + r.width*.5f, r.y + r.height*.5f};
    f32 lengthsq = Vector2LengthSqr(h_centre - mouse_world);
    if (lengthsq < closest_hitbox_lengthsq && 
        lengthsq <= SQUARE(h_radius) && !e->is_item)
    {
      e_hovering= e;
      closest_hitbox_lengthsq = lengthsq;
      e_hovering_rect = {r.x + r.width*.5f, r.y + r.height*.5f, h_radius, h_radius};
    }

    // TODO: get player hitbox so can get distance from it's centre
    lengthsq = Vector2LengthSqr(h_centre - player_world);
    if (e->is_item && lengthsq < SQUARE(PLAYER_PICKUP_RADIUS))
    {
      state->inventory_items[e->type].amount += 1;
      entity_free(e);
    }
  }
  mem_arena_clear(state->hitbox_arena);
  state->hitbox_stack = NULL;

  // :update entity destroy
  if (e_hovering != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
  {
    e_hovering->health -= 1;
    if (e_hovering->health <= 0)
    {
      switch (e_hovering->type)
      {
        case ENTITY_TYPE_TREE:
        {
          Entity *e = entity_create_item_pinewood();
          e->pos = e_hovering->pos;
        } break;
      }
      entity_free(e_hovering);
    }
  }

  BeginDrawing();
  ClearBackground(RAYWHITE);
  BeginMode2D(state->camera);

  // :render map
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

  //DrawTextureEx(assets_get_texture(str8_lit("assets/rock.png")), {state->camera.target.x, state->camera.target.y + 10 * f32_sin_in(GetTime())}, 0, 1.f, WHITE);

  // NOTE(Ryan): Rendering at 1920; Sprites done on 240
  // :render entities
  f32 entity_scale = 8.0f;
  for (u32 i = 0; i < ARRAY_COUNT(state->entities); i += 1)
  {
    Entity *e = &g_state->entities[i];
    if (!e->is_active) continue;
    Texture e_texture = get_texture_from_entity_type(e->type);
    Vector2 e_world_pos = tile_to_world_pos(e->pos);
    if (e->type == ENTITY_TYPE_ITEM_PINEWOOD)
    {
      e_world_pos.y += (entity_scale * 5 * f32_sin_in_out(GetTime()));
    }
    Color tint = BLACK;
    if (e_texture.id == state->assets.default_texture.id) tint = WHITE;
    DrawTextureEx(e_texture, e_world_pos, 0, entity_scale, tint);

    Vector2 texture_size = V2(e_texture.width, e_texture.height) * entity_scale;
    Rectangle e_hitbox = {e_world_pos.x, e_world_pos.y, texture_size.x, texture_size.y};
    DrawRectangleLinesEx(e_hitbox, 2.0f, MAGENTA);
    Hitbox *h = MEM_ARENA_PUSH_STRUCT(state->hitbox_arena, Hitbox);
    h->r = e_hitbox;
    h->e = e;
    SLL_STACK_PUSH(g_state->hitbox_stack, h);
  }

  // :render overlays
  DrawCircle(e_hovering_rect.x, e_hovering_rect.y, e_hovering_rect.width, {122, 33, 11, 180});
  // :render inventory ui
  {
    f32 h = rh * 0.15f;
    f32 box_w = rw * 0.1f;
    f32 box_m = box_w * 0.3f;
    f32 w = (box_w + box_m) * ARRAY_COUNT(state->inventory_items) - box_m;
    f32 x = rw*0.5f - w*0.5f;
    f32 y = rh - h;
    Vector2 world_xy = GetScreenToWorld2D({x, y}, state->camera);
    Rectangle region = {world_xy.x, world_xy.y, w, h};
    DrawRectangleRec(region, WHITE);
    for (u32 i = 0; i < ARRAY_COUNT(state->inventory_items); i += 1)
    {
      Rectangle box = {region.x + (box_w + box_m) * i, region.y, box_w, h};
      DrawRectangleRec(box, BLACK);

      Item *item = &state->inventory_items[i];
      if (item->amount > 0)
      {
        Texture t = get_texture_from_entity_type((ENTITY_TYPE)i);
        f32 t_scale = 0.f;
        if (t.width > t.height) t_scale = (box_w / t.width);
        else t_scale = (h / t.height);
        t_scale *= 0.75f;

        Vector2 t_scaled = V2(t.width, t.height) * t_scale;
        Vector2 t_centered = {box.x + box.width*0.5f - t_scaled.x*0.5f, 
                             box.y + box.height*0.5f - t_scaled.y*0.5f};
        DrawTextureEx(t, t_centered, 0.f, t_scale, WHITE);
      }
    }

    // get_texture_from_entity_type(e->type)

  }
  


  g_dbg_at_y = 0.f;
  EndMode2D();
  EndDrawing();
  }
}

PROFILER_END_OF_COMPILATION_UNIT
