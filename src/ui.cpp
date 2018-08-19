struct UIID {
  uint32 line;

  int id;
};

#define uiid_gen() uiid_init(__LINE__)
#define uiid_genEx(i) uiid_init(__LINE__, i)

UIID uiid_init(uint32 line) {
  UIID id = {0};
  id.line = line;

  return id;
}

UIID uiid_init(uint32 line, int i) {
  UIID id = uiid_init(line);

  id.id = i;

  return id;
}

bool operator==(UIID l, UIID r) {
  return l.line == r.line && l.id == r.id;
}

enum {
  UI_ELEMENT_WINDOW,
  UI_ELEMENT_BOX,
  UI_ELEMENT_TOOLBAR,
  UI_ELEMENT_TOOLBAR_OPTION,
  UI_ELEMENT_TEXT
};

struct UIElement_Window {
  vec2 next;
};

// TODO(harrison): switch to a UI specific MemoryArena so we can have REAL text
// buffers. None of this 256 char only bullshit.
struct UIElement_Text {
  real32 scale;
  char text[256];
};

#define UI_ELEMENT_BOX_WIDTH (75.0f)
#define UI_ELEMENT_BOX_HEIGHT (75.0f)
struct UIElement_Box {
  uint32 textureID;
  int itemCount;
  bool selected;
};

struct UIElement_Toolbar {
  real32 offset;
};

struct UIElement_ToolbarOption {
  UIID parent;
  bool hasTexture;
  uint32 textureID;

  int count;
  bool selected;
};

struct UIElement {
  UIID id;
  uint32 type;
  bool touched;

  Rect rect;

  union {
    UIElement_Window window;
    UIElement_Box box;

    UIElement_Text text;

    UIElement_Toolbar toolbar;
    UIElement_ToolbarOption toolbarOption;
  };
};

#define UI_ELEMENT_TOOLBAR_OPTION_SIZE 75
#define UI_ELEMENT_TOOLBAR_PADDING 2

#define UI_MAX_ELEMENTS 128
#define UI_MAX_DEPTH 10

struct {
  UIElement elements[UI_MAX_ELEMENTS];
  uint32 elementCount;

  UIElement *parent;
  UIElement *window;

  int paneStackCount;
  Rect paneStack[UI_MAX_DEPTH];
  vec2 next;
  real32 lastHeight;

  Font font;
} ui = {0};

UIElement* ui_getElement(UIID id) {
  for (uint32 i = 0; i < ui.elementCount; i++) {
    UIElement* e = &ui.elements[i];

    if (e->id == id) {
      return e;
    }
  }

  return 0;
}

void ui_pane_push(Rect r) {
  ui.paneStack[ui.paneStackCount] = r;
  ui.paneStackCount += 1;

  ui.next = {0};
}

void ui_pane_pop() {
  ui.paneStackCount -= 1;
}

void ui_begin(Font f) {
  ui.next = {0};

  ui.parent = 0;

  ui.font = f;

  ui.paneStackCount = 0;
}

void ui_end() {
  for (uint32 i = 0; i < ui.elementCount; i++) {
    ui.elements[i].touched = false;
  }
}

UIElement* ui_initElement(UIID id, uint32 type) {
  UIElement* e = &ui.elements[ui.elementCount];
  e->type = type;
  e->id = id;

  ui.elementCount += 1;
  ensure(ui.elementCount < UI_MAX_ELEMENTS);

  return e;
}

void ui_getPos(Rect* r) {
  real32 w = r->w;
  real32 h = r->h;

  int paneIndex = ui.paneStackCount - 1;
  ensure (paneIndex >= 0 && paneIndex < UI_MAX_DEPTH - 1);

  Rect currentPane = ui.paneStack[paneIndex];

  if (ui.next.x + w > currentPane.w) {
    ui.next.x = 0;
    ui.next.y += h;
  }

  vec2 wpos = {0};

  for (int i = 0; i < ui.paneStackCount; i++) {
    wpos.x += ui.paneStack[i].x;
    wpos.y += ui.paneStack[i].y;
  }

  wpos += ui.next;

  r->x = wpos.x;
  r->y = wpos.y;

  ui.next.x += w;
  ui.lastHeight = h;
}

void ui_break() {
  ui.next.x = 0;
  ui.next.y += ui.lastHeight;
}

#define UI_ALIGN_LEFT (0)
#define UI_ALIGN_CENTER_X (0x1)
#define UI_ALIGN_CENTER_Y (0x2)
#define UI_ALIGN_CENTER (UI_ALIGN_CENTER_X | UI_ALIGN_CENTER_Y)
#define UI_ALIGN_RIGHT (0x04)

#define ui_window_begin(width, height, flags) (ui_window_begin_(uiid_gen(), width, height, flags))
void ui_window_begin_(UIID id, real32 w, real32 h, int8 flags=0) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = ui_initElement(id, UI_ELEMENT_WINDOW);
  }

  e->touched = true;

  ensure(e->type == UI_ELEMENT_WINDOW);

  real32 ww = getWindowWidth();
  real32 wh = getWindowHeight();

  real32 x = 0;
  real32 y = 0;

  if (flags & UI_ALIGN_CENTER_X) {
    x = (ww - w)/2.f;
  }

  if (flags & UI_ALIGN_CENTER_Y) {
    y = (wh - h)/2.f;
  }

  e->rect = rect_init(x, y, w, h);

  ui_pane_push(e->rect);

  ui.window = e;
}

void ui_window_end() {
  ensure(ui.window != 0 && ui.window->type == UI_ELEMENT_WINDOW);

  ui.window = 0;

  ui_pane_pop();
}

#define ui_box(texture, selected, count) (ui_box_(uiid_gen(), texture, selected, count))
#define ui_boxi(i, texture, selected, count) (ui_box_(uiid_genEx(i), texture, selected, count))
void ui_box_(UIID id, uint32 texture, bool selected, int count = 0) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = ui_initElement(id, UI_ELEMENT_BOX);
  }
  e->touched = true;

  ensure(e->type == UI_ELEMENT_BOX);

  e->box.textureID = texture;
  e->box.itemCount = count;
  e->box.selected = selected;

  // ui_getPos
  real32 w = UI_ELEMENT_BOX_WIDTH;
  real32 h = UI_ELEMENT_BOX_HEIGHT;
  e->rect = rect_init(0, 0, w, h);

  ui_getPos(&e->rect);
}

#define ui_text(text) (ui_text_(uiid_gen(), text))
#define ui_texti(i, text) (ui_text_(uiid_genEx(i), text))
#define ui_textEx(text, scale) (ui_text_(uiid_gen(), text, scale))
#define ui_textExi(i, text, scale) (ui_text_(uiid_genEx(i), text, scale))
void ui_text_(UIID id, const char* text, real32 scale = 1.0f) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = ui_initElement(id, UI_ELEMENT_TEXT);
  }
  e->touched = true;

  ensure(e->type == UI_ELEMENT_TEXT);

  MemoryIndex s = strlen(text);

  ensure(s < 256);

  strncpy(e->text.text, text, s);

  real32 w = font_getStringWidth(ui.font, e->text.text, scale);
  real32 h = font_getStringHeight(ui.font, scale);
  e->rect = rect_init(0, 0, w, h);

  ui_getPos(&e->rect);
}

void ui_toolbarBegin(UIID id) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->id = id;
    e->type = UI_ELEMENT_TOOLBAR;

    ui.elementCount += 1;
    ensure(ui.elementCount < UI_MAX_ELEMENTS);
  }
  e->touched = true;

  ensure(e->type == UI_ELEMENT_TOOLBAR);

  e->toolbar.offset = 0;
  e->rect = rect_init(getWindowWidth()/2, getWindowHeight()-UI_ELEMENT_TOOLBAR_OPTION_SIZE, 0, UI_ELEMENT_TOOLBAR_OPTION_SIZE);

  ui.parent = e;
}

void ui_toolbarEnd() {
  ensure(ui.parent != 0 && ui.parent->type == UI_ELEMENT_TOOLBAR);

  ui.parent->rect.x -= ui.parent->toolbar.offset/2;
  ui.parent->rect.w = ui.parent->toolbar.offset;

  ui.parent = 0;
}

void ui_toolbarOption(UIID id, bool selected, int count, uint32 tex=MAX_TEXTURE) {
  ensure(ui.parent != 0 && ui.parent->type == UI_ELEMENT_TOOLBAR);

  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->id = id;
    e->type = UI_ELEMENT_TOOLBAR_OPTION;

    ui.elementCount += 1;
    ensure(ui.elementCount < UI_MAX_ELEMENTS);
  }
  e->touched = true;

  ensure(e->type == UI_ELEMENT_TOOLBAR_OPTION);

  if (tex == MAX_TEXTURE) {
    e->toolbarOption.hasTexture = false;
  } else {
    e->toolbarOption.hasTexture = true;
    e->toolbarOption.textureID = tex;
  }

  e->toolbarOption.count = count;

  e->toolbarOption.selected = selected;

  e->toolbarOption.parent = ui.parent->id;

  UIElement* parent = ui_getElement(e->toolbarOption.parent);
  ensure(parent->type == UI_ELEMENT_TOOLBAR);

  e->rect = rect_init(parent->toolbar.offset, 0, UI_ELEMENT_TOOLBAR_OPTION_SIZE, UI_ELEMENT_TOOLBAR_OPTION_SIZE);

  parent->toolbar.offset += e->rect.w + UI_ELEMENT_TOOLBAR_PADDING;
}

void ui_draw() {
  for (uint32 i = 0; i < ui.elementCount; i++) {
    UIElement elem = ui.elements[i];

    if (!elem.touched) {
      continue;
    }

    switch (elem.type) {
      case UI_ELEMENT_WINDOW:
        {
          Rect r = elem.rect;

          draw_rectangle(r, vec4(115, 115, 115, 0.8f));
        } break;
      case UI_ELEMENT_BOX:
        {
          Rect r = elem.rect;

          UIElement_Box box = elem.box;

          draw_rectangle(r, vec4(200, 200, 200, 1.f));

          if (box.textureID != MAX_TEXTURE) {
            draw_sprite(elem.rect, texture(box.textureID));
          }

          if (box.itemCount > 0) {
            float countWidth = 30;
            Rect r = elem.rect;
            r.x += r.w - countWidth;
            r.y += r.h - countWidth;
            r.w = countWidth;
            r.h = countWidth;

            draw_rectangle(r, vec4(0, 0, 0, 0.5f));

            vec2 textPos = vec2(r.x + r.w/2, r.y + r.h/2);

            char str[4] = {0};

            snprintf(str, 4, "%d", box.itemCount);

            draw_text(str, textPos, 1.0f, ui.font, vec3_white, TEXT_ALIGN_CENTER);
          }
        } break;
      case UI_ELEMENT_TEXT:
        {
          UIElement_Text text = elem.text;

          draw_text(text.text, vec2(elem.rect.x, elem.rect.y), 1.0f, ui.font, vec3_white, TEXT_ALIGN_LEFT);
        } break;
      case UI_ELEMENT_TOOLBAR:
        {
          Rect r = elem.rect;
          r.x -= UI_ELEMENT_TOOLBAR_PADDING;
          r.y -= UI_ELEMENT_TOOLBAR_PADDING;
          r.w += UI_ELEMENT_TOOLBAR_PADDING*2;
          r.h += UI_ELEMENT_TOOLBAR_PADDING*2;

          draw_rectangle(r, vec4(115, 115, 115, 1.0f));
        } break;
      case UI_ELEMENT_TOOLBAR_OPTION:
        {
          UIElement_ToolbarOption toolbarOption = elem.toolbarOption;

          UIElement* p = ui_getElement(toolbarOption.parent);

          vec4 color = vec4(10, 10, 60, 1.0f);

          if (toolbarOption.selected) {
            color = vec4(30, 30, 100, 1.0f);
          }

          elem.rect.x += p->rect.x;
          elem.rect.y += p->rect.y;

          draw_rectangle(elem.rect, color);

          if (toolbarOption.hasTexture) {
            draw_sprite(elem.rect, texture(toolbarOption.textureID));
          }

          if (toolbarOption.count > 0) {
            float countWidth = 30;
            Rect r = elem.rect;
            r.x += UI_ELEMENT_TOOLBAR_OPTION_SIZE - countWidth;
            r.y += UI_ELEMENT_TOOLBAR_OPTION_SIZE - countWidth;
            r.w = countWidth;
            r.h = countWidth;

            draw_rectangle(r, vec4(0, 0, 0, 0.5f));

            vec2 textPos = vec2(r.x + r.w/2, r.y + r.h/2);

            char str[4] = {0};

            snprintf(str, 4, "%d", toolbarOption.count);

            draw_text(str, textPos, 1.0f, ui.font, vec3_white, TEXT_ALIGN_CENTER);
          }
        } break;
      default:
        {
          ensure(false);
        } break;
    }
  }
}
