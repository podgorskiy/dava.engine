#ifndef __DAVAENGINE_ICLIPBOARDIMPL_H__
#define __DAVAENGINE_ICLIPBOARDIMPL_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
Interface to implement platform clipboard helper.
*/
class IClipboardImpl
{
public:
    /**
    Destructor.
    */
    virtual ~IClipboardImpl() = default;

    /**
    Return status of clipboard helper.
    Return true if helper ready to work with clipboard.
    */
    virtual bool IsReadyToUse() const = 0;

    /**
    Clear system clipboard.
    Return true if successful.
    */
    virtual bool ClearClipboard() const = 0;

    /**
    Check that system clipboard contains Unicode text.
    Return true if system clipboard contains Unicode text.
     */
    virtual bool HasText() const = 0;

    /**
    Copy to system clipboard WideString as Unicode string.
    Return true if successful.
    */
    virtual bool SetText(const WideString& str) = 0;

    /**
    Get from system clipboard Unicode text data as WideString.
    */
    virtual WideString GetText() const = 0;
};
}

#endif //__DAVAENGINE_ICLIPBOARDIMPL_H__
