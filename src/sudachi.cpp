#include <stdio.h>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <sudachi-utils.hpp>
#include <taskflow/taskflow.hpp>
#include <tfhe++.hpp>
#include <yosys-json-parser.hpp>

using namespace Sudachi;

int main(int argc, char *argv[])
{
    #ifdef USE_M12
    using P = TFHEpp::lvlMparam;
    #else
    using P = TFHEpp::lvl1param;
    #endif

    // To see performance
    std::chrono::system_clock::time_point start, init, end;
    start = std::chrono::system_clock::now();

    // Parse JSON netlist
    std::ifstream ifs("./circuit.json", std::ios::in);
    if (ifs.fail()) {
        std::cerr << "failed to read json file" << std::endl;
        return 1;
    }
    const std::string json((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());
    ifs.close();
    const YosysJSONparser::ParsedBC BCnetlist(json);

    // Allocate vectors which holds ciphertexts
    std::vector<TFHEpp::TLWE<P>> cipherin(
        BCnetlist.input_vector.size());
    std::vector<TFHEpp::TLWE<P>> cipherout(
        BCnetlist.output_vector.size());
    std::vector<TFHEpp::TLWE<P>> cipherdffq(
        BCnetlist.DFF_Q_vector.size());
    std::vector<TFHEpp::TLWE<P>> cipherdffd(
        BCnetlist.DFF_D_vector.size());
    std::vector<TFHEpp::TLWE<P>> cipherwire(
        BCnetlist.wire_vector.size());

    // Read input ciphertexts
    {
        std::ifstream ifs("./cloud.data", std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(cipherin);
    }

    TFHEpp::EvalKey ek;

    // reads the cloud key from file
    {
        const std::string path = "./eval.key";
        std::ifstream ifs("./eval.key", std::ios::in | std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        ek.serialize(ar);
    }

    // These hold infomations about a task network
    tf::Executor executor;
    tf::Taskflow taskflow;
    std::vector<tf::Task> gatetasknet(BCnetlist.gate_vector.size());

    // Initialize registers by 0
    for (TFHEpp::TLWE<P> &dffq : cipherdffq)
        TFHEpp::HomCONSTANTZERO(dffq);

    // Generaete tasks for Cpp Taskflow
    for (int gate_index = 1; gate_index <= BCnetlist.gate_vector.size();
         gate_index++) {
        const YosysJSONparser::GateStruct &gate =
            BCnetlist.gate_vector[gate_index - 1];
        const std::set<std::string> in2out1gates{
            "NAND", "NOR", "XNOR", "AND", "OR", "XOR", "ANDYN", "ORYN"};
        #ifdef USE_M12
        const std::set<std::string> in3out1gates{
            "MUX","NMUX","AOI3","OAI3"};
        #else
        const std::set<std::string> in3out1gates{
            "MUX","NMUX"};
        #endif
        const std::set<std::string> in2out2gates{
            "ha"};
        const std::set<std::string> in3out2gates{
            "fa"};
        if (gate.name == "NOT") {
            // 1 input gate
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher =
                outsearchiterator(gate.out[0], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inacipher =
                    insearchiterator(gate.in[0], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in0");
            gatetasknet[gate_index - 1] = taskflow.emplace(
                [=]() { TFHEpp::HomNOT(*outcipher, *inacipher); });
        }
        else if (in2out1gates.find(gate.name) != in2out1gates.end()) {
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher =
                outsearchiterator(gate.out[0], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inacipher =
                    insearchiterator(gate.in[0], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in0");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inbcipher =
                    insearchiterator(gate.in[1], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in1");
            // 2 input gates
            if (gate.name == "NAND")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNAND<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "NOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNOR<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "XNOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomXNOR<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "AND")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomAND<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "OR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomOR<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "XOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomXOR<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "ANDYN")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomANDYN<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "ORYN")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomORYN<P>(*outcipher, *inacipher, *inbcipher, ek);
                });
            else {
                std::cout << "GATE PARSE ERROR" << std::endl;
                std::cout << gate.name << std::endl;
            }
        }
        else if(in3out1gates.find(gate.name) != in3out1gates.end()){
            // 3 input gate, MUX or NMUX
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher =
                outsearchiterator(gate.out[0], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inacipher =
                    insearchiterator(gate.in[0], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in0");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inbcipher =
                    insearchiterator(gate.in[1], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in1");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inscipher =
                    insearchiterator(gate.in[2], BCnetlist, cipherin,
                                     cipherdffq, cipherwire, cipherout, "in2");
            if (gate.name == "MUX")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomMUX<P>(*outcipher, *inscipher, *inbcipher,
                                   *inacipher, ek);
                });
            else if(gate.name == "NMUX")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNMUX<P>(*outcipher, *inscipher, *inbcipher,
                                    *inacipher, ek);
                });
            #ifdef USE_M12
            else if(gate.name == "AOI3")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomAOI3<P>(*outcipher, *inacipher, *inbcipher,
                                    *inscipher, ek);
                });
                else if(gate.name == "OAI3")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomOAI3<P>(*outcipher, *inacipher, *inbcipher,
                                    *inscipher, ek);
                });
            #endif
        }
        else if(in2out2gates.find(gate.name) != in2out2gates.end()){
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher0 =
                outsearchiterator(gate.out[0], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output0");
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher1 =
                outsearchiterator(gate.out[1], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output1");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inacipher =
                    insearchiterator(gate.in[0], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in0");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inbcipher =
                    insearchiterator(gate.in[1], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in1");
            gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomHalfAdder<P>(*outcipher1, *outcipher0, *inacipher,
                                    *inbcipher, ek);
                    }).name("HA");
        }
        else if(in3out2gates.find(gate.name) != in3out2gates.end()){
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher0 =
                outsearchiterator(gate.out[0], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output0");
            const std::vector<TFHEpp::TLWE<P>>::iterator outcipher1 =
                outsearchiterator(gate.out[1], BCnetlist, cipherdffd, cipherwire,
                                cipherout, "output1");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inacipher =
                    insearchiterator(gate.in[0], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in0");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inbcipher =
                    insearchiterator(gate.in[1], BCnetlist, cipherin, cipherdffq,
                                    cipherwire, cipherout, "in1");
            const std::vector<TFHEpp::TLWE<P>>::const_iterator
                inccipher =
                    insearchiterator(gate.in[2], BCnetlist, cipherin,
                                     cipherdffq, cipherwire, cipherout, "in2");
            #ifdef USE_M12
            gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                TFHEpp::HomFullAdder(*outcipher1, *outcipher0, *inacipher, *inbcipher,
                                *inccipher, ek);
            }).name("FA");
            #else
            gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                TFHEpp::Hom2BRFullAdder(*outcipher1, *outcipher0, *inacipher, *inbcipher,
                                *inccipher, ek);
            }).name("FA");
            #endif
        }
        else{
            std::cout <<"GATE PARSE ERROR!"<<std::endl;
        }
    }

    // Construct task network for Cpp Taskflow
    tf::Task finishtask = taskflow.emplace([]() {
        static uint cycle = 1;
        std::cout << "Finished cycle " << cycle << std::endl;
        cycle++;
    });

    for (int gate_index = 1; gate_index <= BCnetlist.gate_vector.size();
         gate_index++) {
        const YosysJSONparser::GateStruct &gate =
            BCnetlist.gate_vector[gate_index - 1];
        int num_input = 2;
        if (gate.name == "NOT")
            num_input = 1;
        else if (gate.name == "MUX" || gate.name == "NMUX" || gate.name == "fa")
            num_input = 3;
        for (int i = 0; i < num_input; i++) {
            const int depend_gate = BCnetlist.dependency_vector[gate.in[i]];
            if (depend_gate != 0)
                gatetasknet[depend_gate - 1].precede(
                    gatetasknet[gate_index - 1]);
        }
        if (std::find(BCnetlist.output_vector.begin(),
                      BCnetlist.output_vector.end(),
                      gate.out[0]) != BCnetlist.output_vector.end() ||
            std::find(BCnetlist.DFF_D_vector.begin(),
                      BCnetlist.DFF_D_vector.end(),
                      gate.out[0]) != BCnetlist.DFF_D_vector.end())
            gatetasknet[gate_index - 1].precede(finishtask);
        else if (gate.name == "fa" || gate.name == "ha")
            if (std::find(BCnetlist.output_vector.begin(),
                      BCnetlist.output_vector.end(),
                      gate.out[1]) != BCnetlist.output_vector.end() ||
            std::find(BCnetlist.DFF_D_vector.begin(),
                      BCnetlist.DFF_D_vector.end(),
                      gate.out[1]) != BCnetlist.DFF_D_vector.end())
            gatetasknet[gate_index - 1].precede(finishtask);
    }

    std::vector<tf::Task> dfftasknet(BCnetlist.DFF_Q_vector.size());
    for (int dff_index = 0; dff_index < BCnetlist.DFF_Q_vector.size();
         dff_index++) {
        dfftasknet[dff_index] = taskflow.emplace([&, dff_index]() {
            TFHEpp::HomCOPY(cipherdffq[dff_index], cipherdffd[dff_index]);
        });  // Since dff_index is local variable, do copy.

        int depend_gate =
            BCnetlist.dependency_vector[BCnetlist.DFF_D_vector[dff_index]];
        if (depend_gate)
            gatetasknet[depend_gate - 1].precede(
                dfftasknet[dff_index]);  // 0 means N/A
        dfftasknet[dff_index].precede(finishtask);
    }

    // Get how many clock this circuit should be evaluated.
    int number_of_clock;
    if (argc == 1) {
        number_of_clock = 1;
    }
    else {
        number_of_clock = std::atoi(argv[1]);
    }

    init = std::chrono::system_clock::now();
    // taskflow.dump(std::cout);
    std::cout << "Start Evalution" << std::endl;
    executor.run_n(taskflow, number_of_clock).wait();

    for (int copy_index = 0;
         copy_index < BCnetlist.direct_port_pair_vector.size(); copy_index++) {
        const uint inindex = std::distance(
            BCnetlist.DFF_Q_vector.begin(),
            std::find(BCnetlist.DFF_Q_vector.begin(),
                      BCnetlist.DFF_Q_vector.end(),
                      BCnetlist.direct_port_pair_vector[copy_index][0]));
        const uint outindex = std::distance(
            BCnetlist.output_vector.begin(),
            std::find(BCnetlist.output_vector.begin(),
                      BCnetlist.output_vector.end(),
                      BCnetlist.direct_port_pair_vector[copy_index][1]));
        TFHEpp::HomCOPY(cipherout[outindex], cipherdffq[inindex]);
    }

    // export the result ciphertexts to a file
    {
        std::ofstream ofs{"./result.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(cipherout);
    }

    end = std::chrono::system_clock::now();
    double time = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(init - start)
            .count() /
        1000.0);
    printf("init time %lf[ms]\n", time);

    time = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - init)
            .count() /
        1000.0);
    printf("run time %lf[ms]\n", time);

    time = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count() /
        1000.0);
    printf("total time %lf[ms]\n", time);
}