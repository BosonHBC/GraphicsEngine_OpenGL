#pragma once
#include <inttypes.h>
#include "assert.h"
// define the data type under x86 and x64
#if _WIN32 || _WIN64
#if _WIN64
// uint64_t
#define uint_t unsigned long long
#else
// uint32_t
#define uint_t unsigned long
#endif
#endif

#define BITS_PER_ITEM (sizeof(uint_t)*8);
namespace Core {
	class cBitArray
	{
	public:
		// Factory function to create and delete the bit array
		static cBitArray* CreateBitArray(size_t i_numBits, bool i_bInitToZero);
		virtual ~cBitArray();

		/** Manipulation functions */
		void ClearAll(void);
		void SetAll(void);
		void SetBit(size_t i_index);
		void ClearBit(size_t i_index);

		/** Checker functions */
		bool AreAllClear() const;
		bool AreAllSet() const;
		// Check if the bit in this index is set, which in binary format, is 1
		bool IsBitSet(size_t i_index) const;
		// Check if the bit in this index is set, which in binary format, is 0
		bool IsBitClear(size_t i_index) const;

		/** Getter functions */
		bool GetFirstClearBit(unsigned long & o_bit_Index) const;
		bool GetFirstSetBit(unsigned long & o_bit_Index) const;
		size_t GetSizeInBytes() const { return m_arraySize * sizeof(uint_t); };

	private:
		// private constructor
		cBitArray() {}

		// The array for storing all bits
		uint_t* m_pBits;
		size_t m_requiredBits;
		// The size of array of type: uint_t
		size_t m_arraySize;
		void InitBitArray(size_t i_numBits, bool i_bInitToZero);
	};

}
