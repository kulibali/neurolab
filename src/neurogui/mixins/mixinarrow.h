#ifndef MIXINARROW_H
#define MIXINARROW_H

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

#include "../neurogui_global.h"
#include "../neuroitem.h"
#include "../../neurolib/neurocell.h"

#include <QLineF>
#include <QVector2D>
#include <QVariant>
#include <QPainterPath>

class QGraphicsScene;

namespace NeuroGui
{

    class LabScene;

    /// Mixin class for drawing arrows.
    class NEUROGUISHARED_EXPORT MixinArrow
    {
        NeuroItem *_self;

    protected:
        QLineF _line;
        mutable QVector2D c1, c2;

        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool _dragFront, _settingLine;

        QSet<NeuroItem *> _incoming;

    public:
        /// Constructor.
        /// You must call this in derived classes' initialization, in order to correctly set the self pointer.
        explicit MixinArrow(NeuroItem *self);

        /// Destructor.
        virtual ~MixinArrow();

        NeuroItem *self() const { return _self; }

        /// The link's back (\c p1) and front (\c p2) position, in scene coordinates.
        QLineF line() const;

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        /// You can also specify a center point.
        void setLine(const QLineF & l, const QPointF *const c = 0);

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        void setLine(const QPointF & p1, const QPointF & p2);

        /// If the link is being moved, returns whether or not the front end of the node is being dragged.
        /// \see MixinArrow::setDragFront()
        bool dragFront() const { return _dragFront; }

        /// Sets whether or not the link is being dragged by the front or back.
        /// \see MixinArrow::dragFront()
        void setDragFront(bool df) { _dragFront = df; }

        /// The item that the front of the link is currently attached to.
        /// \see line()
        /// \see setFrontLinkTarget()
        NeuroItem *frontLinkTarget() const { return _frontLinkTarget; }

        /// Sets the item that the front of the link is currently attached to.
        /// \see line()
        /// \see frontLinkTarget()
        virtual void setFrontLinkTarget(NeuroItem *linkTarget);

        /// The item that the back of the link is currently attached to.
        /// \see line()
        /// \see setBackLinkTarget()
        NeuroItem *backLinkTarget() const { return _backLinkTarget; }

        /// Sets the item that the back of the link is currently attached to.
        /// \see line()
        /// \see backLinkTarget()
        virtual void setBackLinkTarget(NeuroItem *linkTarget);

        /// Used to shortcut setting the gradient for subconnection items.
        virtual void setPenGradient(QPen &, const QLineF &) const {}

        virtual QList<NeuroLib::NeuroCell::Index> getFrontwardCells() const { return QList<NeuroLib::NeuroCell::Index>(); }
        virtual QList<NeuroLib::NeuroCell::Index> getBackwardCells() const { return QList<NeuroLib::NeuroCell::Index>(); }

    protected:
        /// Draws the arrow's shaft.
        void addLine(QPainterPath & drawPath) const;

        /// Draws an arrowhead.
        void addPoint(QPainterPath & drawPath, const QPointF & pos, const QVector2D & dir, const qreal & len) const;

        /// Handles dragging one end of the arrow.
        void changePos(QPointF & value, bool canDragFront = true, bool canDragBack = true);

        /// Handles breaking a link when it is moved away from its target.
        void breakLinks(const QPointF & mousePos);

        void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map);

        /// Write to a data stream.
        void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;

        /// Read from a data stream.
        void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        /// Write pointer Ids.
        /// \see NeuroItem::writePointerIds()
        void writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const;

        /// Read pointer Ids.
        /// \see NeuroItem::readPointerIds()
        void readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version);

        /// Convert pointer Ids that were read from a file into pointers to the loaded items.
        /// \see NeuroItem::idsToPointers()
        void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);
    }; // class MixinArrow

} // namespace NeuroGui

#endif // MIXINARROW_H
