#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <ggpinreal.hpp>
#include <vector>

int main(int argc, char** argv)
{

    // generate a random key
    std::unique_ptr<TFHEpp::SecretKey> sk(new TFHEpp::SecretKey);
    TFHEpp::EvalKey ek;
    ek.emplacebkfft<TFHEpp::lvl01param>(*sk);
    ek.emplaceiksk<TFHEpp::lvl10param>(*sk);

    // export the secret key to file for later use
    {
        std::ofstream ofs{"secret.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        sk->serialize(ar);
    };

    // export the cloud key to a file (for the cloud)
    {
        std::ofstream ofs{"eval.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ek.serialize(ar);
    };

    // generate encrypt the input
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    constexpr uint bitwidth = 8;
    std::uniform_int_distribution<uint8_t> inrand(0, (1 << bitwidth) - 1);

    const uint8_t in = inrand(engine);
    std::array<uint8_t,bitwidth> p;
    for (int i = 0; i < bitwidth; i++) p[i] = (in>>i) & 1;

    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> ciphertext =
        TFHEpp::bootsSymEncrypt(p, *sk);

    // export the 2+2*bitwith ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };

}