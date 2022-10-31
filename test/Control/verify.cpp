#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <tfhe++.hpp>
int main()
{
    // reads the cloud key from file
    TFHEpp::SecretKey sk;
    {
        std::ifstream ifs{"secret.key", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        sk.serialize(ar);
    };

    // read the 3 ciphertexts of the result
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> result;
    {
        std::ifstream ifs{"result.data", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(result);
    };

    // decrypt and print plaintext answer
    std::vector<uint8_t> p =
        TFHEpp::bootsSymDecrypt<TFHEpp::lvl1param>(result, sk);
    std::cout<<"pc_sel: " << p[0] + (p[1]<<1) << std::endl;
}