#ifndef PROBLEM_TREE_H
#define PROBLEM_TREE_H

#include <QTreeWidget>

#include <rml_problem.h>

#include <rgl_file_chooser_button.h>

class ProblemTree : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ProblemTree(QWidget *parent = nullptr);

    protected slots:

        //! Populate.
        void populate();

        //! Model has changed.
        void onModelChanged(uint);

        //! Problem has changed.
        void onProblemChanged(uint);

        //! Model selection has changed.
        void onModelSelectionChanged(uint);

        //! Time solver has changed.
        void onTimeSolverChanged(const RTimeSolver &timeSolver);

        //! Modal setup has changed.
        void onModalSetupChanged(const RModalSetup &modalSetup);

        //! Radiation setup has changed.
        void onRadiationSetupChanged(const RRadiationSetup &radiationSetup);

        //! Mesh setup has changed.
        void onMeshSetupChanged(const RMeshSetup &meshSetup);

};

#endif // PROBLEM_TREE_H
