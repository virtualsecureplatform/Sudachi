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
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::uniform_int_distribution<> inrand(0, 3);
    int ina = inrand(engine);
    int inb = inrand(engine);
    std::cout << ina << std::endl;
    std::cout << inb << std::endl;
    std::vector<uint8_t> p(6);
    p[2] = ina & 1;
    p[3] = (ina >> 1) & 1;
    p[4] = inb & 1;
    p[5] = (inb >> 1) & 1;
    std::vector<TFHEpp::TLWElvl0> ciphertext = TFHEpp::bootsSymEncrypt(p, *sk);

    // export the 6 ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}