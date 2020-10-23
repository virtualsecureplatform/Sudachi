#include <tfhe++.hpp>
#include <taskflow/taskflow.hpp>
#include <compile_time_crc.hpp>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include<functional>
#include <chrono>
#include <yosys-json-parser.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>

int main (int argc, char* argv[]){
    std::chrono::system_clock::time_point start, end; 
    start = std::chrono::system_clock::now();

    //Parse JSON netlist
    std::ifstream ifs("./circuit.json",std::ios::in);
    if (ifs.fail()) {
        std::cerr << "failed to read json file" << std::endl;
        return 1;
    }
    const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    const YosysJSONparser::ParsedBC BCnetlist(json);

    // Allocate vectors
    std::vector<TFHEpp::TLWElvl0> cipherin(BCnetlist.input_vector.size());
    std::vector<TFHEpp::TLWElvl0> cipherout(BCnetlist.input_vector.size());
    std::vector<TFHEpp::TLWElvl0> cipherwire(BCnetlist.wire_vector.size());

    {
        std::ifstream ifs("./cloud.data", std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(cipherin);
    }

    TFHEpp::GateKey gk;

    //reads the cloud key from file
    {
        const std::string  path = "./gate.key";
        std::ifstream ifs("./gate.key" , std::ios::in | std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        gk.serialize(ar);
    }

    //Generaete task network for Cpp Taskflow
    tf::Executor executor;
    tf::Taskflow taskflow;
    std::vector<tf::Task> gatetasknet(BCnetlist.gate_vector.size());

    for(int gate_index = 1; gate_index<=BCnetlist.gate_vector.size();gate_index++){
        const YosysJSONparser::GateStruct& gate = BCnetlist.gate_vector[gate_index-1];
        const std::vector<uint>::const_iterator outiterator =  std::find(BCnetlist.output_vector.begin(),BCnetlist.output_vector.end(),gate.out);
        std::unique_ptr<TFHEpp::TLWElvl0> outcipher;
        if(outiterator != BCnetlist.output_vector.end()) outcipher = std::make_unique<TFHEpp::TLWElvl0>(cipherout[std::distance(BCnetlist.output_vector.begin(),outiterator)]);
        else outcipher = std::make_unique<TFHEpp::TLWElvl0>(cipherwire[std::distance(BCnetlist.wire_vector.begin(),std::find(BCnetlist.wire_vector.begin(),BCnetlist.wire_vector.end(),gate.out))]);

        const std::vector<uint>::const_iterator initerator0 =  std::find(BCnetlist.input_vector.begin(),BCnetlist.input_vector.end(),gate.in[0]);
        std::unique_ptr<TFHEpp::TLWElvl0> inacipher;
        if(initerator0 != BCnetlist.input_vector.end()) inacipher = std::make_unique<TFHEpp::TLWElvl0>(cipherin[std::distance(BCnetlist.input_vector.begin(),initerator0)]);
        else inacipher = std::make_unique<TFHEpp::TLWElvl0>(cipherwire[std::distance(BCnetlist.wire_vector.begin(),std::find(BCnetlist.wire_vector.begin(),BCnetlist.wire_vector.end(),gate.in[0]))]);

        if(gate.name == "NOT") {
            gatetasknet[gate_index-1] = taskflow.emplace([&](){ TFHEpp::HomNOT(*outcipher,*inacipher);});
            continue;
        }
        const std::vector<uint>::const_iterator initerator1 =  std::find(BCnetlist.input_vector.begin(),BCnetlist.input_vector.end(),gate.in[1]);
        std::unique_ptr<TFHEpp::TLWElvl0> inbcipher;
        if(initerator1 != BCnetlist.input_vector.end()) inbcipher = std::make_unique<TFHEpp::TLWElvl0>(cipherin[std::distance(BCnetlist.input_vector.begin(),initerator1)]);
        else inbcipher = std::make_unique<TFHEpp::TLWElvl0>(cipherwire[std::distance(BCnetlist.wire_vector.begin(),std::find(BCnetlist.wire_vector.begin(),BCnetlist.wire_vector.end(),gate.in[1]))]);
        if(gate.name!="MUX"){
            std::function<void(TFHEpp::TLWElvl0&,const TFHEpp::TLWElvl0&,const TFHEpp::TLWElvl0&,const TFHEpp::GateKey&)> gate_func;
            const char* charname = gate.name.c_str();
            switch (crcdetail::compute(charname, sizeof(charname)-1))
            {
            case CRC32_STR("NAND"):
                gate_func = TFHEpp::HomNAND;
                break;
            case CRC32_STR("NOR"):
                gate_func = TFHEpp::HomNOR;
                break;
            case CRC32_STR("XNOR"):
                gate_func = TFHEpp::HomXNOR;
                break;
            case CRC32_STR("AND"):
                gate_func = TFHEpp::HomAND;
                break;
            case CRC32_STR("OR"):
                gate_func = TFHEpp::HomOR;
                break;
            case CRC32_STR("XOR"):
                gate_func = TFHEpp::HomXOR;
                break;
            case CRC32_STR("ANDYN"):
                gate_func = TFHEpp::HomANDYN;
                break;
            case CRC32_STR("ORYN"):
                gate_func = TFHEpp::HomORYN;
                break;
            
            default:
                break;
            }
            gatetasknet[gate_index-1] = taskflow.emplace([&](){ gate_func(*outcipher,*inacipher,*inbcipher,gk);});
        }else{
            const std::vector<uint>::const_iterator initerator2 =  std::find(BCnetlist.input_vector.begin(),BCnetlist.input_vector.end(),gate.in[2]);
            std::unique_ptr<TFHEpp::TLWElvl0> inscipher;
            if(initerator2 != BCnetlist.input_vector.end()) inscipher = std::make_unique<TFHEpp::TLWElvl0>(cipherin[std::distance(BCnetlist.input_vector.begin(),initerator2)]);
            else inscipher = std::make_unique<TFHEpp::TLWElvl0>(cipherwire[std::distance(BCnetlist.wire_vector.begin(),std::find(BCnetlist.wire_vector.begin(),BCnetlist.wire_vector.end(),gate.in[2]))]);
            gatetasknet[gate_index-1] = taskflow.emplace([&](){ TFHEpp::HomMUX(*outcipher,*inscipher,*inbcipher,*inacipher,gk);});
        }
    }
    
    for(int gate_index = 1; gate_index<=BCnetlist.gate_vector.size();gate_index++){
        const YosysJSONparser::GateStruct& gate = BCnetlist.gate_vector[gate_index-1];
        int num_input = 2;
        if (gate.name == "NOT") num_input = 1;
        else if (gate.name == "MUX") num_input = 3;
        for(int i = 0;i<num_input;i++){
            const int depend_gate = BCnetlist.dependency_vector[gate.in[i]];
            if(depend_gate!=0) gatetasknet[depend_gate-1].precede(gatetasknet[gate_index-1]);
        }
    }
    
    std::vector<tf::Task> dfftasknet(BCnetlist.DFF_vector.size());
    for(int dff_index = 0; dff_index<BCnetlist.DFF_vector.size();dff_index++){
        uint inindex = std::distance(BCnetlist.input_vector.begin(),std::find(BCnetlist.input_vector.begin(),BCnetlist.input_vector.end(),BCnetlist.DFF_vector[dff_index][0]));
        uint outindex = std::distance(BCnetlist.output_vector.begin(),std::find(BCnetlist.output_vector.begin(),BCnetlist.output_vector.end(),BCnetlist.DFF_vector[dff_index][1]));
        dfftasknet[dff_index] = taskflow.emplace([&cipherin,cipherout,inindex,outindex](){TFHEpp::HomCOPY(cipherin[inindex],cipherout[outindex]);});
        gatetasknet[BCnetlist.dependency_vector[BCnetlist.DFF_vector[dff_index][1]]-1].precede(dfftasknet[dff_index]);
    }

    int number_of_clock;
    if(argc == 1){
        number_of_clock = 1;
    }else{
        number_of_clock = std::atoi(argv[1]);
    }
    executor.run_n(taskflow, number_of_clock).wait();
    
    for(int copy_index = 0; copy_index<BCnetlist.direct_port_pair_vector.size();copy_index++){
        uint inindex = std::distance(BCnetlist.input_vector.begin(),std::find(BCnetlist.input_vector.begin(),BCnetlist.input_vector.end(),BCnetlist.direct_port_pair_vector[copy_index][0]));
        uint outindex = std::distance(BCnetlist.output_vector.begin(),std::find(BCnetlist.output_vector.begin(),BCnetlist.output_vector.end(),BCnetlist.direct_port_pair_vector[copy_index][1]));
        TFHEpp::HomCOPY(cipherout[outindex],cipherin[inindex]);
    }

    //export the result ciphertexts to a file
    {
        std::ofstream ofs{"./result.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(cipherout);
    }

    end = std::chrono::system_clock::now();
    double time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0);
    printf("cloud time %lf[ms]\n", time);
}