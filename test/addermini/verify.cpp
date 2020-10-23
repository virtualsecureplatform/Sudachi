#include <tfhe++.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
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
    int16_t int_answer = 0;
    for (int i=0; i<3; i++) {
        int int_answer = p[i]<<i;
    }
    std::cout<<int_answer<<std::endl;
}