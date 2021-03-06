#ifndef GRIDVIEWER_H
#define GRIDVIEWER_H

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

#include "griditems_global.h"
#include "neurogriditem.h"

#include <QGLWidget>
#include <QSettings>

using namespace NeuroGui;

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT GridViewer
        : public QGLWidget
    {
        Q_OBJECT

        qreal _distance;
        qreal _angle;

        QPointF _last_mouse_pos;

        NeuroGridItem *_grid_item;

    public:
        explicit GridViewer(const QGLFormat & format, QWidget *parent = 0);
        virtual ~GridViewer();

        void loadSettings(QSettings &);
        void saveSettings(QSettings &);

    public slots:
        void newNetworkOpened(LabNetwork *);
        void networkChanged();
        void selectedItem(NeuroItem *);
        void changedItem(NeuroItem *);
        void deletedItem(NeuroItem *);
        void postStep();

    protected:
        void setGridItem(NeuroGridItem *grid_item);

        virtual void initializeGL();
        virtual void resizeGL(int w, int h);
        virtual void paintGL();

        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void mouseReleaseEvent(QMouseEvent *);
        virtual void wheelEvent(QWheelEvent *);
    };

} // namespace GridItems

#endif // GRIDVIEWER_H
