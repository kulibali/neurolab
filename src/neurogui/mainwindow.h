#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include "propertyobj.h"
#include "../automata/exception.h"

#include <QtGui/QMainWindow>
#include <QVBoxLayout>
#include <QMap>
#include <QList>
#include <QDir>

class QSpinBox;
class QProgressBar;

class QtTreePropertyBrowser;
class QtVariantEditorFactory;
class QtVariantPropertyManager;

/// Qt Designer generated code.
namespace Ui
{
    class MainWindow;
}

/// GUI code.
namespace NeuroLab
{

    extern NEUROGUISHARED_EXPORT const QString VERSION;

    class LabNetwork;
    class LabDataFile;
    class NeuroItem;

    /// The main window class for the NeuroLab application.
    class NEUROGUISHARED_EXPORT MainWindow
        : public QMainWindow
    {
        Q_OBJECT

        static MainWindow *_instance; ///< Singleton instance of the main window.

        QString _title;

        Ui::MainWindow *_ui; ///< The UI class generated by Qt Designer.
        QVBoxLayout *_networkLayout; ///< The main layout for the window's content.

        QSpinBox *_numStepsSpinBox;
        QAction *_numStepsSpinBoxAction;
        QProgressBar *_stepProgressBar;

        LabNetwork *_currentNetwork; ///< The current network being viewed/edited.
        LabDataFile *_currentDataFile; ///< The current data file.

        QtTreePropertyBrowser *_propertyEditor; ///< The property editor widget.
        QtVariantEditorFactory *_propertyFactory; ///< Property factory for the property editor.
        QtVariantPropertyManager *_propertyManager; ///< Manager for the property editor.

        QList<PropertyObject *> _propertyObjects; ///< The objects whose properties are currently on display.
        PropertyObject *_noncePropertyObject; ///< A temporary object to hold common properties for multiple selected objects.

        QMap<QString, QList<PropertyObject::PropertyBase *> > _rememberedProperties; ///< Remembered values for properties.
        bool _rememberProperties;

    public:
        /// Constructor.
        /// \param parent Parent widget (should normally be 0).
        /// \param title Title of the window.
        /// \param initialFname Initial filename to load.
        MainWindow(QWidget *parent, const QString & title, const QString & initialFname = QString());
        ~MainWindow();

        /// \return A pointer to the singleton instance of the main window.
        static MainWindow *instance();

        /// \return A pointer to the main window's UI object.
        Ui::MainWindow *ui() { return _ui; }

        /// \return A pointer to the property editor widget.
        QtTreePropertyBrowser *propertyEditor() { return _propertyEditor; }

        /// \return Objects whose properties are currently being displayed.
        QList<PropertyObject *> propertyObjects() { return _propertyObjects; }

        static QDir LAST_DIRECTORY;

    protected:
        /// Handles attempts to close the window.
        virtual void closeEvent(QCloseEvent *);

    public slots:
        /// Sets the title of the window to the given string plus the current network's filename and dirty status.
        void setTitle(const QString & title = QString());

        void setStatus(const QString & status);
        void setProgressRange(int minimum, int maximum);
        void setProgressValue(int value);

        void setActionsEnabled(bool enabled = true);

        /// Sets the objects whose properties are displayed in the property widget.
        void setPropertyObjects(const QList<PropertyObject *> & property_objects);
        void setPropertyObject(PropertyObject *);

        void createdItem(NeuroItem *);
        void propertyValueChanged(QtProperty *, const QVariant &);

    private:
        void loadPlugins();
        void loadPlugins(const QString & dirPath);
        void loadStateSettings();
        void saveStateSettings();
        void setupConnections();

        bool newNetwork();
        bool openNetwork();
        bool saveNetwork();
        bool closeNetwork();

        bool newDataFile();
        bool closeDataFile();

        void setNetwork(LabNetwork *network);

    private slots:
        void on_action_Simulation_Toolbar_triggered();
        void filterEditMenu();
        void filterFileMenu();

        void on_action_About_NeuroLab_triggered();
        void on_action_Manual_triggered();
        void on_action_New_triggered();
        void on_action_Open_triggered();
        void on_action_Close_triggered();
        void on_action_Save_triggered();
        void on_action_Quit_triggered();
        void on_action_Sidebar_triggered();
        void on_action_View_Toolbar_triggered();
        void on_action_Main_Toolbar_triggered();
        void on_action_Start_triggered();
        void on_action_Stop_triggered();
        void on_action_Step_triggered();
        void on_action_Reset_triggered();
        void on_action_Delete_triggered();
        void on_action_Save_Data_Set_triggered();
        void on_action_New_Data_Set_triggered();
        void on_action_Close_Data_Set_triggered();
        void on_action_Cut_triggered();
        void on_action_Copy_triggered();
        void on_action_Paste_triggered();

        void on_action_Print_triggered();
        void on_action_SVG_triggered();
        void on_action_PDF_triggered();
        void on_action_PS_triggered();
        void on_action_PNG_triggered();

        void on_action_Zoom_Out_triggered();
        void on_action_Zoom_In_triggered();
    };

    /// Base class for exceptions used by the NeuroLab software.
    class LabException
        : public Automata::Exception
    {
    public:
        LabException(const QString & message) : Automata::Exception(message) {}
    };

} // namespace NeuroLab

#endif // MAINWINDOW_H
