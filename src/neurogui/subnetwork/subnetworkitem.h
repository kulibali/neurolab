#ifndef SUBNETWORKITEM_H
#define SUBNETWORKITEM_H

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
#include "../labtree.h"
#include "../neuronetworkitem.h"
#include "../mixins/mixinremember.h"
#include "subconnectionitem.h"

#include <QMap>
#include <QRectF>

namespace NeuroGui
{

    class LabTreeNode;

    /// An item that represents a sub-network that can be opened.
    class NEUROGUISHARED_EXPORT SubNetworkItem
        : public NeuroNetworkItem, public MixinRemember, public LabTreeNodeController
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        quint32 _treeNodeIdNeeded;
        LabTreeNode *_treeNode;

        QRectF _rect;

        /// Maps from inputs in the outer network to connection items in the inner network.
        QMap<const NeuroItem *, SubConnectionItem *> _subconnections;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit SubNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~SubNetworkItem();

        LabTreeNode *treeNode() { return _treeNode; }
        const QRectF & rect() const { return _rect; }
        void setRect(const QRectF & r) { _rect = r; }

        virtual NeuroLib::NeuroCell::Value outputValue() const { return 0; }
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value &) { }

        virtual QList<Index> allCells() const { return QList<Index>(); }

        virtual QList<Index> getIncomingCellsFor(const NeuroItem *item) const;
        virtual QList<Index> getOutgoingCellsFor(const NeuroItem *item) const;

    signals:
        /// Emitted when the subnetwork item is deleted.
        void subnetworkDeleted(LabTreeNode *treeNode);

    public slots:
        /// Synchronize the item's data with the property editor.
        virtual void propertyInBrowserChanged(QtProperty *, const QVariant &);

    protected:
        virtual void onEnterView() {}
        virtual void onLeaveView() {}
        virtual bool allowZoom() const { return true; }
        virtual bool canCreateNewItem(const QString &, const QPointF &) const;

        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const;
        virtual bool canAttachTo(const QPointF &, NeuroItem *) const { return false; }
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *) const;

        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *);

        virtual void adjustLinks();
        virtual QVector2D getAttachPos(MixinArrow *, const QVector2D &);

        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        /// Handles double-clicks to open the sub-network.
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);
        virtual void postLoad();

        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);

        virtual void makeSubNetwork();

    private:
        void addConnectionItem(NeuroItem *governingLink, const SubConnectionItem::Directions & direction);
        void removeConnectionItem(NeuroItem *governingLink);
    };

} // namespace NeuroGui

#endif // SUBNETWORKITEM_H
