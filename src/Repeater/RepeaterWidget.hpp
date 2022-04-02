struct RepeaterWidget : ModuleWidget
{
	RepeaterWidget(Repeater* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/repeater/repeater_front_panel.svg")));

    // Load up the background PNG and add it to the panel
    PNGPanel *png_panel = new PNGPanel("res/repeater/repeater_all.png", 81.28, 128.5);
    addChild(png_panel);

    // Medium Knobs
		addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.563736,122.269234), module, Repeater::CLOCK_DIVISION_KNOB));
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.375656,191.931137), module, Repeater::POSITION_KNOB));
		addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.425659,261.852417), module, Repeater::SAMPLE_SELECT_KNOB));
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.402100,332.388794), module, Repeater::PITCH_KNOB));

    float trigger_input_x = 67.7;
		float trigger_input_y = 42.0;
    float clock_division_x = 7.0;
		float clock_division_y = 42.0;
    float position_x = 7.0;
		float position_y = 64.0;
    float sample_select_x = 7.0;
    float sample_select_y = 86.0;
    float pitch_x = 7.0;
    float pitch_y = 108.0;

    // CV Inputs
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,121.665604), module, Repeater::CLOCK_DIVISION_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,191.426514), module, Repeater::POSITION_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,261.237152), module, Repeater::SAMPLE_SELECT_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,331.698181), module, Repeater::PITCH_INPUT));

    // Attenuators
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,121.868500), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,191.479111), module, Repeater::POSITION_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,261.189728), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,331.700439), module, Repeater::PITCH_ATTN_KNOB));

    // Clock input
    addInput(createInputCentered<VoxglitchInputPort>(Vec(196.701385,121.515594), module, Repeater::TRIG_INPUT));

    // Smooth switch
    addParam(createParamCentered<squareToggle>(Vec(197.249832,193.950241), module, Repeater::SMOOTH_SWITCH));

    // Outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(204.649963,331.449402), module, Repeater::WAV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(204.699982,281.450653), module, Repeater::TRG_OUTPUT));

    /*
		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		float trigger_input_x = 67.7;
		float trigger_input_y = 42.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(trigger_input_x, trigger_input_y)), module, Repeater::TRIG_INPUT));

		// Input, Knob, and Attenuator for the clock division
		float clock_division_x = 7.0;
		float clock_division_y = 42.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(clock_division_x, clock_division_y)), module, Repeater::CLOCK_DIVISION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(clock_division_x + 16, clock_division_y + 1)), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(clock_division_x + 30, clock_division_y - 3)), module, Repeater::CLOCK_DIVISION_KNOB));

		// Input, Knob, and Attenuator for the position input
		float position_x = 7.0;
		float position_y = 64.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(position_x, position_y)), module, Repeater::POSITION_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(position_x + 16, position_y + 1)), module, Repeater::POSITION_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(position_x + 30, position_y - 3)), module, Repeater::POSITION_KNOB));

		// Input, Knob, and Attenuator for the Sample Select
		float sample_select_x = 7.0;
		float sample_select_y = 86.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(sample_select_x, sample_select_y)), module, Repeater::SAMPLE_SELECT_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(sample_select_x + 16, sample_select_y + 1)), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(sample_select_x + 30, sample_select_y - 3)), module, Repeater::SAMPLE_SELECT_KNOB));

		// Input, Knob, and Attenuator for Pitch
		float pitch_x = 7.0;
		float pitch_y = 108.0;
		addInput(createInput<PJ301MPort>(mm2px(Vec(pitch_x, pitch_y)), module, Repeater::PITCH_INPUT));
		addParam(createParam<Trimpot>(mm2px(Vec(pitch_x + 16, pitch_y + 1)), module, Repeater::PITCH_ATTN_KNOB));
		addParam(createParam<RoundLargeBlackKnob>(mm2px(Vec(pitch_x + 30, pitch_y - 3)), module, Repeater::PITCH_KNOB));

		// Smooth
		addParam(createParam<CKSS>(Vec(205, 190), module, Repeater::SMOOTH_SWITCH));

		Readout *readout = new Readout();
		readout->box.pos = mm2px(Vec(11.0, 22.0)); //22,22
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// Outputs
		addOutput(createOutput<PJ301MPort>(Vec(200, 324), module, Repeater::WAV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(200, 259), module, Repeater::TRG_OUTPUT));
    */
	}

	void appendContextMenu(Menu *menu) override
	{
		Repeater *module = dynamic_cast<Repeater*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

		//
		// Options
		// =====================================================================

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Options"));

		// Retrigger option

		struct RetriggerMenuItem : MenuItem {
			Repeater* module;
			void onAction(const event::Action& e) override {
				module->retrigger = !(module->retrigger);
			}
		};

		RetriggerMenuItem* retrigger_menu_item = createMenuItem<RetriggerMenuItem>("Retrigger");
		retrigger_menu_item->rightText = CHECKMARK(module->retrigger == 1);
		retrigger_menu_item->module = module;
		menu->addChild(retrigger_menu_item);
	}

};
