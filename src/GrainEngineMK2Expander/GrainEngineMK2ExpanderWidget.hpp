struct GrainEngineMK2ExpanderWidget : VoxglitchModuleWidget
{
  GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_mk2_expander_front_panel.svg")));

    PNGPanel *png_panel = new PNGPanel("res/grain_engine_mk2_expander/grain_engine_expander_baseplate.png", 30.48, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_expander_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    addInput(createInputCentered<VoxglitchInputPort>(Vec(40.417305,59.081532), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(50.382683,96.631500), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.117329,178.481842), module, GrainEngineMK2Expander::RECORD_START_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(24.996094,207.496094), module, GrainEngineMK2Expander::RECORD_START_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.267303,178.482361), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(65.250000,207.750000), module, GrainEngineMK2Expander::RECORD_STOP_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.067327,260.582733), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(65.332687,261.082764), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(24.917328,342.437317), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.082687,342.437317), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander*>(this->module);
    assert(module);
  }
};
