#if !defined(DESKTOP_H)
#define DESKTOP_H

#include "base/base-inc.h"
#include "desktop-assets.h"
#include <raylib.h>
#include <raymath.h>

#define V2(x, y) CCOMPOUND(Vector2){(f32)x, (f32)y}
#if defined(LANG_CPP)
INTERNAL Vector2 operator*(Vector2 a, Vector2 b) { return Vector2Multiply(a, b); }
INTERNAL Vector2 operator*(f32 s, Vector2 a) { return Vector2Scale(a, s); }
INTERNAL Vector2 operator*(Vector2 a, f32 s) { return Vector2Scale(a, s); }
INTERNAL Vector2 & operator*=(Vector2 &a, f32 s) { a = a * s; return a; } 

INTERNAL Vector2 operator+(f32 b, Vector2 a) { return Vector2AddValue(a, b); }
INTERNAL Vector2 operator+(Vector2 a, f32 b) { return Vector2AddValue(a, b); }
INTERNAL Vector2 operator+(Vector2 a, Vector2 b) { return Vector2Add(a, b); }
INTERNAL Vector2 & operator+=(Vector2 &a, Vector2 b) { a = a + b; return a; }

INTERNAL Vector2 operator-(f32 b, Vector2 a) { return Vector2SubtractValue(a, b); }
INTERNAL Vector2 operator-(Vector2 a, f32 b) { return Vector2SubtractValue(a, b); }
INTERNAL Vector2 operator-(Vector2 a, Vector2 b) { return Vector2Subtract(a, b); }
INTERNAL Vector2 & operator-=(Vector2 &a, Vector2 b) { a = a - b; return a; }
INTERNAL Vector2 operator-(Vector2 a) { return Vector2Negate(a); }

INTERNAL Vector2 operator/(Vector2 a, f32 b) { return {a.x/b, a.y/b}; }
INTERNAL Vector2 operator/(Vector2 a, Vector2 b) { return Vector2Divide(a, b); }

INTERNAL bool operator==(Vector2 a, Vector2 b) { return f32_eq(a.x, b.x) && f32_eq(a.y, b.y); }
INTERNAL bool operator!=(Vector2 a, Vector2 b) { return !(a == b); }
#endif

GLOBAL f32 g_dbg_at_y = 0.f;
INTERNAL void
draw_debug_text(String8 s)
{
  f32 at_x = 50.f;
  char text[64] = ZERO_STRUCT;
  str8_to_cstr(s, text, sizeof(text));
  DrawText(text, at_x, g_dbg_at_y, 48, RED);
  g_dbg_at_y += 50.f;
}
#if DEBUG_BUILD
#define DBG_U32(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %" PRIu32, var))
#define DBG_S32(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %" PRId32, var))
#define DBG_U64(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %" PRIu64, var))
#define DBG_S64(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %" PRId64, var))
#define DBG_F32(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %f", var))
#define DBG_F64(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = %lf", var))
#define DBG_V2(var) \
  draw_debug_text(str8_fmt(g_state->frame_arena, STRINGIFY(var) " = (%f, %f)", var.x, var.y))
#else
#define DBG_U32(var)
#define DBG_S32(var)
#define DBG_U64(var)
#define DBG_S64(var)
#define DBG_F32(var)
#define DBG_F64(var)
#define DBG_V2(var)
#endif

typedef enum
{
  ENTITY_TYPE_NIL = 0,
  ENTITY_TYPE_ROCK = 1,
  ENTITY_TYPE_TREE = 2,
  ENTITY_TYPE_PLAYER = 3,
  ENTITY_TYPE_ITEM_ROCK = 4,
  ENTITY_TYPE_ITEM_PINEWOOD = 5,
  ENTITY_TYPE_COUNT
} ENTITY_TYPE;

typedef struct Item Item;
struct Item
{
  int amount;
};

typedef struct Entity Entity;
struct Entity
{
  ENTITY_TYPE type;
  b32 is_active;
  Vector2 pos;
  u32 health;

  // TODO: make mask
  bool is_item;
  //ItemID item;
  // TODO:
  // bool render_texture;
  // TEXTURE_ID texture_id;
};
/*
Texture *get_texture(TEXTURE_ID id)
{
  if (id > TEXTURE_ID_NIL && id < TEXTURE_ID_MAX) return &g_textures[id];
  else return &g_textures[TEXTURE_ID_NIL];
}
*/

typedef struct Hitbox Hitbox;
struct Hitbox
{
  Hitbox *next;
  Rectangle r;
  Entity *e;
};

typedef struct State State;
INTROSPECT() struct State
{
  b32 is_initialised;

  Assets assets;

  MemArena *arena;
  MemArena *frame_arena;
  u64 frame_counter;

  Entity entities[1024];
  // TODO: use generation handles
  Entity *player;

  MemArena *hitbox_arena;
  Hitbox *hitbox_stack;

  // type implicit from index
  Item inventory_items[ENTITY_TYPE_COUNT];

  Camera2D camera;
};

typedef void (*code_preload_t)(State *s);
typedef void (*code_update_t)(State *s);
typedef void (*code_postload_t)(State *s);
typedef void (*code_profiler_end_and_print_t)(State *s);

typedef struct ReloadCode ReloadCode;
struct ReloadCode
{
  code_preload_t preload;
  code_update_t update;
  code_postload_t postload;
  code_profiler_end_and_print_t profiler_end_and_print;
};

#endif
