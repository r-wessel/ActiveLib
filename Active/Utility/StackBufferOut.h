/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_STACK_BUFFER_OUT
#define ACTIVE_UTILITY_STACK_BUFFER_OUT

#include "Active/Utility/BufferOut.h"

namespace active::file {
	
	class File;

}

namespace active::utility {
	
		//Class to buffer data to a specified destination using stack-based storage (no heap allocation)
	template<size_t S>
	class StackBufferOut: public BufferOut {
	public:
		/*!
			Default constructor
		*/
		StackBufferOut() : BufferOut() {}
		/*!
			Constructor
			@param destFile The destination data file
		*/
		StackBufferOut(file::File& destFile) : BufferOut(destFile) {}
		/*!
			Constructor
			@param memory The destination memory block
		*/
		StackBufferOut(Memory& memory) : BufferOut(memory) {}
		/*!
			Constructor
			@param destString The destination string
		*/
		StackBufferOut(String& destString) : BufferOut(destString) {}
		 //No move constructor
		StackBufferOut(BufferOut&& source) noexcept = delete;
			//No copy constructor
		StackBufferOut(const BufferOut& source) = delete;

		//MARK: - Operators
		
			//No assignment
		StackBufferOut& operator= (BufferOut&& source) noexcept = delete;
			//No assignment
		StackBufferOut& operator= (const BufferOut&) = delete;
		
	protected:
		/*!
			Determine is the buffer is owned by this object
			@return True if this object owns the buffer
		*/
		bool isMyBuffer() const override { return true; }

		/*!
			Confirm the buffer is mine and prepared to receive data
			@return True if the buffer is confirmed (false = bad buffer state - writing will be rejected)
		*/
		bool confirmBuffer() const override {
			if (getBuffer().data() != m_fixed)
				setBuffer(m_fixed, S);
			return true;
		}

	private:
		mutable char m_fixed[S];
	};

}

#endif	//ACTIVE_UTILITY_STACK_BUFFER_OUT
