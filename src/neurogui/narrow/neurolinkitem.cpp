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

#include "neurolinkitem.h"
#include "neuronodeitem.h"
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

    NeuroCell::Value NeuroLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.last());
        return cell ? cell->current().weight() : 0;
    }

    void NeuroLinkItem::setWeight(const NeuroLib::NeuroCell::Value & value)
    {
        for (int i = 0; i < _cellIndices.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices[i]);
            if (cell)
            {
                if (i == _cellIndices.size() - 1)
                {
                    cell->current().setWeight(value);
                }
                else
                {
                    cell->current().setWeight(1.1);
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
            bool fr = frozen();
            int per = persist();

            // disconnect connections
            foreach (NeuroItem *ni, connections())
                removeEdges(ni);

            // add or delete cells from the automaton as necessary
            QList<NeuroCell::Index> cellsToDelete;

            if (newLength > _cellIndices.size())
            {
                while (_cellIndices.size() < newLength)
                {
                    NeuroCell::Index firstIndex = _cellIndices.first();
                    NeuroCell::Index newIndex = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT));
                    _cellIndices.insert(0, newIndex);

                    neuronet->addEdge(firstIndex, newIndex);
                }
            }
            else if (newLength < _cellIndices.size())
            {
                while (_cellIndices.size() > newLength)
                {
                    NeuroCell::Index firstIndex = _cellIndices.first();
                    cellsToDelete.append(firstIndex);
                    _cellIndices.removeFirst();
                }
            }

            // delete the unused cells
            foreach (NeuroCell::Index i, cellsToDelete)
                neuronet->removeNode(i);

            // re-connect
            foreach (NeuroItem *ni, connections())
                addEdges(ni);

            setFrozen(fr);
            setPersist(per);
        }

        // update property if necessary
        if (updateValue)
            _length_property.setValueInPropertyBrowser(QVariant(newLength));
    }

    QList<NeuroLinkItem::Index> NeuroLinkItem::getFrontwardCells() const
    {
        return _cellIndices;
    }

    void NeuroLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        MixinArrow::addLine(drawPath);
    }

    void NeuroLinkItem::setPenProperties(QPen & pen) const
    {
        setPenGradient(pen, _line);

        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);
    }

    void NeuroLinkItem::setPenGradient(QPen &pen, const QLineF &line) const
    {
        // get weight
        qreal w = qBound(0.0f, qAbs(weight()), 1.0f);

        // get output values
        QList<qreal> steps;

        for (int i = 0; i < _cellIndices.size(); ++i)
        {
            const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices[i]);
            if (cell)
                steps.append(qBound(0.0f, qAbs(cell->current().outputValue()), 1.0f));
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

        QLinearGradient gradient(line.p1(), line.p2());

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
        pen = QPen(gradient, pen.width());
    }

    void NeuroLinkItem::adjustLinks()
    {
        QVector2D front(_line.p2());
        QVector2D back(_line.p1());
        QVector2D center = front + (c2 - front) * 0.33 + (back - front) * 0.1;

        foreach (NeuroItem *ni, _incoming)
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(ni);
            if (link)
                link->setLine(link->line().p1(), mapToScene(center.toPointF()));
        }
    }

    QList<NeuroLinkItem::Index> NeuroLinkItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        QList<Index> result;
        const NeuroInhibitoryLinkItem *inhibit = dynamic_cast<const NeuroInhibitoryLinkItem *>(item);
        if (inhibit)
        {
            result.append(_cellIndices.last());
        }
        else if (item == _backLinkTarget)
        {
            result.append(_cellIndices.first());
        }
        return result;
    }

    QList<NeuroLinkItem::Index> NeuroLinkItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        QList<Index> result;
        if (item == _frontLinkTarget)
        {
            result.append(_cellIndices.last());
        }
        return result;
    }

    bool NeuroLinkItem::canAttachTo(const QPointF &, NeuroItem *item) const
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !_dragFront)
            return false;

        // don't link to what we're already linked to
        if (_dragFront && _frontLinkTarget == item)
            return false;
        if (!_dragFront && _backLinkTarget == item)
            return false;

        return true;
    }

    bool NeuroLinkItem::canAttachTwice(NeuroItem *item) const
    {
        return !(_frontLinkTarget == item && _backLinkTarget == item);
    }

    bool NeuroLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item) const
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

        NeuroNarrowItem::onAttachTo(item);
    }

    void NeuroLinkItem::onAttachedBy(NeuroItem *item)
    {
        _incoming.insert(item);
        NeuroNarrowItem::onAttachedBy(item);
        addEdges(item);
        adjustLinks();
    }

    void NeuroLinkItem::onDetach(NeuroItem *item)
    {
        if (item == _frontLinkTarget)
            setFrontLinkTarget(0);
        else if (item == _backLinkTarget)
            setBackLinkTarget(0);

        _incoming.remove(item);
        NeuroNarrowItem::onDetach(item);
    }

    bool NeuroLinkItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // move line
        MixinArrow::changePos(movePos);

        // break links
        MixinArrow::breakLinks(mousePos);

        // attach
        return NeuroNarrowItem::handleMove(mousePos, movePos);
    }

    void NeuroLinkItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        MixinArrow::writeClipboard(ds, id_map);
    }

    void NeuroLinkItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        NeuroNarrowItem::readClipboard(ds, id_map);
        MixinArrow::readClipboard(ds, id_map);
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

    NEUROITEM_DEFINE_CREATOR(NeuroExcitoryLinkItem, QObject::tr("Narrow"), QObject::tr("Excitory Link"), ":/icons/narrow_excitory.png");

    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            _cellIndices.clear();
            _cellIndices.append(addNewCell());
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }

    NeuroCell::Index NeuroExcitoryLinkItem::addNewCell()
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroCell::Index index = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT));
        return index;
    }

    void NeuroExcitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D front(_line.p2());
        QVector2D back(_line.p1());
        QVector2D dir = (front - c2).normalized() * 0.6
                + (front - c1).normalized() * 0.25
                + (front - back).normalized() * 0.15;

        addPoint(drawPath, _line.p2(), dir.normalized(), ELLIPSE_WIDTH);
    }

    bool NeuroExcitoryLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item) const
    {
        // cannot attach to a link at all
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(item);
        if (link)
            return false;

        return NeuroLinkItem::canAttachTo(pos, item);
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroInhibitoryLinkItem, QObject::tr("Narrow"), QObject::tr("Inhibitory Link"), ":/icons/narrow_inhibitory.png");

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        _weight_property.setEditable(false);

        if (context == CREATE_UI)
        {
            _cellIndices.clear();
            _cellIndices.append(addNewCell());
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }

    NeuroCell::Index NeuroInhibitoryLinkItem::addNewCell()
    {
        NeuroCell::Index index = network()->neuronet()->addNode(NeuroCell(NeuroCell::INHIBITORY_LINK, -NeuroCell::DEFAULT_LINK_WEIGHT));
        return index;
    }

    void NeuroInhibitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D center(pos());
        QVector2D front(line().p2());

        drawPath.addEllipse((front-center).toPointF(), ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
    }

} // namespace NeuroGui
