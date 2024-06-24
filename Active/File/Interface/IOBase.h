/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_FILE_IO_BASE
#define ACTIVE_FILE_IO_BASE

#include <ios>

namespace active::file {
	
	class IOBase {
	public:

		// MARK: - Types
			
			///The state flag representation type
		using flag_type = std::ios_base::iostate;

		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		IOBase() noexcept { m_stateFlags = std::ios_base::goodbit; }
		/*!
			Copy constructor
			@param source The object to copy
		*/
		IOBase(const IOBase& source) noexcept { m_stateFlags = source.m_stateFlags; }
		/*!
			Destructor
		*/
		virtual ~IOBase() noexcept = default;

		// MARK: - Operators
		
		/*!
			Logical not operator
			@return True if an error has occured
		*/
		bool operator! () const noexcept { return fail(); }
		/*!
			Determine if the next operation will succeed
			@return Non-zero if the next operation will succeed
		*/
		operator void* () const noexcept { return fail() ? nullptr : (void*) 1; }

		// MARK: - Functions (const)
		
		/*!
			Determine if the last operation was successful, i.e. eof, fail and bad bits are all 0)
			@return True if the last operation was successful
		*/
		virtual bool good() const noexcept { return (m_stateFlags == 0); }
		/*!
			Determine if end of input has been seen
			@return True if end of input has been seen
		*/
		virtual bool eof() const noexcept { return ((m_stateFlags & std::ios_base::eofbit) != 0); }
		/*!
			Determine if the next operation will fail
			@return True if the next operation will fail
		*/
		virtual bool fail() const noexcept { return ((m_stateFlags & (std::ios_base::failbit | std::ios_base::badbit)) != 0); }
		/*!
			Determine if the stream is bad
			@return True if the stream is bad
		*/
		virtual bool bad() const noexcept { return ((m_stateFlags & std::ios_base::badbit) != 0); }
		/*!
			Get the stream state flags
			@return The stream state flags
		*/
		virtual flag_type state() const noexcept { return m_stateFlags; }

		// MARK: - Functions (mutating)
		
		/*!
			Set the stream state flags
			@param f The stream state flags
		*/
		virtual void clear(flag_type f = std::ios_base::goodbit) { m_stateFlags = f; }
		/*!
			Set stream state flags
			@param f The flag to modify
		*/
		virtual void setState(flag_type f) { clear(state() | f); }
		/*!
			Unset stream state flags
			@param f The flag to unset (clear)
		*/
		virtual void unsetState(flag_type f) { clear((state() | f) ^ f); }
		
	protected:
		/*!
			Modify the stream state flags
			@param state The flag to modify
		*/
		void setCompleteState(flag_type state) { m_stateFlags = state; }
		/*!
			Set the stream state flags
			@param f The stream state flags
		*/
		void clear(flag_type f = std::ios_base::goodbit) const { m_stateFlags = f; }
		/*!
			Modify the stream state flags
			@param f The flag to modify
		*/
		void setState(flag_type f) const { clear(state() | f); }
		/*!
			Unset stream state flags
			@param f The flag to unset (clear)
		*/
		virtual void unsetState(flag_type f) const { clear((state() | f) ^ f); }
		
	private:
			///The stream state flags
		mutable flag_type m_stateFlags;
	};
	
}

#endif	//ACTIVE_FILE_IO_BASE
