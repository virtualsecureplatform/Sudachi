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
    constexpr uint bitwidth = 2*2*31;
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
    
    std::array<std::array<float,2>,2> out_img;
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++){
            uint32_t temp = 0;
            for(int bit = 0; bit < 31; bit++) temp |= p[i*2*31+j*31+bit]<<bit;
            out_img[i][j] = reinterpret_cast<float&>(temp);
            std::cout<<i<<":"<<j<<":"<<out_img[i][j]<<std::endl;
        }
}