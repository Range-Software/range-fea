#ifndef MESH_SETUP_WIDGET_H
#define MESH_SETUP_WIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include <rml_mesh_setup.h>

#include "value_line_edit.h"

class MeshSetupWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! Mesh setup.
        RMeshSetup meshSetup;
        //! Minimum edge length edit.
        ValueLineEdit *minEdgeLengthEdit;
        //! Maximum edge length edit.
        ValueLineEdit *maxEdgeLengthEdit;

    public:

        //! Constructor.
        explicit MeshSetupWidget(const RMeshSetup &meshSetup,
                                 const std::vector<RVariableType> &variableTypes,
                                 QWidget *parent = nullptr);

    signals:

        //! Mesh setup has changed.
        void changed(const RMeshSetup &meshetup);

    protected slots:

        //! On variable list item changed.
        void onVariableListItemChanged(QListWidgetItem *item);

        //! Minimum edge length changed.
        void onMinEdgeLengthChanged(double value);

        //! Maximum edge length changed.
        void onMaxEdgeLengthChanged(double value);

};

#endif // MESH_SETUP_WIDGET_H
