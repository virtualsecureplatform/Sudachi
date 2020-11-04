#include <tfhe++.hpp>
#include <fstream>
#include <iostream>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
int main() {

    //reads the cloud key from file
    TFHEpp::SecretKey sk;
    {
        std::ifstream ifs{"secret.key", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        sk.serialize(ar);
    };


    //read the 3 ciphertexts of the result
    std::vector<TFHEpp::TLWElvl0> result;
    {
        std::ifstream ifs{"result.data", std::ios::binary};
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(result);
    };

    //decrypt and print plaintext answer
    std::vector<uint8_t>p = bootsSymDecrypt(result, sk);
    int int_answer = 0;
    for (int i=0; i<p.size(); i++) {
        int_answer += p[i]<<i;
    }
    std::cout<<int_answer<<std::endl;
}