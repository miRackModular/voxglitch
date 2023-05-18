//
// ModuleManager.hpp
//
// TODO: 
// - Send through sample rate
// - Figure out how to represent PI
// - Update findOutModule to use the Type instead of the number of outputs

#pragma once

#include "Sport.hpp"
#include "IModule.h"
#include "VoxbuilderLogger.hpp"

// Utility modules
#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/ParamModule.hpp"

// arpeggiator
// AD
// quantizer
// sequencer
// slew limiter
// Advanced distortion
/*
Adding more inputs can make the DistortionModule more versatile and fun. Here are a few ideas:

    Tone control: Similar to the OverdriveModule, adding a tone control input allows users to adjust the balance between low and high frequencies in the output signal. This can be done by implementing a low-pass filter (LPF) and high-pass filter (HPF).

    Wet/Dry mix: Add a wet/dry mix input to control the balance between the original audio signal (dry) and the distorted audio signal (wet). This allows users to blend the distortion effect with the original signal, creating a more subtle effect or more aggressive distortion.

    Asymmetry: Introduce an asymmetry input to control the balance between positive and negative distortion. Asymmetrical distortion can create more complex harmonics and a different character in the sound.

    Envelope follower: Add an envelope follower input that controls the drive parameter based on the amplitude of the input audio signal. This can create a dynamic distortion effect that responds to the input signal's volume, making it more expressive.

    Multi-stage distortion: Implement a multi-stage distortion by adding a "Stages" input that selects the number of times the distortion is applied to the input signal. This can create a more aggressive and complex distortion effect.

Remember that adding more inputs can increase the complexity of the module, so it's essential to maintain a balance between versatility and ease of use.
*/

// Synth modules
#include "submodules/ADSRModule.hpp"
#include "submodules/ADModule.hpp"
#include "submodules/AdditionModule.hpp"
#include "submodules/AmplifierModule.hpp"
#include "submodules/ClockDividerModule.hpp"
#include "submodules/ClockModule.hpp"
#include "submodules/DelayModule.hpp"
#include "submodules/DelayLineModule.hpp"
#include "submodules/DistortionModule.hpp"
#include "submodules/DivisionModule.hpp"
#include "submodules/ExponentialVCAModule.hpp"
#include "submodules/FuzzModule.hpp"
#include "submodules/LinearVCAModule.hpp"
#include "submodules/LFOModule.hpp"
#include "submodules/LowpassFilterModule.hpp"
#include "submodules/Mixer2Module.hpp"
#include "submodules/Mixer3Module.hpp"
#include "submodules/Mixer4Module.hpp"
#include "submodules/Mixer8Module.hpp"
#include "submodules/MoogLowpassFilterModule.hpp"
#include "submodules/MorphingFilterModule.hpp"
#include "submodules/MultiplicationModule.hpp"
#include "submodules/NoiseModule.hpp"
#include "submodules/OverdriveModule.hpp"
#include "submodules/RampOscillatorModule.hpp"
#include "submodules/SampleAndHoldModule.hpp"
#include "submodules/ScaleQuantizerModule.hpp"
#include "submodules/SchroederReverbModule.hpp"
#include "submodules/Selector2Module.hpp"
#include "submodules/Selector3Module.hpp"
#include "submodules/Selector4Module.hpp"
#include "submodules/Selector6Module.hpp"
#include "submodules/Selector8Module.hpp"
#include "submodules/SubtractionModule.hpp"
#include "submodules/TableLookupModule.hpp"
#include "submodules/TB303OscillatorModule.hpp"
#include "submodules/TB303FilterModule.hpp"
#include "submodules/VCOModule.hpp"
#include "submodules/VoltageSequencerModule.hpp"
#include "submodules/WaveFolderModule.hpp"
#include "submodules/WaveShaperModule.hpp"
#include "submodules/WavetableOscillatorModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

class Connection
{
public:
    std::string source_module_uuid;
    unsigned int source_port_index;
    std::string destination_module_uuid;
    unsigned int destination_port_index;

    Connection(
        std::string src_mod_uuid, 
        unsigned int src_port_idx, 
        std::string dest_mod_uuid, 
        unsigned int dest_port_idx
    ) : 
    source_module_uuid(src_mod_uuid), 
    source_port_index(src_port_idx), 
    destination_module_uuid(dest_mod_uuid), 
    destination_port_index(dest_port_idx) 
    {}
};

class ModuleManager
{

private:
    std::map<std::string, IModule *> modules;

    // module_config_map is a map of module names to module configs
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

    std::vector<Connection> connections_config_forward;    

    IModule *terminal_output_module = nullptr;

    // Pointers
    float *pitch_ptr;
    float *gate_ptr;
    float *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;

public:

    bool ready = false;

    ModuleManager(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        this->pitch_ptr = pitch_ptr;
        this->gate_ptr = gate_ptr;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->p4 = p4;
        this->p5 = p5;
        this->p6 = p6;
        this->p7 = p7;
        this->p8 = p8;
    }

    /*

        █▀▀ █▀█ █▄░█ █▀ ▀█▀ █▀█ █░█ █▀▀ ▀█▀   █▀█ ▄▀█ ▀█▀ █▀▀ █░█
        █▄▄ █▄█ █░▀█ ▄█ ░█░ █▀▄ █▄█ █▄▄ ░█░   █▀▀ █▀█ ░█░ █▄▄ █▀█
        Text created using https://fsymbols.com/generators/carty/

    */
    bool createPatch()
    {
        VoxbuilderLogger::getInstance().log("CreatePatch Initiated");

        // It's assumed that the module_config_map and connections_config_forward have been set

        // instantiate all modules
        instantiateModules(pitch_ptr, gate_ptr, p1, p2, p3, p4, p5, p6, p7, p8);

        // connect all modules
        if(! connectModules()) return false;

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain and sets the member variable "terminal_output_module"
        terminal_output_module = findOutModule();

        // If there's no terminal output module, then the patch is invalid
        if (terminal_output_module == nullptr) 
        {
            VoxbuilderLogger::getInstance().log("No output module found.  The patch must have an output module.");
            return false;
        }

        // Set the ready flag to true
        ready = true;
        
        VoxbuilderLogger::getInstance().log("CreatePatch was successful.  Setting 'ready' to TRUE.");

        return ready;
    }

    bool isReady()
    {
        return ready;
    }

    void setReady(bool ready)
    {
        this->ready = ready;
    }

    void setModuleConfigMap(std::vector<ModuleConfig*>& moduleConfigs) 
    {
        for (ModuleConfig* config : moduleConfigs) 
        {
            module_config_map[config->uuid] = config;
        }
    }

    IModule *getTerminalOutputModule()
    {
        return terminal_output_module;
    }

    // It can be assumed that the ports on the left are outputs and the ports
    // on the right are inputs, but adding the word "outputs" and "inputs" will
    // make it more clear to people.

    void setConnections(std::vector<Connection> connections)
    {
        connections_config_forward = connections;
    }

    //
    // instantiateAllModules()
    //
    // This function creates instances of the module classes and adds them to the
    // "modules" map, where the map is in the format of: modules[name] = module
    //

    void instantiateModules(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_config_map)
        {
            ModuleConfig *config = module_config_data.second;

            std::string module_uuid = config->uuid;
            std::string type = config->type;

            json_t* data = config->data;
            json_t* defaults = config->defaults;

            IModule *module = nullptr;

            try
            {
                if (type == "AD") module = new ADModule();
                if (type == "ADDITION") module = new AdditionModule();
                if (type == "ADSR") module = new ADSRModule();
                if (type == "AMPLIFIER") module = new AmplifierModule();
                if (type == "CLOCK") module = new ClockModule();
                if (type == "CLOCK_DIVIDER") module = new ClockDividerModule();
                if (type == "DISTORTION") module = new DistortionModule();
                if (type == "DELAY") module = new DelayModule();
                if (type == "DELAY_LINE") module = new DelayLineModule();
                if (type == "DIVISION") module = new DivisionModule();
                if (type == "EXPONENTIAL_VCA") module = new ExponentialVCAModule();
                if (type == "FUZZ") module = new FuzzModule();
                if (type == "GATE_INPUT") module = new GateInputModule(gate_ptr);
                if (type == "LFO") module = new LFOModule();
                if (type == "LINEAR_VCA") module = new LinearVCAModule();
                if (type == "LOWPASS_FILTER") module = new LowpassFilterModule();
                if (type == "MIXER2") module = new Mixer2Module();
                if (type == "MIXER3") module = new Mixer3Module();
                if (type == "MIXER4") module = new Mixer4Module();
                if (type == "MIXER8") module = new Mixer8Module();
                if (type == "MOOG_LOWPASS_FILTER") module = new MoogLowpassFilterModule();
                if (type == "MORPHING_FILTER") module = new MorphingFilterModule();
                if (type == "MULTIPLICATION") module = new MultiplicationModule();
                if (type == "NOISE") module = new NoiseModule();
                if (type == "OUTPUT") module = new OutputModule();
                if (type == "OVERDRIVE") module = new OverdriveModule();
                if (type == "PARAM1") module = new ParamModule(p1);
                if (type == "PARAM2") module = new ParamModule(p2);
                if (type == "PARAM3") module = new ParamModule(p3);
                if (type == "PARAM4") module = new ParamModule(p4);
                if (type == "PARAM5") module = new ParamModule(p5);
                if (type == "PARAM6") module = new ParamModule(p6);
                if (type == "PARAM7") module = new ParamModule(p7);
                if (type == "PARAM8") module = new ParamModule(p8);
                if (type == "PITCH_INPUT") module = new PitchInputModule(pitch_ptr);
                if (type == "RAMP_OSCILLATOR") module = new RampOscillatorModule();
                if (type == "SAMPLE_AND_HOLD") module = new SampleAndHoldModule();
                if (type == "SCALE_QUANTIZER") module = new ScaleQuantizerModule();
                if (type == "SCHROEDER_REVERB") module = new SchroederReverbModule();
                if (type == "SUBTRACTION") module = new SubtractionModule();
                if (type == "SELECTOR2") module = new Selector2Module();
                if (type == "SELECTOR3") module = new Selector3Module();
                if (type == "SELECTOR4") module = new Selector4Module();
                if (type == "SELECTOR6") module = new Selector6Module();
                if (type == "SELECTOR8") module = new Selector8Module();
                if (type == "TABLE_LOOKUP") module = new TableLookupModule(data);
                if (type == "TB303_OSCILLATOR") module = new TB303OscillatorModule();
                if (type == "TB303_FILTER") module = new TB303FilterModule();
                if (type == "VCO") module = new VCOModule();
                if (type == "VOLTAGE_SEQUENCER") module = new VoltageSequencerModule(data);
                if (type == "WAVE_FOLDER") module = new WaveFolderModule();
                if (type == "WAVE_SHAPER") module = new WaveShaperModule();
                if (type == "WAVETABLE_OSCILLATOR") module = new WavetableOscillatorModule();

                if(module == nullptr) 
                {
                    VoxbuilderLogger::getInstance().log("ModuleManager.hpp::instantiateModules() - Unknown module type: " + type);
                }
                else
                {
                    VoxbuilderLogger::getInstance().log("Created module of type " + type + " having uuid " + module_uuid);
                }
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                VoxbuilderLogger::getInstance().log("ModuleManager.hpp::instantiateModules()" + error);
            }

            if (module != nullptr)
            {
                modules[module_uuid] = module;

                // Example defaults look like:
                //
                // "defaults": {
                //    "BPM": 120
                // },

                // Iterate over the defaults and set them on the module
                if(defaults != nullptr)
                {
                    // const char *key;
                    json_t *value;
                    void *iter = json_object_iter(defaults);
                    unsigned int key_index = 0;

                    while(iter)
                    {
                        value = json_object_iter_value(iter);

                        if(json_is_number(value)) // Checks for real or integer values
                        {
                            float real_value = static_cast<float>(json_number_value(value));
                            module->setParameter(key_index, real_value);
                        }

                        iter = json_object_iter_next(defaults, iter);
                        key_index++;
                    }
                }

            }
        }
    }

    bool connectModules()
    {
        for (const auto& connection : connections_config_forward)
        {
            // Connections go from "source" to "destination"
            VoxbuilderLogger::getInstance().log(
                "Connecting ports from module " 
                + connection.source_module_uuid + ", port " 
                + std::to_string(connection.source_port_index) 
                + " to module " 
                + connection.destination_module_uuid + ", port " 
                + std::to_string(connection.destination_port_index)
            );

            try 
            {
                IModule* source_module = modules.at(connection.source_module_uuid);
                IModule* dest_module = modules.at(connection.destination_module_uuid);
                Sport* source_port = source_module->getOutputPort(connection.source_port_index);
                Sport* dest_port = dest_module->getInputPort(connection.destination_port_index);

                source_port->connectToInputPort(dest_port);
                dest_port->connectToOutputPort(source_port);
            } 
            catch (const std::out_of_range& e) 
            {
                VoxbuilderLogger::getInstance().log("Trouble connecting ports! Please check module and port names.");
                return false;
            }           
        }

        return true;
    }

    //
    // Note: If I update this function to find the output module by type,
    // I may be able to remove the getOutputPorts function from all modules
    // and from the IModule interface
    IModule *findOutModule()
    {
        IModule *out_module = nullptr;

        // Find the last module in the chain
        // Why module.second? Because module is a pair: module.first is the id
        //   and module.second is the module object

        for (auto &module : modules)
        {
            if (module.second->getOutputPorts().size() == 0)
            {
                out_module = module.second;
            }
        }

        return(out_module);
    }


    void clear()
    {
        modules.clear();
        module_config_map.clear();
        connections_config_forward.clear();

        terminal_output_module = nullptr;
    }

    /*


        █▀█ █░█ █▄░█   █▀█ ▄▀█ ▀█▀ █▀▀ █░█
        █▀▄ █▄█ █░▀█   █▀▀ █▀█ ░█░ █▄▄ █▀█
        Text created using https://fsymbols.com/generators/carty/

    */
   /*
    // This runs at sample rate
    void processModule(IModule *module, unsigned int sample_rate)
    {
        if (module->processing) return;
        module->processing = true;

        std::vector<Sport *> input_ports = module->getInputPorts();

        for (auto &input_port : input_ports)
        {
            if (input_port->isConnected())
            {
                std::vector<Sport *> connected_outputs = input_port->getConnectedOutputs();

                if(connected_outputs.size() > 0)
                {
                    IModule *connected_module = connected_outputs[0]->getParentModule();

                    if (!connected_module->processing)
                    {
                        processModule(connected_module, sample_rate);
                    }
                    else
                    {
                        // If the connected module is currently being processed,
                        // use the output from the last timestep instead.
                        input_port->setVoltage(connected_outputs[0]->getLastVoltage());
                    }
                }
            }
        }

        module->process(sample_rate);
        module->processing = false;
    }


    //
    // process()
    //
    // Starts processing a patch to get the output.  The output of the entire
    // patch is returned as a float.
    //
    // This is called at sample rate.
    //
    // TODO: Handle passing in sample rate
    //


    float process(unsigned int sample_rate)
    {
        // Reset all module processed flags to false
        resetProcessingFlags();

        if(! terminal_output_module)
        {
            return(4.04);
        }

        // Compute the outputs of the system by starting with the last module,
        // then working backwards through the chain.
        processModule(terminal_output_module, sample_rate);

        // This might be a litte confusing, so let me explain it a bit.
        // The terminal output module is the last module in the patch. It has
        // an input port called INPUT_PORT.  The value at INPUT_PORT is basically
        // the value that the entire patch outputs.  So here, we're getting the
        // value at INPUT_PORT and returning it.  The function processModule()
        // will have computed the value at INPUT_PORT by processing the entire
        // patch, so we're just returning that value.

        Sport *input_port = terminal_output_module->getInputPort(0);

        return (input_port->getVoltage());
    }

    // Reset all module processed flags to false
    void resetProcessingFlags()
    {
        for (auto &module : modules)
        {
            module.second->processing = false;
        }
    }
    */
};