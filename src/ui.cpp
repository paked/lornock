struct UIID {
  uint32 line;
};

#define uiid_gen() uiid_init(__LINE__)

UIID uiid_init(uint32 line) {
  UIID id = {0};
  id.line = line;

  return id;
}

bool operator==(UIID l, UIID r) {
  return l.line == r.line;
}

enum {
  UI_ELEMENT_TOOLBAR,
  UI_ELEMENT_TOOLBAR_OPTION
};

struct UIElement {
  UIID id;
  uint32 type;

  Rect rect;

  union {
    struct { // Toolbar
      real32 offset;
    };

    struct { // Toolbar option
      UIID parent;
      bool hasTexture;
      uint32 textureID;

      bool selected;
    };
  };
};

#define UI_ALIGN_LEFT 0x00
#define UI_ALIGN_RIGHT 0x01
#define UI_ALIGN_BOTTOM 0x02
#define UI_ALIGN_TOP 0x04

#define UI_ELEMENT_TOOLBAR_OPTION_SIZE 75
#define UI_ELEMENT_TOOLBAR_PADDING 2

#define UI_MAX_ELEMENTS 128

struct {
  UIElement elements[UI_MAX_ELEMENTS];
  uint32 elementCount;

  UIElement *parent;

  real32 nextX;
  real32 nextY;
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

void ui_begin() {
  ui.nextX = 0;
  ui.nextY = 0;

  ui.parent = 0;
}

void ui_end() {}

void ui_toolbarBegin(UIID id) {
  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->id = id;
    e->type = UI_ELEMENT_TOOLBAR;

    ui.elementCount += 1;
    assert(ui.elementCount < UI_MAX_ELEMENTS);
  }

  e->offset = 0;
  e->rect = rect_init(getWindowWidth()/2, getWindowHeight()-UI_ELEMENT_TOOLBAR_OPTION_SIZE, 0, UI_ELEMENT_TOOLBAR_OPTION_SIZE);

  ui.parent = e;
}

void ui_toolbarEnd() {
  assert(ui.parent != 0 && ui.parent->type == UI_ELEMENT_TOOLBAR);

  ui.parent->rect.x -= ui.parent->offset/2;
  ui.parent->rect.w = ui.parent->offset;

  ui.parent = 0;
}

void ui_toolbarOption(UIID id, bool selected, int32 tex=-1) {
  assert(ui.parent != 0 && ui.parent->type == UI_ELEMENT_TOOLBAR);

  UIElement* e = ui_getElement(id);
  if (e == 0) {
    e = &ui.elements[ui.elementCount];
    e->id = id;
    e->type = UI_ELEMENT_TOOLBAR_OPTION;

    ui.elementCount += 1;
    assert(ui.elementCount < UI_MAX_ELEMENTS);
  }

  if (tex == -1) {
    e->hasTexture = false;
  } else {
    e->hasTexture = true;
    e->textureID = tex;
  }

  e->selected = selected;

  e->parent = ui.parent->id;

  UIElement* parent = ui_getElement(e->parent);

  e->rect = rect_init(parent->offset, 0, UI_ELEMENT_TOOLBAR_OPTION_SIZE, UI_ELEMENT_TOOLBAR_OPTION_SIZE);

  parent->offset += e->rect.w + UI_ELEMENT_TOOLBAR_PADDING;
}

void ui_draw() {
  for (uint32 i = 0; i < ui.elementCount; i++) {
    UIElement elem = ui.elements[i];

    switch (elem.type) {
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
          UIElement* p = ui_getElement(elem.parent);

          vec4 color = vec4(10, 10, 60, 1.0f);

          elem.rect.x += p->rect.x;
          elem.rect.y += p->rect.y;

          draw_rectangle(elem.rect, color);

          if (elem.hasTexture) {
            draw_sprite(elem.rect, texture(elem.textureID));
          }
        } break;
      default:
        {
          assert(false);
        } break;
    }
  }
}