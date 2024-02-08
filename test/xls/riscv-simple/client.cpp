#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
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

    constexpr uint32_t pc = 0;
    constexpr uint32_t ins = 0b00000000000000110000001010010011; //addi t0 t1 0
    std::array<uint32_t,32> regs = {};
    regs[5] = 1; //t0
    regs[6] = 2; //t1
    constexpr uint DMEM_SIZE = 16;
    constexpr std::array<uint8_t, 16> dmem = {};

    constexpr uint bitwidth = 32 + 32 + 32*32 + 8*DMEM_SIZE;
    std::vector<uint8_t> p(bitwidth);
    for (int i = 0; i < 32; i++) p[i] = (pc>>i) & 1;
    for (int i = 0; i < 32; i++) p[i+32] = (ins>>i) & 1;
    for (int i = 0; i < 32; i++) for(int k = 0; k < 32; k++) p[i*32+k+32+32] = (regs[i]>>k) & 1;
    for (int i = 0; i < DMEM_SIZE; i++) for(int k = 0; k < 8; k++) p[i*8+k+32+32+32*32] = (dmem[i]>>k) & 1;
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> ciphertext =
        TFHEpp::bootsSymEncrypt(p, *sk);

    // export the ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };

}