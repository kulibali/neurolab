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

#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labview.h"
#include "../neurogui/labtree.h"
#include "../neurogui/mainwindow.h"
#include "gridedgeitem.h"
#include "multigridioitem.h"

using namespace NeuroGui;

namespace GridItems
{

    const QString
#include "../version.txt"
    ;

    NEUROITEM_DEFINE_PLUGIN_CREATOR(NeuroGridItem, QString("Grid Items"), QObject::tr("Grid Item"), ":/griditems/icons/grid_item.png", GridItems::VERSION)

    NeuroGridItem::NeuroGridItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : SubNetworkItem(network, scenePos, context),
          _horizontal_property(this, &NeuroGridItem::horizontalCols, &NeuroGridItem::setHorizontalCols, tr("Horizontal Repeats")),
          _vertical_property(this, &NeuroGridItem::verticalRows, &NeuroGridItem::setVerticalRows, tr("Vertical Repeats")),
          _num_horiz(1), _num_vert(1), _pattern_changed(true)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            const int width = NODE_WIDTH*4;
            const int height = NODE_WIDTH*2;
            const int left = -width/2;
            const int top = -height/2;

            setRect(QRectF(left, top, width, height));
            setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
            treeNode()->setLabel(tr("Grid Item %1").arg(treeNode()->id()));
        }

        connect(network, SIGNAL(networkChanged()), this, SLOT(networkChanged()));
        connect(network, SIGNAL(stepClicked()), this, SLOT(networkStepClicked()));
        connect(network, SIGNAL(stepFinished()), this, SLOT(networkStepFinished()));
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    void NeuroGridItem::makeSubNetwork()
    {
        SubNetworkItem::makeSubNetwork();
        resizeScene();
    }

    void NeuroGridItem::networkChanged()
    {
        _pattern_changed = true;
    }

    void NeuroGridItem::networkStepClicked()
    {
        generateGrid();
    }

    void NeuroGridItem::networkStepFinished()
    {
        // this gets set by networkChanged(), but since we've just finished a step,
        // we don't want to re-generate the grid unless something else changes
        _pattern_changed = false;
    }

    void NeuroGridItem::resizeScene()
    {
        LabView *view = network()->view();
        Q_ASSERT(view);

        QRect viewRect = view->viewport()->rect();
        QPointF topLeft = view->mapToScene(viewRect.topLeft());
        QPointF bottomRight = view->mapToScene(viewRect.bottomRight());

        QRect newSceneRect(topLeft.toPoint(), bottomRight.toPoint());

        treeNode()->scene()->setSceneRect(newSceneRect);
    }

    QList<NeuroGridItem::Index> NeuroGridItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        NeuroNetworkItem *ni = const_cast<NeuroNetworkItem *>(dynamic_cast<const NeuroNetworkItem *>(item));

        if (_top_connections.contains(ni))
            return _top_incoming;
        else if (_bottom_connections.contains(ni))
            return _bot_incoming;

        return QList<Index>();
    }

    QList<NeuroGridItem::Index> NeuroGridItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        NeuroNetworkItem *ni = const_cast<NeuroNetworkItem *>(dynamic_cast<const NeuroNetworkItem *>(item));

        if (_top_connections.contains(ni))
            return _top_outgoing;
        else if (_bottom_connections.contains(ni))
            return _bot_outgoing;

        return QList<Index>();
    }

    static void add_edges_in_groups(const QList<NeuroGridItem::Index> & ins, const QList<NeuroGridItem::Index> & outs, NeuroLib::NeuroNet *neuronet, QMap<NeuroGridItem::Index, NeuroGridItem::Index> & edges)
    {
        int in_step = 1;
        int out_step = 1;

        if (ins.size() > outs.size())
        {
            in_step = ins.size() / outs.size();
            if (in_step == 0)
                in_step = 1;
        }
        else if (ins.size() < outs.size())
        {
            out_step = outs.size() / ins.size();
            if (out_step == 0)
                out_step = 1;
        }

        int in_pos = 0;
        int out_pos = 0;
        int i = 0;

        while (in_pos < ins.size() && out_pos < outs.size())
        {
            NeuroGridItem::Index in = ins[in_pos];
            NeuroGridItem::Index out = outs[out_pos];

            if (in != -1 && out != -1)
            {
                edges.insert(in, out);
                neuronet->addEdge(in, out);
            }

            ++i;
            if ((i % in_step) == 0)
                ++in_pos;
            if ((i % out_step) == 0)
                ++out_pos;
        }
    }

    void NeuroGridItem::addEdges(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());
        NeuroLib::NeuroNet *neuronet = network()->neuronet();

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            QList<Index> myIns = this->getIncomingCellsFor(item);
            QList<Index> myOuts = this->getOutgoingCellsFor(item);
            QList<Index> itemOuts;
            QList<Index> itemIns;

            MultiGridIOItem *gi;
            MixinArrow *link;

            // connect in equal proportions to multi io items
            if ((gi = dynamic_cast<MultiGridIOItem *>(item)))
            {
                itemOuts = ni->getOutgoingCellsFor(this);
                itemIns = ni->getIncomingCellsFor(this);

                if (myIns.size() > 0 && myOuts.size() > 0 && itemOuts.size() > 0 && itemIns.size() > 0)
                {
                    // my in to item out
                    add_edges_in_groups(myIns, itemOuts, neuronet, _edges[ni]);

                    // item in to my out
                    add_edges_in_groups(itemIns, myOuts, neuronet, _edges[ni]);

                    return; // don't connect up the regular way
                }
            }

            // connect to the TARGET of a link instead of the link itself
            else if ((link = dynamic_cast<MixinArrow *>(item)))
            {
                NeuroNetworkItem *tgt;

                if ((link->frontLinkTarget() == this && (tgt = dynamic_cast<NeuroNetworkItem *>(link->backLinkTarget())))
                    || (link->backLinkTarget() == this && (tgt = dynamic_cast<NeuroNetworkItem *>(link->frontLinkTarget()))))
                {
                    itemOuts = tgt->getOutgoingCellsFor(item);
                    itemIns = tgt->getIncomingCellsFor(item);

                    if (itemIns.size() > 0)
                        ni->setOverrideIndex(itemIns.first());
                }
                else
                {
                    itemOuts = ni->getOutgoingCellsFor(this);
                    itemIns = ni->getIncomingCellsFor(this);
                }
            }

            // otherwise
            else
            {
                itemOuts = ni->getOutgoingCellsFor(this);
                itemIns = ni->getIncomingCellsFor(this);
            }

            // connect
            foreach (Index myIn, myIns)
            {
                foreach (Index itemOut, itemOuts)
                {
                    if (myIn != -1 && itemOut != -1)
                    {
                        _edges[ni].insert(myIn, itemOut);
                        neuronet->addEdge(myIn, itemOut);
                    }
                }
            }

            foreach (Index myOut, myOuts)
            {
                foreach (Index itemIn, itemIns)
                {
                    if (myOut != -1 && itemIn != -1)
                    {
                        _edges[ni].insert(itemIn, myOut);
                        neuronet->addEdge(itemIn, myOut);
                    }
                }
            }
        }
    }

    void NeuroGridItem::removeEdges(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());
        NeuroLib::NeuroNet *neuronet = network()->neuronet();

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni && _edges.contains(ni))
        {
            ni->setOverrideIndex(-1);

            QMap<Index, Index> & item_edges = _edges[ni];
            QMap<Index, Index>::const_iterator i = item_edges.constBegin(), end = item_edges.constEnd();
            while (i != end)
            {
                neuronet->removeEdge(i.key(), i.value());
                ++i;
            }
            item_edges.clear();
        }
    }

    void NeuroGridItem::onEnterView()
    {
        resizeScene();
    }

    void NeuroGridItem::onLeaveView()
    {
    }

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);

        const QRectF & r = rect();
        drawPath.addRect(r);

        const int num_vertical = 5;
        for (int i = 0; i < num_vertical; ++i)
        {
            int x = r.left() + (i+1)*r.width()/(num_vertical+1);
            drawPath.moveTo(x, r.top());
            drawPath.lineTo(x, r.top() + r.height());
        }

        const int num_horizontal = 3;
        for (int i = 0; i < num_horizontal; ++i)
        {
            int y = r.top() + (i+1)*r.height()/(num_horizontal+1);
            drawPath.moveTo(r.left(), y);
            drawPath.lineTo(r.left() + r.width(), y);
        }
    }

    bool NeuroGridItem::canCreateNewItem(const QString & typeName, const QPointF &) const
    {
        if (typeName.contains("ExcitoryLinkItem"))
            return true;
        if (typeName.contains("InhibitoryLinkItem"))
            return true;
        if (typeName.contains("NeuroNodeItem"))
            return true;
        if (typeName.contains("NeuroOscillatorItem"))
            return true;
        if (typeName.contains("TextItem"))
            return true;
        if (typeName.contains("GridEdgeItem"))
            return true;

        return false;
    }

    bool NeuroGridItem::canBeAttachedBy(const QPointF & pos, NeuroItem *item) const
    {
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(item);

        if (!link && !gi) return false;

        // only allow connections to the top or bottom
        QPointF topLeft = mapToScene(rect().topLeft());
        QPointF bottomRight = mapToScene(rect().bottomRight());

        if (pos.y() > topLeft.y() && pos.y() < bottomRight.y())
            return false;

        if (pos.y() <= topLeft.y())
        {
            foreach (NeuroNetworkItem *ni, _top_connections)
            {
                MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(ni);
                if (gi)
                    return false;
            }
        }
        else if (pos.y() >= bottomRight.y())
        {
            foreach (NeuroNetworkItem *ni, _bottom_connections)
            {
                MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(ni);
                if (gi)
                    return false;
            }
        }

        return true;
    }

    void NeuroGridItem::onAttachedBy(NeuroItem *item)
    {
        NeuroNetworkItem::onAttachedBy(item); // we don't want any sub-connection items

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            QPointF mousePos = network()->scene()->lastMousePos();

            bool top = mousePos.y() < scenePos().y();
            if (top)
                _top_connections.insert(ni);
            else
                _bottom_connections.insert(ni);

            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(ni);
            if (gi)
                adjustIOItem(gi, top);
        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);
        }
    }

    void NeuroGridItem::onDetach(NeuroItem *item)
    {
        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            _top_connections.remove(ni);
            _bottom_connections.remove(ni);
        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onDetach(link);
        }

        NeuroNetworkItem::onDetach(item); // no subconnection items
    }

    void NeuroGridItem::adjustLinks()
    {
        SubNetworkItem::adjustLinks();

        foreach (NeuroNetworkItem *item, _top_connections)
        {
            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(item);
            if (gi)
                adjustIOItem(gi, true);
        }

        foreach (NeuroNetworkItem *item, _bottom_connections)
        {
            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(item);
            if (gi)
                adjustIOItem(gi, false);
        }
    }

    void NeuroGridItem::adjustIOItem(MultiGridIOItem *gi, bool top)
    {
        if (top)
            gi->setPos(scenePos().x(), scenePos().y() - rect().height()/2 - gi->rect().height()/2);
        else
            gi->setPos(scenePos().x(), scenePos().y() + rect().height()/2 + gi->rect().height()/2);
    }

    static QMap<NeuroGridItem::Index, NeuroGridItem::Index> &
    get_to_copy(QVector<QMap<NeuroGridItem::Index, NeuroGridItem::Index> > & all_copies, int row, int col, int num_cols)
    {
        return all_copies[(row * num_cols) + col];
    }

    void NeuroGridItem::generateGrid()
    {
        if (!_pattern_changed)
            return;

        // get neuronet pointer
        NeuroLib::NeuroNet *neuronet = 0;

        if (network())
            neuronet = network()->neuronet();

        if (!neuronet)
            return;

        // clear outside edges
        foreach (NeuroItem *item, connections())
            removeEdges(item);

        // set status; this might take a while
        MainWindow::instance()->setStatus(tr("Generating neural network grid..."));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // delete all existing used cells from the automaton
        foreach (const Index & index, _all_grid_cells)
        {
            neuronet->removeNode(index);
        }

        // get all cells used by all items
        QSet<Index> all_pattern_cells;

        typedef QPair<QSet<Index>, QSet<Index> > ConnectSets;

        QList<ConnectSets> top_outgoing_to_bottom_incoming;
        QList<ConnectSets> bottom_outgoing_to_top_incoming;
        QList<ConnectSets> left_outgoing_to_right_incoming;
        QList<ConnectSets> right_outgoing_to_left_incoming;

        // collect all cells in the pattern and cells on the edges
        foreach (QGraphicsItem *item, treeNode()->scene()->items())
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
            if (ni)
            {
                foreach (Index index, ni->allCells())
                {
                    if (index != -1)
                        all_pattern_cells.insert(index);
                }
            }

            GridEdgeItem *ei = dynamic_cast<GridEdgeItem *>(item);
            if (ei)
            {
                top_outgoing_to_bottom_incoming.append(ConnectSets());
                bottom_outgoing_to_top_incoming.append(ConnectSets());
                left_outgoing_to_right_incoming.append(ConnectSets());
                right_outgoing_to_left_incoming.append(ConnectSets());

                foreach (NeuroItem *cx, ei->connections())
                {
                    NeuroNetworkItem *cxni = dynamic_cast<NeuroNetworkItem *>(cx);
                    if (cxni)
                    {
                        if (ei->isVertical())
                        {
                            if (ei->isConnectedToTop(cxni))
                            {
                                foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                    top_outgoing_to_bottom_incoming.back().first.insert(idx);
                                foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                    bottom_outgoing_to_top_incoming.back().second.insert(idx);
                            }
                            if (ei->isConnectedToBottom(cxni))
                            {
                                foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                    bottom_outgoing_to_top_incoming.back().first.insert(idx);
                                foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                    top_outgoing_to_bottom_incoming.back().second.insert(idx);
                            }
                        }
                        else
                        {
                            if (ei->isConnectedToLeft(cxni))
                            {
                                foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                    left_outgoing_to_right_incoming.back().first.insert(idx);
                                foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                    right_outgoing_to_left_incoming.back().second.insert(idx);
                            }
                            if (ei->isConnectedToRight(cxni))
                            {
                                foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                    right_outgoing_to_left_incoming.back().first.insert(idx);
                                foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                    left_outgoing_to_right_incoming.back().second.insert(idx);
                            }
                        }
                    }
                }
            }
        }

        // make copies of the pattern for each grid square
        QVector<QMap<Index, Index> > pattern_to_all_copies(_num_vert * _num_horiz);

        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                // copy pattern cells
                foreach (const Index & pat_index, all_pattern_cells)
                {
                    if (pat_index != -1)
                    {
                        Index copy_index = neuronet->addNode((*neuronet)[pat_index].current());
                        _all_grid_cells.insert(copy_index);
                        pat_to_copy[pat_index] = copy_index;
                    }
                }

            }
        }

        // duplicate internal edges
        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                const QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                foreach (const Index & pat_index, pat_to_copy.keys())
                {
                    const Index & copy_index = pat_to_copy[pat_index];

                    const QVector<Index> & pat_neighbors = neuronet->neighbors(pat_index);
                    foreach (const Index & nbr_idx, pat_neighbors)
                    {
                        if (pat_to_copy.contains(nbr_idx))
                        {
                            const Index & copy_neighbor = pat_to_copy[nbr_idx];
                            neuronet->addEdge(copy_index, copy_neighbor);
                        }
                    }
                }
            }
        }

        // now connect up all the copies, connecting the top row and bottom row to the outside...
        // remember that neighbors in the neuronet are incoming cells, so the edges seem backwards!
        QSet<Index> top_incoming_temp;
        QSet<Index> top_outgoing_temp;
        QSet<Index> bot_incoming_temp;
        QSet<Index> bot_outgoing_temp;

        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                const QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                int left_col = (col + _num_horiz - 1) % _num_horiz;
                int right_col = (col + 1) % _num_horiz;
                int up_row = row - 1;
                int down_row = row + 1;

                // upwards
                if (row == 0)
                {
                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                            top_outgoing_temp.insert(pat_to_copy[out]);
                    }

                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & in, cnx.second)
                            top_incoming_temp.insert(pat_to_copy[in]);
                    }
                }
                else
                {
                    const QMap<Index, Index> & copy_above = get_to_copy(pattern_to_all_copies, up_row, col, _num_horiz);

                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                        {
                            foreach (const Index & in, cnx.second)
                            {
                                neuronet->addEdge(copy_above[in], pat_to_copy[out]);
                            }
                        }
                    }
                }

                // downwards
                if (row == _num_vert - 1)
                {
                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                            bot_outgoing_temp.insert(pat_to_copy[out]);
                    }

                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & in, cnx.second)
                            bot_incoming_temp.insert(pat_to_copy[in]);
                    }
                }
                else
                {
                    const QMap<Index, Index> & copy_below = get_to_copy(pattern_to_all_copies, down_row, col, _num_horiz);

                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                        {
                            foreach (const Index & in, cnx.second)
                            {
                                neuronet->addEdge(copy_below[in], pat_to_copy[out]);
                            }
                        }
                    }
                }

                // leftwards
                const QMap<Index, Index> & copy_on_left = get_to_copy(pattern_to_all_copies, row, left_col, _num_horiz);

                foreach (const ConnectSets & cnx, left_outgoing_to_right_incoming)
                {
                    foreach (const Index & out, cnx.first)
                    {
                        foreach (const Index & in, cnx.second)
                        {
                            neuronet->addEdge(copy_on_left[in], pat_to_copy[out]);
                        }
                    }
                }

                // rightwards
                const QMap<Index, Index> & copy_on_right = get_to_copy(pattern_to_all_copies, row, right_col, _num_horiz);

                foreach (const ConnectSets & cnx, right_outgoing_to_left_incoming)
                {
                    foreach (const Index & out, cnx.first)
                    {
                        foreach (const Index & in, cnx.second)
                        {
                            neuronet->addEdge(copy_on_right[in], pat_to_copy[out]);
                        }
                    }
                }
            }
        }

        _top_incoming = top_incoming_temp.toList();
        _top_outgoing = top_outgoing_temp.toList();
        _bot_incoming = bot_incoming_temp.toList();
        _bot_outgoing = bot_outgoing_temp.toList();

        foreach (NeuroItem *item, connections())
            addEdges(item);

        // done
        QApplication::restoreOverrideCursor();
        MainWindow::instance()->setStatus(tr("Generated neural network grid."));
        _pattern_changed = false;

#ifdef DEBUG
        // dump graph
        {
            QFile file("neuronet.gv");
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream ts(&file);
                neuronet->dumpGraph(ts, true);
            }
        }
#endif
    }

    void NeuroGridItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        SubNetworkItem::writeBinary(ds, file_version);

        ds << _num_horiz;
        ds << _num_vert;

        ds << static_cast<quint32>(_edges.size());
        QMap<NeuroNetworkItem *, QMap<Index, Index> >::const_iterator i = _edges.constBegin(), i_end = _edges.constEnd();
        while (i != i_end)
        {
            if (i.key())
            {
                ds << static_cast<IdType>(i.key()->id());

                ds << static_cast<quint32>(i.value().size());
                QMap<Index, Index>::const_iterator j = i.value().constBegin(), j_end = i.value().constEnd();
                while (j != j_end)
                {
                    ds << j.key();
                    ds << j.value();
                    ++j;
                }
            }
            ++i;
        }
    }

    void NeuroGridItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        SubNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            ds >> _num_horiz;
            ds >> _num_vert;

            _edges.clear();
            if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_11)
            {
                quint32 num_i;
                ds >> num_i;
                for (quint32 i = 0; i < num_i; ++i)
                {
                    IdType id;
                    ds >> id;
                    NeuroNetworkItem *id_ptr = reinterpret_cast<NeuroNetworkItem *>(id);
                    QMap<Index, Index> & edge_map = _edges[id_ptr];

                    quint32 num_j;
                    ds >> num_j;
                    for (quint32 j = 0; j < num_j; ++j)
                    {
                        Index key, val;
                        ds >> key;
                        ds >> val;
                        edge_map.insert(key, val);
                    }
                }
            }
        }
    }

    void NeuroGridItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        SubNetworkItem::writePointerIds(ds, file_version);

        quint32 num = _top_connections.size();
        ds << num;
        foreach (const NeuroNetworkItem *ni, _top_connections)
            ds << static_cast<IdType>(ni ? ni->id() : 0);

        num = _bottom_connections.size();
        ds << num;
        foreach (const NeuroNetworkItem *ni, _bottom_connections)
            ds << static_cast<IdType>(ni ? ni->id() : 0);
    }

    void NeuroGridItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        SubNetworkItem::readPointerIds(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            quint32 num;

            _top_connections.clear();
            ds >> num;
            for (quint32 i = 0; i < num; ++i)
            {
                IdType id;
                ds >> id;
                if (id) _top_connections.insert(reinterpret_cast<NeuroNetworkItem *>(id));
            }

            _bottom_connections.clear();
            ds >> num;
            for (quint32 i = 0; i < num; ++i)
            {
                IdType id;
                ds >> id;
                if (id) _bottom_connections.insert(reinterpret_cast<NeuroNetworkItem *>(id));
            }
        }
    }

    void NeuroGridItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        SubNetworkItem::idsToPointers(idMap);

        QSet<NeuroNetworkItem *> toAdd;

        foreach (NeuroNetworkItem *ni, _top_connections)
        {
            IdType wanted_id = reinterpret_cast<IdType>(ni);
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(wanted_id));
        }

        _top_connections = toAdd;

        toAdd.clear();
        foreach (NeuroNetworkItem *ni, _bottom_connections)
        {
            IdType wanted_id = reinterpret_cast<IdType>(ni);
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(wanted_id));
        }

        _bottom_connections = toAdd;

        // edges
        QMap<NeuroNetworkItem *, QMap<Index, Index> > new_edges;
        QMap<NeuroNetworkItem *, QMap<Index, Index> >::const_iterator i = _edges.constBegin(), end = _edges.constEnd();
        while (i != end)
        {
            IdType wanted_id = reinterpret_cast<IdType>(i.key());
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (wanted_item)
                new_edges[wanted_item] = i.value();
            else
                throw Common::FileFormatError(tr("Dangling edge node in file: %1").arg(wanted_id));

            ++i;
        }

        _edges = new_edges;
    }

    void NeuroGridItem::postLoad()
    {
        SubNetworkItem::postLoad();
        generateGrid();
    }

} // namespace GridItems
