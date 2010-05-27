#ifndef LABTREE_H
#define LABTREE_H

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

#include "neurogui_global.h"

#include <QList>

class QDataStream;
class QWidget;
class QGraphicsItem;

namespace NeuroLab
{

    class LabScene;
    class LabView;
    class LabTree;
    class LabNetwork;

    /// A node in the hierarchy of scenes.
    class NEUROGUISHARED_EXPORT LabTreeNode
    {
        quint32 _id;
        static quint32 NEXT_ID;

        LabTree *_tree;
        LabTreeNode *_parent;

        LabScene *_scene;
        LabView *_view;

        QList<LabTreeNode *> _children;

    public:
        /// Constructor.
        LabTreeNode(LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent = 0);
        virtual ~LabTreeNode();

        quint32 id() const { return _id; }

        LabScene *scene() { return _scene; }
        LabView *view() { return _view; }
        QList<LabTreeNode *> & children() { return _children; }

        /// Resets all the items in the scene.
        void reset();

        /// Updates all the items in the scene.
        void update();

        void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);
    };


    /// Encapsulates the hierarchy of scenes.  A NeuroItem may contain a subnetwork within it, whose scene can be opened
    /// by double-clicking.  The lab tree encapsulates this.
    class NEUROGUISHARED_EXPORT LabTree
    {
        QWidget *_parent;

        LabNetwork *_network;
        LabTreeNode *_root;
        LabTreeNode *_current;

        friend class LabTreeNode;

    public:
        /// Constructor.
        LabTree(QWidget *_parent, LabNetwork *_network);
        virtual ~LabTree();

        /// \return The tree's network object.
        LabNetwork *network() { return _network; }

        /// \return The root node of the scene tree.
        LabTreeNode *root() { return _root; }

        /// \return The current node whose scene is being displayed.
        LabTreeNode *current() { return _current; }
        void setCurrent(LabTreeNode *node) { _current = node; }

        /// \return The current node's scene.
        LabScene *scene() { return _current ? _current->scene() : 0; }

        /// \return The current node's scene's view.
        LabView *view() { return _current ? _current->view() : 0; }

        /// \return All the items in the tree's scenes.
        QList<QGraphicsItem *> items(LabTreeNode *n = 0) const;

        /// Resets all the items in the network.
        void reset(LabTreeNode *n = 0);

        /// Updates all the items in the network.
        void update(LabTreeNode *n = 0);

        void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);
    };

} // namespace NeuroLab

#endif // LABTREE_H
