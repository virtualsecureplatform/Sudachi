#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <tfhe++.hpp>
#include <ggpinreal.hpp>
int main()
{
    using shortiksP = GGPinReal::shortlvl2lvl1param;

    // reads the cloud key from file
    TFHEpp::SecretKey sk;
    {
        std::ifstream ifs{"secret.key", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        sk.serialize(ar);
    };

    // read the 3 ciphertexts of the result
    std::vector<GGPinReal::EncryptedLabel<shortiksP>> result;
    {
        std::ifstream ifs{"result.data", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(result);
    };

    GGPinReal::IOlabels iolabels;
    {
        std::ifstream ifs("iolabels.data", std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(iolabels);
    };


    // decrypt and print plaintext answer
    constexpr uint DMEM_SIZE = 16;
    constexpr uint bitwidth = 32+32*31+8*DMEM_SIZE;
    std::vector<bool> p(bitwidth);
    for (int i = 0; i < bitwidth; i++){
        std::array<bool,shortiksP::domainP::k*shortiksP::domainP::n+1> pres;
        for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++){
            pres[j] = TFHEpp::tlweSymDecrypt<typename shortiksP::labelP>(result[i][j],sk.key.get<typename shortiksP::labelP>());
        }
        // for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) std::cout<<pres[j]<<":"<<iolabels.output_labels[i][j]<<":"<<(iolabels.output_labels[i][j]^iolabels.Δ[j])<<std::endl;
        p[i] = true;
        for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) p[i] = p[i] && (pres[j] == iolabels.output_labels[i][j]);
        if(p[i]){ p[i] = false; continue;}
        p[i] = true;
        for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) p[i] = p[i] && (pres[j] == (iolabels.output_labels[i][j]) ^ iolabels.Δ[j]);
        if(p[i]) continue;
        else{
            std::cout<<"Verification ERROR!"<<std::endl;
            std::cout<<i<<std::endl;
            exit(1);
        }
    }
    std::array<uint8_t,DMEM_SIZE> dmem = {};
    for(int i = 0; i < DMEM_SIZE; i++) for(int j = 0; j < 8; j++) dmem[i] |= p[i*8+j]<<j;
    std::array<uint32_t,32> regs = {};
    for(int i = 1; i < 32; i++) for(int j = 0; j < 32; j++) regs[i] |= p[(i-1)*32+j+8*DMEM_SIZE]<<j;
    uint32_t pc = 0;
    for(int i = 0; i < 32; i++) pc |= p[i+31*32+8*DMEM_SIZE]<<i;
    std::cout<<"pc:" << pc<<std::endl;
    for(int i = 0; i < 32; i++)
    std::cout<<"R" <<i<<":"<<regs[i]<<std::endl;  
    for(int i = 0 ; i < DMEM_SIZE ; i++)
    std::cout<<"MEM[" <<i<<"]:"<<static_cast<uint>(dmem[i])<<std::endl;  
}