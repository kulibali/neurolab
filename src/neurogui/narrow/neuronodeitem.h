#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

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
#include "neuronarrowitem.h"
#include "../mixins/mixinremember.h"

namespace NeuroGui
{

    class NeuroLinkItem;

    /// Base class for node objects.
    class NEUROGUISHARED_EXPORT NeuroNodeItemBase
        : public NeuroNarrowItem, public MixinRemember
    {
        Q_OBJECT

        QRectF _rect;

    public:
        explicit NeuroNodeItemBase(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroNodeItemBase();

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see setRect()
        const QRectF & rect() const { return _rect; }

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see rect()
        void setRect(const QRectF & r) { _rect = r; update(_rect); }

        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *) const;
        virtual bool canBeAttachedToTwice(NeuroItem *) const;
        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *);

        virtual void adjustLinks();
        virtual void postLoad();

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map);

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        virtual QList<Index> getIncomingCellsFor(const NeuroItem *item) const { return _cellIndices; }
        virtual QList<Index> getOutgoingCellsFor(const NeuroItem *item) const { return _cellIndices; }

    protected:
        virtual bool canCreateNewOnMe(const QString & typeName, const QPointF & pos) const;

        virtual QVector2D getAttachPos(MixinArrow *link, const QVector2D &);

        virtual void setBrushProperties(QBrush &brush) const;
    };


    /// An item that represents a node in Narrow notation.
    class NEUROGUISHARED_EXPORT NeuroNodeItem
        : public NeuroNodeItemBase
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroNodeItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _inputs_property;
        Property<NeuroNodeItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _run_property;

    public:
        explicit NeuroNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroNodeItem();

        NeuroLib::NeuroCell::Value inputs() const;
        void setInputs(const NeuroLib::NeuroCell::Value &);

        NeuroLib::NeuroCell::Value run() const;
        void setRun(const NeuroLib::NeuroCell::Value &);

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual void setPenProperties(QPen &pen) const;
    };


    /// An item that represents an oscillator.
    class NEUROGUISHARED_EXPORT NeuroOscillatorItem
        : public NeuroNodeItemBase
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::Step> _phase_property;
        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::Step> _peak_property;
        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::Step> _gap_property;

    public:
        explicit NeuroOscillatorItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroOscillatorItem();

        NeuroLib::NeuroCell::Step phase() const;
        void setPhase(const NeuroLib::NeuroCell::Step &);

        NeuroLib::NeuroCell::Step peak() const;
        void setPeak(const NeuroLib::NeuroCell::Step &);

        NeuroLib::NeuroCell::Step gap() const;
        void setGap(const NeuroLib::NeuroCell::Step &);

    public slots:
        virtual void reset();

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

} // namespace NeuroGui

#endif // NEURONODEITEM_H
