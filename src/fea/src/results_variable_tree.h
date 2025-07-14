#ifndef RESULTS_VARIABLE_TREE_H
#define RESULTS_VARIABLE_TREE_H

#include <QTreeWidget>
#include <QCheckBox>

#include "color_scale_combo_box.h"
#include "value_scale.h"
#include "session_entity_id.h"

class ResultsVariableTree : public QTreeWidget
{

    Q_OBJECT

    protected:

        //! Variable type.
        RVariableType variableType;
        //! Apply as scalar check-box.
        QCheckBox *applyAsScalarCheckBox;
        //! Apply as simple vector check-box.
        QCheckBox *applyAsVectorSimpleCheckBox;
        //! Apply as vector check-box.
        QCheckBox *applyAsVectorCheckBox;
        //! Apply as colored vector check-box.
        QCheckBox *applyAsVectorColoredCheckBox;
        //! Apply as displacement check-box.
        QCheckBox *applyAsDisplacementCheckBox;
        //! Color-scale combo.
        ColorScaleComboBox *colorScaleComboBox;
        //! Minimum range value line editor.
        ValueScale *minValueScale;
        //! Maximum range value line editor.
        ValueScale *maxValueScale;

    public:

        //! Constructor.
        explicit ResultsVariableTree(QWidget *parent = nullptr);

    protected:

        //! Populate combo.
        void populate();

        //! Process apply as states.
        void processApplyAsStates();

    public slots:

        //! Catch materialSelected signal.
        void onResultsVariableSelected(RVariableType variableType);

    protected slots:

        //! Apply as scalar toggle state has changed.
        void onApplyAsScalarStateChanged(Qt::CheckState);

        //! Apply as displacement toggle state has changed.
        void onApplyAsDisplacementStateChanged(Qt::CheckState);

        //! Apply on node button has been clicked.
        void onApplyOnNodeClicked();

        //! Remove variable button has been clicked.
        void onRemoveVariableClicked();

        //! Range name has changed.
        void onColorScaleChanged(int index);

        //! Range min value has changed.
        void onRangeMinValueChanged(double minValue);

        //! Range max value has changed.
        void onRangeMaxValueChanged(double maxValue);

        //! Display range toggle state has changed.
        void onRangeDisplayStateChanged(Qt::CheckState state);

        //! Range autofill default clicked.
        void onRangeAutofillClicked();

        //! Range graph default clicked.
        void onRangeGraphClicked();

        //! Variable scale has changed.
        void onVariableScaleChanged(double scale);

        //! Variable data has changed.
        void onVariableDataChanged(uint mid, RVariableType variableType);

        //! Variable data has changed.
        void onVariableDataChanged(const SessionEntityID &entityID, RVariableType variableType);

    signals:

        //! Variable data has changed.
        void variableDataChanged(uint mid, RVariableType variableType);

        //! Variable data has changed.
        void variableDataChanged(const SessionEntityID &entityID, RVariableType variableType);

};

#endif // RESULTS_VARIABLE_TREE_H
