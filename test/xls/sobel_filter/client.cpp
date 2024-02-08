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

    GGPinReal::IOlabels iolabels;
    {
        std::ifstream ifs("iolabels.data", std::ios::binary);
        cereal::PortableBinaryInputArchive ar(ifs);
        ar(iolabels);
    }

    // generate encrypt the input
    std::array<std::array<float, 4>,4> in_img;
    in_img[0][0] = 1;
    in_img[0][1] = 1;
    in_img[0][2] = 1;
    in_img[0][3] = 1;
    in_img[1][0] = 1;
    in_img[1][1] = -1;
    in_img[1][2] = -2;
    in_img[1][3] = 1;
    in_img[2][0] = 1;
    in_img[2][1] = -4;
    in_img[2][2] = -3;
    in_img[2][3] = 1;
    in_img[3][0] = 1;
    in_img[3][1] = 1;
    in_img[3][2] = 1;
    in_img[3][3] = 1;
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());

    constexpr uint bitwidth = 4*4*32;
    std::array<uint8_t,bitwidth> p;
    for (int i = 0; i < 4; i++) for(int j = 0; j < 4; j++) for(int k = 0; k < 32; k++) p[i*4*32+j*32+k] = (reinterpret_cast<uint32_t&>(in_img[i][j])>>k) & 1;
    std::cout<<std::endl;

    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> ciphertext =
        TFHEpp::bootsSymEncrypt(p, *sk);

    // export the ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}