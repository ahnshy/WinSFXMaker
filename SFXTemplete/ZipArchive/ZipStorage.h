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
* \file ZipStorage.h
* Includes the CZipStorage class.	
*
*/

#if !defined(ZIPARCHIVE_ZIPSTORAGE_DOT_H)
#define ZIPARCHIVE_ZIPSTORAGE_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipFile.h"	
#include "ZipAutoBuffer.h"
#include "ZipString.h"
#include "ZipMemFile.h"
#include "ZipExport.h"
#include "ZipCallback.h"



/**
	Represents the storage layer for an archive.
*/
class ZIP_API CZipStorage  
{
	friend class CZipArchive;
	friend class CZipCentralDir;
public:
	
	/**
		The type of the segmentation of the archive.

		\see
			<a href="kb">0610051553</a>
		\see
			CZipArchive::GetSegmMode
	*/
	enum ZipSegmentationMode
	{
		noSegments,			///< No archive segmentation.
		spannedArchive,		///< A spanned archive.
		splitArchive,		///< A split archive.

		/**
			\brief .
			The archive segmentation type will be auto-detected.			
			If the archive is on the removable device,
			assume a spanned archive, otherwise assume a split archive.
		*/
		suggestedAuto,	

		/**
			\brief .
			If a segmented archive is on a removable device, assume a split archive.
			Normally you create spanned archives on removable devices.
		*/
		suggestedSplit
	};
	CZipStorage();
	virtual ~CZipStorage();

	void Initialize();
	/**
		Opens a new or existing archive in memory.
		The meaning for the parameters is the same as in the CZipArchive::Open(CZipAbstractFile& , int) method.
	*/
	void Open(CZipAbstractFile& af, int iMode);

	/**
		Opens or creates an archive.

		The meaning for the parameters is the same as in the CZipArchive::Open(LPCTSTR, int, ZIP_SIZE_TYPE) method.
	*/
	void Open(LPCTSTR szPathName, int iMode, ZIP_SIZE_TYPE uVolumeSize);


	/**
		Closes a segmented archive in creation and reopens it as an existing segmented archive (no modifications allowed).
		The archive may also turn out to be a not segmented archive.
	*/
	void FinalizeSegm();

	
	/**
		Called only by CZipCentralDir::Read when opening an existing archive.

		\param	uLastDisk
			The number of the disk the central directory is on.

		\note Throws exceptions.

	*/
	void UpdateSegmMode(ZIP_PART_TYPE uLastDisk);

	/**
		Ensures than in a segmented archive, there is enough free space on the current volume.

		\param uNeeded
			The size of the required free space in bytes.

		\return
			The number of free bytes on the current volume.

		\note
			Throws exceptions.
	*/
	ZIP_SIZE_TYPE AssureFree(ZIP_SIZE_TYPE uNeeded);

	/**
		Writes a chunk of data to the archive.

		\param	pBuf
			The buffer with data.

		\param	iSize
			The number of bytes to write.

		\param	bAtOnce
			If \c true, the whole chunk must fit in the current volume.
			If there is not enough free space, a disk change is performed.

		\note
			Throws exceptions.
	*/
	void Write(const void *pBuf, DWORD iSize, bool bAtOnce);

	/** 
		Gets the total size currently occupied by the archive.

		\return
			The length of the current archive file increased by the number of bytes in the write buffer.	
	*/
	ZIP_SIZE_TYPE GetOccupiedSpace() const
	{
		return ZIP_SIZE_TYPE(m_pFile->GetLength() + m_uBytesInWriteBuffer);
	}

	/**
		The same as the CZipArchive::IsClosed method.
	*/
	bool IsClosed(bool bArchive) const 
	{
		if (bArchive)
			return GetCurrentDisk() == ZIP_DISK_NUMBER_UNSPECIFIED;
		else
			return !m_pFile || !m_bInMemory && m_pFile->IsClosed();
	}

	/**
		Reads a chunk of data from the archive.

		\param	pBuf
			The buffer to receive the data.

		\param	iSize
			The number of bytes to read.

		\param	bAtOnce
			If \c true, the specified number of bytes must be read from the same 
			volume (no disk change is allowed).

		\note
			Throws exceptions.
	*/
	DWORD Read(void* pBuf, DWORD iSize, bool bAtOnce);

	/**
		Gets the position in the file, taking into account the number of bytes in the write buffer 
		and the number of bytes before the archive.

		\return 
			The position in the file.

		\note
			Throws exceptions.
	*/
	ZIP_SIZE_TYPE GetPosition() const
	{
		ZIP_SIZE_TYPE uPos = (ZIP_SIZE_TYPE)(m_pFile->GetPosition()) + m_uBytesInWriteBuffer;
		if (m_uCurrentDisk == 0)
			uPos -= m_uBytesBeforeZip;
		return uPos;
	}


	/**
		Flushes the data from the read buffer to the disk.

		\note
			Throws exceptions.
	*/
	void Flush();


	/**
		Forces any data remaining in the file buffer to be written to the disk.
	*/
	void FlushFile()
	{
		if (!m_bInMemory && !IsReadOnly())
			m_pFile->Flush();
	}

	void FlushBuffers()
	{
		Flush();
		FlushFile();
	}

	/**
		Changes disks during writing to a segmented archive.

		\param	uNeeded
			The number of bytes needed on the disk.

		\param	lpszFileName
			The archive filename.

		\note
			Throws exceptions.
	*/
	void NextDisk(ZIP_SIZE_TYPE uNeeded, LPCTSTR lpszFileName = NULL);


	/**
		Gets a zero-based number of the current disk.
	*/
	ZIP_PART_TYPE GetCurrentDisk() const {return m_uCurrentDisk;}

 
	/**
		Changes the disk during extract operations.

		\param	uNumber
			A zero-based requested disk number.
	*/
	void ChangeDisk(ZIP_PART_TYPE uNumber);

	/**
		Detects the segmentation mode.

		\return	
			- \c -1 : An existing segmented archive is opened.
			- \c 0 : The archive is not segmented.
			- \c 1 : A segmented archive in creation.
	*/
	int IsSegmented() const
	{
		return m_iSegmMode == noSegments ? 0 : (m_bNewSegm ? 1 : -1);
	}

	/**
		The same as the CZipArchive::IsReadOnly method.
	*/
	bool IsReadOnly()
	{
		return m_bReadOnly || IsSegmented() < 0;
	}
	
	/**
		Performs the seeking operation on the #m_pFile.

		\param lOff
			The new position in the file.

		\param fromBeginning
			The direction of the seek operation. If \c true, 
			\a lOff is counted from the beginning of the file;
			otherwise the \a lOff is counted from the end of the file.
	*/
	ULONGLONG Seek(ULONGLONG lOff , bool fromBeginning = true);	

	/**
		Gets the number of free bytes on the current volume.	

		\return 
			The number of free bytes on the current volume.
	*/
	ZIP_SIZE_TYPE VolumeLeft() const;

	/**	
		Closes the storage.

		\param	bAfterException
			Set to \c true, if an exception was thrown before.

		\return
			The file path of the archive.

		\note
			Throws exceptions.
	*/
	CZipString Close(bool bAfterException);

	/**
		Represents the physical storage of the current archive segment. 				
	*/
	CZipAbstractFile* m_pFile;

	/**
		The signature of the extended header.
	*/
	static char m_gszExtHeaderSignat[];

protected:
	
	/**
		Reverse-finds the location of the given signature starting from the current position in file.

		\param szSignature
			The signature to locate.

		\param uMaxDepth
			The maximum number of bytes to search for \a szSignature.

		\return
			The location of the signature.

		\note
			Throws exceptions.
	*/
	ZIP_FILE_USIZE LocateSignature(char* szSignature, ZIP_SIZE_TYPE uMaxDepth);
		

	/**
		Flushes without writing. Can be used only on not segmented archives.
	*/
	void EmptyWriteBuffer()
	{
		m_uBytesInWriteBuffer = 0;
	}

	/**
		Opens a physical file.

		\param	lpszName
			The name of the file to open.

		\param	uFlags
			The file open flags.

		\param	bThrow
			If \c true, throw an exception in case of failure.

		\return
			\c true if successful; \c false otherwise.
	*/
	bool OpenFile(LPCTSTR lpszName, UINT uFlags, bool bThrow = true);

	/**
		Renames the last segment file in a split archive when finalizing the whole archive.
	*/
	CZipString RenameLastFileInSplitArchive();

	/**
		Writes data to the internal buffer.

		\param	*pBuf
			The buffer to copy the data from.

		\param	uSize
			The number of bytes to write.

		\note
			Throws exceptions.
	*/
	void WriteInternalBuffer(const char *pBuf, DWORD uSize);

	/**
		Gets the free space size on the current removable disk.

		\return
			The free space in bytes.
	*/
	ZIP_SIZE_TYPE GetFreeVolumeSpace() const;

	/**
		Notifies the callback object.
		Throws an exception if the callback method returns \c false.

		\param uNeeded
			The minimum number of free bytes required on the disk.

		\param	iCode
			The code to be passed to the callback method.

		\param	szTemp
			The string to be used as a filename (as an argument
			in the CZipException::Throw method) when an exception must be thrown.

		\note
			Throws exceptions.
		\see
			CZipArchive::SetSpanCallback
	*/
	void CallCallback(ZIP_SIZE_TYPE uNeeded, int iCode, CZipString szTemp);

	/**
		Constructs the name of a segment in a split archive.

		\param	bLast
			Set it to \c true, if constructing the last volume name.

		\param	lpszZipName
			The name of the archive.

		\return	
			The segment name.
	*/
	CZipString GetSplitVolumeName(bool bLast, LPCTSTR lpszZipName = NULL) const;

	/**
		Changes the disk when processing a split archive.
	*/
	CZipString ChangeSplitRead();

	/**
		Changes the disk when processing a spanned archive.
	*/
	CZipString ChangeSpannedRead();

	/**
		Gets the free space left in the write buffer.

		\return
			The free space left in the write buffer in bytes.
	*/
	DWORD GetFreeInBuffer() const {return m_pWriteBuffer.GetSize() - m_uBytesInWriteBuffer;}	

	/**
		The value it holds, depends on the current mode:		
		- An opened existing split archive - stores the number of the last disk ( the one with "zip" extension).
		- A split archive in creation - the size of the volume.

		This method is used only when processing split archives.
	*/
	ZIP_SIZE_TYPE m_uSplitData;
	
	/**
		The extension of the last segment.
	*/
	CZipString m_szSplitExtension;
	
	/**
		The number of bytes available in the write buffer.		
	*/
	DWORD m_uBytesInWriteBuffer;

	/**
		The value it holds depends on the segmentation mode:
		- A split archive : the total size of the current volume.
		- A spanned archive: the free space on the current volume.
	*/
	ZIP_SIZE_TYPE m_uCurrentVolSize;

	/**
		The write buffer caching data.
	*/
	CZipAutoBuffer m_pWriteBuffer;

	/**
		Stores the number of bytes that have been written physically to the current segment.
		Used only when processing a segmented archive in creation.
	*/
	ZIP_SIZE_TYPE m_uBytesWritten;

	/**
		\c true, if the current archive is a new segmented archive; \c false otherwise.
	*/
	bool m_bNewSegm;

	/**
		The current disk in a segmented archive.
		The value is zero-based.
	*/
	ZIP_PART_TYPE m_uCurrentDisk;

	/**
		\c true when the archive is created in memory; \e false otherwise.
	*/
	bool m_bInMemory;

	/**
		\c true if OpenMode::zipOpenReadOnly was specified when opening the archive.
	*/
	bool m_bReadOnly;

	/**
		The number of bytes before the actual zip archive in a file.
		\see
			CZipArchive::GetBytesBeforeZip
	*/
	ZIP_SIZE_TYPE m_uBytesBeforeZip;


	/**
		The size of the write buffer. 		

		\see
			CZipArchive::SetAdvanced
	*/
	int m_iWriteBufferSize;

	/**
		The size of the buffer used in searching for the central directory.

		\see
			CZipArchive::SetAdvanced
	*/
	int m_iLocateBufferSize;	

	/**
		Takes one of the #ZipSegmentationMode values.	
	*/
	int m_iSegmMode;

	/**
		A callback object called when there is a need for a disk change in a spanned archive.

		\see
			CZipArchive::SetSpanCallback
	*/
	CZipSpanCallback* m_pChangeDiskFunc;

private:
	CZipFile m_internalfile;
	static const ZIP_FILE_USIZE SignatureNotFound;
	void ThrowError(int err);
};

#endif // !defined(ZIPARCHIVE_ZIPSTORAGE_DOT_H)
