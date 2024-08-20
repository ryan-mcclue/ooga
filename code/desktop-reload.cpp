// SPDX-License-Identifier: zlib-acknowledgement
#if !TEST_BUILD
#define PROFILER 1
#endif

#include "desktop.h"

#include <rlgl.h>

State *g_state = NULL;

#include "desktop-assets.cpp"

// TODO: merge these into an introspected struct for UI tweaking
// :tweaks
#define TILE_WIDTH 120
#define TILE_HEIGHT TILE_WIDTH
#define TILE_SIZE V2(TILE_WIDTH, TILE_HEIGHT) 
#define ROCK_HEALTH 3
#define TREE_HEALTH 3
#define PLAYER_PICKUP_RADIUS 40
#define TOOLTIP_BOX_COLOUR
#define UI_Z_LAYER 50
#define WORLD_Z_LAYER 20

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

INTERNAL bool
hover_consume(Rectangle r)
{
  if (g_state->hover_consumed) return false;
  Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), g_state->camera);
  return (g_state->hover_consumed = CheckCollisionPointRec(mouse_world, r));
}

INTERNAL bool
left_click_consume(void)
{
  if (g_state->left_click_consumed) return false;
  return (g_state->left_click_consumed = IsMouseButtonReleased(MOUSE_BUTTON_LEFT));
}

INTERNAL void
draw_rect(Rectangle rec, Color color, Vector2 origin = {0, 0}, float rotation = 0.f)
{
    Vector2 topLeft = { 0 };
    Vector2 topRight = { 0 };
    Vector2 bottomLeft = { 0 };
    Vector2 bottomRight = { 0 };

    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (Vector2){ x, y };
        topRight = (Vector2){ x + rec.width, y };
        bottomLeft = (Vector2){ x, y + rec.height };
        bottomRight = (Vector2){ x + rec.width, y + rec.height };
    }
    else
    {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }

#if defined(SUPPORT_QUADS_DRAW_MODE)
    rlSetTexture(texShapes.id);

    rlBegin(RL_QUADS);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex3f(topLeft.x, topLeft.y, g_state->z_stack->z);

        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex3f(bottomLeft.x, bottomLeft.y, g_state->z_stack->z);

        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex3f(bottomRight.x, bottomRight.y, g_state->z_stack->z);

        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex3f(topRight.x, topRight.y, g_state->z_stack->z);

    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex3f(topLeft.x, topLeft.y        , g_state->z_stack->z);
        rlVertex3f(bottomLeft.x, bottomLeft.y  , g_state->z_stack->z);
        rlVertex3f(topRight.x, topRight.y      , g_state->z_stack->z);

        rlVertex3f(topRight.x, topRight.y      , g_state->z_stack->z);
        rlVertex3f(bottomLeft.x, bottomLeft.y  , g_state->z_stack->z);
        rlVertex3f(bottomRight.x, bottomRight.y, g_state->z_stack->z);

    rlEnd();
#endif
}

INTERNAL void
draw_texture(Texture2D texture, Vector2 position, float scale, Color tint, float rotation = 0.f)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, (float)texture.width*scale, (float)texture.height*scale };
    Vector2 origin = { 0.0f, 0.0f };

    // Check if texture is valid
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (source.width < 0) { flipX = true; source.width *= -1; }
        if (source.height < 0) source.y -= source.height;

        Vector2 topLeft = { 0 };
        Vector2 topRight = { 0 };
        Vector2 bottomLeft = { 0 };
        Vector2 bottomRight = { 0 };

        // Only calculate rotation if needed
        if (rotation == 0.0f)
        {
            float x = dest.x - origin.x;
            float y = dest.y - origin.y;
            topLeft = (Vector2){ x, y };
            topRight = (Vector2){ x + dest.width, y };
            bottomLeft = (Vector2){ x, y + dest.height };
            bottomRight = (Vector2){ x + dest.width, y + dest.height };
        }
        else
        {
            float sinRotation = sinf(rotation*DEG2RAD);
            float cosRotation = cosf(rotation*DEG2RAD);
            float x = dest.x;
            float y = dest.y;
            float dx = -origin.x;
            float dy = -origin.y;

            topLeft.x = x + dx*cosRotation - dy*sinRotation;
            topLeft.y = y + dx*sinRotation + dy*cosRotation;

            topRight.x = x + (dx + dest.width)*cosRotation - dy*sinRotation;
            topRight.y = y + (dx + dest.width)*sinRotation + dy*cosRotation;

            bottomLeft.x = x + dx*cosRotation - (dy + dest.height)*sinRotation;
            bottomLeft.y = y + dx*sinRotation + (dy + dest.height)*cosRotation;

            bottomRight.x = x + (dx + dest.width)*cosRotation - (dy + dest.height)*sinRotation;
            bottomRight.y = y + (dx + dest.width)*sinRotation + (dy + dest.height)*cosRotation;
        }

        rlSetTexture(texture.id);
        rlBegin(RL_QUADS);

            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

            // Top-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width)/width, source.y/height);
            else rlTexCoord2f(source.x/width, source.y/height);
            rlVertex3f(topLeft.x, topLeft.y, g_state->z_stack->z);

            // Bottom-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
            else rlTexCoord2f(source.x/width, (source.y + source.height)/height);
            rlVertex3f(bottomLeft.x, bottomLeft.y, g_state->z_stack->z);

            // Bottom-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x/width, (source.y + source.height)/height);
            else rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
            rlVertex3f(bottomRight.x, bottomRight.y, g_state->z_stack->z);

            // Top-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x/width, source.y/height);
            else rlTexCoord2f((source.x + source.width)/width, source.y/height);
            rlVertex3f(topRight.x, topRight.y, g_state->z_stack->z);

        rlEnd();
        rlSetTexture(0);

        // NOTE: Vertex position can be transformed using matrices
        // but the process is way more costly than just calculating
        // the vertex positions manually, like done above.
        // I leave here the old implementation for educational purposes,
        // just in case someone wants to do some performance test
        /*
        rlSetTexture(texture.id);
        rlPushMatrix();
            rlTranslatef(dest.x, dest.y, 0.0f);
            if (rotation != 0.0f) rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
            rlTranslatef(-origin.x, -origin.y, 0.0f);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);
                rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

                // Bottom-left corner for texture and quad
                if (flipX) rlTexCoord2f((source.x + source.width)/width, source.y/height);
                else rlTexCoord2f(source.x/width, source.y/height);
                rlVertex2f(0.0f, 0.0f);

                // Bottom-right corner for texture and quad
                if (flipX) rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
                else rlTexCoord2f(source.x/width, (source.y + source.height)/height);
                rlVertex2f(0.0f, dest.height);

                // Top-right corner for texture and quad
                if (flipX) rlTexCoord2f(source.x/width, (source.y + source.height)/height);
                else rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
                rlVertex2f(dest.width, dest.height);

                // Top-left corner for texture and quad
                if (flipX) rlTexCoord2f(source.x/width, source.y/height);
                else rlTexCoord2f((source.x + source.width)/width, source.y/height);
                rlVertex2f(dest.width, 0.0f);
            rlEnd();
        rlPopMatrix();
        rlSetTexture(0);
        */
    }
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
round_world_to_tile(Vector2 world)
{
  Vector2 r_world = ZERO_STRUCT;
  r_world.x = u32_round_to_nearest(F32_ROUND_U32(world.x), 
                                TILE_WIDTH * g_state->camera.zoom);
  r_world.y = u32_round_to_nearest(F32_ROUND_U32(world.y), 
                                TILE_HEIGHT * g_state->camera.zoom);
  return r_world;
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

INTERNAL Entity *
entity_create_building_furnace(void)
{
  Entity *e = entity_alloc();
  e->type = ENTITY_TYPE_BUILDING_FURNACE;
  return e;
}

INTERNAL void
inc_inventory_item_count(ENTITY_TYPE t, s32 inc)
{
  if (t >= ENTITY_TYPE_ITEM_FIRST && t <= ENTITY_TYPE_ITEM_LAST)
  {
    u32 i = t - ENTITY_TYPE_ITEM_FIRST;
    g_state->inventory_items[i].amount += inc;
  }
}

INTERNAL Texture
get_texture_from_entity_type(ENTITY_TYPE type)
{
  String8 texture_string = ZERO_STRUCT;
  switch (type)
  {
    default:
    {
      TraceLog(LOG_WARNING, "Failed to get texture");
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
    case ENTITY_TYPE_BUILDING_FURNACE:
    {
      texture_string = str8_lit("assets/building-furnace.png");
    } break;
    case ENTITY_TYPE_BUILDING_WORKBENCH:
    {
      texture_string = str8_lit("assets/building-workbench.png");
    } break;
  }
  return assets_get_texture(texture_string);
}

INTERNAL char *
get_pretty_name_from_entity_type(ENTITY_TYPE type)
{
  switch (type)
  {
    default:
    {
      return "Default";
    } break;
    case ENTITY_TYPE_PLAYER:
    {
      return "Player";
    } break;
    case ENTITY_TYPE_ROCK:
    {
      return "Rock";
    } break;
    case ENTITY_TYPE_TREE:
    {
      return "Tree";
    } break;
    case ENTITY_TYPE_ITEM_PINEWOOD:
    {
      return "Pinewood";
    } break;
    case ENTITY_TYPE_BUILDING_FURNACE:
    {
      return "Furnace";
    } break;
    case ENTITY_TYPE_BUILDING_WORKBENCH:
    {
      return "Workbench";
    } break;
  }
}

INTERNAL void
push_z(s32 z)
{
  S32Node *n = MEM_ARENA_PUSH_STRUCT(g_state->frame_arena, S32Node);
  n->z = z;
  SLL_STACK_PUSH(g_state->z_stack, n);
}

// TODO: ui_render() { push_z_layer(UI_Z_LAYER); pop_z_layer() }
// so can have it at top
// ui_render(); ... world_render()

  // TODO: have input consumption to establish a hierarchical nature (mouse_hovering and clicking important)
  // e.g: consume = inputs[code] &= ~(KEY_PRESSED);
  // simply add hover_consumed and click_consumed on Frame struct
EXPORT void 
code_update(State *state)
{ 
  PROFILE_FUNCTION() {
  g_state = state;

// TODO: move these into a WorldFrame struct
  f32 dt = GetFrameTime();
  u32 rw = GetRenderWidth();
  u32 rh = GetRenderHeight();

  push_z(0);

  if (!state->is_initialised)
  {
    state->is_initialised = true;
    state->camera.zoom = 1.f;

    state->hitbox_arena = mem_arena_allocate(MB(64), MB(64));

    state->player = entity_create_player();
    state->player->pos = world_to_tile_pos({rw/2, rh/2});

    #if DEBUG_BUILD
    u32 rand_seed = 1337;
    for (u32 i = 0; i < 10; i += 1)
    {
      Entity *e = entity_create_rock();
      e->pos = {f32_rand_range(&rand_seed, 0, 20), f32_rand_range(&rand_seed, 0, 20)};
      e = entity_create_tree();
      e->pos = {f32_rand_range(&rand_seed, 0, 20), f32_rand_range(&rand_seed, 0, 20)};
    }
    inc_inventory_item_count(ENTITY_TYPE_ITEM_PINEWOOD, 5);

    Entity *e = entity_create_building_furnace();
    e->pos = {10, 2};
    
    #endif
  }

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
      inc_inventory_item_count(e->type, 1);
      entity_free(e);
    }
  }
  mem_arena_clear(state->hitbox_arena);
  state->hitbox_stack = NULL;

  // :update entity destroy
  if (e_hovering != NULL && left_click_consume())
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
  rlEnableDepthTest();
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
    draw_rect(tile_rec, c);
  }

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

    draw_texture(e_texture, e_world_pos, 0, entity_scale, tint);

    Vector2 texture_size = V2(e_texture.width, e_texture.height) * entity_scale;
    Rectangle e_hitbox = {e_world_pos.x, e_world_pos.y, texture_size.x, texture_size.y};
    DrawRectangleLinesEx(e_hitbox, 2.0f, MAGENTA);
    Hitbox *h = MEM_ARENA_PUSH_STRUCT(state->hitbox_arena, Hitbox);
    h->r = e_hitbox;
    h->e = e;
    SLL_STACK_PUSH(g_state->hitbox_stack, h);
  }

  if (IsKeyReleased(KEY_TAB)) 
  {
    if (state->ui_state == UI_STATE_INVENTORY) state->ui_state = UI_STATE_NIL;
    else state->ui_state = UI_STATE_INVENTORY;
  }

  if (IsKeyReleased(KEY_C))
  {
    if (state->ui_state == UI_STATE_BUILDINGS) state->ui_state = UI_STATE_NIL;
    else state->ui_state = UI_STATE_BUILDINGS;
  }

  // :render overlays
  DrawCircle(e_hovering_rect.x, e_hovering_rect.y, e_hovering_rect.width, {122, 33, 11, 180});
  // :render inventory ui
  // TODO: stack based ui values, e.g. UI_Opacity(230), UI_FontSize() {}
  f32 ui_inventory_alpha_target_t = !!(f32)(state->ui_state == UI_STATE_INVENTORY);
  state->ui_inventory_alpha_t += (ui_inventory_alpha_target_t - state->ui_inventory_alpha_t) * f32_exp_out_fast(dt);
  if (f32_eq(state->ui_inventory_alpha_t, 0.f))
  {
    f32 h = rh * 0.15f;
    f32 box_w = rw * 0.1f;
    f32 box_m = box_w * 0.3f;
    // TODO: make fixed width and wrap when necessary
    f32 w = (box_w + box_m) * ARRAY_COUNT(state->inventory_items) - box_m;
    f32 x = rw*0.5f - w*0.5f;
    f32 y = rh - h;
    Vector2 world_xy = GetScreenToWorld2D({x, y}, state->camera);
    Rectangle region = {world_xy.x, world_xy.y, w, h};
    //draw_rect(region, WHITE);
    for (u32 i = 0; i < ARRAY_COUNT(state->inventory_items); i += 1)
    {
      Rectangle box = {region.x + (box_w + box_m) * i, region.y, box_w, h};
      draw_rect(box, {0, 0, 0, 125});

      ItemData *item = &state->inventory_items[i];
      ENTITY_TYPE type = (i + ENTITY_TYPE_ITEM_FIRST);
      if (item->amount > 0)
      {
        Texture t = get_texture_from_entity_type(type);
        f32 t_scale = 0.f;
        if (t.width > t.height) t_scale = (box_w / t.width);
        else t_scale = (h / t.height);
        t_scale *= 0.75f;

        f32 t_rotation = 0.f;
        // f32 t_rotation = 360 * f32_sin_out(GetTime());

        bool box_hover = hover_consume(box);
        if (box_hover)
        {
          t_scale += (t_scale * 0.1f);
          // IMPORTANT: this is polishing. just need minimum feedback for prototype
          // f32_sin_in_out(GetTime());
        }

        Vector2 t_scaled = V2(t.width, t.height) * t_scale;
        Vector2 t_centered = {box.x + box.width*0.5f - t_scaled.x*0.5f, 
                             box.y + box.height*0.5f - t_scaled.y*0.5f};

        // IMPORTANT: after setting origin to centre, we must now pass the centre as draw point
        DrawTexturePro(t, {0, 0, t.width, t.height}, 
                      {t_centered.x + t_scaled.x*0.5f, t_centered.y + t_scaled.y*0.5f, t_scaled.x, t_scaled.y},
                      {t_scaled.x*0.5f, t_scaled.y*0.5f},
                      t_rotation, WHITE);
        if (box_hover)
        {
          Vector2 t_centre = {t_centered.x + t_scaled.x*0.5f, t_centered.y + t_scaled.y*0.5f};
          f32 tooltip_w = box_w * 1.5f;
          f32 tooltip_h = h * 0.75f;
          Rectangle tooltip = {t_centre.x - tooltip_w*0.5f, 
                               t_centre.y, tooltip_w, tooltip_h};
          // draw tooltip
          draw_rect(tooltip, {0, 0, 255, 100});

          f32 font_size = 48.f;
          String8 text_fmt = str8_fmt(state->frame_arena, "%s (%d)", 
                                      get_pretty_name_from_entity_type(type),
                                      item->amount);
          char *text = (char *)text_fmt.content;
          Font font = assets_get_font(str8_lit("assets/Alegreya-Regular.ttf"));
          Vector2 text_size = MeasureTextEx(font, text, font_size, 1.f);
          Vector2 text_pos = {tooltip.x + tooltip.width*0.5f - text_size.x*0.5f, tooltip.y};
          DrawTextEx(font, text, text_pos, font_size, 1.f, WHITE);
        }
      }
    }
  }
  // :render buildings ui  
  if (state->ui_state == UI_STATE_BUILDINGS)
  {
    f32 h = rh * 0.15f;
    f32 box_w = rw * 0.1f;
    f32 box_m = box_w * 0.3f;
    // TODO: make fixed width and wrap when necessary
    f32 w = (box_w + box_m) * ARRAY_COUNT(state->buildings) - box_m;
    f32 x = rw*0.5f - w*0.5f;
    f32 y = 0.0f;
    Vector2 world_xy = GetScreenToWorld2D({x, y}, state->camera);
    Rectangle region = {world_xy.x, world_xy.y, w, h};
    for (u32 i = 0; i < ARRAY_COUNT(state->buildings); i += 1)
    {
      Rectangle box = {region.x + (box_w + box_m) * i, region.y, box_w, h};

      draw_rect(box, {0, 0, 0, 125});

      ItemData *item = &state->inventory_items[i];
      ENTITY_TYPE type = (i + ENTITY_TYPE_BUILDING_FIRST);
      Texture t = get_texture_from_entity_type(type);
      f32 t_scale = 0.f;
      if (t.width > t.height)
        t_scale = (box_w / t.width);
      else
        t_scale = (h / t.height);
      t_scale *= 0.75f;

      f32 t_rotation = 0.f;
      // f32 t_rotation = 360 * f32_sin_out(GetTime());

      bool box_hover = hover_consume(box);
      if (box_hover)
      {
        t_scale += (t_scale * 0.1f);
        // IMPORTANT: this is polishing. just need minimum feedback for prototype
        // f32_sin_in_out(GetTime());
      }

      Vector2 t_scaled = V2(t.width, t.height) * t_scale;
      Vector2 t_centered = {box.x + box.width * 0.5f - t_scaled.x * 0.5f,
                            box.y + box.height * 0.5f - t_scaled.y * 0.5f};

      // IMPORTANT: after setting origin to centre, we must now pass the centre as draw point
      DrawTexturePro(t, {0, 0, t.width, t.height},
                     {t_centered.x + t_scaled.x * 0.5f, t_centered.y + t_scaled.y * 0.5f, t_scaled.x, t_scaled.y},
                     {t_scaled.x * 0.5f, t_scaled.y * 0.5f},
                     t_rotation, WHITE);

      // draw tooltip
      if (box_hover)
      {
        Vector2 t_centre = {t_centered.x + t_scaled.x * 0.5f, t_centered.y + t_scaled.y * 0.5f};
        f32 tooltip_w = box_w * 1.5f;
        f32 tooltip_h = h * 0.75f;
        Rectangle tooltip = {t_centre.x - tooltip_w * 0.5f,
                             t_centre.y, tooltip_w, tooltip_h};
        draw_rect(tooltip, {0, 0, 255, 100});

        f32 font_size = 48.f;
        String8 text_fmt = str8_fmt(state->frame_arena, "%s",
                                    get_pretty_name_from_entity_type(type));
        char *text = (char *)text_fmt.content;
        Font font = assets_get_font(str8_lit("assets/Alegreya-Regular.ttf"));
        Vector2 text_size = MeasureTextEx(font, text, font_size, 1.f);
        Vector2 text_pos = {tooltip.x + tooltip.width * 0.5f - text_size.x * 0.5f, tooltip.y};
        DrawTextEx(font, text, text_pos, font_size, 1.f, WHITE);

        if (left_click_consume()) state->active_building_type = type;
      }
    }
  }

  // :render building mode ui
  if (state->active_building_type != ENTITY_TYPE_NIL)
  {
    BuildingData *bd = &state->buildings[state->active_building_type - ENTITY_TYPE_BUILDING_FIRST];
    
    Texture t = get_texture_from_entity_type(state->active_building_type);
    // TODO: get_aligned_vec_from_rect(rect, ALIGN_CENTRE);

    Vector2 pos = round_world_to_tile(mouse_world);
    draw_texture(t, pos, 0.f, entity_scale, WHITE);
    //DrawRectangleLines(pos.x, pos.y, t.width, t.height, MAGENTA);
    if (left_click_consume()) 
    {
      Entity *e = entity_create_building_furnace();
      e->pos = pos;
      state->active_building_type = ENTITY_TYPE_NIL;
    }
  }


  g_dbg_at_y = 0.f;
  state->hover_consumed = false;
  state->left_click_consumed = false;

  EndMode2D();
  EndDrawing();
  }
}

PROFILER_END_OF_COMPILATION_UNIT
