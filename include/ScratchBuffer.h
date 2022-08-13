#ifndef SCRATCH_BUFFER_H__
#define SCRATCH_BUFFER_H__

#include <cstring>
#include <stdexcept>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
// scratch memory buffer utility
///////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename SCRATCH_MEMORY_TYPE,int START_TYPE_COUNT,int GROWN_TYPE_COUNT,int MAXIMUM_GROW_COUNT> struct ScratchBuffer
{
	ScratchBuffer():mMemory(new SCRATCH_MEMORY_TYPE[START_TYPE_COUNT]),mCount(START_TYPE_COUNT),mNextIndex(0){}
	~ScratchBuffer(){delete []mMemory;}
	
	/**
	 * @brief Start filling your data from the start of the buffer, overwriting what maybe there. This is the core speed up we get.
	 */
	void Restart(){mNextIndex = 0;}

	/**
	 * @brief For when you know in advance how much space you need.
	 */
	SCRATCH_MEMORY_TYPE* Restart(size_t pCount)
	{
		mNextIndex = 0;
		return Next(pCount);
	}

	/**
	 * @brief Return enough memory to put the next N items into, you can only safety write the number of items requested.
	 */
	SCRATCH_MEMORY_TYPE* Next(size_t pCount = 1)
	{
		EnsureSpace(pCount);
		SCRATCH_MEMORY_TYPE* next = mMemory + mNextIndex;
		mNextIndex += pCount;
		return next;
	}

	/**
	 * @brief How many items have been written since Restart was called.
	 */
	const size_t Used()const{return mNextIndex;}

	/**
	 * @brief Diagnostics tool, how many bytes we're using.
	 */
	const size_t MemoryUsed()const{return mCount * sizeof(SCRATCH_MEMORY_TYPE);}

	/**
	 * @brief The root of our memory, handy for when you've finished filling the buffer and need to now do work with it.
	 * You should fetch this memory pointer AFTER you have done your work as it may change as you fill the data.
	 */
	const SCRATCH_MEMORY_TYPE* Data()const{return mMemory;}

private:
	SCRATCH_MEMORY_TYPE* mMemory; //<! Our memory, only reallocated when it's too small. That is the speed win!
	size_t mCount; //<! How many there are available to write too.
	size_t mNextIndex; //<! Where we can write to next.

	/**
	 * @brief Makes sure we always have space.
	 */
	void EnsureSpace(size_t pExtraSpaceNeeded)
	{
		if( pExtraSpaceNeeded > MAXIMUM_GROW_COUNT )
		{
			throw std::runtime_error("Scratch memory type tried to grow too large in one go, you may have a memory bug. Tried to add " + std::to_string(pExtraSpaceNeeded) + " items");
		}

		if( (mNextIndex + pExtraSpaceNeeded) >= mCount )
		{
			const size_t newCount = mCount + pExtraSpaceNeeded + GROWN_TYPE_COUNT;
			SCRATCH_MEMORY_TYPE* newMemory = new SCRATCH_MEMORY_TYPE[newCount];
			std::memmove(newMemory,mMemory,mCount);
			delete []mMemory;
			mMemory = newMemory;
			mCount = newCount;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif //SCRATCH_BUFFER_H__