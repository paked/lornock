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
  UI_ELEMENT_TOOLBAR_OPTION
};

struct UIElement_Window {
  vec2 next;
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

void ui_end() {}

UIElement* ui_initElement(UIID id, uint32 type) {
  UIElement* e = &ui.elements[ui.elementCount];
  e->type = type;
  e->id = id;

  ui.elementCount += 1;
  ensure(ui.elementCount < UI_MAX_ELEMENTS);

  return e;
}

void ui_window_begin(UIID id, real32 w, real32 h, int8 flags=0) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = ui_initElement(id, UI_ELEMENT_WINDOW);
  }

  ensure(e->type == UI_ELEMENT_WINDOW);

  e->rect = rect_init(0, 0, w, h);

  ui_pane_push(e->rect);

  ui.window = e;
}

void ui_box_(UIID id, uint32 texture, bool selected, int count = 0) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = ui_initElement(id, UI_ELEMENT_BOX);
  }

  ensure(e->type == UI_ELEMENT_BOX);

  e->box.textureID = texture;
  e->box.itemCount = count;
  e->box.selected = selected;

  // ui_getPos
  
  {
    real32 w = UI_ELEMENT_BOX_WIDTH;
    real32 h = UI_ELEMENT_BOX_HEIGHT;

    Rect currentPane = ui.paneStack[ui.paneStackCount - 1];

    if (ui.next.x + w > currentPane.x + currentPane.w) {
      logln("fuck");

      ui.next.x = 0;
      ui.next.y += h;
    }

    vec2 wpos = {0};

    for (int i = 0; i < ui.paneStackCount; i++) {
      wpos.x += ui.paneStack[i].x;
      wpos.y += ui.paneStack[i].y;
    }

    wpos += ui.next;

    e->rect = rect_init(wpos.x, wpos.y, w, h);
  }
}

void ui_window_end() {
  ensure(ui.window != 0 && ui.window->type == UI_ELEMENT_WINDOW);
  
  ui.window = 0;

  ui_pane_pop();
}

void ui_windowBegin(UIID id, real32 w, real32 h) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->type = UI_ELEMENT_WINDOW;
    e->id = id;

    ui.elementCount += 1;

    ensure(ui.elementCount < UI_MAX_ELEMENTS);
  }

  ensure(e->type == UI_ELEMENT_WINDOW);

  real32 width = getWindowWidth() * w;
  real32 height = getWindowHeight() * h;

  e->rect = rect_init(
      (getWindowWidth() - width)/2,
      (getWindowHeight() - height)/2,
      width,
      height);

  e->window.next = vec2(e->rect.x, e->rect.y);

  ui.window = e;
}

void ui_windowEnd() {
  ui.window = 0;
}

void ui_box(UIID id, uint32 texture, bool selected, int count = 0) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->id = id;
    e->type = UI_ELEMENT_BOX;

    ui.elementCount += 1;
    ensure(ui.elementCount < UI_MAX_ELEMENTS);
  }

  ensure(e->type == UI_ELEMENT_BOX);

  // NOTE(harrison): SUPER HACKY AND MAYBE UNCLEAR POSITIONING CODE.
  real32 extraSpace = 0.0f;
  real32 elemCount = 0.0f;

  {
    extraSpace = fmodf(ui.window->rect.w, UI_ELEMENT_BOX_WIDTH);
    elemCount = (ui.window->rect.w - extraSpace) / UI_ELEMENT_BOX_WIDTH;
  }

  real32 paddingx = extraSpace/elemCount;

  {
    extraSpace = fmodf(ui.window->rect.h, UI_ELEMENT_BOX_HEIGHT);
    elemCount = (ui.window->rect.h - extraSpace) / UI_ELEMENT_BOX_HEIGHT;
  }

  real32 paddingy = extraSpace/elemCount;

  Rect r = rect_init(0, 0, UI_ELEMENT_BOX_WIDTH, UI_ELEMENT_BOX_HEIGHT);

  if (ui.window->window.next.x + r.w + paddingx > ui.window->rect.x + ui.window->rect.w) {
    ui.window->window.next.x = ui.window->rect.x;
    ui.window->window.next.y += UI_ELEMENT_BOX_HEIGHT + paddingy/2; 
  }

  r.x = paddingx/2 + ui.window->window.next.x;
  r.y = ui.window->window.next.y + paddingy/2;

  e->rect = r;

  ui.window->window.next.x = r.x + r.w + paddingx/2;

  e->box.textureID = texture;
  e->box.itemCount = count;
  e->box.selected = selected;
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
