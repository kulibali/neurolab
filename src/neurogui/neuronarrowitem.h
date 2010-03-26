#ifndef NEURONARROWITEM_H
#define NEURONARROWITEM_H

#include "neurogui_global.h"
#include "neuroitem.h"
#include "../neurolib/neuronet.h"

namespace NeuroLab
{

    /// Base class for items in Narrow notation.  These include nodes and one-directional links.
    class NEUROGUISHARED_EXPORT NeuroNarrowItem
        : public NeuroItem
    {
        Q_OBJECT

        NeuroLib::NeuroCell::NeuroIndex _cellIndex; ///< The index of the neural network cell that underlies this item.

        QtVariantProperty *frozen_property; ///< Property for controlling whether or not the item is frozen.
        QtVariantProperty *inputs_property; ///< Property for controlling the item's input weight.
        QtVariantProperty *weight_property; ///< Property for controlling the item's output weight.
        QtVariantProperty *run_property;    ///< Property for controlling the item's sigmoid curve's slope.
        QtVariantProperty *value_property;  ///< Property for controlling the item's output value.

    public:
        NeuroNarrowItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex = -1);
        virtual ~NeuroNarrowItem();

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void updateProperties();

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual void setPenProperties(QPen & pen) const;

    public slots:
        /// Resets the item.  If it is not frozen, sets the output value to zero.
        virtual void reset();

        /// Toggles the output value of the item between 0 and 1.
        virtual void toggleActivated();

        /// Toggles whether or not the item is frozen (i.e. whether or not its output value will change during a time step).
        virtual void toggleFrozen();

        virtual void propertyValueChanged(QtProperty *property, const QVariant & value);

    protected:
        void buildActionMenuAux(LabScene *scene, const QPointF &pos, QMenu &menu);

        /// \return A pointer to the neural network cell's previous and current state.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell() const;

        /// \return A pointer to the neural network cell's previous and current state.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell();

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };

}

#endif // NEURONARROWITEM_H