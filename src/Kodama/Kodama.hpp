//
// Kodama - Performance sampler with sliders

struct Kodama : VoxglitchModule
{
    bool dirty = false;
    std::string text;
    bool grid_data[COLS][ROWS];

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Kodama()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (unsigned int row = 0; row < ROWS; row++)
        {
            for (unsigned int col = 0; col < COLS; col++)
            {
                grid_data[col][row] = false;
            }
        }
    }

    //
    // Autosave module data.  VCV Rack decides when this should be called.
    //
    // My saving and loading code could be more compact by saving arrays of
    // "ball" data tidily packaged up.  I'll do that if this code ever goes
    // through another iteration.
    //

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *json_root) override
    {
    }

    void process(const ProcessArgs &args) override
    {
    }
};