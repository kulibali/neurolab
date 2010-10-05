/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
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
#include "../labexception.h"
#include "../labtree.h"
#include "../labscene.h"
#include "../labview.h"
#include "../labnetwork.h"
#include "../mainwindow.h"
#include "../narrow/neurolinkitem.h"
#include "subconnectionitem.h"

#include <QScrollBar>

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubNetworkItem, QObject::tr("Misc|Sub-Network"));

    SubNetworkItem::SubNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context), MixinRemember(this),
          _treeNodeIdNeeded(static_cast<quint32>(-1)), _treeNode(0),
          _rect(-15, -10, 30, 20)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            makeSubNetwork();
        }
    }

    SubNetworkItem::~SubNetworkItem()
    {
        // the tree node will be deleted by the tree itself, so we don't need to delete it
    }

    void SubNetworkItem::propertyValueChanged(QtProperty *p, const QVariant & val)
    {
        if (_label_property.isPropertyFor(p) && _treeNode)
        {
            _treeNode->setLabel(val.toString());
        }

        NeuroItem::propertyValueChanged(p, val);
    }

    bool SubNetworkItem::addIncoming(NeuroItem *linkItem)
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(linkItem);
        if (NeuroItem::addIncoming(linkItem) && link)
        {
            addConnectionItem(link, true);
            return true;
        }
        return false;
    }

    bool SubNetworkItem::removeIncoming(NeuroItem *linkItem)
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(linkItem);
        if (NeuroItem::removeIncoming(linkItem) && link)
        {
            removeConnectionItem(link);
            return true;
        }
        return false;
    }

    bool SubNetworkItem::addOutgoing(NeuroItem *linkItem)
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(linkItem);
        if (NeuroItem::addOutgoing(linkItem) && link)
        {
            addConnectionItem(link, false);
            return true;
        }
        return false;
    }

    bool SubNetworkItem::removeOutgoing(NeuroItem *linkItem)
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(linkItem);
        if (NeuroItem::removeOutgoing(linkItem) && link)
        {
            removeConnectionItem(link);
            return true;
        }
        return false;
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

    void SubNetworkItem::addConnectionItem(NeuroLinkItem *governingLink, bool is_incoming)
    {
        if (!(network() && network()->scene()))
            return;

        if (!governingLink)
            return;

        makeSubNetwork();
        if (!(_treeNode && _treeNode->scene() && _treeNode->view()))
            return;

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

        sPos = rCenter + sDir * (sPos - rCenter).length() * 0.8f;

        // direction is only in/out for now...
        quint32 direction = is_incoming ? SubConnectionItem::INCOMING : SubConnectionItem::OUTGOING;
        SubConnectionItem *subItem = new SubConnectionItem(network(), sPos.toPointF(), NeuroItem::CREATE_UI, this, governingLink, direction, sPos, -sDir);
        _treeNode->scene()->addItem(subItem);
    }

    void SubNetworkItem::removeConnectionItem(NeuroLinkItem *governingLink)
    {
        SubConnectionItem *subItem = _connections[governingLink];
        if (subItem)
        {
            _connections.remove(governingLink);
            delete subItem;
        }
    }

    bool SubNetworkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        return dynamic_cast<NeuroLinkItem *>(item) != 0;
    }

    void SubNetworkItem::onAttachedBy(NeuroItem *item)
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);
        }
    }

    void SubNetworkItem::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    QVector2D SubNetworkItem::getAttachPos(const QVector2D & dirTo)
    {
        // just extend beyond us and clip
        QVector2D newPos = dirTo * (_rect.width() + _rect.height());
        clipTo(newPos, _rect);
        return newPos;
    }

    void SubNetworkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

        drawPath.addRect(_rect);
    }

    void SubNetworkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
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
        MainWindow *mainWindow = MainWindow::instance();
        Q_ASSERT(mainWindow);
        Q_ASSERT(mainWindow->currentNetwork());

        // we have an ID from the file, and we need to find it in the current network
        if (_treeNodeIdNeeded != static_cast<quint32>(-1) && mainWindow->currentNetwork())
        {
            _treeNode = mainWindow->currentNetwork()->findSubNetwork(_treeNodeIdNeeded);
        }

        // we don't have a tree node; make a new one
        if (!_treeNode && mainWindow->currentNetwork())
        {
            _treeNode = mainWindow->currentNetwork()->newSubNetwork();
        }

        // don't look for existing node anymore
        if (_treeNode)
        {
            _treeNodeIdNeeded = static_cast<quint32>(-1);

            setLabel(_treeNode->label());
            connect(_treeNode, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));
        }
    }

    void SubNetworkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroItem::writeBinary(ds, file_version);

        quint32 id_to_write = _treeNode ? _treeNode->id() : static_cast<quint32>(-1);
        ds << id_to_write;
    }

    void SubNetworkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_3)
        {
            ds >> _treeNodeIdNeeded;
        }
    }

    void SubNetworkItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroItem::writePointerIds(ds, file_version);

        QList<NeuroItem *> keys = _connections.keys();
        quint32 n = keys.size();
        ds << n;

        for (quint32 i = 0; i < n; ++i)
        {
            NeuroItem::IdType id = keys[i]->id();
            ds << id;

            id = _connections[keys[i]]->id();
            ds << id;
        }
    }

    void SubNetworkItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readPointerIds(ds, file_version);

        _connections.clear();
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

                _connections[key_ptr] = val_ptr;
            }
        }
    }

    void SubNetworkItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        NeuroItem::idsToPointers(idMap);

        QMap<NeuroItem *, SubConnectionItem *> newConnections;
        QList<NeuroItem *> keys = _connections.keys();

        for (QListIterator<NeuroItem *> i(keys); i.hasNext(); )
        {
            NeuroItem *key_ptr = i.next();
            SubConnectionItem *val_ptr = _connections[key_ptr];

            NeuroItem::IdType key_id = reinterpret_cast<NeuroItem::IdType>(key_ptr);
            NeuroItem::IdType val_id = reinterpret_cast<NeuroItem::IdType>(val_ptr);

            key_ptr = idMap[key_id];
            val_ptr = dynamic_cast<SubConnectionItem *>(idMap[val_id]);

            if (key_ptr && val_ptr)
            {
                newConnections[key_ptr] = val_ptr;
            }
            else
            {
                throw new LabException(tr("Something went wrong trying to get the subnetwork connection items: %1 %2").arg(key_id).arg(val_id));
            }
        }

        _connections = newConnections;
    }

} // namespace NeuroGui