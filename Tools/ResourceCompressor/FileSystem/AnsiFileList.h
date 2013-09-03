/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __LOGENGINE_ANSIFILELIST_H__
#define __LOGENGINE_ANSIFILELIST_H__

#include "IFileList.h"

#include <vector>

namespace Log
{
namespace IO
{

//! Class used to enumerate files in directories
//! list of files.
class	AnsiFileList : public IFileList
{
public:

	//! constructor
	AnsiFileList();
	//! destructor
	~AnsiFileList();

	//! Get file count
	virtual	int32				GetCount();
	
	//! Get file name
	virtual	const char8 *		GetFilename(int32 Index);
    
	//! Get path name
	virtual const char8 *		GetPathname(int32 Index);

	//! is [Index] file a directory
	virtual bool				IsDirectory(int32 Index);
private:
	struct FileEntry
	{
		std::string Name;
		std::string PathName;
		uint32		Size;
		bool		IsDirectory;
	};
	std::string					Path;
	std::vector< FileEntry >	FileList;
};


}; // end of namespace IO
}; // end of namespace Log


#endif // __LOGENGINE_ANSIFILESYSTEM_H__