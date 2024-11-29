////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2007 by Artpol Software - Tadeusz Dracz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details refer to the License.txt file.
//
// Web Site: http://www.artpol-software.com
////////////////////////////////////////////////////////////////////////////////

/**
* \file DeflateCompressor.h
*	Includes the ZipArchiveLib::CDeflateCompressor class.
*
*/

#if !defined(ZIPARCHIVE_DEFLATECOMPRESSOR_DOT_H)
#define ZIPARCHIVE_DEFLATECOMPRESSOR_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipExport.h"
#include "ZipCompressor.h"
#include "ZipCollections.h"
#include "ZipException.h"
#include "zlib/zlib.h"

namespace ZipArchiveLib
{

/**
	Compresses and decompresses data using the Zlib library.
*/
class ZIP_API CDeflateCompressor : public CZipCompressor
{	
	bool m_bDetectZlibMemoryLeaks;
	bool m_bCheckLastBlock;
public:

	/**
		Initializes a new instance of the CDeflateCompressor class.

		\param pStorage
			The current storage.

		\param pBuffer
			A pre-allocated buffer that receives compressed data or provides data for decompression.

		\param bDetectZlibMemoryLeaks			
			Set to \c true, if you want to control the allocation and freeing of memory by the Zlib library 
			or to \c false otherwise.
	*/
	CDeflateCompressor(CZipStorage* pStorage, CZipAutoBuffer* pBuffer, bool bDetectZlibMemoryLeaks = true);

	bool CanProcess(WORD uMethod) {return uMethod == methodStore || uMethod == methodDeflate;}

	void InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph);
	void InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph);

	DWORD Decompress(void *pBuffer, DWORD uSize);
	void Compress(const void *pBuffer, DWORD uSize);
	
	void FinishCompression(bool bAfterException);
	void FinishDecompression(bool bAfterException);

	/**
		Enables or disables checking, if the compressed data ends with a correct block.
		This should be enabled to protect against malformed data.
		
		\param bCheckLastBlock
			\c true, if the checking of the last block should be enabled; \c false otherwise.

		\see 
			GetCheckLastBlock
	*/
	void SetCheckLastBlock(bool bCheckLastBlock)
	{
		m_bCheckLastBlock = bCheckLastBlock;
	}

	/**
		Returns the value indicating whether checking of the last block is enabled or not.

		\return
			\c true, if checking of the last block is enabled; \c false otherwise.

		\see 
			SetCheckLastBlock
	*/
	bool GetCheckLastBlock()
	{
		return m_bCheckLastBlock;
	}

	~CDeflateCompressor()
	{
		EmptyPtrList();
	}
	
private:		
	zarch_z_stream m_stream;
	typedef CZipPtrList<void*>::iterator CZipPtrListIter;

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (push)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

	CZipPtrList<void*> m_list; ///< A list holding pointers to the memory areas allocated by the Zlib library.

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( pop)
#endif

	void CheckForError(int iErr);

	void ThrowError(int err, bool bZlib = false)
	{
		if (bZlib)
			err = CZipException::ZlibErrToZip(err);
		CZipException::Throw(err, m_pStorage->IsClosed(true) ? _T("") : (LPCTSTR)m_pStorage->m_pFile->GetFilePath());
	}


	static void* _zliballoc(void* opaque, UINT items, UINT size) ///< The memory allocator called by the Zlib library.
	{
		void* p = new char[size * items];
		if (opaque)
		{
			CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
			list->AddTail(p);
		}
		return p;
	}
	static void _zlibfree(void* opaque, void* address) ///< The memory deallocator called by the Zlib library.	
	{
		if (opaque)
		{
			CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
			CZipPtrListIter iter = list->Find(address);
			if (list->IteratorValid(iter))
				list->RemoveAt(iter);
		}
		delete[] (char*) address;
	}

	/**
		Frees the memory allocated by the Zlib library that hasn't been freed
		due to an error in the Zlib library (usually never happens).
	*/
	void EmptyPtrList();
};

} // namespace

#endif

