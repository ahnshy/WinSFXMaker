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

#include "stdafx.h"
#include "DeflateCompressor.h"

namespace ZipArchiveLib
{

// TODO: [postponed] move processing of the stored data to the base class, so every class can handle it and there is no need to switch compressors
#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif


CDeflateCompressor::CDeflateCompressor(CZipStorage* pStorage, CZipAutoBuffer* pBuffer, bool bDetectZlibMemoryLeaks)
	:CZipCompressor(pStorage, pBuffer)
{
	m_stream.zalloc = (zarch_alloc_func)_zliballoc;
	m_stream.zfree = (zarch_free_func)_zlibfree;
	m_bDetectZlibMemoryLeaks = bDetectZlibMemoryLeaks;
	m_bCheckLastBlock = true;
}

void CDeflateCompressor::InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CZipCompressor::InitDecompression(pFile, pCryptograph);
	if (m_pFile->m_uMethod == methodDeflate)
	{
		m_stream.opaque = m_bDetectZlibMemoryLeaks ? &m_list : 0;

		int err = zarch_inflateInit2_(&m_stream, -MAX_WBITS, ZLIB_VERSION, sizeof(zarch_z_stream));
		//			* windowBits is passed < 0 to tell that there is no zlib header.
		//          * Note that in this case inflate *requires* an extra "dummy" byte
		//          * after the compressed stream in order to complete decompression and
		//          * return Z_STREAM_END. 
		CheckForError(err);
	}
	m_uComprLeft = m_pFile->GetDataSize(false, true);
	m_uUncomprLeft = m_pFile->m_uUncomprSize;
	m_uCrc32 = 0;
	m_stream.total_out = 0;
	m_stream.avail_in = 0;
}

DWORD CDeflateCompressor::Decompress(void *pBuffer, DWORD uSize)
{
	m_stream.next_out = (zarch_Bytef*)pBuffer;
	m_stream.avail_out = uSize > m_uUncomprLeft 
		? (DWORD)m_uUncomprLeft : uSize;
			
	DWORD uRead = 0;

	// may happen when the file is 0 sized
	bool bForce = m_stream.avail_out == 0 && m_uComprLeft > 0;
	while (m_stream.avail_out > 0 || (bForce && m_uComprLeft > 0))
	{
		if ((m_stream.avail_in == 0) &&
			(m_uComprLeft >= 0)) // Also when there are zero bytes left
		{
			DWORD uToRead = m_pBuffer->GetSize();
			if (m_uComprLeft < uToRead)
				uToRead = (DWORD)m_uComprLeft;
			
			if (uToRead == 0)
				uToRead = 1; // Add dummy byte at end of compressed data.
			else
			{
				m_pStorage->Read(*m_pBuffer, uToRead, false);
				if (m_pCryptograph)
					m_pCryptograph->Decode(*m_pBuffer, uToRead);
			}

			m_uComprLeft -= uToRead;
			
			m_stream.next_in = (zarch_Bytef*)(char*)*m_pBuffer;
			m_stream.avail_in = uToRead;
		}
		
		if (m_pFile->m_uMethod == methodStore)
		{
			DWORD uToCopy = m_stream.avail_out < m_stream.avail_in 
				? m_stream.avail_out : m_stream.avail_in;
			
			memcpy(m_stream.next_out, m_stream.next_in, uToCopy);
			
			m_uCrc32 = zarch_crc32(m_uCrc32, m_stream.next_out, uToCopy);
			
			m_uUncomprLeft -= uToCopy;
			m_stream.avail_in -= uToCopy;
			m_stream.avail_out -= uToCopy;
			m_stream.next_out += uToCopy;
			m_stream.next_in += uToCopy;
			m_stream.total_out += uToCopy;
			uRead += uToCopy;
		}
		else
		{
			ZIP_SIZE_TYPE uTotal = m_stream.total_out;
			zarch_Bytef* pOldBuf =  m_stream.next_out;
			int ret = zarch_inflate(&m_stream, Z_SYNC_FLUSH);
			// will not exceed DWORD
			DWORD uToCopy = (DWORD)(m_stream.total_out - uTotal);
			
			m_uCrc32 = zarch_crc32(m_uCrc32, pOldBuf, uToCopy);
			
			m_uUncomprLeft -= uToCopy;
			uRead += uToCopy;
            
			if (ret == Z_STREAM_END)
				return uRead;
			
			CheckForError(ret);
		}
	}

	if (!uRead && m_bCheckLastBlock && uSize != 0 && m_pFile->m_uMethod == methodDeflate)
	{
		if (zarch_inflate(&m_stream, Z_SYNC_FLUSH) != Z_STREAM_END)
			// there were no more bytes to read and there was no ending block, 
			// otherwise the method would return earlier
			ThrowError(CZipException::badZipFile);
	}
	
	return uRead;
}

void CDeflateCompressor::FinishDecompression(bool bAfterException)
{
	if (!bAfterException && m_pFile->m_uMethod == methodDeflate)
		zarch_inflateEnd(&m_stream);
	EmptyPtrList();
}

void CDeflateCompressor::InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CZipCompressor::InitCompression(iLevel, pFile, pCryptograph);

	m_uComprLeft = 0;
	m_stream.avail_in = (zarch_uInt)0;
	m_stream.avail_out = (zarch_uInt)m_pBuffer->GetSize();
	m_stream.next_out = (zarch_Bytef*)(char*)*m_pBuffer;
	m_stream.total_in = 0;
	m_stream.total_out = 0;
	
	if (pFile->m_uMethod == methodDeflate)
	{
		m_stream.opaque = m_bDetectZlibMemoryLeaks ? &m_list : 0;
		
		int err = zarch_deflateInit2_(&m_stream, iLevel,
			Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(zarch_z_stream));
		
		CheckForError(err);
	}
}

void CDeflateCompressor::Compress(const void *pBuffer, DWORD uSize)
{ 
	m_stream.next_in = (zarch_Bytef*)pBuffer;
	m_stream.avail_in = uSize;
	m_pFile->m_uCrc32 = zarch_crc32(m_pFile->m_uCrc32, (zarch_Bytef*)pBuffer, uSize);
	
	
	while (m_stream.avail_in > 0)
	{
		if (m_stream.avail_out == 0)
		{
			if (m_pCryptograph)
				m_pCryptograph->Encode(*m_pBuffer, (DWORD)m_uComprLeft);
			m_pStorage->Write(*m_pBuffer, (DWORD)m_uComprLeft, false);
			m_uComprLeft = 0;
			m_stream.avail_out = m_pBuffer->GetSize();
			m_stream.next_out = (zarch_Bytef*)(char*)*m_pBuffer;
		}
		
		if (m_pFile->m_uMethod == methodDeflate)
		{
			ZIP_ZLIB_TYPE uTotal = m_stream.total_out;
			int err = zarch_deflate(&m_stream,  Z_NO_FLUSH);
			CheckForError(err);
			m_uComprLeft += m_stream.total_out - uTotal;
		}
		else
		{
			DWORD uToCopy = (m_stream.avail_in < m_stream.avail_out) 
				? m_stream.avail_in : m_stream.avail_out;
			
			memcpy(m_stream.next_out, m_stream.next_in, uToCopy);
			
			m_stream.avail_in -= uToCopy;
			m_stream.avail_out -= uToCopy;
			m_stream.next_in += uToCopy;
			m_stream.next_out += uToCopy;
			m_stream.total_in += uToCopy;
			m_stream.total_out += uToCopy;
			m_uComprLeft += uToCopy;
		}
	}
}

void CDeflateCompressor::FinishCompression(bool bAfterException)
{
	m_stream.avail_in = 0;
	if (!bAfterException)
	{
		int err = Z_OK;
		if (m_pFile->m_uMethod == methodDeflate)
			while (err == Z_OK)
			{
				if (m_stream.avail_out == 0)
				{
					if (m_pCryptograph)
						m_pCryptograph->Encode(*m_pBuffer, (DWORD)m_uComprLeft);
					m_pStorage->Write(*m_pBuffer, (DWORD)m_uComprLeft, false);
					m_uComprLeft = 0;
					m_stream.avail_out = m_pBuffer->GetSize();
					m_stream.next_out = (zarch_Bytef*)(char*)*m_pBuffer;
				}
				ZIP_SIZE_TYPE uTotal = m_stream.total_out;
				err = zarch_deflate(&m_stream,  Z_FINISH);
				m_uComprLeft += m_stream.total_out - uTotal;
			}
			
		if (err == Z_STREAM_END)
			err = Z_OK;
		CheckForError(err);
		
		if (m_uComprLeft > 0)
		{
			if (m_pCryptograph)
				m_pCryptograph->Encode(*m_pBuffer, (DWORD)m_uComprLeft);
			m_pStorage->Write(*m_pBuffer, (DWORD)m_uComprLeft, false);
		}
		
		if (m_pFile->m_uMethod == methodDeflate)
		{
			err = zarch_deflateEnd(&m_stream);
			CheckForError(err);
		}
		
		
		// it may be increased by the encrypted header size in CZipFileHeader::PrepareData
		m_pFile->m_uComprSize += m_stream.total_out;
		m_pFile->m_uUncomprSize = m_stream.total_in;
	}
	EmptyPtrList();
}

void CDeflateCompressor::EmptyPtrList()
{
	if (m_list.GetCount())
	{
		// if some memory hasn't been freed due to an error in zlib, so free it now
		CZipPtrListIter iter = m_list.GetHeadPosition();
		while (m_list.IteratorValid(iter))
			delete[] (char*) m_list.GetNext(iter);
	}
	m_list.RemoveAll();
}

void CDeflateCompressor::CheckForError(int iErr)
{
	if ((iErr == Z_OK) ||(iErr == Z_NEED_DICT))
		return;
	
	ThrowError(iErr, true);
}
} // namespace
