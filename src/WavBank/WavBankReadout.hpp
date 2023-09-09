struct WavBankReadout : TransparentWidget
{
	WavBank *module;
	float text_rotation_angle = -M_PI / 2.0f;
	std::shared_ptr<Font> font;
	std::string text_to_display;

	WavBankReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		if(module->samples.size() > module->selected_sample_slot)
		{
			text_to_display = module->samples[module->selected_sample_slot].filename;
			text_to_display.resize(30); // truncate long text
		}
		else
			text_to_display = "No folder selected";

		// Set font information
		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgRotate(args.vg, text_rotation_angle);

		// Print out the text
		nvgText(args.vg, 5, 5, text_to_display.c_str(), NULL);
	}
};
