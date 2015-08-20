/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _REGISTRARREFSTORAGE_H_
#define _REGISTRARREFSTORAGE_H_

#include <cstring>

namespace easyTest
{
	template<class C> class RegistrarRefStorage final
	{
	public:
		RegistrarRefStorage() = default;
		~RegistrarRefStorage();

		//Because of noexcept, program will terminate immediately if underlying
		//allocation of storage space fails. This method is called by
		//TestSuiteRegistrar and TestCaseRegistrar constructors during static
		//constants initialization at program start. The purpose is to list
		//automatically all defined test suites and test cases for each test
		//suite. So, it would be useless to try to catch any std::bad_alloc
		//exception at this step as running program would have no meanings
		//without these lists.
		void addRegistrarRef(const C* pRef) noexcept;

		size_t getRegistrarCount() const
		{
			return m_refCount;
		}

		const C** getRegistrarList() const
		{
			return m_refList;
		}

	private:
		RegistrarRefStorage(const RegistrarRefStorage&) = delete;
		RegistrarRefStorage& operator=(const RegistrarRefStorage&) = delete;

		size_t m_refCount = 0;
		const C** m_refList = nullptr;
	};

	template<class C> RegistrarRefStorage<C>::~RegistrarRefStorage()
	{
		m_refCount = 0;
		if (m_refList)
		{
			delete[] m_refList;
			m_refList = nullptr;
		}
	}

	template<class C> void RegistrarRefStorage<C>::addRegistrarRef(const C* pRef) noexcept
	{
		if (pRef)
		{
			const C** refList = new const C*[m_refCount + 1];

			if (m_refList)
			{
				std::memcpy(refList, m_refList, m_refCount * sizeof(C*));
				delete[] m_refList;
			}

			m_refList = refList;
			m_refList[m_refCount++] = pRef;
		}
	}
}

#endif //_REGISTRARREFSTORAGE_H_
