#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <tfhe++.hpp>
int main()
{
    #ifdef USE_M12
    using P = TFHEpp::lvlMparam;
    #else
    using P = TFHEpp::lvl1param;
    #endif

    // reads the cloud key from file
    TFHEpp::SecretKey sk;
    {
        std::ifstream ifs{"secret.key", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        sk.serialize(ar);
    };

    // read the 3 ciphertexts of the result
    std::vector<TFHEpp::TLWE<P>> result;
    {
        std::ifstream ifs{"result.data", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(result);
    };

    // decrypt and print plaintext answer
    std::vector<uint8_t> p =
        TFHEpp::bootsSymDecrypt<P>(result, sk);
    int int_answer = 0;
    for (int i = 0; i < 17; i++) {
        int_answer += static_cast<int>(p[i]) << i;
    }
    std::cout << int_answer << std::endl;
    std::cout << std::bitset<17>(int_answer) << std::endl;
}