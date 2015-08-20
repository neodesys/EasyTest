/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "../include/easyTest.h"

#ifdef _MSC_VER //Visual C++ compiler
#pragma warning(disable:4309)
//WARNING: this file must include the UTF-8 BOM (0xEF 0xBB 0xBF) at the
//beginning in order to be correctly compiled by the Visual C++ compiler.
//If this BOM is not present, Microsoft compiler will misinterpret UTF-8
//literal characters which leads to tests failure because of incompatible
//encodings in wide/utf8/utf16/utf32 literal strings.
#endif //_MSC_VER

TEST_SUITE(DataCompare)
{
	TEST_CASE(assertStringsEqual_ascii);
	TEST_CASE(assertStringsEqual_wide);
	TEST_CASE(assertStringsEqual_utf8);
	TEST_CASE(assertStringsEqual_utf16);
	TEST_CASE(assertStringsEqual_utf32);

	TEST_CASE(assertStringsDifferent_ascii);
	TEST_CASE(assertStringsDifferent_wide);
	TEST_CASE(assertStringsDifferent_utf8);
	TEST_CASE(assertStringsDifferent_utf16);
	TEST_CASE(assertStringsDifferent_utf32);

	TEST_CASE(assertBitwiseEqual);
	TEST_CASE(assertBitwiseDifferent);

	TEST_CASE(assertSameData);
	TEST_CASE(assertDifferentData);

private:
	const char* const asciiStrA = "az";
	const char asciiStrB[3] = {0x61, 0x7A, 0x00};
	const char* const asciiStrC = "ab";

	const wchar_t* const wideStrA = L"éà";
	const wchar_t wideStrB[3] = {0xE9, 0xE0, 0x00};
	const wchar_t* const wideStrC = L"éè";

	const char* const utf8StrA = u8"éà";
	const char utf8StrB[5] = {static_cast<char>(0xC3), static_cast<char>(0xA9), static_cast<char>(0xC3), static_cast<char>(0xA0), 0x00};
	const char* const utf8StrC = u8"éè";

	const char16_t* const utf16StrA = u"éà";
	const char16_t utf16StrB[3] = {0xE9, 0xE0, 0x00};
	const char16_t* const utf16StrC = u"éè";

	const char32_t* const utf32StrA = U"éà";
	const char32_t utf32StrB[3] = {0xE9, 0xE0, 0x00};
	const char32_t* const utf32StrC = U"éè";

	const short a = -1;
	const int b = -1;
	const unsigned short c = 65535;

	const short bufferA[6] = {-1, -2, -3, -4, -5, -6};
	const short bufferB[6] = {-1, -2, -2, -4, -5, -6};
	const unsigned short bufferC[6] = {65535, 65534, 65533, 65532, 65531, 65530};
	const size_t sizeInBytes = sizeof(bufferA);
};

TEST_IMPL(DataCompare, assertStringsEqual_ascii)
{
	ASSERT_STRING_EQUAL(asciiStrA, asciiStrB);
	ASSERT_TRUE(!areStringEqual(asciiStrA, asciiStrC, "asciiStrA", "asciiStrC", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsEqual_wide)
{
	ASSERT_STRING_EQUAL(wideStrA, wideStrB);
	ASSERT_TRUE(!areStringEqual(wideStrA, wideStrC, "wideStrA", "wideStrC", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsEqual_utf8)
{
	ASSERT_STRING_EQUAL(utf8StrA, utf8StrB);
	ASSERT_TRUE(!areStringEqual(utf8StrA, utf8StrC, "utf8StrA", "utf8StrC", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsEqual_utf16)
{
	ASSERT_STRING_EQUAL(utf16StrA, utf16StrB);
	ASSERT_TRUE(!areStringEqual(utf16StrA, utf16StrC, "utf16StrA", "utf16StrC", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsEqual_utf32)
{
	ASSERT_STRING_EQUAL(utf32StrA, utf32StrB);
	ASSERT_TRUE(!areStringEqual(utf32StrA, utf32StrC, "utf32StrA", "utf32StrC", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsDifferent_ascii)
{
	ASSERT_STRING_DIFFERENT(asciiStrA, asciiStrC);
	ASSERT_TRUE(!areStringDifferent(asciiStrA, asciiStrB, "asciiStrA", "asciiStrB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsDifferent_wide)
{
	ASSERT_STRING_DIFFERENT(wideStrA, wideStrC);
	ASSERT_TRUE(!areStringDifferent(wideStrA, wideStrB, "wideStrA", "wideStrB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsDifferent_utf8)
{
	ASSERT_STRING_DIFFERENT(utf8StrA, utf8StrC);
	ASSERT_TRUE(!areStringDifferent(utf8StrA, utf8StrB, "utf8StrA", "utf8StrB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsDifferent_utf16)
{
	ASSERT_STRING_DIFFERENT(utf16StrA, utf16StrC);
	ASSERT_TRUE(!areStringDifferent(utf16StrA, utf16StrB, "utf16StrA", "utf16StrB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertStringsDifferent_utf32)
{
	ASSERT_STRING_DIFFERENT(utf32StrA, utf32StrC);
	ASSERT_TRUE(!areStringDifferent(utf32StrA, utf32StrB, "utf32StrA", "utf32StrB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertBitwiseEqual)
{
	ASSERT_EQUAL(static_cast<int>(a), b);
	ASSERT_TRUE(!areBitwiseEqual(a, b, "a", "b", SRC_INFO()));
	ASSERT_DIFFERENT(static_cast<int>(a), static_cast<int>(c));
	ASSERT_BITWISE_EQUAL(a, c);
	return true;
}

TEST_IMPL(DataCompare, assertBitwiseDifferent)
{
	ASSERT_EQUAL(static_cast<int>(a), b);
	ASSERT_BITWISE_DIFFERENT(a, b);
	ASSERT_DIFFERENT(static_cast<int>(a), static_cast<int>(c));
	ASSERT_TRUE(!areBitwiseDifferent(a, c, "a", "c", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertSameData)
{
	ASSERT_SAME_DATA(bufferA, bufferC, sizeInBytes);
	ASSERT_TRUE(!haveSameData(bufferA, bufferB, sizeInBytes, "bufferA", "bufferB", SRC_INFO()));
	return true;
}

TEST_IMPL(DataCompare, assertDifferentData)
{
	ASSERT_DIFFERENT_DATA(bufferA, bufferB, sizeInBytes);
	ASSERT_TRUE(!haveDifferentData(bufferA, bufferC, sizeInBytes, "bufferA", "bufferC", SRC_INFO()));
	return true;
}
