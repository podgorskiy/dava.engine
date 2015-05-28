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


#ifndef ITERPRIV_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define ITERPRIV_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include "yaml-cpp/ltnode.h"
#include <vector>
#include <map>

namespace YAML
{
	class Node;

	// IterPriv
	// . The implementation for iterators - essentially a union of sequence and map iterators.
	struct IterPriv
	{
		IterPriv(): type(IT_NONE) {}
		IterPriv(std::vector <Node *>::const_iterator it): type(IT_SEQ), seqIter(it) {}
		IterPriv(std::map <Node *, Node *, ltnode>::const_iterator it): type(IT_MAP), mapIter(it) {}

		enum ITER_TYPE { IT_NONE, IT_SEQ, IT_MAP };
		ITER_TYPE type;

		std::vector <Node *>::const_iterator seqIter;
		std::map <Node *, Node *, ltnode>::const_iterator mapIter;
	};
}

#endif // ITERPRIV_H_62B23520_7C8E_11DE_8A39_0800200C9A66
