#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <vector>

int main()
{
    #ifdef USE_M12
    using P = TFHEpp::lvlMparam;
    #else
    using P = TFHEpp::lvl1param;
    #endif

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
    constexpr uint bitwidth = 16;
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::uniform_int_distribution<> inrand(0, (1 << bitwidth) - 1);
    int ina = inrand(engine);
    int inb = inrand(engine);
    std::cout << "A:" << ina << std::endl;
    std::cout << "B:" << inb << std::endl;
    std::cout << "A+B:" << ina + inb << std::endl;
    std::vector<uint8_t> p(2 + 2 * bitwidth);
    for (int i = 0; i < bitwidth; i++) p[i + 2] = (ina >> i) & 1;
    for (int i = 0; i < bitwidth; i++) p[i + 2 + bitwidth] = (inb >> i) & 1;
    std::vector<TFHEpp::TLWE<P>> ciphertext =
        TFHEpp::bootsSymEncrypt<P>(p, *sk);

    // export the 2+2*bitwith ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}