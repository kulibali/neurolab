#ifndef NEUROGUI_GLOBAL_H
#define NEUROGUI_GLOBAL_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <QtGlobal>
#include <QString>
#include "../common/common.h"

#if defined(NEUROGUI_LIBRARY)
#  define NEUROGUISHARED_EXPORT Q_DECL_EXPORT
#else
#  define NEUROGUISHARED_EXPORT Q_DECL_IMPORT
#endif

/// The main user interface.
namespace NeuroGui
{

    extern NEUROGUISHARED_EXPORT const QString & VERSION();

    struct NEUROGUISHARED_EXPORT NeuroLabFileVersion
    {
        quint16 neurolab_version;
    };

    enum NeuroLabVersionNumber
    {
        NEUROLAB_FILE_VERSION_OLD = 0,
        NEUROLAB_FILE_VERSION_1   = 1,
        NEUROLAB_FILE_VERSION_2   = 2,
        NEUROLAB_FILE_VERSION_3   = 3,
        NEUROLAB_FILE_VERSION_4   = 4,
        NEUROLAB_FILE_VERSION_5   = 5,
        NEUROLAB_FILE_VERSION_6   = 6,
        NEUROLAB_FILE_VERSION_7   = 7,
        NEUROLAB_FILE_VERSION_8   = 8,
        NEUROLAB_FILE_VERSION_9   = 9,
        NEUROLAB_FILE_VERSION_10  = 10,
        NEUROLAB_FILE_VERSION_11  = 11,
        NEUROLAB_FILE_VERSION_12  = 12,
        NEUROLAB_FILE_VERSION_13  = 13,
        NEUROLAB_NUM_FILE_VERSIONS
    };

} // namespace NeuroGui

#endif // NEUROGUI_GLOBAL_H
