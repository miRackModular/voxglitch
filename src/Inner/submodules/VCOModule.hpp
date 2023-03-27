#pragma once
#include <string> 
#include <cmath>
#include "../BaseModule.hpp"

class VCOModule : public BaseModule {

private:
    float phase = 0.0f;
    const float C4_FREQUENCY = 261.6256f;

public:

    Sport *output_port = nullptr;
    Sport *frequence_input_port = nullptr;

    VCOModule() 
    {
        setParameter("frequency", 440.0f);
        output_port = new Sport(this);
        frequence_input_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override 
    {
        if(frequence_input_port->isConnected())
        {
            this->setParameter("frequency", frequence_input_port->getValue());
        }

        float frequency_voltage = getParameter("frequency");

        // Convert the voltage to a frequency in Hertz using the 1 V/oct standard
        float frequency = C4_FREQUENCY * powf(2.0f, frequency_voltage);

        // Calculate phase increment based on the frequency        
        float phase_increment = frequency / 44100.0;

        // Update phase
        phase += phase_increment;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }

        // Calculate output
        float out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);

        // Set output value, which will also alert any connected ports
        output_port->setValue(out * 5.0f);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT"){
            return(output_port);
        }
        else if (port_name == "FREQUENCY_INPUT_PORT"){
            return(frequence_input_port);
        }
        else {
           return(nullptr);
        }
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
            frequence_input_port     
        };
    }    
};