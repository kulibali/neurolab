#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include "neuronarrowitem.h"

#include <QVector2D>
#include <QPainterPath>
#include <QList>

namespace NeuroLab
{

    /// A link item that represents a one-directional link in Narrow notation.
    class NeuroLinkItem
        : public NeuroNarrowItem
    {
    protected:
        QLineF _line;
        mutable QVector2D c1, c2;

        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool dragFront, settingLine;

    public:
        /// Constructor.
        /// \param network The network this item is a part of.
        /// \param cellIndex The index of the neural network cell that underlies this item.
        NeuroLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroLinkItem();

        /// The link's back (\c p0) and front (\c p1) position, in scene coordinates.
        QLineF line() const { return _line; }

        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const QLineF & l);
        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const QPointF & p1, const QPointF & p2);

        /// Adds an incoming item.  Overrides the default to create an edge in the neural network.
        virtual bool addIncoming(NeuroItem *linkItem);
        /// Removes an incoming item.  Overrides the default to remove an edge in the neural network.
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

        /// The item that the front of the link is currently attached to.
        /// \see line()
        /// \see setFrontLinkTarget()
        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }
        /// Sets the item that the front of the link is currently attached to.
        /// \see line()
        /// \see frontLinkTarget()
        void setFrontLinkTarget(NeuroItem *linkTarget);

        /// The item that the back of the link is currently attached to.
        /// \see line()
        /// \see setBackLinkTarget()
        NeuroItem *backLinkTarget() { return _backLinkTarget; }
        /// Sets the item that the back of the link is currently attached to.
        /// \see line()
        /// \see backLinkTarget()
        void setBackLinkTarget(NeuroItem *linkTarget);

        /// Adds the link's shape to the drawing painter path.
        virtual void addToShape() const;

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void attachTo(NeuroItem *);
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);
        virtual void adjustLinks();

    private:
        /// Updates the link's position to be halfway between its front and back points.
        void updatePos();

    protected:
        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const { return false; }

        virtual void setPenProperties(QPen & pen) const;
        virtual void setBrushProperties(QBrush & brush) const;

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);

        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);
        virtual void idsToPointers(QGraphicsScene *);
    };


    /// An excitory link in narrow notation.
    class NeuroExcitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroExcitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroExcitoryLinkItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void addToShape() const;
    };


    /// An inhibitory link in narrow notation.
    class NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroInhibitoryLinkItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void addToShape() const;
    };

} // namespace NeuroLab

#endif // NEUROLINKITEM_H
