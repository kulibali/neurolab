#ifndef NEUROTEXTITEM_H
#define NEUROTEXTITEM_H

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
#include "../neuroitem.h"

#include <QFont>

namespace NeuroGui
{

    /// Displays a string of text in a particular font.
    class NEUROGUISHARED_EXPORT NeuroTextItem
        : public NeuroItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        QFont _font;
        QString _text;

        Property<NeuroTextItem, QVariant::Font, QFont, QFont> _font_property;
        Property<NeuroTextItem, QVariant::String, QString, QString> _text_property;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit NeuroTextItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);

        /// Destructor.
        virtual ~NeuroTextItem();

        /// The font to use when drawing the text.
        /// \see NeuroTextItem::setFont()
        QFont font() const { return _font; }

        /// Set the font to use when drawing the text.
        /// \see NeuroTextItem::font()
        void setFont(const QFont & f) { _font = f; }

        /// The text to draw.
        /// \see NeuroTextItem::setText()
        QString text() const { return _text; }

        /// Set the text to draw.
        /// \see NeuroTextItem::text()
        void setText(const QString & t) { _text = t; }

        virtual bool canCutAndPaste() const { return true; }

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

    protected:
        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void setPenProperties(QPen &pen) const;
        virtual void setBrushProperties(QBrush &brush) const;
    }; // class NeuroTextItem

}

#endif
