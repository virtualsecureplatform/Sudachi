#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <vector>

int main()
{
    // generate a random key
    std::unique_ptr<TFHEpp::SecretKey> sk(new TFHEpp::SecretKey);
    std::unique_ptr<TFHEpp::GateKey> gk(new TFHEpp::GateKey(*sk));

    // export the secret key to file for later use
    {
        std::ofstream ofs{"secret.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        sk->serialize(ar);
    };

    // export the cloud key to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        gk->serialize(ar);
    };

    // generate encrypt the input
    int16_t plaintext;
    std::cout << "Input amt(0~15)" << std::endl;
    std::cin >> plaintext;
    std::vector<uint8_t> p(6);
    for (int i = 0; i < 4; i++) p[i + 2] = (plaintext >> i) & 1;
    std::vector<TFHEpp::TLWE<TFHEpp::lvl0param>> ciphertext = TFHEpp::bootsSymEncrypt(p, *sk);

    // export the 6 ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}