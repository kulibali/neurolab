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

#include "subnetworkitem.h"
#include "../labtree.h"
#include "../labscene.h"
#include "../labview.h"
#include "../labnetwork.h"
#include "../mainwindow.h"
#include "../narrow/neurolinkitem.h"
#include "subconnectionitem.h"

#include <QScrollBar>

using namespace NeuroLib;

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubNetworkItem, QObject::tr("Misc"), QObject::tr("Sub-Network"), ":/icons/sub_network.png");

    SubNetworkItem::SubNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNetworkItem(network, scenePos, context), MixinRemember(this), LabTreeNodeController(),
          _treeNodeIdNeeded(static_cast<quint32>(-1)), _treeNode(0),
          _rect(-15, -10, 30, 20)
    {
        _value_property.setVisible(false);

        connect(this, SIGNAL(subnetworkDeleted(LabTreeNode*)), MainWindow::instance(), SLOT(treeNodeDeleted(LabTreeNode*)));

        if (context == NeuroItem::CREATE_UI)
        {
            makeSubNetwork();
            setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
        }
    }

    SubNetworkItem::~SubNetworkItem()
    {
        if (_ui_delete && _treeNode)
        {
            emit subnetworkDeleted(_treeNode);

            _treeNode->setUIDelete();
            delete _treeNode;
        }
    }

    void SubNetworkItem::propertyInBrowserChanged(QtProperty *p, const QVariant & val)
    {
        if (_label_property.propertyInBrowser() == p && _treeNode)
        {
            _treeNode->setLabel(val.toString());
        }

        NeuroNetworkItem::propertyInBrowserChanged(p, val);
    }

    static void clipTo(QVector2D & p, const QRectF & r)
    {
        if (p.x() < r.x())
        {
            qreal new_x = r.x();
            qreal new_y = p.y() * new_x / p.x();

            p.setX(new_x);
            p.setY(new_y);
        }

        if (p.x() > r.x() + r.width())
        {
            qreal new_x = r.x() + r.width();
            qreal new_y = p.y() * new_x / p.x();

            p.setX(new_x);
            p.setY(new_y);
        }

        if (p.y() < r.y())
        {
            qreal new_y = r.y();
            qreal new_x = p.x() * new_y / p.y();

            p.setX(new_x);
            p.setY(new_y);
        }

        if (p.y() > r.y() + r.height())
        {
            qreal new_y = r.y() + r.height();
            qreal new_x = p.x() * new_y / p.y();

            p.setX(new_x);
            p.setY(new_y);
        }
    }

    void SubNetworkItem::addConnectionItem(NeuroItem *governingLink, const SubConnectionItem::Directions & direction)
    {
        if (!(network() && network()->scene()))
            return;

        if (!governingLink)
            return;

        makeSubNetwork();
        if (!(_treeNode && _treeNode->scene() && _treeNode->view()))
            return;

        qDebug("adding connection item");

        // get the view's viewport (expanded to the size of ours, if necessary)
        QRectF vr = network()->treeNode()->view()->viewport()->rect();

        QPointF tl(_treeNode->view()->horizontalScrollBar()->value(), _treeNode->view()->verticalScrollBar()->value());
        QPointF br = tl + vr.bottomRight();
        QMatrix mat = _treeNode->view()->matrix().inverted();

        QRectF rect = mat.mapRect(QRectF(tl, br));

        // get a point on the edge of the viewport correspond to the outer position
        QVector2D gPos(network()->scene()->lastMousePos());
        QVector2D sDir = gPos - QVector2D(scenePos());
        sDir.normalize();

        QVector2D rCenter(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
        QVector2D sPos = rCenter + sDir * (rect.width() + rect.height());
        clipTo(sPos, rect);

        sPos = rCenter + sDir * (sPos - rCenter).length() * 0.5f;

        // direction is only in/out for now...
        SubConnectionItem *subItem = new SubConnectionItem(network(), sPos.toPointF(), NeuroItem::CREATE_UI,
                                                           this, governingLink, direction, sPos, -sDir);
        _treeNode->scene()->addItem(subItem);

        // record connection
        _subconnections[governingLink] = subItem;
    }

    void SubNetworkItem::removeConnectionItem(NeuroItem *governingLink)
    {
        if (_subconnections.contains(governingLink))
        {
            SubConnectionItem *subItem = _subconnections[governingLink];
            _subconnections.remove(governingLink);

            subItem->setUIDelete();
            delete subItem;
        }
    }

    bool SubNetworkItem::canCreateNewItem(const QString &, const QPointF &) const
    {
        return true;
    }

    bool SubNetworkItem::canCreateNewOnMe(const QString & typeName, const QPointF &) const
    {
        return typeName.contains("LinkItem");
    }

    QList<SubNetworkItem::Index> SubNetworkItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        if (_subconnections.contains(item))
            return _subconnections[item]->getIncomingCellsFor(this);
        else
            return QList<Index>();
    }

    QList<SubNetworkItem::Index> SubNetworkItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        if (_subconnections.contains(item))
            return _subconnections[item]->getOutgoingCellsFor(this);
        else
            return QList<Index>();
    }

    bool SubNetworkItem::canBeAttachedBy(const QPointF &, NeuroItem *item) const
    {
        return dynamic_cast<MixinArrow *>(item) != 0;
    }

    void SubNetworkItem::onAttachedBy(NeuroItem *item)
    {
        NeuroNetworkItem::onAttachedBy(item);

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);

            SubConnectionItem::Directions dirs(0);
            if (item->isBidirectional())
            {
                dirs |= SubConnectionItem::BOTH;
            }
            else
            {
                if (link->frontLinkTarget() == this)
                    dirs |= SubConnectionItem::INCOMING;
                if (link->backLinkTarget() == this)
                    dirs |= SubConnectionItem::OUTGOING;
            }

            addConnectionItem(item, dirs);
        }
    }

    void SubNetworkItem::onDetach(NeuroItem *item)
    {
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            removeConnectionItem(item);
            MixinRemember::onDetach(link);
        }

        NeuroNetworkItem::onDetach(item);
    }

    void SubNetworkItem::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    QVector2D SubNetworkItem::getAttachPos(MixinArrow *, const QVector2D & pos)
    {
        // just extend beyond us and clip
        QVector2D newPos = pos.normalized() * (_rect.width() + _rect.height());
        clipTo(newPos, _rect);
        return newPos;
    }

    void SubNetworkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);

        drawPath.addRect(_rect);
    }

    void SubNetworkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        makeSubNetwork();

        if (_treeNode)
        {
            setSelected(false);
            MainWindow::instance()->setSubNetwork(_treeNode);
        }
    }

    void SubNetworkItem::makeSubNetwork()
    {
        bool hook_up = false;
        LabNetwork *current_network = network();

        // we have an ID from the file, and we need to find it in the current network
        if (_treeNodeIdNeeded != static_cast<quint32>(-1) && current_network)
        {
            _treeNode = current_network->findSubNetwork(_treeNodeIdNeeded);
            hook_up = true;
        }

        // we don't have a tree node; make a new one
        if (!_treeNode && current_network)
        {
            _treeNode = current_network->newSubNetwork();
            hook_up = true;
        }

        // don't look for existing node anymore
        if (hook_up && _treeNode)
        {
            _treeNodeIdNeeded = static_cast<quint32>(-1);

            setLabel(_treeNode->label());
            connect(_treeNode, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));
        }

        // make sure we're the tree node's controller
        if (_treeNode)
            _treeNode->setController(this);
    }

    void SubNetworkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNetworkItem::writeBinary(ds, file_version);

        quint32 id_to_write = _treeNode ? _treeNode->id() : static_cast<quint32>(-1);
        ds << id_to_write;
        ds << _rect;
    }

    void SubNetworkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_3)
        {
            ds >> _treeNodeIdNeeded;
        }

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            ds >> _rect;
        }
    }

    void SubNetworkItem::postLoad()
    {
        NeuroNetworkItem::postLoad();

        QVector2D center(scenePos());
        rememberItems(connections(), center);
        makeSubNetwork();

        setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
    }

    void SubNetworkItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writePointerIds(ds, file_version);

        QList<const NeuroItem *> keys = _subconnections.keys();
        quint32 n = keys.size();
        ds << n;

        for (quint32 i = 0; i < n; ++i)
        {
            NeuroNetworkItem::IdType id = keys[i]->id();
            ds << id;

            id = _subconnections[keys[i]]->id();
            ds << id;
        }
    }

    void SubNetworkItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readPointerIds(ds, file_version);

        _subconnections.clear();
        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_5)
        {
            quint32 num;
            ds >> num;

            for (quint32 i = 0; i < num; ++i)
            {
                NeuroItem::IdType key, value;
                ds >> key;
                ds >> value;

                NeuroItem *key_ptr = reinterpret_cast<NeuroItem *>(key);
                SubConnectionItem *val_ptr = reinterpret_cast<SubConnectionItem *>(value);

                _subconnections[key_ptr] = val_ptr;
            }
        }
    }

    void SubNetworkItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        NeuroNetworkItem::idsToPointers(idMap);

        QMap<const NeuroItem *, SubConnectionItem *> newConnections;
        QList<const NeuroItem *> keys = _subconnections.keys();

        foreach (const NeuroItem *key_ptr, keys)
        {
            SubConnectionItem *val_ptr = _subconnections[key_ptr];

            IdType key_id = static_cast<IdType>(reinterpret_cast<quint64>(key_ptr));
            IdType val_id = static_cast<IdType>(reinterpret_cast<quint64>(val_ptr));

            key_ptr = idMap[key_id];
            val_ptr = dynamic_cast<SubConnectionItem *>(idMap[val_id]);

            if (key_ptr && val_ptr)
            {
                newConnections[key_ptr] = val_ptr;
            }
            else
            {
                throw Common::FileFormatError(tr("Something went wrong trying to get the subnetwork connection items: %1 %2").arg(key_id).arg(val_id));
            }
        }

        _subconnections = newConnections;
    }

} // namespace NeuroGui
