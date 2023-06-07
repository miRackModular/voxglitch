/*

    PatchConstructor.hpp
    █▀█ ▄▀█ ▀█▀ █▀▀ █░█   █▀▀ █▀█ █▄░█ █▀ ▀█▀ █▀█ █░█ █▀▀ ▀█▀ █▀█ █▀█
    █▀▀ █▀█ ░█░ █▄▄ █▀█   █▄▄ █▄█ █░▀█ ▄█ ░█░ █▀▄ █▄█ █▄▄ ░█░ █▄█ █▀▄
    Text created using https://fsymbols.com/generators/carty/
    
*/

//
// This class takes JSON as input and creates a Patch object.
// Patches need to be "runnable".  Eventually, I may store more than one
// patch in memory, and they'll need to be loaded in advance.
//
// TODO: 
// - Send through sample rate
// - Figure out how to represent PI
// - Update findOutModule to use the Type instead of the number of outputs
/*
 Notes:  

Thinking ahead to patching together connections to modules within macros...
- A module may have a connection to a port on a macro (inbound)
  - My goal is to adjust that connection to go to the module attached to the correct module within the macro.
    This means:
    1. I take note of the module and output port that's connecting to the macro's input port _store that connection as "FOO"
    2. Maybe from the port index on the macro's port, I can look up the correct MACRO_INPUT_PORT module
    2. From there, I can find the module and input port that's the destination of the MACRO_INPUT_PORT's output (BAR)
    3. The, I can adjust the original connection (FOO) to connect directly to BAR
    4. Once I've done this will all of the input and output connections associated with the macro, I can delete the macro,
       thus "flattening" the network, and making it easier to traverse.  The patch traversal code will NOT need to be updated.

However, this may lead to issues when recursively entering macro's patches to continue to build the network.  Well, remember
that we're no longer dealing with a tree structure.  It's already been flattened.

SInce it's flat, there will be a disconnect between the modules within a macro and those outside of it.  With the current
structure, you wouldn't know from looking at modules from within a macro what their parent is.  Maybe I store the uuid
 of the parent module within all module inside of it?  I think this might work, and be better than my old plan.

- A macro may have a connection to a port on a module (outbound)

*/

#pragma once

#include "Sport.hpp"
#include "IModule.h"
#include "Patch.hpp"
#include "Connection.hpp"
#include "VoxbuilderLogger.hpp"

// Utility modules
#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/ParamModule.hpp"

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

class PatchConstructor
{

private:
    // Pointers
    // Patch *patch = nullptr;
    float *pitch_ptr;
    float *gate_ptr;
    float *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;    
    std::vector<Connection> connections_config;

public:

    bool ready = false;

    PatchConstructor(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
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

    /**
     * Creates a Patch object based on the provided JSON configuration.
     *
     * @param root The root JSON object representing the patch configuration.
     * @return A pointer to the created Patch object, or nullptr if the patch creation failed.
     *
     * The function extracts the module configurations and connection configurations from the JSON.
     * It calls the `parseModulesConfiguration` function to parse the module configurations and retrieve a map of module UUIDs to ModuleConfig pointers.
     * It calls the `parseConnectionsConfiguration` function to parse the connection configurations and retrieve a vector of Connection objects.
     * Then, it calls the `instantiateModules` function to create instances of the module classes based on the module configurations.
     * The instantiated modules are stored in a map, where the key is the module UUID and the value is a pointer to the module.
     * The function then calls the `connectModules` function to establish the connections between the modules using the connection configurations.
     * If the connection process fails, the function returns nullptr.
     * Next, the function finds the terminal output module in the module map and sets it as the terminal output module for the patch.
     * If no terminal output module is found, the function returns nullptr.
     * Finally, it creates a new Patch object, sets the terminal output module and the modules map, and returns a pointer to the created Patch object.
     * The function logs messages to the VoxbuilderLogger instance to indicate the progress and any errors encountered.
     */

    Patch *createPatch(json_t* root)
    {
        VoxbuilderLogger::getInstance().log("createPatch Initiated");

        // Get the module configurations and connection configurations from the JSON
        // std::unordered_map<std::string, ModuleConfig*> module_config_map = parseModulesConfiguration(root);;
        // std::vector<Connection> connections_config_vector = parseConnectionsConfiguration(root);

        std::unordered_map<std::string, ModuleConfig*> module_config_map;
        std::vector<Connection> connections_config_vector;

        std::tie(module_config_map, connections_config_vector) = parsePatchConfiguration(root, "");

        // instantiate all modules
        std::map<std::string, IModule*> modules_map = instantiateModules(module_config_map, pitch_ptr, gate_ptr, p1, p2, p3, p4, p5, p6, p7, p8);

        // Debugging notes
        // The modules_map does not have the uuid keys set properly.  
        // This is because the uuid is set in the constructor of the module, and
        // the module is created in the instantiateModules function.

        VoxbuilderLogger::getInstance().log("FOR TESTING");
        
        for (const auto& uuid_module_ptr_map : modules_map)
        {
            // log it
            VoxbuilderLogger::getInstance().log("Module uuid: " + uuid_module_ptr_map.first + ", type: " + uuid_module_ptr_map.second->uuid);
        }

        // connect all modules.  Note that module_map is being sent in by reference
        // and will be updated with the connections.
        if(! connectModules(modules_map, connections_config_vector)) return nullptr;

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain and sets the member variable "terminal_output_module"
        IModule *terminal_output_module = findOutModule(modules_map);

        // If there's no terminal output module, then the patch is invalid
        if (terminal_output_module == nullptr) 
        {
            VoxbuilderLogger::getInstance().log("No output module found.  The patch must have an output module.");
            return nullptr;
        }
        else
        {
            VoxbuilderLogger::getInstance().log("Found terminal output module with uuid " + terminal_output_module->uuid);
        }

        // Set ready flag to true
        // (Do I need the ready flag anymore?)
        VoxbuilderLogger::getInstance().log("CreatePatch was successful.  Setting 'ready' to TRUE.");

        // Populate the patch object
        Patch *patch = new Patch();
        patch->setTerminalOutputModule(terminal_output_module);
        patch->setModules(modules_map);

        return patch;
    }

    bool isReady()
    {
        return ready;
    }

    void setReady(bool ready)
    {
        this->ready = ready;
    }


    /**
     * Instantiates module classes based on the provided module configurations and returns a map of the modules,
     * where the key is the module's uuid, and the value is a pointer to the module
     *
     * @param module_configurations An unordered map of module UUIDs to corresponding ModuleConfig pointers.
     * @param pitch_ptr A pointer to the pitch value.
     * @param gate_ptr A pointer to the gate value.
     * @param p1 A pointer to the parameter value for PARAM1.
     * @param p2 A pointer to the parameter value for PARAM2.
     * @param p3 A pointer to the parameter value for PARAM3.
     * @param p4 A pointer to the parameter value for PARAM4.
     * @param p5 A pointer to the parameter value for PARAM5.
     * @param p6 A pointer to the parameter value for PARAM6.
     * @param p7 A pointer to the parameter value for PARAM7.
     * @param p8 A pointer to the parameter value for PARAM8.
     * @return A map of the modules, where the key is the module's UUID and the value is a pointer to the module.
     *
     * The function iterates over the module configurations and creates instances of the corresponding module classes.
     * It retrieves the UUID, type, data, and defaults for each module from the ModuleConfig object.
     * Based on the module type, it instantiates the appropriate module class and assigns it to the module pointer.
     * If an unknown module type is encountered, an error message is logged.
     * The function also sets the parameter values from the defaults on the module, if provided.
     * Finally, it sets the UUID on the module, adds it to the modules map, and returns the map of modules.
     * The function logs messages to the VoxbuilderLogger instance to indicate the creation of each module and any errors encountered.
     */

    std::map<std::string, IModule *> instantiateModules(std::unordered_map<std::string, ModuleConfig*> module_configurations, float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        // This is what we want to populate and return
        std::map<std::string, IModule*> modules;

        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_configurations)
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
                    VoxbuilderLogger::getInstance().log("PatchConstructor.hpp::instantiateModules() - Unknown module type: " + type);
                }
                else
                {
                    VoxbuilderLogger::getInstance().log("Created module of type " + type + " having uuid " + module_uuid);
                }
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                VoxbuilderLogger::getInstance().log("PatchConstructor.hpp::instantiateModules()" + error);
            }

            if (module != nullptr)
            {
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

                module->setUuid(module_uuid);
                modules[module_uuid] = module;
            }
        }

        return modules;
    }

    /**
     * Connects the modules based on the provided connections configuration.
     *
     * @param modules_map A map of module UUIDs to corresponding IModule pointers.
     * @param connections_config A vector of Connection objects representing the connections between modules.
     * @return A boolean indicating whether the module connections were successfully established.
     *
     * The function iterates over the connections in the connections configuration and establishes the connections between
     * the corresponding modules. It retrieves the source and destination modules and their respective ports based on the
     * UUIDs and port indices provided in the Connection objects. It then establishes the connections between the ports.
     * If any module or port is not found in the modules_map, the function logs an error message and returns false.
     * The logged error message includes the contents of the modules_map and connections_config for debugging purposes.
     * If all connections are successfully established, the function returns true.
     */

    bool connectModules(std::map<std::string, IModule*> modules_map, std::vector<Connection> connections_config)
    {
        for (const auto& connection : connections_config)
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
                IModule* source_module = modules_map.at(connection.source_module_uuid);
                IModule* dest_module = modules_map.at(connection.destination_module_uuid);
                Sport* source_port = source_module->getOutputPort(connection.source_port_index);
                Sport* dest_port = dest_module->getInputPort(connection.destination_port_index);

                source_port->connectToInputPort(dest_port);
                dest_port->connectToOutputPort(source_port);
            } 
            catch (const std::out_of_range& e) 
            {
                VoxbuilderLogger::getInstance().log("Trouble connecting ports! Please check module and port names.");

                // Log each module's uuid and type
                VoxbuilderLogger::getInstance().log("Contents of modules_map: ");

                for (const auto& module : modules_map)
                {
                    VoxbuilderLogger::getInstance().log("Module uuid: " + module.first + ", type: " + module.second->uuid);
                }

                // Log each connection
                VoxbuilderLogger::getInstance().log("Contents of connections_config: ");

                for (const auto& connection : connections_config)
                {
                    VoxbuilderLogger::getInstance().log("Connection source module uuid: " + connection.source_module_uuid + ", source port index: " + std::to_string(connection.source_port_index) + ", destination module uuid: " + connection.destination_module_uuid + ", destination port index: " + std::to_string(connection.destination_port_index));
                }

                return false;
            }           
        }

        return true;
    }

    //
    // When parsePatchConfiguration returned both modules and connections, if there were any
    // macros in the patch, there won't be any connections that link the inside of the macro
    // to the modules outside of the macro.  
    //
    // Here's where we are:
    // 
    //   some         Macro Module (parent)
    //   module      ┌───────────────────────────────────────┐
    //   ┌─────┐     │  MACRO_INPUT_MODULE     Other Module  │
    //   │     ├────►i1      ┌─────┐            ┌─────┐      │
    //   └─────┘     │       │ i1  ├────────────►     │      │
    //               │       └─────┘            └─────┘      │
    //               └───────────────────────────────────────┘
    //               (i1 means port index "1")
    //
    //
    // And here's where we want to be:
    //
    //   some         
    //   module      
    //   ┌─────┐                               Other Module       
    //   │     ├────────────────────┐           ┌─────┐          
    //   └─────┘                    └───────────►     │          
    //                                          └─────┘          
    //
    //
    //
    // (Diagrams created using: https://asciiflow.com/#/)
    //
    // The modules inside the macro will have their "parent_uuid" set to the macro module's uuid.
    //


    bool bridgeMacroConnections(std::map<std::string, IModule*> modules_map, std::vector<Connection> connections_config)
    {

        std::vector<Module *> removed_modules;

        // Iterate through all of the modules
        for (auto &module_map : modules_map)
        {
            Module* module = dynamic_cast<Module*>(module_map.second);

            // Find the modules that are MACRO_INPUT_PORT or MACRO_OUTPUT_PORT
            if (module->getType() == "MACRO_INPUT_PORT")
            {
                // Here's the tricky part.  We need to extract the "macro_module_input_port_index" from the data json stored
                // in the module's config.  The data json is stored in the module's config as a jsont
                int macro_module_input_port_index = module->data["macro_input_port_index"];
                std::string parent_macro_module_uuid = module->parent_uuid;

                //
                // I'm going to be referring to this diagram both in the comments and in the code:
                //
                //   some         Macro Module (uuid)
                //   module      ┌───────────────────────────────────────┐
                //   ┌─────┐     │  MACRO_INPUT_MODULE     Other Module  │
                //   │     ├───►(i)      ┌─────┐            ┌─────┐      │
                //   └─────┘     │       │     ├────────────►     │      │
                //               │       └─────┘            └─────┘      │
                //               └───────────────────────────────────────┘
                //
                // We now have:
                // 1. The parent_macro_module_uuid (uuid)
                // 2. The macro_module_input_port_index (i)

                // Now we need to find the connection that has the same destination_module_uuid and destination_port_index
                // as the MACRO_INPUT_PORT we're looking at.  From there, we can find "some module"

                Connection *connection_to_macro_module = this->findConnection(connections_config, parent_macro_module_uuid, macro_module_input_port_index);

                if (connection_to_macro_module != nullptr)
                {
                    // We found the connection that we need to bridge
                    // Now we need to find "some module" (see diagram above)
                    IModule* some_module = modules_map.at(connection_to_macro_module.source_module_uuid);

                    // Look up the source port
                    Sport* some_module_src_port = some_module->getOutputPort(connection_to_macro_module.source_port_index);

                    // The goal at this point it to link the some_module_src_port to the input port of "Other Module"
                    // In order to find "other module", we need to find the connection that starts with the MACRO_INPUT_MODULE, 
                    // which we can do by calling findConnection with the MACRO_INPUT_MODULE's uuid and "0" as the port index
                    // (The output port index of the MACRO_INPUT_MODULE is always 0)
                    Connection *connection_to_other_module = this->findConnection(connections_config, module->uuid, 0);

                    if(connection_to_other_module != nullptr)
                    {
                        // We found the connection that we need to bridge
                        // Now we need to find "other module" (see diagram above)
                        IModule* other_module = modules_map.at(connection_to_other_module.destination_module_uuid);

                        // We have the connection from the MACRO_INPUT_MODULE to "other module".  Using that 
                        // connection, we can look up the port on "other module" that we need to bridge to.
                        Sport* other_module_dst_port = other_module->getInputPort(connection_to_other_module.destination_port_index);

                        // Now connect the source port of "some module" to the destination port of "other module"
                        some_module_src_port->connect(other_module_dst_port);

                        // At this point, it's safe to remove the MACRO_INPUT_MODULE from the modules_map
                        // However, due to my memory management paranoia, let's push it onto a vector and delete it later
                        removed_modules.push_back(module);
                    }
                }
            }

            // OK, it's time to bridge the other side:
            //
            //   We have this...
            //
            //    Macro Module (uuid)
            //   ┌───────────────────────────────────────────┐         blah
            //   │     Other_Module     MACRO_OUTPUT_MODULE  │         module
            //   │       ┌─────┐            ┌─────┐          │         ┌─────┐
            //   │       │     ├────────────►     │         (i)────────►     │
            //   │       └─────┘            └─────┘          │         └─────┘
            //   │                                           │
            //   └───────────────────────────────────────────┘
            //
            //    And we want this...
            //
            //    Macro Module
            //   ┌───────────────────────────────────────────┐         blah
            //   │     Other_Module                          │         module
            //   │       ┌─────┐                             │         ┌─────┐
            //   │       │     ├─────────────────────────────┼─────────►     │
            //   │       └─────┘                             │         └─────┘
            //   │                                           │
            //   └───────────────────────────────────────────┘


            if (module->getType() == "MACRO_OUTPUT_PORT")
            {
                int macro_module_output_port_index = module->data["macro_output_port_index"]; // This is (i) in the diagram above
                std::string parent_macro_module_uuid = module->parent_uuid; // This is (uuid) in the diagram above

                // We need to find the connection that links "other_module" to the MACRO_OUTPUT_MODULE
                // For this, we can use findConnection, sending in port_index "0" since that's always
                // the input port index of the MACRO_OUTPUT_MODULE.
                Connection *connection_to_macro_output_module = this->findConnection(connections_config, module, 0);

                // From the connection, we can find the source module
                IModule* other_module = modules_map.at(connection_to_macro_output_module.source_module_uuid);
                Sport *other_module_src_port = other_module->getOutputPort(connection_to_macro_output_module.source_port_index);

                // Find the connection from the Macro Module to "blah module" by using the
                // Macro Module's UUID and the macro_module_output_port_index.
                // This is the connection that goes between (i) and "blah module" in the diagram above.
                Connection *connection_to_blah_module = this->findConnection(connections_config, parent_macro_module_uuid, macro_module_output_port_index);

                Module *blah_module = modules_map.at(connection_to_blah_module.destination_module_uuid);
                Sport* blah_module_dst_port = blah_module->getInputPort(connection_to_blah_module.destination_port_index);

                // Now connect the source port of "other module" directly to the destination port of "blah module"
                other_module_src_port->connect(blah_module_dst_port);

                // At this point, it's safe to remove the MACRO_OUTPUT_MODULE from the modules_map
                // However, due to my memory management paranoia, let's push it onto a vector and delete it later
                removed_modules.push_back(module);
            }
        }

        // Remove the modules that we pushed onto the removed_modules vector
        for (auto &module : removed_modules)
        {
            modules_map.erase(module->uuid);
            delete module;
        }
    }

    // Pass in the destination module's uuid and the destination port index
    Connection *findConnection(std::vector<Connection> connections_config, std::string module_uuid, int port_index)
    {
        Connection *connection = nullptr;

        for (const auto& connection : connections_config)
        {
            if (connection.destination_module_uuid == module_uuid &&
                connection.destination_port_index == port_index)
            {
                connection = &connection;
                break;
            }
        }

        return(connection);
    }


    //
    // Note: If I update this function to find the output module by type,
    // I may be able to remove the getOutputPorts function from all modules
    // and from the IModule interface
    IModule *findOutModule(std::map<std::string, IModule*> modules_map)
    {
        IModule *out_module = nullptr;

        // Find the last module in the chain
        // Why module.second? Because module is a pair: module.first is the id
        //   and module.second is the module object

        for (auto &module : modules_map)
        {
            if (module.second->getOutputPorts().size() == 0)
            {
                out_module = module.second;
            }
        }

        return(out_module);
    }

    std::unordered_map<std::string, ModuleConfig*> convertToMap(std::vector<ModuleConfig*> moduleConfigs) 
    {
        std::unordered_map<std::string, ModuleConfig*> module_config_map;

        for (ModuleConfig* config : moduleConfigs) 
        {
            module_config_map[config->uuid] = config;
        }

        return(module_config_map);
    }

  
    std::pair<std::unordered_map<std::string, ModuleConfig*>, std::vector<Connection>> parsePatchConfiguration(json_t* root, std::string parent_uuid = "")
    {
        std::vector<ModuleConfig*> module_config_vector;
        std::vector<Connection> connections_config;
        std::unordered_map<std::string, ModuleConfig*> module_config_map;

        connections_config = parseConnectionsConfiguration(root);

        json_t* modules_array = json_object_get(root, "modules");
        size_t modules_size = json_array_size(modules_array);

        for (size_t i = 0; i < modules_size; ++i)
        {
            json_t* module_obj = json_array_get(modules_array, i);

            std::string uuid = json_string_value(json_object_get(module_obj, "uuid"));
            std::string type = json_string_value(json_object_get(module_obj, "type"));

            // log if the uuid is empty
            if(uuid.empty() || uuid == "none") 
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Module uuid not present. UUID: " + uuid);
            }

            // log if the type is empty
            if(type.empty()) 
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Module type is empty.");
            }

            // If I didn't use json_deep_copy below, problems would occur when
            // root is freed, which could cause intermittent crashes.

            /*
            //
            //  Load "defaults"
            // 
            json_t* defaults = nullptr;
            if(json_t* defaults_obj = json_object_get(module_obj, "defaults")) 
            {
                defaults = json_deep_copy(defaults_obj);
            }

            //
            //  Load "data"
            // 
            json_t* data = nullptr;
            if(json_t* data_obj = json_object_get(module_obj, "data")) 
            {
                data = json_deep_copy(data_obj);
            }
            */

            // Same code as above, but with error checking
            //
            //  Load "defaults"
            // 
            json_t* defaults = nullptr;
            json_t* defaults_obj = json_object_get(module_obj, "defaults");
            if (defaults_obj)
            {
                if (!json_is_object(defaults_obj)) 
                {
                    VoxbuilderLogger::getInstance().log("parseModulesConfiguration: defaults is not a JSON object.");
                }
                else 
                {
                    defaults = json_deep_copy(defaults_obj);
                    if (defaults == nullptr)
                    {
                        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Failed to copy defaults.");
                    }
                }
            }
            else
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: defaults field not present.");
            }

            //
            //  Load "data"
            // 
            json_t* data = nullptr;
            json_t* data_obj = json_object_get(module_obj, "data");
            if (data_obj)
            {
                if (!json_is_object(data_obj)) 
                {
                    VoxbuilderLogger::getInstance().log("parseModulesConfiguration: data is not a JSON object.");
                }
                else 
                {
                    data = json_deep_copy(data_obj);
                    if (data == nullptr)
                    {
                        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Failed to copy data.");
                    }
                }
            }
            else
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: data field not present.");
            }

            module_config_vector.push_back(new ModuleConfig(uuid, type, defaults, data, parent_uuid));

            // If the module type is "MACRO", then we need to parse the patch inside of the macro.
            // We do this by calling parsePatchConfiguration recursively.
            if(type == "MACRO") 
            {
                json_t* macro_patch = json_object_get(module_obj, "patch");

                std::pair<std::unordered_map<std::string, ModuleConfig*>, std::vector<Connection>> macro_patch_result = parsePatchConfiguration(macro_patch, uuid);

                // Add the modules in the macro patch to the module_config_map
                for (const auto& module_config : macro_patch_result.first)
                {
                    module_config_map.insert(module_config);
                }

                // Add the connections in the macro patch to the connections_config
                for (const Connection& connection : macro_patch_result.second)
                {
                    connections_config.push_back(connection);
                }
            }
        }

        // Get the results from convertToMap(module_config_vector) and append them to module_config_map
        std::unordered_map<std::string, ModuleConfig*> new_module_config_map = convertToMap(module_config_vector);

        // Append the new_module_config_map to the module_config_map
        for (const auto& module_config : new_module_config_map)
        {
            module_config_map.insert(module_config);
        }

        // Test to see if the map is empty and if so, log a message
        if(module_config_map.empty()) 
        {
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: No modules found in configuration json.");

            // Also log the contents of the json for module_config_map
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Contents of module_config_map: ");

            for (const auto& module_config : module_config_map)
            {
                VoxbuilderLogger::getInstance().log("Module uuid: " + module_config.first + ", type: " + module_config.second->type);
            }
        }
        else
        {
            // Log how many modules were found
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Found " + std::to_string(module_config_map.size()) + " modules in configuration json.");
        }

        // Here's sample output, in jSON for easy reading
        // {
        //   "7042ca37-819f-4f11-8b4c-cdfd71873824": {
        //     "type": "OUTPUT",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "c6ad45a3-0013-462e-a09d-bdb42327f5b2": {
        //     "type": "PARAM2",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "66f2169c-815a-486f-881c-395b778f8eb5": {
        //     "type": "PARAM1",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "29584a27-c30c-4bf3-b065-9658e6054d79": {
        //     "type": "WAVETABLE_OSCILLATOR",
        //     "data": {},
        //     "defaults": {}
        //   }
        // }

        // log the entire module_config_map
        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Contents of module_config_map: ");

        for (const auto& module_config : module_config_map)
        {
            VoxbuilderLogger::getInstance().log("Module uuid: " + module_config.first + ", type: " + module_config.second->type);
        }

        return std::make_pair(module_config_map, connections_config);
    }


    /**
     * Parses the connections configuration from the given JSON object and returns a vector of Connection objects.
     *
     * @param root A pointer to the JSON object containing the connections configuration.
     * @return A vector of Connection objects representing the connections between modules.
     *
     * The function iterates over the connections in the JSON object and extracts the source and destination module UUIDs
     * and port IDs for each connection. It creates a new Connection object for each connection and adds it to a vector.
     * Finally, it returns the vector of Connection objects representing the connections between modules.
     * If the JSON object is empty or does not contain any connections, an empty vector is returned.
     */

    std::vector<Connection> parseConnectionsConfiguration(json_t* root)
    {
        std::vector<Connection> connections_config;

        json_t* connections_array = json_object_get(root, "connections");
        size_t connections_size = json_array_size(connections_array);

        for (size_t i = 0; i < connections_size; ++i)
        {
            json_t* connection_obj = json_array_get(connections_array, i);

            json_t* src_obj = json_object_get(connection_obj, "src");
            std::string src_module_uuid = json_string_value(json_object_get(src_obj, "module_uuid"));
            unsigned int src_port_id = json_integer_value(json_object_get(src_obj, "port_id"));

            json_t* dst_obj = json_object_get(connection_obj, "dst");
            std::string dst_module_uuid = json_string_value(json_object_get(dst_obj, "module_uuid"));
            unsigned int dst_port_id = json_integer_value(json_object_get(dst_obj, "port_id"));

            connections_config.push_back(Connection(src_module_uuid, src_port_id, dst_module_uuid, dst_port_id));
        }

        //
        // Example json output for this function
        //
        //[
        //  {
        //    "dst": {
        //      "module_uuid": "7042ca37-819f-4f11-8b4c-cdfd71873824",
        //      "port_id": 0
        //    },
        //    "src": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 0
        //    }
        //  },
        //  {
        //    "dst": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 1
        //    },
        //    "src": {
        //      "module_uuid": "c6ad45a3-0013-462e-a09d-bdb42327f5b2",
        //      "port_id": 0
        //    }
        //  },
        //  {
        //    "dst": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 0
        //    },
        //    "src": {
        //      "module_uuid": "66f2169c-815a-486f-881c-395b778f8eb5",
        //      "port_id": 0
        //    }
        //  }
        //]


        return connections_config;
    }

};