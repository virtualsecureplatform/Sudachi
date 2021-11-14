#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

// Parse JSON netlist
namespace YosysJSONparser {
// Assuming there is a limit of a number of inputs for each gate.
constexpr uint MAX_NUM_INPUT = 3;
constexpr uint MAX_NUM_OUTPUT = 3;

// Holding information of each gate
struct GateStruct {
    std::string name;
    std::array<uint, MAX_NUM_INPUT> in;
    std::array<uint, MAX_NUM_OUTPUT> out;
};

struct ParsedBC {
    std::vector<uint> input_vector;
    std::vector<uint> output_vector;
    std::vector<uint> DFF_Q_vector;
    std::vector<uint> DFF_D_vector;
    std::vector<uint> wire_vector;
    std::vector<GateStruct> gate_vector;
    std::vector<std::array<uint, 2>> direct_port_pair_vector;
    // Because input ports, output ports and wires have unique index, we can
    // hold the information about which gate output to specific wires or output
    // ports by this vector.
    std::vector<uint> dependency_vector;
    ParsedBC(const std::string &);
};

ParsedBC::ParsedBC(const std::string &jsonstring)
{
    uint gate_index = 1;
    const nlohmann::json &jsonnetlist = nlohmann::json::parse(jsonstring);
    for (const nlohmann::json &module : jsonnetlist["modules"]) {
        for (const nlohmann::json &port_json : module["ports"]) {
            const std::string &direction =
                port_json["direction"].get<std::string>();
            if (direction == "input")
                for (const int &bit : port_json["bits"].get<std::vector<int>>())
                    input_vector.push_back(bit);
            else if (direction == "output")
                for (const int &bit : port_json["bits"].get<std::vector<int>>())
                    output_vector.push_back(bit);
            else {
                std::cout << "Port Definition Error" << std::endl;
                throw 1;
            }
        }

        for (const nlohmann::json &cell_json : module["cells"]) {
            std::string gate_name = cell_json["type"].get<std::string>();
            if(gate_name=="$fa") gate_name = "fa";
            else{
                gate_name.erase(gate_name.begin(), gate_name.begin() + 2);
                gate_name.pop_back();
            }
            if (gate_name == "ANDNOT")
                gate_name =
                    "ANDYN";  // Yosys"s ANDNOT is equivalent to TFHE"s ANDYN
            else if (gate_name == "ORNOT")
                gate_name = "ORYN";  // Yosys"s ORNOT is equivalent to TFHE"s
                                     // ORYN

            if (gate_name == "SDFF_PP0") {
                DFF_Q_vector.push_back(
                    cell_json["connections"]["Q"][0].get<uint>());
                DFF_D_vector.push_back(
                    cell_json["connections"]["D"][0].get<uint>());
            }
            else {
                if (gate_vector.size() < gate_index)
                    gate_vector.resize(gate_index);
                {
                    const uint output_bit =
                        cell_json["connections"]["Y"][0].get<uint>();
                    gate_vector[gate_index - 1].out[0] = output_bit;
                    if (std::find(output_vector.begin(), output_vector.end(),
                                output_bit) == output_vector.end() &&
                        std::find(wire_vector.begin(), wire_vector.end(),
                                output_bit) == wire_vector.end())
                        wire_vector.push_back(output_bit);
                    if (dependency_vector.size() <= output_bit)
                        dependency_vector.resize(output_bit + 1);
                    dependency_vector[output_bit] = gate_index;
                }

                gate_vector[gate_index - 1].in[0] =
                    cell_json["connections"]["A"][0].get<uint>();
                if (gate_name != "NOT") {
                    gate_vector[gate_index - 1].in[1] =
                        cell_json["connections"]["B"][0].get<uint>();
                    if (gate_name == "MUX" || gate_name == "NMUX")
                        gate_vector[gate_index - 1].in[2] =
                            cell_json["connections"]["S"][0].get<uint>();
                    else if (gate_name == "fa"){
                        {
                            const uint output_bit =
                            cell_json["connections"]["X"][0].get<uint>();
                            gate_vector[gate_index - 1].out[1] = output_bit;
                            if (std::find(output_vector.begin(), output_vector.end(),
                                        output_bit) == output_vector.end() &&
                                std::find(wire_vector.begin(), wire_vector.end(),
                                        output_bit) == wire_vector.end())
                                wire_vector.push_back(output_bit);
                            if (dependency_vector.size() <= output_bit)
                                dependency_vector.resize(output_bit + 1);
                            dependency_vector[output_bit] = gate_index;
                        }
                        if(cell_json["connections"]["C"][0].is_string()){
                            gate_name = "ha";
                        }else{
                            gate_vector[gate_index - 1].in[2] =
                                cell_json["connections"]["C"][0].get<uint>();
                        }
                    }
                    #ifdef USE_M12
                    else if (gate_name == "AOI3" || gate_name == "OAI3")
                        gate_vector[gate_index - 1].in[2] =
                                    cell_json["connections"]["C"][0].get<uint>();
                    #endif
                }
                gate_vector[gate_index - 1].name = gate_name;
                gate_index++;
            }
        }
    }

    // If DFF's Q port is connected to output, needs copy at last.
    for (int i = 0; i < DFF_Q_vector.size(); i++) {
        const std::vector<uint>::iterator output_itr = std::find(
            output_vector.begin(), output_vector.end(), DFF_Q_vector[i]);
        if (output_itr != output_vector.end())
            direct_port_pair_vector.push_back({DFF_Q_vector[i], *output_itr});
    }

    // for convinience
    std::sort(input_vector.begin(), input_vector.end());
    std::sort(output_vector.begin(), output_vector.end());
    std::sort(wire_vector.begin(), wire_vector.end());
}
}  // namespace YosysJSONparser