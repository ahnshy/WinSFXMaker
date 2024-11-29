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
* \file _features.h
*	Contains definitions that enable or disable certain features in the ZipArchive Library.
*
*/

#if !defined(ZIPARCHIVE_FEATURES_DOT_H)
#define ZIPARCHIVE_FEATURES_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "_platform.h"

#ifdef __GNUC__

#ifndef __int64
	#define __int64 long long
#endif

#endif

/************ Feel free to adjust the definitions in the following block ************/
/************************************ BLOCK START ***********************************/

// comment this out, if you don't use ZIP64
// #define _ZIP64
// comment this out, if you don't use AES
// #define _ZIP_AES

#ifndef _ZIP64
// Uncomment this to have the index and part types defined as WORD. Otherwise they are defined as int
#define _ZIP_STRICT_U16
#endif

/************************************* BLOCK END ***********************************/
/********* The contents below this line are not intended for modification **********/

#if defined _ZIP64 && (defined __BORLANDC__ || (defined _MSC_VER && _MSC_VER < 1300 && defined ZIP_ARCHIVE_MFC))
	#error Zip64 Cannot be used under Borland and Visual Studio 6.0 MFC
		#undef _ZIP64
#endif

#endif // !defined(ZIPARCHIVE_FEATURES_DOT_H)
