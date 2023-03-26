// IModule.h 

#pragma once

#include <vector>
#include <unordered_map>
#include "Sport.h"

class Sport;

class IModule : public std::enable_shared_from_this<IModule> {

protected:
    std::unordered_map<std::string, float> parameters;
    std::unordered_map<std::string, std::shared_ptr<Sport>> input_ports;
    std::unordered_map<std::string, std::shared_ptr<Sport>> output_ports;

public:
    virtual void process(unsigned int sample_rate) = 0;
    unsigned int id = 0;
    bool processing = false;

    virtual std::vector<std::string> getInputPortNames() 
    {
        std::vector<std::string> names;
        for (auto it = input_ports.begin(); it != input_ports.end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

    virtual std::vector<std::string> getOutputPortNames() 
    {
        std::vector<std::string> names;
        for (auto it = output_ports.begin(); it != output_ports.end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

    // Methods for setting and getting parameters
    virtual void setParameter(const std::string& name, float value) {
        parameters[name] = value;
    }

    virtual float getParameter(const std::string& name) const {
        return parameters.at(name);
    }   

    virtual void addInput(const std::string& name) {
        std::shared_ptr<Sport> input = std::make_shared<Sport>(shared_from_this());
        input_ports[name] = input;
    }

    virtual void addOutput(const std::string& name) {
        std::shared_ptr<Sport> output = std::make_shared<Sport>(shared_from_this());
        output_ports[name] = output;
    }

    std::unordered_map<std::string, std::shared_ptr<Sport>> getOutputPorts() {
        return output_ports;
    }

    std::unordered_map<std::string, std::shared_ptr<Sport>> getInputPorts() {
        return input_ports;
    }


    std::shared_ptr<Sport> getInputPortByName(const std::string& name) {
        auto it = input_ports.find(name);
        if (it != input_ports.end()) {
            return it->second;
        } else {
            // Handle error case
            return nullptr;
        }
    }

    std::shared_ptr<Sport> getOutputPortByName(const std::string& name) {
        auto it = output_ports.find(name);
        if (it != output_ports.end()) {
            return it->second;
        } else {
            // Handle error case
            return nullptr;
        }
    }        
};