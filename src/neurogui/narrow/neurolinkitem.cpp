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

#include "neurolinkitem.h"
#include "../subnetwork/subconnectionitem.h"
#include "../labnetwork.h"
#include "../labscene.h"
#include "../mainwindow.h"
#include "../../neurolib/neuronet.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPolygonF>
#include <QVector2D>
#include <QLinearGradient>

#include <QtProperty>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace NeuroLib;

namespace NeuroGui
{

    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNarrowItem(network, scenePos, context), MixinArrow(this),
        _weight_property(this, &NeuroLinkItem::weight, &NeuroLinkItem::setWeight,
                         tr("Weight"), tr("The output weight of the link.")),
        _length_property(this, &NeuroLinkItem::length, &NeuroLinkItem::setLength,
                         tr("Length"), tr("The number of timesteps for a signal to propagate through this link."))
    {
        Q_ASSERT(network != 0);
    }

    NeuroLinkItem::~NeuroLinkItem()
    {
    }

    NeuroCell::NeuroValue NeuroLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.last());
        return cell ? cell->current().weight() : 0;
    }

    void NeuroLinkItem::setWeight(const NeuroLib::NeuroCell::NeuroValue & value)
    {
        for (int i = 0; i < _cellIndices.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices[i]);
            if (cell)
            {
                if (i == _cellIndices.size() - 1)
                {
                    cell->current().setWeight(value);
                    cell->former().setWeight(value);
                }
                else
                {
                    cell->current().setWeight(1.1);
                    cell->former().setWeight(1.1);
                }
            }
        }
    }

    void NeuroLinkItem::setLength(const int & value)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroLib::NeuroNet *neuronet = network()->neuronet();

        bool updateValue = false;
        int newLength = value;
        if (newLength < 1)
        {
            newLength = 1;
            updateValue = true;
        }

        if (newLength != _cellIndices.size())
        {
            // add or delete cells from the automaton as necessary
            QList<NeuroCell::NeuroIndex> cellsToDelete;
            NeuroCell::NeuroIndex oldLastIndex = _cellIndices.last();

            if (newLength > _cellIndices.size())
            {
                while (_cellIndices.size() < newLength)
                {
                    NeuroCell::NeuroIndex lastIndex = _cellIndices.last();
                    addNewCell();

                    NeuroCell::NeuroIndex newIndex = _cellIndices.last();
                    neuronet->addEdge(newIndex, lastIndex);
                }
            }
            else if (newLength < _cellIndices.size())
            {
                while (_cellIndices.size() > newLength)
                {
                    NeuroCell::NeuroIndex lastIndex = _cellIndices.last();
                    cellsToDelete.append(lastIndex);
                    _cellIndices.removeAt(_cellIndices.size()-1);
                }
            }

            // now adjust the neighbors of our front target to reflect the new last cell
            NeuroCell::NeuroIndex newLastIndex = _cellIndices.last();
            NeuroNarrowItem *front = dynamic_cast<NeuroNarrowItem *>(_frontLinkTarget);
            if (front && newLastIndex != oldLastIndex)
            {
                NeuroCell::NeuroIndex frontIndex = front->cellIndices().first();

                // remember, the neighbors of a cell are its inputs
                neuronet->removeEdge(frontIndex, oldLastIndex);
                neuronet->addEdge(frontIndex, newLastIndex);
            }

            // delete the unused cells
            for (QListIterator<NeuroCell::NeuroIndex> i(cellsToDelete); i.hasNext(); )
            {
                neuronet->removeNode(i.next());
            }
        }

        // update property if necessary
        if (updateValue)
            _length_property.setValue(QVariant(newLength));
    }

    bool NeuroLinkItem::addIncoming(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addIncoming(linkItem);
    }

    bool NeuroLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _backLinkTarget)
            setBackLinkTarget(0);
        return NeuroNarrowItem::removeIncoming(linkItem);
    }

    bool NeuroLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addOutgoing(linkItem);
    }

    bool NeuroLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _frontLinkTarget)
            setFrontLinkTarget(0);
        return NeuroNarrowItem::removeOutgoing(linkItem);
    }

    void NeuroLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        MixinArrow::addLine(drawPath);
    }

    void NeuroLinkItem::setPenProperties(QPen & pen) const
    {
        NeuroNarrowItem::setPenProperties(pen);
        int oldWidth = pen.width();

        // get weight
        qreal w = qBound(0.0f, qAbs(weight()), 1.0f);

        // get output values
        QList<qreal> steps;

        for (int i = 0; i < _cellIndices.size(); ++i)
        {
            const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices[i]);
            if (cell)
                steps.append(qBound(0.0f, cell->current().outputValue(), 1.0f));
        }

        if (steps.size() == 1)
        {
            steps.append(steps.last());
        }
        else if (steps.size() == 0)
        {
            steps.append(0);
            steps.append(0);
        }

        // get gradient
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.last());
        bool frozen = cell && cell->current().frozen();

        QLinearGradient gradient(_line.p1(), _line.p2());

        for (int i = 0; i < steps.size(); ++i)
        {
            qreal t = i * (1.0f / (steps.size() - 1));

            QColor c = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, steps[i]);
            c = lerp(Qt::lightGray, c, w);

            if (frozen)
                c = lerp(c, Qt::gray, 0.5f);

            gradient.setColorAt(t, c);
        }

        // new pen
        pen = QPen(gradient, oldWidth);
    }

    void NeuroLinkItem::setBrushProperties(QBrush &brush) const
    {
        brush.setStyle(Qt::NoBrush);
    }

    void NeuroLinkItem::adjustLinks()
    {
        QVector2D center = QVector2D(scenePos()) + ((c1 + c2) * 0.5f);

        for (QListIterator<NeuroItem *> i(incoming()); i.hasNext(); )
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(i.next());
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }

    bool NeuroLinkItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !_dragFront)
            return false;

        // don't link to what we're already link to
        if (_dragFront && outgoing().contains(item))
            return false;
        else if (!_dragFront && incoming().contains(item))
            return false;

        return true;
    }

    bool NeuroLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be attached to by an inhibitory link
        bool result = dynamic_cast<NeuroInhibitoryLinkItem *>(item) != 0;

        // or a subconnection that is an inhibitory link
        SubConnectionItem *sub = dynamic_cast<SubConnectionItem *>(item);
        result = result || (sub && dynamic_cast<NeuroInhibitoryLinkItem *>(sub->governingItem()));

        return result;
    }

    void NeuroLinkItem::onAttachTo(NeuroItem *item)
    {
        if (_dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);
    }

    bool NeuroLinkItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // break links
        NeuroItem *linkedItem = _dragFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem && !linkedItem->contains(linkedItem->mapFromScene(mousePos)))
        {
            if (_dragFront)
                setFrontLinkTarget(0);
            else
                setBackLinkTarget(0);
        }

        // attach
        return NeuroNarrowItem::handleMove(mousePos, movePos);
    }

    QVariant NeuroLinkItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene = dynamic_cast<LabScene *>(scene());

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if (!_settingLine && labScene && dynamic_cast<NeuroLinkItem *>(labScene->itemUnderMouse()) == this)
                return MixinArrow::changePos(labScene, value);

        default:
            break;
        }

        return _settingLine ? value : NeuroNarrowItem::itemChange(change, value);
    }

    void NeuroLinkItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        ds << _line;

        if (_frontLinkTarget && id_map[_frontLinkTarget->id()])
            ds << static_cast<qint32>(id_map[_frontLinkTarget->id()]);
        else
            ds << static_cast<qint32>(0);

        if (_backLinkTarget && id_map[_backLinkTarget->id()])
            ds << static_cast<qint32>(id_map[_backLinkTarget->id()]);
        else
            ds << static_cast<qint32>(0);
    }

    void NeuroLinkItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        NeuroNarrowItem::readClipboard(ds, id_map);
        ds >> _line;

        qint32 id;

        // front link target
        ds >> id;
        if (id && id_map[id])
            setFrontLinkTarget(id_map[id]);

        // back link target
        ds >> id;
        if (id && id_map[id])
            setBackLinkTarget(id_map[id]);
    }

    void NeuroLinkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);
    }

    void NeuroLinkItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readBinary(ds, file_version);
        MixinArrow::readBinary(ds, file_version);
    }

    void NeuroLinkItem::writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writePointerIds(ds, file_version);
        MixinArrow::writePointerIds(ds, file_version);
    }

    void NeuroLinkItem::readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readPointerIds(ds, file_version);
        MixinArrow::readPointerIds(ds, file_version);
    }

    void NeuroLinkItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap)
    {
        NeuroNarrowItem::idsToPointers(idMap);
        MixinArrow::idsToPointers(idMap);
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroExcitoryLinkItem, QObject::tr("Narrow|Excitory Link"));

    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            _cellIndices.clear();
            addNewCell();
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }

    void NeuroExcitoryLinkItem::addNewCell()
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroCell::NeuroIndex index = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
        _cellIndices.append(index);
    }

    void NeuroExcitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D front(_line.p2());
        QVector2D dir(front - c2);

        addPoint(drawPath, _line.p2(), dir.normalized(), ELLIPSE_WIDTH);
    }

    bool NeuroExcitoryLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // cannot attach to a link at all
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(item);
        if (link)
            return false;

        return NeuroLinkItem::canAttachTo(pos, item);
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroInhibitoryLinkItem, QObject::tr("Narrow|Inhibitory Link"));

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        _weight_property.setEditable(false);

        if (context == CREATE_UI)
        {
            _cellIndices.clear();
            addNewCell();
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }

    void NeuroInhibitoryLinkItem::addNewCell()
    {
        NeuroCell::NeuroIndex index = network()->neuronet()->addNode(NeuroCell(NeuroCell::INHIBITORY_LINK, -1, 1));
        _cellIndices.append(index);
    }

    void NeuroInhibitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D center(pos());
        QVector2D front(line().p2());

        drawPath.addEllipse((front-center).toPointF(), ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
    }

} // namespace NeuroGui
