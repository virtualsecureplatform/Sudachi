#pragma once
#include <tfhe++.hpp>

#include "yosys-json-parser.hpp"

namespace Sudachi {

std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::iterator outsearchiterator(
    const uint id, const YosysJSONparser::ParsedBC& BCnetlist,
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherdffd,
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherwire,
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherout,
    const std::string bitname)
{
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::iterator res;

    const std::vector<uint>::const_iterator outiterator = std::find(
        BCnetlist.output_vector.begin(), BCnetlist.output_vector.end(), id);
    const std::vector<uint>::const_iterator initerator = std::find(
        BCnetlist.input_vector.begin(), BCnetlist.input_vector.end(), id);
    const std::vector<uint>::const_iterator dffditerator = std::find(
        BCnetlist.DFF_D_vector.begin(), BCnetlist.DFF_D_vector.end(), id);
    const std::vector<uint>::const_iterator wireiterator = std::find(
        BCnetlist.wire_vector.begin(), BCnetlist.wire_vector.end(), id);

    if (dffditerator != BCnetlist.DFF_D_vector.end())
        res = cipherdffd.begin() +
              std::distance(BCnetlist.DFF_D_vector.begin(), dffditerator);
    else if (wireiterator != BCnetlist.wire_vector.end())
        res = cipherwire.begin() +
              std::distance(BCnetlist.wire_vector.begin(), wireiterator);
    else if (outiterator != BCnetlist.output_vector.end())
        res = cipherout.begin() +
              std::distance(BCnetlist.output_vector.begin(), outiterator);
    else
        std::cout << bitname << " bit parse error" << std::endl;
    return res;
}

std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::const_iterator insearchiterator(
    const uint id, const YosysJSONparser::ParsedBC& BCnetlist,
    const std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherin,
    const std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherdffq,
    const std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherwire,
    const std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>& cipherout,
    const std::string bitname)
{
    std::vector<TFHEpp::TLWE<TFHEpp::lvl1param>>::const_iterator res;
    const std::vector<uint>::const_iterator outiterator = std::find(
        BCnetlist.output_vector.begin(), BCnetlist.output_vector.end(), id);
    const std::vector<uint>::const_iterator initerator = std::find(
        BCnetlist.input_vector.begin(), BCnetlist.input_vector.end(), id);
    const std::vector<uint>::const_iterator dffqiterator = std::find(
        BCnetlist.DFF_Q_vector.begin(), BCnetlist.DFF_Q_vector.end(), id);
    const std::vector<uint>::const_iterator wireiterator = std::find(
        BCnetlist.wire_vector.begin(), BCnetlist.wire_vector.end(), id);
    if (initerator != BCnetlist.input_vector.end())
        res = cipherin.begin() +
              std::distance(BCnetlist.input_vector.begin(), initerator);
    else if (dffqiterator != BCnetlist.DFF_Q_vector.end())
        res = cipherdffq.begin() +
              std::distance(BCnetlist.DFF_Q_vector.begin(), dffqiterator);
    else if (wireiterator != BCnetlist.wire_vector.end())
        res = cipherwire.begin() +
              std::distance(BCnetlist.wire_vector.begin(), wireiterator);
    else if (outiterator != BCnetlist.output_vector.end())
        res = cipherout.begin() +
              std::distance(BCnetlist.output_vector.begin(), outiterator);
    else
        std::cout << bitname << " bit parse error" << std::endl;
    return res;
}
}  // namespace Sudachi