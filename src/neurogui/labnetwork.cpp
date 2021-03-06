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

#include "mainwindow.h"
#include "labnetwork.h"
#include "labtree.h"
#include "labview.h"
#include "labscene.h"
#include "neuronetworkitem.h"
#include "../neurolib/neuronet.h"

#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QVector2D>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QSvgGenerator>

#include <QtVariantPropertyManager>
#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroGui
{

    /// Constructor.
    /// \param parent The QObject that should own this network object.
    LabNetwork::LabNetwork(QWidget *parent)
        : PropertyObject(parent),
        _tree(0), _neuronet(0), _loading(false), _running(false), _changed(false), first_change(true),
        _filename_property(this, &LabNetwork::fname, 0, tr("Filename"), "", false),
        _decay_property(this, &LabNetwork::decay, &LabNetwork::setDecay,
                        tr("Decay Rate"), tr("Rate at which active nodes and links will decay.")),
        _link_learn_property(this, &LabNetwork::linkLearnRate, &LabNetwork::setLinkLearnRate,
                             tr("Link Learn Rate"), tr("Controls the rate of link learning.")),
        _node_learn_property(this, &LabNetwork::nodeLearnRate, &LabNetwork::setNodeLearnRate,
                             tr("Node Learn Rate"), tr("Controls the rate of node threshold raising or.")),
        _node_forget_property(this, &LabNetwork::nodeForgetRate, &LabNetwork::setNodeForgetRate,
                              tr("Node Forget Rate"), tr("Controls the rate of node threshold lowering.")),
        _learn_time_property(this, &LabNetwork::learnTime, &LabNetwork::setLearnTime,
                             tr("Learn Window"), tr("Window of time used to calculate running average for link and node learning.")),
        _current_step(0), _max_steps(0), _cancel_step(false)
    {
        _neuronet = new NeuroLib::NeuroNet();
        _tree = new LabTree(parent, this);

        connect(&_future_watcher, SIGNAL(finished()), this, SLOT(futureFinished()), Qt::UniqueConnection);
    }

    LabNetwork::~LabNetwork()
    {
        delete _tree; _tree = 0;
        delete _neuronet; _neuronet = 0;
    }

    void LabNetwork::setChanged(bool changed)
    {
        if (_changed != changed)
            _changed = changed;

        if (changed)
            emit networkChanged();
    }

    LabTreeNode *LabNetwork::treeNode()
    {
        return _tree ? _tree->current() : 0;
    }

    void LabNetwork::setTreeNode(LabTreeNode *node)
    {
        if (node && _tree)
            _tree->setCurrent(node);
    }

    LabScene *LabNetwork::scene() const
    {
        return _tree ? _tree->scene() : 0;
    }

    QList<QGraphicsItem *> LabNetwork::items() const
    {
        return _tree ? _tree->items() : QList<QGraphicsItem *>();
    }

    LabView *LabNetwork::view() const
    {
        return _tree ? _tree->view() : 0;
    }

    QString LabNetwork::fname() const
    {
        int index = _fname.lastIndexOf("/");
        if (index != -1)
            return _fname.mid(index+1);
        else
            return _fname;
    }

    QString LabNetwork::subNetworkLabel() const
    {
        if (_tree && _tree->current())
            return _tree->current()->label();
        return tr("???");
    }

    void LabNetwork::setSubNetworkLabel(const QString & label)
    {
        if (_tree && _tree->current())
            _tree->current()->setLabel(label);
    }

    NeuroCell::Value LabNetwork::decay() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->decay();
    }

    void LabNetwork::setDecay(const NeuroLib::NeuroCell::Value & decay)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setDecay(decay);
    }

    NeuroCell::Value LabNetwork::linkLearnRate() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->linkLearnRate();
    }

    void LabNetwork::setLinkLearnRate(const NeuroLib::NeuroCell::Value & learnRate)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setLinkLearnRate(learnRate);
    }

    NeuroCell::Value LabNetwork::nodeLearnRate() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->nodeLearnRate();
    }

    void LabNetwork::setNodeLearnRate(const NeuroLib::NeuroCell::Value & rate)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setNodeLearnRate(rate);
    }

    NeuroCell::Value LabNetwork::nodeForgetRate() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->nodeForgetRate();
    }

    void LabNetwork::setNodeForgetRate(const NeuroLib::NeuroCell::Value & rate)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setNodeForgetRate(rate);
    }

    NeuroCell::Value LabNetwork::learnTime() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->learnTime();
    }

    void LabNetwork::setLearnTime(const NeuroLib::NeuroCell::Value & learnTime)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setLearnTime(learnTime);
    }

    /// Handles setting the main window's properties when the selected item changes.
    void LabNetwork::selectionChanged()
    {
        if (!scene())
            return;

        QList<PropertyObject *> property_objects;

        QList<QGraphicsItem *> items = scene()->selectedItems();
        for (int i = 0; i < items.size(); ++i)
        {
            NeuroItem *ni = dynamic_cast<NeuroItem *>(items[i]);
            if (ni)
                ni->onSelected();

            PropertyObject *po = dynamic_cast<PropertyObject *>(items[i]);
            if (po)
                property_objects.append(po);
        }

        if (property_objects.size() == 0)
            property_objects.append(this);

        emit propertyObjectChanged(property_objects);
        emit actionsEnabled(true);

        if (items.size() > 0)
        {
            NeuroItem *ni = dynamic_cast<NeuroItem *>(items[0]);
            if (items.size() == 1 && ni)
                emit itemSelected(ni);
            else
                emit itemSelected(0);
        }

        MainWindow::instance()->buildItemList();
    }

    void LabNetwork::changeItemLabel(NeuroItem *item, const QString & label)
    {
        emit itemLabelChanged(item, label);
    }

    void LabNetwork::resizeView()
    {
        emit viewResized();
    }

    void LabNetwork::setZoom(int new_zoom)
    {
        if (view())
        {
            view()->setZoom(new_zoom);
        }
    }


    /// \deprecated
    static const QString LAB_SCENE_COOKIE_OLD("Neurolab SCENE 009");

    /// Should be changed whenever anything in the the network file format changes.
    static const QString LAB_SCENE_COOKIE_NEW("Neurolab SCENE");

    /// Loads a LabNetwork object and its corresponding NeuroNet from a file.
    /// LabNetwork files have the extension .nln; their corresponding NeuroNet files have the extension .nnn.
    /// \param fname The name of the file from which to load the network.
    /// If this is empty, then the standard file dialog is used to request the name of a file.
    LabNetwork *LabNetwork::open(const QString & fname)
    {
        QString nln_fname = fname;

        if (nln_fname.isEmpty() || nln_fname.isNull())
        {
            nln_fname = QFileDialog::getOpenFileName(MainWindow::instance(),
                                                     tr("Open Network"),
                                                     MainWindow::LAST_DIRECTORY.absolutePath(),
                                                     tr("NeuroLab Networks (*.nln);;All Files (*)"));

            if (nln_fname.isEmpty() || nln_fname.isNull())
                return 0;
            else
                MainWindow::LAST_DIRECTORY = QFileInfo(nln_fname).absoluteDir();
        }

        QString base_fname = nln_fname.endsWith(".nln", Qt::CaseInsensitive) ? nln_fname.left(nln_fname.length() - 4) : nln_fname;
        QString nnn_fname = base_fname + ".nnn";

        if (!QFile::exists(nln_fname))
        {
            throw Common::IOError(tr("NeuroLab Network file %1 does not exist.").arg(nln_fname));
        }

        if (!QFile::exists(nnn_fname))
        {
            throw Common::IOError(tr("NeuroLab Network Automaton file %1 is missing.").arg(nnn_fname));
        }

        // read network
        LabNetwork *ln = new LabNetwork(MainWindow::instance());
        ln->_loading = true;
        ln->_fname = nln_fname;

        {
            QFile file(nnn_fname);
            if (file.open(QIODevice::ReadOnly))
            {
                try
                {
                    QDataStream ds(&file);
                    ds.setVersion(QDataStream::Qt_4_6);

                    Automata::AutomataFileVersion fv;
                    ln->_neuronet->readBinary(ds, fv);
                    ln->updateProperties();
                }
                catch (...)
                {
                    delete ln;
                    throw Common::IOError(tr("Network file %1 is not compatible with this version of NeuroLab.").arg(nnn_fname));
                }
            }
            else
            {
                delete ln;
                throw Common::IOError(tr("Unable to open network file."));
            }
        }

        // read scene data
        {
            QFile file(nln_fname);

            if (file.open(QIODevice::ReadOnly))
            {
                QDataStream ds(&file);
                ds.setVersion(QDataStream::Qt_4_6);

                QString cookie;
                ds >> cookie;

                if (cookie == LAB_SCENE_COOKIE_NEW)
                {
                    quint16 ver;
                    ds >> ver;

                    NeuroLabFileVersion fv;
                    fv.neurolab_version = ver;

                    ln->_tree->readBinary(ds, fv);
                    ln->_idMap.clear();
                }
                else if (cookie == LAB_SCENE_COOKIE_OLD)
                {
                    NeuroLabFileVersion fv;
                    fv.neurolab_version = NeuroGui::NEUROLAB_FILE_VERSION_OLD;

                    ln->_tree->readBinary(ds, fv);
                }
                else
                {
                    delete ln;
                    throw Common::FileFormatError(tr("Network scene file %1 is not compatible with this version of NeuroLab.").arg(nln_fname));
                }

                ln->setChanged(false);
            }
            else
            {
                delete ln;
                throw Common::IOError(tr("Unable to open network scene file."));
            }
        }

        //
        ln->_loading = false;
        ln->actionsEnabled(true);
        return ln;
    }

    /// Saves the network and its NeuroNet.
    /// \param saveAs Forces the file dialog to be used to choose a filename.
    bool LabNetwork::save(bool saveAs)
    {
        bool prevRunning = this->_running;
        stop();

        if (!this->_tree || !this->_neuronet)
            return false;

        if (this->_fname.isEmpty() || this->_fname.isNull() || saveAs)
        {
            this->_fname.clear();

            QString nln_fname = QFileDialog::getSaveFileName(MainWindow::instance(),
                                                             tr("Save Network"),
                                                             MainWindow::LAST_DIRECTORY.absolutePath(),
                                                             tr("NeuroLab Networks (*.nln);;All Files (*)"));

            if (nln_fname.isEmpty() || nln_fname.isNull())
                return false;
            else
                MainWindow::LAST_DIRECTORY = QFileInfo(nln_fname).absoluteDir();

            if (!nln_fname.endsWith(".nln", Qt::CaseInsensitive))
                nln_fname = nln_fname + ".nln";

            this->_fname = nln_fname;
        }

        QString base_name = this->_fname.endsWith(".nln", Qt::CaseInsensitive) ? this->_fname.left(this->_fname.length() - 4) : this->_fname;
        QString network_fname = base_name + ".nnn";

        // write network
        {
            QFile file(network_fname);
            if (file.open(QIODevice::WriteOnly))
            {
                QDataStream ds(&file);
                ds.setVersion(QDataStream::Qt_4_6);

                Automata::AutomataFileVersion fv;
                _neuronet->writeBinary(ds, fv);
            }
            else
            {
                throw Common::IOError(tr("Unable to write network file."));
            }
        }

        // write scene
        {
            QFile file(this->_fname);

            if (file.open(QIODevice::WriteOnly))
            {
                // items
                if (_tree)
                {
                    QDataStream ds(&file);
                    ds.setVersion(QDataStream::Qt_4_6);

                    NeuroLabFileVersion fv;
                    fv.neurolab_version = NeuroGui::NEUROLAB_NUM_FILE_VERSIONS - 1;

                    ds << LAB_SCENE_COOKIE_NEW;
                    ds << static_cast<quint16>(fv.neurolab_version);
                    _tree->writeBinary(ds, fv);
                }
            }
            else
            {
                throw Common::IOError(tr("Unable to write network scene file."));
            }
        }

        setChanged(false);
        emit actionsEnabled(true);

        if (prevRunning)
            start();

        return true;
    }

    /// Saves the network if it has changed since the last change.
    /// \return True if the network was closed successfully.
    bool LabNetwork::close()
    {
        if (_changed && !save())
            return false;
        return true;
    }

    /// Creates a new item with the given type name.
    void LabNetwork::newItem(const QString & typeName)
    {
        if (scene() && view())
        {
            scene()->newItem(typeName, scene()->lastMousePos());
            setChanged(true);
        }
    }

    /// Deletes the selected items.
    void LabNetwork::deleteSelected()
    {
        LabScene *sc = scene();

        if (sc)
        {
            NeuroItem *underMouse = sc->itemUnderMouse();
            if (underMouse)
            {
                sc->setItemUnderMouse(0);

                underMouse->cleanup();
                underMouse->setUIDelete();
                emit itemDeleted(underMouse);
                sc->removeItem(underMouse);
                delete underMouse;
            }

            QList<QGraphicsItem *> selected = sc->selectedItems();
            sc->clearSelection();

            foreach (QGraphicsItem *gi, selected)
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
                if (item)
                {
                    item->cleanup();
                    item->setUIDelete();
                    emit itemDeleted(item);
                }

                sc->removeItem(gi);
                delete gi;
            }

            sc->setItemUnderMouse(0);
            setChanged(true);
        }
    }

    static QString CLIPBOARD_TYPE("application/x-neurocog-items");

    LabTreeNode *LabNetwork::findSubNetwork(const quint32 & id)
    {
        return _tree ? _tree->findSubNetwork(id) : 0;
    }

    LabTreeNode *LabNetwork::newSubNetwork()
    {
        return _tree ? _tree->newSubNetwork() : 0;
    }

    void LabNetwork::removeWidgetsFrom(QLayout *w)
    {
        if (w && _tree)
            _tree->removeWidgetsFrom(w);
    }

    /// \return True if there is something to paste.
    bool LabNetwork::canPaste() const
    {
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard)
        {
            const QMimeData *data = clipboard->mimeData();
            if (data && data->hasFormat(CLIPBOARD_TYPE) && data->data(CLIPBOARD_TYPE).size() > 0)
                return true;
        }

        return false;
    }

    /// Cuts the selected items to the clipboard.
    void LabNetwork::cutSelected()
    {
        copySelected();
        deleteSelected();
    }

    /// Copies the selected items to the clipboard.
    void LabNetwork::copySelected()
    {
        Q_ASSERT(scene());
        Q_ASSERT(view());

        // get items
        QList<QGraphicsItem *> graphicsitems = scene()->selectedItems();
        QList<NeuroItem *> neuroitems;

        foreach (QGraphicsItem *gi, graphicsitems)
        {
            NeuroItem *ni = dynamic_cast<NeuroItem *>(gi);
            if (ni && ni->canCutAndPaste())
                neuroitems.append(ni);
        }

        if (neuroitems.size() == 0)
            return;

        // get center of view and normalized ids
        QVector2D center(0, 0);
        QMap<int, int> id_map;
        int cur_id = 1;

        foreach (NeuroItem *ni, neuroitems)
        {
            center += QVector2D(ni->scenePos());
            id_map[ni->id()] = cur_id++;
        }

        center *= 1.0 / neuroitems.size();

        // write items
        QList<PropertyObject *> old_property_objs = MainWindow::instance()->propertyObjects();

        QByteArray clipboardData;

        {
            QDataStream ds(&clipboardData, QIODevice::WriteOnly);

            ds << static_cast<quint32>(neuroitems.size());

            // write type names so we can create items before reading them...
            foreach (NeuroItem *ni, neuroitems)
            {
                ds << ni->getTypeName();
                ds << static_cast<qint32>(id_map[ni->id()]);
            }

            // write data
            foreach (NeuroItem *ni, neuroitems)
            {
                QVector2D relPos = QVector2D(ni->scenePos()) - center;
                ds << relPos;

                MainWindow::instance()->setPropertyObject(ni); // force properties to be built
                ni->writeClipboard(ds, id_map);
            }
        }

        // set to clipboard
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard)
        {
            QMimeData *data = new QMimeData();
            data->setData(CLIPBOARD_TYPE, clipboardData);
            clipboard->setMimeData(data);
        }

        //
        MainWindow::instance()->setPropertyObjects(old_property_objs);
    }

    /// Pastes any items in the clipboard.
    void LabNetwork::pasteItems()
    {
        scene()->clearSelection();

        // get data from clipboard
        QClipboard *clipboard = QApplication::clipboard();
        if (!clipboard)
            return;

        const QMimeData *data = clipboard->mimeData();
        if (!data || !data->hasFormat(CLIPBOARD_TYPE))
            return;

        QByteArray buf = data->data(CLIPBOARD_TYPE);
        QDataStream ds(&buf, QIODevice::ReadOnly);

        // read item types and create items
        QList<NeuroItem *> new_items;
        QMap<int, NeuroItem *> id_map; // maps from ids in clipboard to new items that are created

        quint32 num_items;
        ds >> num_items;

        for (quint32 i = 0; i < num_items; ++i)
        {
            QString typeName;
            ds >> typeName;

            qint32 clip_id;
            ds >> clip_id;

            if (typeName != "??Unknown??")
            {
                NeuroItem *new_item = NeuroItem::create(typeName, scene(), scene()->lastMousePos(), NeuroItem::CREATE_UI);

                if (new_item)
                {
                    id_map[clip_id] = new_item;

                    new_items.append(new_item);
                    scene()->addItem(new_item);
                }
            }
        }

        // place items in relative order
        QVector2D center(scene()->lastMousePos());

        foreach (NeuroItem *item, new_items)
        {
            if (!item)
                continue;

            QVector2D rel_pos;
            ds >> rel_pos;

            MainWindow::instance()->setPropertyObject(item); // force properties to be built
            item->readClipboard(ds, id_map);

            QVector2D itemPos = center + rel_pos;
//            QRectF view = scene()->sceneRect();

//            if (itemPos.x() < view.x())
//                itemPos.setX(view.x());
//            if (itemPos.y() < view.y())
//                itemPos.setY(view.y());

//            if (itemPos.x() >= view.x() + view.width())
//                itemPos.setX(view.x() + view.width());
//            if (itemPos.y() >= view.y() + view.height())
//                itemPos.setY(view.y() + view.height());

            item->setPos(itemPos.toPointF());
        }

        // update view
        if (new_items.size() > 0)
        {
            foreach (NeuroItem *item, new_items)
                item->adjustLinks();

            _tree->updateItemProperties();
        }

        // link up network cells
        foreach (NeuroItem *item, new_items)
        {
            NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(item);
            if (netItem)
            {
                foreach (NeuroItem *ni, netItem->connections())
                {
                    netItem->addEdges(ni);

                    NeuroNetworkItem *netConn = dynamic_cast<NeuroNetworkItem *>(ni);
                    if (netConn)
                        netConn->addEdges(item);
                }
            }
        }

        // set network as property object
        MainWindow::instance()->setPropertyObject(this);
    }

    void LabNetwork::selectAll()
    {
        if (scene())
        {
            QPainterPath path;
            path.addRect(scene()->sceneRect());

            scene()->setSelectionArea(path);
        }
    }

    /// Starts the network running (currently not implemented).
    void LabNetwork::start()
    {
        _running = true;
        _cancel_step = false;
    }

    /// Stops the network running (currently not implemented).
    void LabNetwork::stop()
    {
        _running = false;
        _tree->updateItemProperties();
    }

    /// Advances the network by one or more timesteps.
    void LabNetwork::step(int numSteps)
    {
        _cancel_step = false;

        if (_running)
            return; // can't happen

        if (numSteps <= 0)
            return;

        _running = true;
        _current_step = 0;
        _max_steps = numSteps * 3; // takes 3 steps of the automaton to fully process
        _step_time.start();


        if (numSteps > 1)
        {
            emit actionsEnabled(false);
            emit statusChanged(tr("Stepping %1 times...").arg(numSteps));
            emit stepProgressRangeChanged(0, _max_steps);
            emit stepProgressValueChanged(0);
        }

        _neuronet->preUpdate();
        emit stepClicked();
        emit preStep();
        _future_watcher.setFuture(_neuronet->stepAsync());
    }

    /// Called when a timestep is complete.
    void LabNetwork::futureFinished()
    {
        _future_watcher.waitForFinished();

        // at the end of each step, do post updates
        _neuronet->postUpdate();

        // at the end of three steps, do post steps
        ++_current_step;
        bool every_three = (_current_step % 3) == 0;
        if (every_three)
        {
            emit postStep();
            emit stepIncremented();
        }

        // are we done?
        if (_current_step == _max_steps || _cancel_step)
        {
            if (_max_steps > 3)
                emit stepProgressValueChanged(_max_steps);

            emit actionsEnabled(true);

            if (_max_steps == 3)
                emit statusChanged(tr("Done stepping 1 time."));
            else
                emit statusChanged(tr("Done stepping %1 times.").arg(_current_step / 3));

            _running = false;
            setChanged(true);
            _tree->updateItemProperties();
            emit stepFinished();
        }
        else
        {
            _neuronet->preUpdate();
            if (every_three)
                emit preStep();
            _future_watcher.setFuture(_neuronet->stepAsync());

            // only change the display 2 times a second
            if (_step_time.elapsed() >= 500)
            {
                _step_time.start();

                if (_max_steps > 3)
                    emit stepProgressValueChanged(_current_step);

                _tree->updateItemProperties();
            }
        }
    }

    void LabNetwork::cancel()
    {
        _cancel_step = true;
    }

    /// Resets the network: all nodes and links that are not frozen have their output values set to 0.
    void LabNetwork::reset()
    {
        emit statusChanged("");
        _tree->reset();
        setChanged(true);
    }

    void LabNetwork::exportPrint()
    {
        if (scene() && view())
        {
            QPrinter printer;
            QPrintDialog pd(&printer, MainWindow::instance());
            pd.setWindowTitle(tr("Print Network"));
            pd.setOption(QAbstractPrintDialog::PrintToFile);

            if (pd.exec() == QDialog::Accepted)
            {
                QPainter painter;
                painter.begin(&printer);
                view()->render(&painter);
                painter.end();

                MainWindow::instance()->setStatus(tr("Printed."));
            }
        }
    }

    void LabNetwork::exportSVG()
    {
        if (scene() && view())
        {
            QString fname = QFileDialog::getSaveFileName(MainWindow::instance(),
                                                         tr("Export Network to SVG"),
                                                         MainWindow::LAST_DIRECTORY.absolutePath(),
                                                         tr("Scalable Vector Graphics files (*.svg);;All Files (*)"));

            if (!fname.isNull() && !fname.isEmpty())
            {
                MainWindow::LAST_DIRECTORY = QFileInfo(fname).absoluteDir();

                QRectF vp = view()->viewport()->rect();

                QSvgGenerator generator;
                generator.setFileName(fname);
                generator.setSize(QSize(static_cast<int>(vp.width()), static_cast<int>(vp.height())));
                generator.setViewBox(vp);
                generator.setTitle(this->fname());

                QPainter painter;
                painter.begin(&generator);
                view()->render(&painter);
                painter.end();

                MainWindow::instance()->setStatus(tr("Exported to SVG: %1").arg(fname));
            }
        }
    }

    void LabNetwork::exportPNG()
    {
        if (scene() && view())
        {
            QString fname = QFileDialog::getSaveFileName(MainWindow::instance(),
                                                         tr("Export Network to PNG"),
                                                         MainWindow::LAST_DIRECTORY.absolutePath(),
                                                         tr("Portable Network Graphics files (*.png);;All Files(*)"));

            if (!fname.isNull() && !fname.isEmpty())
            {
                MainWindow::LAST_DIRECTORY = QFileInfo(fname).absoluteDir();

                QRectF vp = view()->viewport()->rect();

                QImage image(QSize(vp.width(), vp.height()), QImage::Format_ARGB32_Premultiplied);
                image.fill(0x00000000);

                QPainter painter;
                painter.begin(&image);
                view()->render(&painter);
                painter.end();

                if (image.save(fname))
                    MainWindow::instance()->setStatus(tr("Exported to PNG: %1").arg(fname));
                else
                    throw Common::IOError(tr("Unable to save image %1").arg(fname));
            }
        }
    }

    void LabNetwork::exportPS()
    {
        if (scene() && view())
        {
            QString fname = QFileDialog::getSaveFileName(MainWindow::instance(),
                                                         tr("Export Network to Encapsulated PostScript"),
                                                         MainWindow::LAST_DIRECTORY.absolutePath(),
                                                         tr("Encapsulated PostScript files (*.eps);;All Files(*)"));

            if (!fname.isNull() && !fname.isEmpty())
            {
                MainWindow::LAST_DIRECTORY = QFileInfo(fname).absoluteDir();

                QPrinter printer;
                printer.setOutputFileName(fname);
                printer.setOutputFormat(QPrinter::PostScriptFormat);

                QPainter painter;
                painter.begin(&printer);
                view()->render(&painter);
                painter.end();

                MainWindow::instance()->setStatus(tr("Exported to Encapsulated PostScript: %1").arg(fname));
            }
        }
    }

    void LabNetwork::exportPDF()
    {
        if (scene() && view())
        {
            QString fname = QFileDialog::getSaveFileName(MainWindow::instance(),
                                                         tr("Export Network to PDF"),
                                                         MainWindow::LAST_DIRECTORY.absolutePath(),
                                                         tr("Portable Document Format files (*.pdf);;All Files(*)"));

            if (!fname.isNull() && !fname.isEmpty())
            {
                MainWindow::LAST_DIRECTORY = QFileInfo(fname).absoluteDir();

                QPrinter printer;
                printer.setOutputFileName(fname);
                printer.setOutputFormat(QPrinter::PdfFormat);

                QPainter painter;
                painter.begin(&printer);
                view()->render(&painter);
                painter.end();

                MainWindow::instance()->setStatus(tr("Exported to PDF: %1").arg(fname));
            }
        }
    }

} // namespace NeuroGui
