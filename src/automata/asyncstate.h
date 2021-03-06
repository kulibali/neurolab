#ifndef ASYNCSTATE_H
#define ASYNCSTATE_H

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

#include "automata_global.h"

#include <QtGlobal>
#include <QDataStream>

namespace Automata
{

    /// Internal state for an asynchronous automaton, which requires two copies of a cell's state,
    /// one for the previous state, and one for the next state.
    /// Uses a seqlock when copying or assigning to ensure cross-thread consistency.
    /// \param TState A cell's state.
    /// \param TIndex The index type used by the automaton.
    template <typename TState, typename TIndex>
    struct AsyncState
    {
        quint16 _sync_0; ///< First seqlock bookend.
        TIndex index;    ///< Index in cell array.
        TState q0;       ///< First copy of the cell's state.
        TState q1;       ///< Second copy of the cell's state.
        quint16 r;       ///< Used to track asynchronous updates.
        quint16 _sync_1; ///< Second seqlock bookend.

        //@{
        /// Constructor.
        AsyncState() : _sync_0(0), index(static_cast<TIndex>(-1)), r(0), _sync_1(0) {}
        AsyncState(const TState & s0, const TState & s1) : _sync_0(0), index(static_cast<TIndex>(-1)), q0(s0), q1(s1), r(0), _sync_1(0) {}

        /// Copy constructor; makes sure that the data is in a consistent state before it returns.
        AsyncState(const AsyncState & state)
            : _sync_0(0), _sync_1(0)
        {
            ++_sync_1;

            int sync0, sync1;
            do
            {
                sync0 = state._sync_0;
                this->r = state.r;
                this->q1 = state.q1;
                this->q0 = state.q0;
                this->index = state.index;
                sync1 = state._sync_1;
            }
            while (sync0 != sync1);

            ++_sync_0;
        }

        /// Assignment operator.  Increments the synclocks in reverse order so that a read while copying the rest of the data will fail.
        /// Makes sure that the source data is in a consistent state...
        AsyncState & operator= (const AsyncState & state)
        {
            ++_sync_1;

            int sync0, sync1;
            do
            {
                sync0 = state._sync_0;
                r = state.r;
                q1 = state.q1;
                q0 = state.q0;
                index = state.index;
                sync1 = state._sync_1;
            }
            while (sync0 != sync1);

            ++_sync_0;
            return *this;
        }
        //@}

        /// Destructor.
        virtual ~AsyncState() {}

        //@{
        /// The current state of the cell.
        const TState & current() const { return q0; }
        TState & current() { return q0; }
        //@}

        //@{
        /// The former state of the cell.
        const TState & former() const { return q1; }
        TState & former() { return q1; }
        //@}

        /// Write the cell's data.
        void writeBinary(QDataStream & ds, const AutomataFileVersion & file_version) const
        {
            q0.writeBinary(ds, file_version);
            q1.writeBinary(ds, file_version);
            ds << r;
        }

        /// Read the cell's data (both previous and current states).
        void readBinary(QDataStream & ds, const AutomataFileVersion & file_version)
        {
            q0.readBinary(ds, file_version);
            q1.readBinary(ds, file_version);

            if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_3)
            {
                ds >> r;
            }
            else if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_1)
            {
                quint8 num;
                ds >> num;
                r = static_cast<quint16>(r);
            }
            else // if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_OLD)
            {
                int num;
                ds >> num;
                r = static_cast<quint8>(num);
            }
        }
    };

    /// \deprecated
    /// should not be used (is only for old code that will be removed)
    template <typename TState, typename TIndex>
    QDataStream & operator>> (QDataStream & ds, AsyncState<TState,TIndex> & as)
    {
        Automata::AutomataFileVersion fv;
        fv.automata_version = 0;
        fv.client_version = 0;
        as.readBinary(ds, fv);
        return ds;
    }

} // namespace Automata


#endif // ASYNCSTATE_H
