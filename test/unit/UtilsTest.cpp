/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include "gtest/gtest.h"

#include <sstream>

#include <pdal/Utils.hpp>

#include <vector>

using namespace pdal;

TEST(UtilsTest, test_random)
{
    const double rangeMin = 0.0;
    const double rangeMax = 100.0;
    const double avg = (rangeMax - rangeMin) / 2.0;
    const int iters = 1000;

    Utils::random_seed(17);

    // make sure we treat the bounds as inclusive
    double sum=0;
    for (int i=0; i<iters; i++)
    {
        const double x = Utils::random(rangeMin, rangeMax);
        EXPECT_TRUE(x >= rangeMin);
        EXPECT_TRUE(x <= rangeMax);

        sum += x;
    }

    sum = sum / iters;

    EXPECT_TRUE(sum <= avg + 0.1*avg);
    EXPECT_TRUE(sum >= avg - 0.1*avg);
}


TEST(UtilsTest, test_comparators)
{
    bool ok;

    {
        ok = Utils::compare_distance<float>(1.000001f, 1.0f);
        EXPECT_TRUE(!ok);

        ok = Utils::compare_distance<float>(1.0000001f, 1.0f);
        EXPECT_TRUE(ok);

        ok = Utils::compare_distance<float>(1.00000001f, 1.0f);
        EXPECT_TRUE(ok);
    }

    {
        ok = Utils::compare_approx<float>(1.001f, 1.0f, 0.0001f);
        EXPECT_TRUE(!ok);

        ok = Utils::compare_approx<float>(1.001f, 1.0f, 0.001f);
        EXPECT_TRUE(!ok);

        ok = Utils::compare_approx<float>(1.001f, 1.0f, 0.01f);
        EXPECT_TRUE(ok);

        ok = Utils::compare_approx<float>(1.001f, 1.0f, 0.1f);
        EXPECT_TRUE(ok);
    }

    {
        ok = Utils::compare_approx<unsigned int>(10, 12, 2);
        EXPECT_TRUE(ok);

        ok = Utils::compare_approx<unsigned int>(10, 12, 3);
        EXPECT_TRUE(ok);

        ok = Utils::compare_approx<unsigned int>(10, 12, 1);
        EXPECT_TRUE(!ok);
    }
}


TEST(UtilsTest, test_base64)
{
    std::vector<uint8_t> data;
    for (int i=0; i<2; i++) data.push_back((uint8_t)i);

    uint32_t begin_size(0);
    for (std::vector<uint8_t>::size_type i = 0; i < data.size(); ++i)
    {
        begin_size = begin_size + data[i];
    }

    std::string encoded = Utils::base64_encode(data);
    std::vector<uint8_t> decoded = Utils::base64_decode(encoded);

    uint32_t size(0);
    for (std::vector<uint8_t>::size_type i = 0; i < decoded.size(); ++i)
    {
        size = size + decoded[i];
    }

    EXPECT_EQ(decoded.size(), data.size());
    EXPECT_EQ(size, begin_size);
}

TEST(UtilsTest, split)
{
    std::vector<std::string> result;

    std::string input("This is a test");
    auto pred = [](char c)
        { return c == ' '; };

    result = Utils::split(input, pred);
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");

    input = "  This  is a test  ";

    result = Utils::split(input, pred);
    EXPECT_EQ(result.size(), 9U);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "This");
    EXPECT_EQ(result[3], "");
    EXPECT_EQ(result[4], "is");
    EXPECT_EQ(result[5], "a");
    EXPECT_EQ(result[6], "test");
    EXPECT_EQ(result[7], "");
    EXPECT_EQ(result[8], "");
}

TEST(UtilsTest, splitChar)
{
    std::vector<std::string> result;

    std::string input("This is a test");

    result = Utils::split(input, ' ');
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");

    input = "  This  is a test  ";

    result = Utils::split(input, ' ');
    EXPECT_EQ(result.size(), 9U);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "This");
    EXPECT_EQ(result[3], "");
    EXPECT_EQ(result[4], "is");
    EXPECT_EQ(result[5], "a");
    EXPECT_EQ(result[6], "test");
    EXPECT_EQ(result[7], "");
    EXPECT_EQ(result[8], "");
}

TEST(UtilsTest, split2)
{
    std::vector<std::string> result;

    std::string input("This is a test");
    auto pred = [](char c)
        { return c == ' '; };

    result = Utils::split2(input, pred);
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");

    input = "  This  is a test  ";

    result = Utils::split2(input, pred);
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");

    auto pred2 = [](char c)
        { return c == ' ' || c == ','; };

    input = " , This,is ,a test , ";

    result = Utils::split2(input, pred2);
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");
}

TEST(UtilsTest, split2Char)
{
    std::vector<std::string> result;

    std::string input(",,This,is,,a,test,,,");

    result = Utils::split2(input, ',');
    EXPECT_EQ(result.size(), 4U);
    EXPECT_EQ(result[0], "This");
    EXPECT_EQ(result[1], "is");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "test");
}

