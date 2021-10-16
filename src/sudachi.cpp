#include <stdio.h>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <taskflow/taskflow.hpp>
#include <tfhe++.hpp>
#include <yosys-json-parser.hpp>

int main(int argc, char *argv[])
{
    // To see performance
    std::chrono::system_clock::time_point start, end;
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
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> cipherin(BCnetlist.input_vector.size());
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> cipherout(BCnetlist.output_vector.size());
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> cipherdffq(BCnetlist.DFF_Q_vector.size());
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> cipherdffd(BCnetlist.DFF_D_vector.size());
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> cipherwire(BCnetlist.wire_vector.size());

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
    for (TFHEpp::TLWE<TFHEpp::lvl1param> &dffq : cipherdffq) TFHEpp::HomCONSTANTZERO(dffq);

    // Generaete tasks for Cpp Taskflow
    for (int gate_index = 1; gate_index <= BCnetlist.gate_vector.size();
         gate_index++) {
        const YosysJSONparser::GateStruct &gate =
            BCnetlist.gate_vector[gate_index - 1];
        std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::iterator outcipher;
        {
            const std::vector<uint>::const_iterator outiterator =
                std::find(BCnetlist.output_vector.begin(),
                        BCnetlist.output_vector.end(), gate.out);
            const std::vector<uint>::const_iterator dffditerator =
                std::find(BCnetlist.DFF_D_vector.begin(),
                        BCnetlist.DFF_D_vector.end(), gate.out);
            const std::vector<uint>::const_iterator wireiterator =
                std::find(BCnetlist.wire_vector.begin(),
                        BCnetlist.wire_vector.end(), gate.out);
            if (dffditerator != BCnetlist.DFF_D_vector.end())
                outcipher =
                    cipherdffd.begin() +
                    std::distance(BCnetlist.DFF_D_vector.begin(), dffditerator);
            else if (wireiterator != BCnetlist.wire_vector.end())
                outcipher =
                    cipherwire.begin() +
                    std::distance(BCnetlist.wire_vector.begin(), wireiterator);
            else if (outiterator != BCnetlist.output_vector.end())
                outcipher =
                    cipherout.begin() +
                    std::distance(BCnetlist.output_vector.begin(), outiterator);
            else
                std::cout << "output bit parse error" << std::endl;
        }
        std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::const_iterator inacipher;
        {
            const std::vector<uint>::const_iterator outiterator0 =
                    std::find(BCnetlist.output_vector.begin(),
                            BCnetlist.output_vector.end(), gate.in[0]);
            const std::vector<uint>::const_iterator initerator0 =
                std::find(BCnetlist.input_vector.begin(),
                        BCnetlist.input_vector.end(), gate.in[0]);
            const std::vector<uint>::const_iterator dffqiterator0 =
                std::find(BCnetlist.DFF_Q_vector.begin(),
                        BCnetlist.DFF_Q_vector.end(), gate.in[0]);
            const std::vector<uint>::const_iterator wireiterator0 =
                std::find(BCnetlist.wire_vector.begin(),
                        BCnetlist.wire_vector.end(), gate.in[0]);
            if (initerator0 != BCnetlist.input_vector.end())
                inacipher =
                    cipherin.begin() +
                    std::distance(BCnetlist.input_vector.begin(), initerator0);
            else if (dffqiterator0 != BCnetlist.DFF_Q_vector.end())
                inacipher =
                    cipherdffq.begin() +
                    std::distance(BCnetlist.DFF_Q_vector.begin(), dffqiterator0);
            else if (wireiterator0 != BCnetlist.wire_vector.end())
                inacipher =
                    cipherwire.begin() +
                    std::distance(BCnetlist.wire_vector.begin(), wireiterator0);
            else if (outiterator0 != BCnetlist.output_vector.end())
                inacipher =
                    cipherout.begin() +
                    std::distance(BCnetlist.output_vector.begin(), outiterator0);
            else
                std::cout << "in0 bit parse error" << std::endl;
        }

        if (gate.name == "NOT") {
            // 1 input gate
            gatetasknet[gate_index - 1] = taskflow.emplace(
                [=]() { TFHEpp::HomNOT(*outcipher, *inacipher); });
            continue;
        }
        std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::const_iterator inbcipher;
        {
            const std::vector<uint>::const_iterator outiterator1 =
                std::find(BCnetlist.output_vector.begin(),
                        BCnetlist.output_vector.end(), gate.in[1]);
            const std::vector<uint>::const_iterator initerator1 =
                std::find(BCnetlist.input_vector.begin(),
                        BCnetlist.input_vector.end(), gate.in[1]);
            const std::vector<uint>::const_iterator dffqiterator1 =
                std::find(BCnetlist.DFF_Q_vector.begin(),
                        BCnetlist.DFF_Q_vector.end(), gate.in[1]);
            const std::vector<uint>::const_iterator wireiterator1 =
                std::find(BCnetlist.wire_vector.begin(),
                        BCnetlist.wire_vector.end(), gate.in[1]);
            if (initerator1 != BCnetlist.input_vector.end())
                inbcipher =
                    cipherin.begin() +
                    std::distance(BCnetlist.input_vector.begin(), initerator1);
            else if (dffqiterator1 != BCnetlist.DFF_Q_vector.end())
                inbcipher =
                    cipherdffq.begin() +
                    std::distance(BCnetlist.DFF_Q_vector.begin(), dffqiterator1);
            else if (wireiterator1 != BCnetlist.wire_vector.end())
                inbcipher =
                    cipherwire.begin() +
                    std::distance(BCnetlist.wire_vector.begin(), wireiterator1);
            else if (outiterator1 != BCnetlist.output_vector.end())
                inbcipher =
                    cipherout.begin() +
                    std::distance(BCnetlist.output_vector.begin(), outiterator1);
            else
                std::cout << "in1 bit parse error :"<< gate.in[1] << std::endl;
        }
        if (gate.name != "MUX" && gate.name != "NMUX") {
            // 2 input gates
            if (gate.name == "NAND")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNAND(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "NOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNOR(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "XNOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomXNOR(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "AND")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomAND(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "OR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomOR(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "XOR")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomXOR(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "ANDYN")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomANDYN(*outcipher, *inacipher, *inbcipher, ek);
                });
            else if (gate.name == "ORYN")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomORYN(*outcipher, *inacipher, *inbcipher, ek);
                });
            else {
                std::cout << "GATE PARSE ERROR" << std::endl;
                std::cout << gate.name << std::endl;
                continue;
            }
        }
        else {
            // 3 input gate, MUX or NMUX
            std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::const_iterator inscipher;
            {
                const std::vector<uint>::const_iterator outiterator2 =
                    std::find(BCnetlist.output_vector.begin(),
                            BCnetlist.output_vector.end(), gate.in[2]);
                const std::vector<uint>::const_iterator initerator2 =
                    std::find(BCnetlist.input_vector.begin(),
                            BCnetlist.input_vector.end(), gate.in[2]);
                const std::vector<uint>::const_iterator dffqiterator2 =
                    std::find(BCnetlist.DFF_Q_vector.begin(),
                            BCnetlist.DFF_Q_vector.end(), gate.in[2]);
                const std::vector<uint>::const_iterator wireiterator2 =
                    std::find(BCnetlist.wire_vector.begin(),
                            BCnetlist.wire_vector.end(), gate.in[2]);
                if (initerator2 != BCnetlist.input_vector.end())
                    inscipher =
                        cipherin.begin() +
                        std::distance(BCnetlist.input_vector.begin(), initerator2);
                else if (dffqiterator2 != BCnetlist.DFF_Q_vector.end())
                    inscipher = cipherdffq.begin() +
                                std::distance(BCnetlist.DFF_Q_vector.begin(),
                                            dffqiterator2);
                else if (wireiterator2 != BCnetlist.wire_vector.end())
                    inscipher =
                        cipherwire.begin() +
                        std::distance(BCnetlist.wire_vector.begin(), wireiterator2);
                else if (outiterator2 != BCnetlist.output_vector.end())
                    inscipher =
                        cipherout.begin() +
                        std::distance(BCnetlist.output_vector.begin(), outiterator2);
                else
                    std::cout << "ins bit parse error" << std::endl;
            }
            if (gate.name == "MUX")
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomMUX(*outcipher, *inscipher, *inbcipher, *inacipher,
                                ek);
                });
            else
                gatetasknet[gate_index - 1] = taskflow.emplace([=, &ek]() {
                    TFHEpp::HomNMUX(*outcipher, *inscipher, *inbcipher, *inacipher,
                                ek);
                });
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
        else if (gate.name == "MUX" || gate.name == "NMUX")
            num_input = 3;
        for (int i = 0; i < num_input; i++) {
            const int depend_gate = BCnetlist.dependency_vector[gate.in[i]];
            if (depend_gate != 0)
                gatetasknet[depend_gate - 1].precede(
                    gatetasknet[gate_index - 1]);
        }
        if (std::find(BCnetlist.output_vector.begin(),
                      BCnetlist.output_vector.end(),
                      gate.out) != BCnetlist.output_vector.end() ||
            std::find(BCnetlist.DFF_D_vector.begin(),
                      BCnetlist.DFF_D_vector.end(),
                      gate.out) != BCnetlist.DFF_D_vector.end())
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
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count() /
        1000.0);
    printf("cloud time %lf[ms]\n", time);
}