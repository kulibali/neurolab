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

#include "mixinremember.h"
#include "../labscene.h"

namespace NeuroGui
{

    MixinRemember::MixinRemember(NeuroItem *self)
        : _self(self)
    {
    }

    MixinRemember::~MixinRemember()
    {
    }

    void MixinRemember::onAttachedBy(MixinArrow *link)
    {
        if (link->frontLinkTarget() == _self && link->dragFront())
            _incomingAttachments.remove(link);

        if (link->backLinkTarget() == _self && !link->dragFront())
            _outgoingAttachments.remove(link);

        // snap to node
        QList<MixinArrow *> already;
        adjustLink(link, already);

        // get new pos and remember
        QLineF l = link->line();
        QVector2D front(l.p2());
        QVector2D back(l.p1());
        QVector2D center(_self->scenePos());

        if (link->frontLinkTarget() == _self)
            _incomingAttachments[link] = front - center;
        if (link->backLinkTarget() == _self)
            _outgoingAttachments[link] = back - center;
    }

    void MixinRemember::adjustLinks()
    {
        QList<MixinArrow *> alreadyAdjusted;

        for (QListIterator<NeuroItem *> i(_self->incoming()); i.hasNext(); )
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(i.next());
            if (link && !alreadyAdjusted.contains(link))
                adjustLink(link, alreadyAdjusted);
        }

        for (QListIterator<NeuroItem *> i(_self->outgoing()); i.hasNext(); )
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(i.next());
            if (link && !alreadyAdjusted.contains(link))
                adjustLink(link, alreadyAdjusted);
        }
    }

    void MixinRemember::adjustLink(MixinArrow *link, QList<MixinArrow *> & alreadyAdjusted)
    {
        LabScene *lab_scene = dynamic_cast<LabScene *>(_self->scene());
        if (!lab_scene)
            return;

        QVector2D center(_self->scenePos()); // in the scene

        bool frontLink = link->frontLinkTarget() == _self;
        bool backLink = link->backLinkTarget() == _self;

        bool rememberFront = _incomingAttachments.contains(link);
        bool rememberBack = _outgoingAttachments.contains(link);

        QLineF link_line = link->line();
        QVector2D front(link_line.p2()); front -= center; // link's front in my frame
        QVector2D back(link_line.p1()); back -= center; // link's back in my frame

        QVector2D mouse_pos(lab_scene->lastMousePos()); // in the scene
        QVector2D toPos = (mouse_pos - center).normalized();

        if (frontLink)
        {
            if (rememberFront)
            {
                front = _incomingAttachments[link];
            }
            else
            {
                front = getAttachPos(toPos);
                _incomingAttachments[link] = front;
            }
        }

        if (backLink)
        {
            if (rememberBack)
            {
                back = _outgoingAttachments[link];
            }
            else
            {
                back = getAttachPos(toPos);
                _outgoingAttachments[link] = back;
            }
        }

        if (frontLink && backLink)
        {
            QVector2D avg_dir = ((front + back) * 0.5).normalized();
            QPointF new_center = (center + avg_dir * (NeuroNarrowItem::NODE_WIDTH*3)).toPointF();
            QLineF new_line((back + center).toPointF(), (front + center).toPointF());

            link->setLine(new_line, &new_center);
        }
        else
        {
            link->setLine((back + center).toPointF(), (front + center).toPointF());
        }

        alreadyAdjusted.append(link);
    }

} // namespace NeuroGui