void Editbox::SetSelection(int begin, int end) {
  SendMessage(hwnd, EM_SETSEL, begin, end);
  SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
}

bool Editbox::Create(Window *parent_window, const char *style, int x, int y, int width, int height, const char *text) {
  if(!parent_window)return false;

  parent = parent_window;
  id     = CONTROLID_INDEX + parent->control_count;
  type   = EDITBOX;
  state.ws     = WS_CHILD;
  state.es     = 0;
  state.x      = x;
  state.y      = y;
  state.width  = width;
  state.height = height;

stringarray part;
  ParseStyleParam(style, part);
  for(int i = 0; i < count(part); i++) {
    if(!strcmp(part[i], "visible"))state.ws |= WS_VISIBLE;
    if(!strcmp(part[i], "disabled"))state.ws |= WS_DISABLED;
    if(!strcmp(part[i], "border"))state.ws |= WS_BORDER;
    if(!strcmp(part[i], "raised"))state.ws |= WS_DLGFRAME;
    if(!strcmp(part[i], "vscroll"))state.ws |= WS_VSCROLL;
    if(!strcmp(part[i], "hscroll"))state.ws |= WS_HSCROLL;

    if(!strcmp(part[i], "multiline"))state.ws |= ES_MULTILINE;
    if(!strcmp(part[i], "readonly"))state.ws |= ES_READONLY;

    if(!strcmp(part[i], "sunken"))state.es |= WS_EX_STATICEDGE;
    if(!strcmp(part[i], "edge"))state.es |= WS_EX_CLIENTEDGE;
  }

  hwnd = CreateWindowEx(state.es, "EDIT", text, state.ws,
    state.x, state.y, state.width, state.height,
    parent->hwnd, (HMENU)id, GetModuleHandle(0), 0);
  if(!hwnd)return false;

  PostCreate();
  return true;
}
