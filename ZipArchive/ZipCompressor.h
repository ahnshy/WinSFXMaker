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
* \file ZipCompressor.h
* Includes the CZipCompressor class.
*
*/

#if !defined(ZIPARCHIVE_ZIPCOMPRESSOR_DOT_H)
#define ZIPARCHIVE_ZIPCOMPRESSOR_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable : 4100) // unreferenced formal parameter
#endif

#include "ZipExport.h"
#include "ZipAutoBuffer.h"
#include "ZipFileHeader.h"
#include "ZipStorage.h"
#include "ZipCryptograph.h"

/**
	The base class for compressors used in compression and decompression of data.
*/
class ZIP_API CZipCompressor
{
protected:
	CZipStorage* m_pStorage;			///< The current storage object.
	CZipAutoBuffer* m_pBuffer;			///< A buffer that receives compressed data or provides data for decompression.
	CZipCryptograph* m_pCryptograph;	///< Current cryptograph.
	CZipFileHeader* m_pFile;			///< The file header being compressed or decompressed.

	
	/**
		Initializes a new instance of the CZipCompressor class.

		\param pStorage
			The current storage object.
		
		\param pBuffer
			A pre-allocated buffer that receives compressed data or provides data for decompression.
	 */
	CZipCompressor(CZipStorage* pStorage, CZipAutoBuffer* pBuffer)
		:m_pStorage(pStorage), m_pBuffer(pBuffer)
	{
		m_pCryptograph = NULL;
		m_uUncomprLeft = 0;
		m_uComprLeft  = 0;
		m_uCrc32 = 0;
	}
public:
	/**
		The compression level.
	*/
	enum CompressionLevel
	{		
		levelDefault = -1, ///< The default compression level (equals 6 for deflate).
		levelStore = 0, ///< No compression used. Data is stored.
		levelFastest = 1, ///< The fastest compression. The compression ratio is the lowest (apart from #levelStore).
		levelBest = 9 ///< The highest compression ratio. It's usually the slowest.
	};

	/**
		The compression method.
	*/
	enum CompressionMethod
	{
		methodStore = 0, ///< A file is stored, not compressed.
		methodDeflate = 8, ///< The deflate compression method.
		/**
			This value means that WinZip AES encryption is used.
			The original compression method is stored in a WinZip extra field.
			It is only an informational value - you cannot set it as a compression method. The ZipArchive 
			Library handles this value internally.

			\see
				<a href="kb">0610201627|aes</a>
		*/
		methodWinZipAes = 99
	};

	/**
		Returns the value indicating whether the given compression method is supported by the ZipArchive Library.
		
		\param uCompressionMethod
			The compression method. Can be one of the #CompressionMethod values.

		\return 
			\c true, if the compression method is supported, \c false otherwise.
	*/
	static bool IsCompressionSupported(WORD uCompressionMethod)
	{		
		return uCompressionMethod == methodStore || uCompressionMethod == methodDeflate;
	}

	ZIP_SIZE_TYPE m_uUncomprLeft;	///< The number of bytes left to decompress.
	ZIP_SIZE_TYPE m_uComprLeft;		///< The number of bytes left to compress.
	DWORD m_uCrc32;	///< The CRC32 file checksum.	

	/**
		Returns the value indicating, if the current #CZipCompressor object supports the given compression method.
		
		\param uMethod
			The compression method. Can be one of the #CompressionMethod values.

		\return 
			\c true, if the compression method is supported; \c false otherwise.
			
	*/
	virtual bool CanProcess(WORD uMethod) = 0;


	/**
		The method called when a new file is opened for compression.
		
		\param iLevel
			The compression level.
		
		\param pFile
			The file being compressed.
		
		\param pCryptograph
			The current CZipCryptograph. Can be \c NULL, if no encryption is used.

		\see
			Compress
		\see 
			FinishCompression
	 */
	virtual void InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph)	
	{
		m_pFile = pFile;
		m_pCryptograph = pCryptograph;
	}

	/**
		The method called when a new file is opened for extraction.
		
		\param pFile
			The file being extracted.
		
		\param pCryptograph
			The current CZipCryptograph. Can be \c NULL, if no decryption is used.

		\see
			Decompress
		\see 
			FinishDecompression
	 */
	virtual void InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
	{
		m_pFile = pFile;
		m_pCryptograph = pCryptograph;
	}

	/**
		Compresses the given data.
	
		\param pBuffer
			The buffer that holds the data to compress.
	
		\param uSize
			The size of \a pBuffer.	

		\see
			InitCompression
		\see 
			FinishCompression
	 */
	virtual void Compress(const void *pBuffer, DWORD uSize) = 0;

	/**
		Decompresses the given data.
	
		\param pBuffer
			The buffer that receives the decompressed data.
	
		\param uSize
			The size of \a pBuffer.	

		\return 
			The number of bytes decompressed and written to \a pBuffer.

		\note
			This method should be called repeatedly until it returns 0.

		\see
			InitDecompression
		\see 
			FinishDecompression
	 */
	virtual DWORD Decompress(void *pBuffer, DWORD uSize) = 0;

	/**
		The method called at the end of the compression process.
	
		\param bAfterException
			Set to \c true, if an exception occurred before or to \c false otherwise.

		\see
			InitCompression
		\see 
			Compress
	 */
	virtual void FinishCompression(bool bAfterException){}

	/**
		The method called at the end of the decompression process.
	
		\param bAfterException
			Set to \c true, if an exception occurred before or to \c false otherwise.

		\see
			InitDecompression
		\see 
			Decompress
	 */
	virtual void FinishDecompression(bool bAfterException){}

	/**
		A factory method that creates an appropriate compressor for the given compression method.

		\param uMethod
			The compression method to create a compressor for. Can be one of #CompressionMethod values.

		\param pStorage
			The current storage object.

		\param pBuffer
			A pre-allocated buffer that receives compressed data or provides data for decompression.

	*/
	static CZipCompressor* CreateCompressor(WORD uMethod, CZipStorage* pStorage, CZipAutoBuffer* pBuffer);

	virtual ~CZipCompressor()
	{
	}
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif
