#pragma once
#include <nlohmann/json.hpp>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

// Parse JSON netlist
namespace YosysJSONparser{
    // Assuming there is a limit of a number of inputs for each gate.
    constexpr uint MAX_NUM_INPUT = 3;

    //Holding information of each gate
    struct GateStruct{
        std::string name;
        std::array<uint,MAX_NUM_INPUT> in;
        uint out;
    };

    struct ParsedBC{
        std::vector<uint> input_vector;
        std::vector<uint> output_vector;
        std::vector<uint> wire_vector;
        std::vector<GateStruct> gate_vector;
        std::vector<std::array<uint,2>> DFF_vector;
        std::vector<std::array<uint,2>> direct_port_pair_vector;
        //Because input ports, output ports and wires have unique index, we can hold the information about which gate output to specific wires or output ports by this vector.
        std::vector<uint> dependency_vector;
        ParsedBC(const std::string &);
    };

    ParsedBC::ParsedBC(const std::string &jsonstring){
        uint gate_index = 1;
        const nlohmann::json& jsonnetlist = nlohmann::json::parse(jsonstring);
        for( const nlohmann::json & module : jsonnetlist["modules"]){
            for(const nlohmann::json & port_json : module["ports"]){
                const std::string& direction = port_json["direction"].get<std::string>();
                if (direction == "input") for( const int & bit: port_json["bits"].get<std::vector<int>>()) input_vector.push_back(bit);
                else if (direction == "output") for( const int & bit: port_json["bits"].get<std::vector<int>>()) output_vector.push_back(bit);
                else{std::cout<<"Port Definition Error"<<std::endl;throw 1;}
            }

            for(const nlohmann::json & cell_json : module["cells"]){
                std::string gate_name = cell_json["type"].get<std::string>();
                gate_name.erase(gate_name.begin(), gate_name.begin() + 2);
                gate_name.pop_back();
                if(gate_name == "ANDNOT") gate_name = "ANDYN"; //Yosys"s ANDNOT is equivalent to TFHE"s ANDYN
                else if(gate_name == "ORNOT") gate_name = "ORYN"; //Yosys"s ORNOT is equivalent to TFHE"s ORYN

                if(gate_name == "DFF_P"){
                    const uint input_bit = cell_json["connections"]["D"][0].get<uint>();
                    const uint output_bit = cell_json["connections"]["Q"][0].get<uint>();
                    input_vector.push_back(input_bit);
                    output_vector.push_back(output_bit);
                    std::array<uint,2> dff_pair = {input_bit,output_bit};
                    DFF_vector.push_back(dff_pair);
                }
                else{
                    if(gate_vector.size()<gate_index)gate_vector.resize(gate_index);
                    gate_vector[gate_index-1].name = gate_name;

                    const uint output_bit = cell_json["connections"]["Y"][0].get<uint>();
                    gate_vector[gate_index-1].out = output_bit;
                    if(std::find(output_vector.begin(),output_vector.end(),output_bit)==output_vector.end() && std::find(wire_vector.begin(),wire_vector.end(),output_bit)==wire_vector.end()) wire_vector.push_back(output_bit);
                    if(dependency_vector.size()<=output_bit)dependency_vector.resize(output_bit+1);
                    dependency_vector[output_bit]=gate_index;
                    gate_vector[gate_index-1].in[0] = cell_json["connections"]["A"][0].get<uint>();
                    if (gate_name!="NOT") {
                        gate_vector[gate_index-1].in[1] = cell_json["connections"]["B"][0].get<uint>();
                        if  (gate_name=="MUX") gate_vector[gate_index-1].in[2] = cell_json["connections"]["S"][0].get<uint>();
                    }
                    gate_index++;
                }
            }
        }
        //for convinience
        std::sort(input_vector.begin(),input_vector.end());
        std::sort(output_vector.begin(),output_vector.end());
        std::sort(wire_vector.begin(),wire_vector.end());
    }
}