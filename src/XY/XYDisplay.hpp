struct XYDisplay : OpaqueWidget
{
  XY *module;
  bool dragging = false;
  NVGcolor rectangle_colors[30];

  XYDisplay(XY *module): OpaqueWidget()
  {
    this->module = module;
    box.size = Vec(DRAW_AREA_WIDTH_PT, DRAW_AREA_HEIGHT_PT);
  }

  void draw(const DrawArgs &args) override
  {
    OpaqueWidget::draw(args);
    const auto vg = args.vg;

    // if(module)
    {
      float now_x =this->module->drag_position.x;
      float now_y = this->module->drag_position.y - DRAW_AREA_HEIGHT_PT;
      float drag_y = this->module->drag_position.y;

      nvgSave(vg);

      // draw x,y lines, just because I think they look cool

      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 0.5f);
      nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
      nvgMoveTo(vg, now_x, 0);
      nvgLineTo(vg, now_x, DRAW_AREA_HEIGHT_PT);
      nvgStroke(vg);

      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 0.5f);
      nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
      nvgMoveTo(vg, 0, drag_y);
      nvgLineTo(vg, DRAW_AREA_WIDTH_PT, drag_y);
      nvgStroke(vg);

      nvgBeginPath(vg);
      nvgRect(vg, 0, DRAW_AREA_WIDTH_PT, now_x, now_y);
      nvgFillColor(vg, nvgRGB(156, 167, 185));
      nvgFill(vg);

      // FOR TESTING: Draw test rectable to see bounds of box
      /*
      nvgBeginPath(vg);
      nvgStrokeColor(vg, nvgRGB(100, 120, 255));
      nvgStrokeWidth(vg, 0.2f);
      nvgFillColor(vg, nvgRGBA(0,0,0,0));
      nvgRect(vg, 0, 0, DRAW_AREA_WIDTH_PT, DRAW_AREA_HEIGHT_PT);
      nvgStroke(vg);
      */

      nvgRestore(vg);
    }
    // else
    // {
    //   nvgSave(vg);

    //   // draw preview for module browser

    //   // vertical line
    //   nvgBeginPath(vg);
    //   nvgStrokeWidth(vg, 0.5f);
    //   nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
    //   nvgMoveTo(vg, 120, 0);
    //   nvgLineTo(vg, 120, DRAW_AREA_HEIGHT_PT);
    //   nvgStroke(vg);

    //   // horizontal line
    //   nvgBeginPath(vg);
    //   nvgStrokeWidth(vg, 0.5f);
    //   nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
    //   nvgMoveTo(vg, 0, 150);
    //   nvgLineTo(vg, DRAW_AREA_WIDTH_PT, 150);
    //   nvgStroke(vg);

    //   // filled rectangle
    //   nvgBeginPath(vg);
    //   nvgRect(vg, 0, DRAW_AREA_WIDTH_PT, 120, -130);
    //   nvgFillColor(vg, rectangle_colors[29]);
    //   nvgFill(vg);

    //   nvgRestore(vg);
    // }
  }

  Vec clampToDrawArea(Vec location)
  {
    float x = clamp(location.x, 0.0f, DRAW_AREA_WIDTH_PT);
    float y = clamp(location.y, 0.0f, DRAW_AREA_HEIGHT_PT);
    return(Vec(x,y));
  }

  void onButton(const event::Button &e) override
  {
    if(e.button != GLFW_MOUSE_BUTTON_LEFT)
      return;
    if(!api0::windowIsShiftPressed())
      return;
    e.consume(this);
    if(e.action == GLFW_PRESS)
      this->module->drag_position = this->clampToDrawArea(e.pos);

    if(this->module->get_punch_switch_value() == 0) // Punch recording mode NOT enabled
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        this->module->start_recording();
      }

      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE && dragging)
      {
        this->module->start_playback();
      }
    }
    else // Punch recording mode enabled
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        this->module->start_punch_recording();
      }

      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE && dragging)
      {
        this->module->continue_playback();
      }
    }
  }

  void onDragStart(const event::DragStart &e) override
  {
    OpaqueWidget::onDragStart(e);
    dragging = true;
  }

  void onDragEnd(const event::DragEnd &e) override
  {
    OpaqueWidget::onDragEnd(e);
    dragging = false;
  }

  void onDragMove(const event::DragMove &e) override
  {
    OpaqueWidget::onDragMove(e);
    float zoom = std::pow(2.f, settings::zoom);
    this->module->drag_position = this->clampToDrawArea(this->module->drag_position.plus(e.mouseDelta.div(zoom)));
  }

  void step() override {
    OpaqueWidget::step();
  }
};
