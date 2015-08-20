/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "../include/easyTest.h"

namespace
{
	class ClassOpTest final
	{
	public:
		ClassOpTest(int i) : m_i(i) {}

		bool operator<(const ClassOpTest& other) const
		{
			return (m_i < other.m_i);
		}

		bool operator<=(const ClassOpTest& other) const
		{
			return (m_i <= other.m_i);
		}

		bool operator>(const ClassOpTest& other) const
		{
			return (m_i > other.m_i);
		}

		bool operator>=(const ClassOpTest& other) const
		{
			return (m_i >= other.m_i);
		}

		bool operator==(const ClassOpTest& other) const
		{
			return (m_i == other.m_i);
		}

		bool operator!=(const ClassOpTest& other) const
		{
			return (m_i != other.m_i);
		}

	private:
		const int m_i;
	};
}

TEST_SUITE(Asserts)
{
	TEST_CASE(assertTrue);
	TEST_CASE(assertFalse);

	TEST_CASE(assertNaN);
	TEST_CASE(assertInfinite);
	TEST_CASE(assertFinite);

	TEST_CASE(assertLessThan_int);
	TEST_CASE(assertLessThan_float);
	TEST_CASE(assertLessThan_classOperator);

	TEST_CASE(assertLessOrEqual_int);
	TEST_CASE(assertLessOrEqual_float);
	TEST_CASE(assertLessOrEqual_classOperator);

	TEST_CASE(assertGreaterThan_int);
	TEST_CASE(assertGreaterThan_float);
	TEST_CASE(assertGreaterThan_classOperator);

	TEST_CASE(assertGreaterOrEqual_int);
	TEST_CASE(assertGreaterOrEqual_float);
	TEST_CASE(assertGreaterOrEqual_classOperator);

	TEST_CASE(assertEqual_int);
	TEST_CASE(assertEqual_float);
	TEST_CASE(assertEqual_classOperator);

	TEST_CASE(assertDifferent_int);
	TEST_CASE(assertDifferent_float);
	TEST_CASE(assertDifferent_classOperator);

	TEST_CASE(assertAlmostEqual);
	TEST_CASE(assertVeryDifferent);

private:
	const bool bTrue = true;
	const bool bFalse = false;

	const double dNaN = NAN;
	const double dUnity = 1.0;
	const double dInfinity = INFINITY;
	const double dNegInfinity = -INFINITY;

	const int ia = 1;
	const int ib = 1;
	const int ic = 2;

	const float fa = 1.0f;
	const float fb = 1.0f;
	const float fc = 1.1f;
	const float fd = 1.01f;

	const float bigEpsilon = 0.01f;
	const float epsilon = 0.001f;

	const ClassOpTest oa = 1;
	const ClassOpTest ob = 1;
	const ClassOpTest oc = 2;
};

TEST_IMPL(Asserts, assertTrue)
{
	ASSERT_TRUE(bTrue);
	ASSERT_TRUE(!isTrue(bFalse, "bFalse", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertFalse)
{
	ASSERT_FALSE(bFalse);
	ASSERT_TRUE(!isFalse(bTrue, "bTrue", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertNaN)
{
	ASSERT_NAN(dNaN);
	ASSERT_TRUE(!isNaN(dUnity, "dUnity", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertInfinite)
{
	ASSERT_INFINITE(dInfinity);
	ASSERT_INFINITE(dNegInfinity);
	ASSERT_TRUE(!isInfinite(dUnity, "dUnity", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertFinite)
{
	ASSERT_FINITE(dUnity);
	ASSERT_TRUE(!isFinite(dNaN, "dNaN", SRC_INFO()));
	ASSERT_TRUE(!isFinite(dInfinity, "dInfinity", SRC_INFO()));
	ASSERT_TRUE(!isFinite(dNegInfinity, "dNegInfinity", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessThan_int)
{
	ASSERT_LESS_THAN(ia, ic);
	ASSERT_TRUE(!isLessThan(ia, ib, "ia", "ib", SRC_INFO()));
	ASSERT_TRUE(!isLessThan(ic, ia, "ic", "ia", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessThan_float)
{
	ASSERT_LESS_THAN(fa, fc);
	ASSERT_TRUE(!isLessThan(fa, fb, "fa", "fb", SRC_INFO()));
	ASSERT_TRUE(!isLessThan(fc, fa, "fc", "fa", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessThan_classOperator)
{
	ASSERT_LESS_THAN(oa, oc);
	ASSERT_TRUE(!isLessThan(oa, ob, "oa", "ob", SRC_INFO()));
	ASSERT_TRUE(!isLessThan(oc, oa, "oc", "oa", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessOrEqual_int)
{
	ASSERT_LESS_OR_EQUAL(ia, ib);
	ASSERT_LESS_OR_EQUAL(ia, ic);
	ASSERT_TRUE(!isLessOrEqual(ic, ia, "ic", "ia", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessOrEqual_float)
{
	ASSERT_LESS_OR_EQUAL(fa, fb);
	ASSERT_LESS_OR_EQUAL(fa, fc);
	ASSERT_TRUE(!isLessOrEqual(fc, fa, "fc", "fa", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertLessOrEqual_classOperator)
{
	ASSERT_LESS_OR_EQUAL(oa, ob);
	ASSERT_LESS_OR_EQUAL(oa, oc);
	ASSERT_TRUE(!isLessOrEqual(oc, oa, "oc", "oa", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterThan_int)
{
	ASSERT_GREATER_THAN(ic, ia);
	ASSERT_TRUE(!isGreaterThan(ib, ia, "ib", "ia", SRC_INFO()));
	ASSERT_TRUE(!isGreaterThan(ia, ic, "ia", "ic", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterThan_float)
{
	ASSERT_GREATER_THAN(fc, fa);
	ASSERT_TRUE(!isGreaterThan(fb, fa, "fb", "fa", SRC_INFO()));
	ASSERT_TRUE(!isGreaterThan(fa, fc, "fa", "fc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterThan_classOperator)
{
	ASSERT_GREATER_THAN(oc, oa);
	ASSERT_TRUE(!isGreaterThan(ob, oa, "ob", "oa", SRC_INFO()));
	ASSERT_TRUE(!isGreaterThan(oa, oc, "oa", "oc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterOrEqual_int)
{
	ASSERT_GREATER_OR_EQUAL(ib, ia);
	ASSERT_GREATER_OR_EQUAL(ic, ia);
	ASSERT_TRUE(!isGreaterOrEqual(ia, ic, "ia", "ic", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterOrEqual_float)
{
	ASSERT_GREATER_OR_EQUAL(fb, fa);
	ASSERT_GREATER_OR_EQUAL(fc, fa);
	ASSERT_TRUE(!isGreaterOrEqual(fa, fc, "fa", "fc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertGreaterOrEqual_classOperator)
{
	ASSERT_GREATER_OR_EQUAL(ob, oa);
	ASSERT_GREATER_OR_EQUAL(oc, oa);
	ASSERT_TRUE(!isGreaterOrEqual(oa, oc, "oa", "oc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertEqual_int)
{
	ASSERT_EQUAL(ia, ib);
	ASSERT_TRUE(!areEqual(ia, ic, "ia", "ic", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertEqual_float)
{
	ASSERT_EQUAL(fa, fb);
	ASSERT_TRUE(!areEqual(fa, fc, "fa", "fc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertEqual_classOperator)
{
	ASSERT_EQUAL(oa, ob);
	ASSERT_TRUE(!areEqual(oa, oc, "oa", "oc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertDifferent_int)
{
	ASSERT_DIFFERENT(ia, ic);
	ASSERT_TRUE(!areDifferent(ia, ib, "ia", "ib", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertDifferent_float)
{
	ASSERT_DIFFERENT(fa, fc);
	ASSERT_TRUE(!areDifferent(fa, fb, "fa", "fb", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertDifferent_classOperator)
{
	ASSERT_DIFFERENT(oa, oc);
	ASSERT_TRUE(!areDifferent(oa, ob, "oa", "ob", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertAlmostEqual)
{
	ASSERT_ALMOST_EQUAL(fa, fd, bigEpsilon);
	ASSERT_TRUE(!areAlmostEqual(fa, fd, epsilon, "fa", "fd", SRC_INFO()));
	ASSERT_TRUE(!areAlmostEqual(fa, fc, bigEpsilon, "fa", "fc", SRC_INFO()));
	return true;
}

TEST_IMPL(Asserts, assertVeryDifferent)
{
	ASSERT_VERY_DIFFERENT(fa, fc, bigEpsilon);
	ASSERT_VERY_DIFFERENT(fa, fd, epsilon);
	ASSERT_TRUE(!areVeryDifferent(fa, fd, bigEpsilon, "fa", "fd", SRC_INFO()));
	return true;
}
