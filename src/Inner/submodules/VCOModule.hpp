#pragma once
#include <string> 
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class VCOModule : public BaseModule {

private:
    float phase = 0.0f;
    const float C4_FREQUENCY = 261.6256f;
    float previous_sync_input = 0.0f;

    enum class Waveform {
        SINE,
        TRIANGLE,
        PULSE,
        SAWTOOTH,
        NUM_WAVEFORMS
    };

public:

    // Inputs
    Sport *frequency_input_port = new Sport(this); //  Ranges 0.0V to 10.0V
    Sport *waveform_input_port = new Sport(this); //  Ranges 0.0V to 10.0V
    Sport *sync_threshold_input_port = new Sport(this); //  Ranges -5V to +5V
    Sport *sync_input_port = new Sport(this); // Ranges from -5V to +5V

    // Outputs
    Sport *output_port = new Sport(this);


    VCOModule() 
    {
        setParameter("frequency", 1.0f);
        setParameter("waveform", 4.0f);
        setParameter("sync_threshold", 5.0f);
        setParameter("pulse_width", 0.5f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency_voltage = frequency_input_port->isConnected() ? frequency_input_port->getValue() : getParameter("frequency");
        float waveform_voltage = waveform_input_port->isConnected() ? waveform_input_port->getValue() : getParameter("waveform");
        float sync_threshold = sync_threshold_input_port->isConnected() ? sync_threshold_input_port->getValue() : getParameter("sync_threshold");
        float sync_input = sync_input_port->isConnected() ? sync_input_port->getValue() : 0.0f;

        // Get the selected waveform based on the waveform voltage
        Waveform selected_waveform = getSelectedWaveform(waveform_voltage);

        // If the sync input is connected, process it.  This will reset the phase
        // when the sync_input crosses the sync_threshold.
        processSyncInput(sync_input, sync_threshold);

        // Calculate the phase increment and update the phase
        float phase_increment = getPhaseIncrement(frequency_voltage, sample_rate);
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Calculate the output based on the selected waveform
        float out = computeWaveform(selected_waveform, phase);

        // Set output value, which will also alert any connected ports
        output_port->setValue(out * 5.0f);
    }

    void processSyncInput(float sync_input, float sync_threshold)
    {
        if (sync_input_port->isConnected())
        {
            // The sync_threshold input port is a voltage that ranges from 0V to 10V.
            // However, we want to be able to compare the sync input to the sync threshold.
            // Since the sync input ranges from -5V to +5V, we need to convert the sync threshold
            // to a value between -5V and +5V. We do this by subtracting 5V from the sync threshold.
            sync_threshold -= 5.0f;

            // If the sync input port has a rising edge (i.e. changes from a value less than 
            // the sync threshold to a value greater than or equal to the sync threshold),
            // reset the phase to 0.
            if ((sync_input >= sync_threshold) && (previous_sync_input < sync_threshold)) 
            {
                phase = 0.0f;
            }

            // Update previous sync input value
            previous_sync_input = sync_input;
        }
    }

    float computeWaveform(Waveform waveform, float phase)
    {
        float out = 0.0f;

        switch (waveform)
        {
            case Waveform::SINE:
                out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);
                break;
            case Waveform::TRIANGLE:
                out = (phase < 0.5f) ? (-1.0f + 4.0f * phase) : (3.0f - 4.0f * phase);
                break;
            case Waveform::PULSE:
                out = (phase < getParameter("pulse_width")) ? 1.0f : -1.0f;
                break;
            case Waveform::SAWTOOTH:
                out = -1.0f + 2.0f * phase;
                break;
            default:
                break;
        }

        return out;
    }

    Waveform getSelectedWaveform(float waveform_voltage)
    {
        // The voltage range for selecting a waveform will range from 0v to 10v.
        int waveform_selection = (waveform_voltage / 10.0f) * (int)Waveform::NUM_WAVEFORMS;
        waveform_selection = clamp(waveform_selection, 0, (int)Waveform::NUM_WAVEFORMS - 1);

        return static_cast<Waveform>(waveform_selection);
    }

    float getPhaseIncrement(float frequency_voltage, unsigned int sample_rate)
    {
        // Convert the voltage to a frequency in Hertz using the 1 V/oct standard
        float frequency = C4_FREQUENCY * powf(2.0f, frequency_voltage);

        // Calculate phase increment based on the frequency        
        return frequency / static_cast<float>(sample_rate);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "FREQUENCY_INPUT_PORT") return(frequency_input_port); 
        if (port_name == "WAVEFORM_INPUT_PORT") return(waveform_input_port);
        if (port_name == "SYNC_INPUT_PORT") return(sync_input_port);
        if (port_name == "SYNC_THRESHOLD_INPUT_PORT") return(sync_threshold_input_port);
        if (port_name == "OUTPUT_PORT") return(output_port);
        
        DEBUG("Invalid port name: ");
        DEBUG(port_name.c_str());

        return(nullptr);
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {
            output_port
        };
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {    
            frequency_input_port, waveform_input_port, sync_input_port, sync_threshold_input_port
        };
    }    
};