#include <tfhe++.hpp>
#include <fstream>
#include <memory>
#include <vector>

int main() {
    //generate a random key
    std::unique_ptr<TFHEpp::SecretKey> sk(new TFHEpp::SecretKey);
    std::unique_ptr<TFHEpp::GateKey> gk(new TFHEpp::GateKey(*sk));

    //export the secret key to file for later use
    {
        std::ofstream ofs{"secret.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        sk->serialize(ar);
    };

    //export the cloud key to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        gk->serialize(ar);
    };
   
    //generate encrypt the input
    constexpr int8_t plaintext = 0b111100;//in_b,in_a,reset,clock
    std::vector<uint8_t> p(6);
    for (int i=0; i<6; i++) {
        p[i] = (plaintext>>i)&1;
    }
    std::vector<TFHEpp::TLWElvl0> ciphertext = TFHEpp::bootsSymEncrypt(p, *sk);

    //export the 6 ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}