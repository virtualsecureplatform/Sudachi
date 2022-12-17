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
	std::uniform_int_distribution<uint8_t> U8dist(0, UINT8_MAX);

    std::vector<uint8_t> p(256);
	unsigned char plaintext[16];
	unsigned char key[16];
	for(int i = 0; i < 16; i++) plaintext[i] = U8dist(engine);
	for(int i = 0; i < 16; i++) key[i] = U8dist(engine);
    
    for (int i = 0; i < 16; i++) for(int j = 0; j < 8; j++) p[i*8+j] = (plaintext[i] >> j) & 1;
    for (int i = 0; i < 16; i++) for(int j = 0; j < 8; j++) p[128+i*8+j] = (key[i] >> j) & 1;

    // AES aes(AESKeyLength::AES_128);

    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>> ciphertext =
        TFHEpp::bootsSymEncrypt(p, *sk);

    // export the 2+2*bitwith ciphertexts to a file (for the cloud)
    {
        std::ofstream ofs{"cloud.data", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext);
    };
}