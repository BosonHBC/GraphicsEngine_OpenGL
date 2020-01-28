#include "BitArray.h"
#include "stdio.h"
#include <string.h>
#include <intrin.h>

#pragma intrinsic(_BitScanForward)

namespace Core {
	cBitArray* cBitArray::CreateBitArray(size_t i_numBits, bool i_bInitToZero)
	{
		cBitArray* _pTemp = new cBitArray();
		_pTemp->InitBitArray(i_numBits, i_bInitToZero);
		return _pTemp;
	}

	cBitArray::~cBitArray()
	{
		if (m_pBits) {
			delete m_pBits;
			m_pBits = nullptr;
		}
	}

	void cBitArray::ClearAll()
	{
		for (size_t i = 0; i < m_arraySize; i++)
		{
			m_pBits[i] = 0;
		}
	}

	void cBitArray::SetAll()
	{
		for (size_t i = 0; i < m_arraySize; i++)
		{
			m_pBits[i] = -1;
		}
	}

	void cBitArray::SetBit(size_t i_index)
	{
		// i_bitNumber range should be [0,m_requiredBits-1]
		if (i_index < m_requiredBits) {
			// in which index of the m_pBits
			size_t index = m_arraySize - 1 - i_index / BITS_PER_ITEM;
			// the shifting amount
			size_t offset = i_index % BITS_PER_ITEM;
			//DebugLog("Number: %d, Index: %d, Offset: %d", i_bitNumber, index, offset);
			// The pattern will look 0x0000 0001 for 1 byte before shifting
			// after shifting it will look like 0x0000 1000, such as the offset is 3
			uint_t _default = 1 << offset;
			// using inclusive OR
			m_pBits[index] |= _default;
		}

	}

	void cBitArray::ClearBit(size_t i_index)
	{
		// i_bitNumber range should be [0,m_requiredBits-1]
		if (i_index < m_requiredBits) {
			// in which index of the m_pBits
			size_t index = m_arraySize - 1 - i_index / BITS_PER_ITEM;
			// the shifting amount
			size_t offset = i_index % BITS_PER_ITEM;
			//DebugLog("Number: %d, Index: %d, Offset: %d",i_bitNumber, index, offset);
			// The pattern will look 0x0000 0001 for 1 byte before shifting
			// after shifting it will look like 0x0000 1000, such as the offset is 3
			uint_t _default = 1 << offset;
			// using exclusive OR
			m_pBits[index] ^= _default;
		}
	}

	bool cBitArray::AreAllClear() const
	{
		unsigned char isNonzero = false;
		size_t index = m_arraySize;
		unsigned long i_firstAvaliable = 0;
		while (!isNonzero && index > 0)
		{
			index--;
			isNonzero = _BitScanForward(&i_firstAvaliable, m_pBits[index]);
			if (isNonzero)
				return false;
		}
		printf("Are all clear: %s\n", isNonzero == true ? "FALSE" : "TRUE");
		return !isNonzero;
	}

	bool cBitArray::AreAllSet() const
	{
		size_t bitsPerItem = BITS_PER_ITEM;
		unsigned char hasZero = false;
		size_t index = m_arraySize;
		size_t offset = 0;
		while (!hasZero && index > 0)
		{
			index--;
			unsigned char bit;
			for (offset = 0; offset < bitsPerItem; offset++)
			{
				// get the bit in certain offset
				bit = (m_pBits[index] >> offset) & 1U;
				if (bit == 0) {
					hasZero = true;
					printf("Are all Set: %s", hasZero == true ? "FALSE" : "TRUE");
					return false;
				}
			}
		}
		printf("Are all Set: %s\n", hasZero == true ? "FALSE" : "TRUE");
		return true;
	}

	 bool cBitArray::IsBitSet(size_t i_index) const
	{
		if (i_index < m_requiredBits) {
			size_t bitsPerItem = BITS_PER_ITEM;
			// in which index of the m_pBits
			size_t index = m_arraySize - 1 - i_index / bitsPerItem;
			// the shifting amount
			size_t offset = i_index % bitsPerItem;
			//DebugLog("Number: %d, Index: %d, Offset: %d",i_bitNumber, index, offset);
			// The pattern will look 0x0000 0001 for 1 byte before shifting
			// after shifting it will look like 0x0000 1000, such as the offset is 3
			unsigned char bit;
			bit = (m_pBits[index] >> offset) & 1U;
			// using exclusive OR
			return bit == 1 ? true : false;
		}
		return false;
	}

	 bool cBitArray::IsBitClear(size_t i_index) const
	{
		// i_bitNumber range should be [0,m_requiredBits-1]
		if (i_index < m_requiredBits) {
			return !IsBitSet(i_index);
		}
		return false;
	}

	bool cBitArray::GetFirstClearBit(unsigned long & o_bitNumber) const
	{
		size_t bitsPerItem = BITS_PER_ITEM;
		unsigned char hasZero = false;
		size_t index = m_arraySize;
		size_t offset = 0;
		while (!hasZero && index > 0)
		{
			index--;
			unsigned char bit;
			for (offset = 0; offset < bitsPerItem; offset++)
			{
				// get the bit in certain offset
				bit = (m_pBits[index] >> offset) & 1U;
				if (bit == 0) {
					hasZero = true;
					break;
				}
			}
		}
		o_bitNumber = (m_arraySize - index - 1) * bitsPerItem + offset;
		// the overflow blocks are not allowed to used because the fixed allocators block is no more than the m_requiredBits
		if (hasZero && o_bitNumber < m_requiredBits) {
			printf("The first clear bit index is: %d\n", o_bitNumber);
			return true;
		}
		else {
			printf("Can not find a clear bit\n");
			o_bitNumber = 0;
			return false;
		}
	}

	bool cBitArray::GetFirstSetBit(unsigned long & o_bitNumber) const
	{

		unsigned char isNonzero = false;
		size_t index = m_arraySize;
		while (!isNonzero && index > 0)
		{
			index--;
			isNonzero = _BitScanForward(&o_bitNumber, m_pBits[index]);
		}
		o_bitNumber += (m_arraySize - index - 1) * BITS_PER_ITEM;
		// the overflow blocks are not allowed to used because the fixed allocators block is no more than the m_requiredBits
		if (isNonzero && o_bitNumber < m_requiredBits) {
			printf("The first set bit index is: %d\n", o_bitNumber);
			return true;
		}
		else {
			printf("Can not find a set bit\n");
			o_bitNumber = 0;
			return false;
		}
	}

	void cBitArray::InitBitArray(size_t i_numBits, bool i_bInitToZero)
	{
		const size_t _size = BITS_PER_ITEM;
		m_requiredBits = i_numBits;
		m_arraySize = (_size + i_numBits + 1) / _size;

		m_pBits = new uint_t[m_arraySize];
		printf("Size of my bit array: %d\n", m_arraySize * sizeof(uint_t));
		assert(m_pBits);
		for (size_t i = 0; i < m_arraySize; i++)
		{
			// all bit to 0 or 1 according to i_bInitToZero, setting to -1 will  be overflow and all single bit will become 1 
			m_pBits[i] = i_bInitToZero ? 0 : -1;
		}
	}

}
