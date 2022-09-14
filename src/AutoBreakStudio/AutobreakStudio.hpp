/*

Autobreak Studio

Automatic Breakbeat module for VCV Rack by Voxglitch,
with extra stuff.

*/

struct AutobreakStudio : VoxglitchSamplerModule
{
  unsigned int selected_sample_slot = 0;

  // Actual index into the sample's array for playback
  float actual_playback_position = 0;

  // A location in a theoretical sample that's 8 bars at the selected BPM.
  // This value is stepped and repositioned when jumping around in a breakbeat.
  // This value is then used to figure out the actual_playback_position based
  // on the sample's length.

  float theoretical_playback_position = 0;

  // Pattern lock is a flag which, when true, stops the pattern from changing
  // due to changes in the pattern knob or the CV input.  This flag is set
  // to true when the user is actively editing the current pattern.

  double time_counter = 0.0;
  double bpm = 0.0;
  double timer_before = 0;
  bool clock_triggered = false;
  bool ratchet_triggered = false;
  unsigned int ratchet_counter = 0;

  // StereoSmoothSubModule loop_smooth;
  DeclickFilter declick_filter;
  StereoPan stereo_pan;

  std::string root_dir;
  std::string path;

  Sample samples[NUMBER_OF_SAMPLES];
  std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger clockTrigger;
  // dsp::SchmittTrigger ratchetTrigger;

  VoltageSequencer position_sequencer;
  VoltageSequencer *selected_position_sequencer = &position_sequencer;

  VoltageSequencer sample_sequencer;
  VoltageSequencer *selected_sample_sequencer = &sample_sequencer;

  VoltageSequencer volume_sequencer;
  VoltageSequencer *selected_volume_sequencer = &volume_sequencer;

  VoltageSequencer pan_sequencer;
  VoltageSequencer *selected_pan_sequencer = &pan_sequencer;

  VoltageSequencer reverse_sequencer;
  VoltageSequencer *selected_reverse_sequencer = &reverse_sequencer;

  VoltageSequencer ratchet_sequencer;
  VoltageSequencer *selected_ratchet_sequencer = &ratchet_sequencer;

  float left_output = 0;
  float right_output = 0;

  enum ParamIds
  {
    WAV_KNOB,
    WAV_ATTN_KNOB,
    NUM_PARAMS
  };
  enum InputIds
  {
    CLOCK_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds
  {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    DEBUG_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds
  {
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  AutobreakStudio()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
    configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");

    configInput(CLOCK_INPUT, "Clock Input");

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    // There are some hacks here to modify the snap divisions
    // that come standard with the voltage sequencer.  I should
    // rethink how the voltage sequencer is assigned the snap
    // division in the future, possibly moving the snap values
    // array into the module. 

    // Position sequencer
    position_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    position_sequencer.setSnapDivisionIndex(4);

    // Volume sequencer
    volume_sequencer.assign(NUMBER_OF_STEPS, 1.0);
    
    // Sample selection sequencer
    sample_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    sample_sequencer.snap_divisions[1] = NUMBER_OF_SAMPLES - 1;
    sample_sequencer.setSnapDivisionIndex(1);
    
    // Pan sequencer
    pan_sequencer.assign(NUMBER_OF_STEPS, 0.5);

    // Reverse sequencer
    reverse_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    
    ratchet_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    ratchet_sequencer.snap_divisions[1] = 5;
    ratchet_sequencer.setSnapDivisionIndex(1);    
  }

  // Autosave settings
  json_t *dataToJson() override
  {
    //
    // Save selected samples
    //

    json_t *json_root = json_object();
    for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
    {
      json_object_set_new(json_root, ("loaded_sample_path_" + std::to_string(i + 1)).c_str(), json_string(samples[i].path.c_str()));
    }

    return json_root;
  }

  // Autoload settings
  void dataFromJson(json_t *json_root) override
  {
    //
    // Load samples
    //

    for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
    {
      json_t *loaded_sample_path = json_object_get(json_root, ("loaded_sample_path_" + std::to_string(i + 1)).c_str());
      if (loaded_sample_path)
      {
        samples[i].load(json_string_value(loaded_sample_path));
        loaded_filenames[i] = samples[i].filename;
      }
    }
  }

  float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float attenuator_value = params[attenuator_index].getValue();

    return (((input_value * scale) * attenuator_value) + (knob_value * scale));
  }

  void process(const ProcessArgs &args) override
  {
    //
    // Handle wav selection
    //


    unsigned int sample_value = (sample_sequencer.getValue() * NUMBER_OF_SAMPLES);

    if (sample_value != selected_sample_slot)
    {
      // Reset the smooth ramp if the selected sample has changed
      declick_filter.trigger();

      // Set the selected sample
      selected_sample_slot = sample_value;
    }
    Sample *selected_sample = &samples[selected_sample_slot];

    //
    // Handle BPM detection
    //

    time_counter += args.sampleTime;

    if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
    {
      if (timer_before != 0)
      {
        // Compute BPM based on incoming clock
        double elapsed_time = time_counter - timer_before;
        if (elapsed_time > 0)
          bpm = 30.0 / elapsed_time;
      }

      timer_before = time_counter;
      clock_triggered = true;
    }

    // If BPM hasn't been determined yet, wait until it has to start 
    // running the engine.
    if(bpm == 0.0) return;

    //
    // Handle ratcheting
    //

    // Ratchet will range from 0 to 1.0
    float ratchet = selected_ratchet_sequencer->getValue();

    if (ratchet > 0)
    {
      unsigned int samples_in_a_beat = ((60.0 / bpm) * args.sampleRate);

      // Ratchet divisions is an array defined in defines.h.  It contains 5 different
      // ratchet divisors for controlling the ratchet timing.  The larger the number,
      // the faster the ratchet.

      float ratchet_division = ratchet_divisions[int(ratchet * 4.0)];

      if (ratchet_counter >= (samples_in_a_beat / ratchet_division)) // double ratchet
      {
        ratchet_triggered = true;
        ratchet_counter = 0;
      }
      else
      {
        ratchet_counter++;
      }
    }
    else
    {
      ratchet_counter = 0;
    }

    //
    // Handle reset input
    //

    if (inputs[RESET_INPUT].isConnected())
    {
      if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
      {
        // Reset counters
        actual_playback_position = 0;
        theoretical_playback_position = 0;

        // Smooth back into playback
        declick_filter.trigger();
      }
    }

    // 60.0 is for conversion from minutes to seconds
    // 8.0 is for 8 beats (2 bars) of loops, which is a typical drum loop length
    float samples_to_play_per_loop = ((60.0 / bpm) * args.sampleRate) * 8.0;

    if (selected_sample->loaded && (selected_sample->size() > 0))
    {
      actual_playback_position = clamp(actual_playback_position, 0.0, selected_sample->size() - 1);

      selected_sample->read((int)actual_playback_position, &left_output, &right_output);

      // Apply volume sequencer to output values
      left_output = selected_volume_sequencer->getValue() * left_output;
      right_output = selected_volume_sequencer->getValue() * right_output;

      // Apply pan sequencer to output values
      stereo_pan.process(&left_output, &right_output, ((selected_pan_sequencer->getValue() * 2.0) - 1.0));

      // Handle smoothing
      declick_filter.process(&left_output, &right_output);

      // Output audio
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output * GAIN);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output * GAIN);
    }

    // Step the theoretical playback position
    if (selected_reverse_sequencer->getValue() >= 0.5)
    {
      theoretical_playback_position = theoretical_playback_position - 1;
    }
    else
    {
      theoretical_playback_position = theoretical_playback_position + 1;
    }

    // Optionally jump to new breakbeat position
    if (clock_triggered)
    {
      // float sequence_value = inputs[SEQUENCE_INPUT].getVoltage() / 10.0;
      // TODO: loop through and step all squencers once memory is implemented
      position_sequencer.step();
      sample_sequencer.step();
      volume_sequencer.step();
      pan_sequencer.step();
      ratchet_sequencer.step();
      reverse_sequencer.step();

      float sequence_value = position_sequencer.getValue();
      int breakbeat_location = (sequence_value * 16) - 1;
      breakbeat_location = clamp(breakbeat_location, -1, 15);

      if (breakbeat_location != -1)
      {
        theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
      }

      clock_triggered = false;
      ratchet_counter = 0;
    }
    else
    {
      if (ratchet_triggered)
      {
        float sequence_value = position_sequencer.getValue();
        int breakbeat_location = (sequence_value * 16) - 1;
        breakbeat_location = clamp(breakbeat_location, -1, 15);

        if (breakbeat_location != -1)
        {
          theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
        }
        ratchet_triggered = false;
      }
    }

    // Loop the theoretical_playback_position
    if (theoretical_playback_position >= samples_to_play_per_loop)
    {
      theoretical_playback_position = 0;
      declick_filter.trigger();
    }
    else if (theoretical_playback_position < 0)
    {
      theoretical_playback_position = samples_to_play_per_loop;
      declick_filter.trigger();
    }

    // Map the theoretical playback position to the actual sample playback position
    actual_playback_position = ((float)theoretical_playback_position / samples_to_play_per_loop) * selected_sample->size();
  }
};
