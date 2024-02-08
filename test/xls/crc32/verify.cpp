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
    constexpr uint bitwidth = 32;
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
            for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) std::cout<<pres[j]?1:0;
            std::cout<<std::endl;
            std::cout<<"Correct"<<std::endl;
            for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) std::cout<<(iolabels.output_labels[0][j]?1:0);
            std::cout<<std::endl;
            for(int j = 0; j <= shortiksP::domainP::k*shortiksP::domainP::n; j++) std::cout<<(iolabels.output_labels[i][j]?1:0);
            std::cout<<std::endl;
            exit(1);
        }
    }
    
    uint32_t out = 0;
    for(int j = 0; j < 32; j++) out |= p[j] << j;
    std::cout<<"out:"<<std::hex<<out<<std::endl;
}