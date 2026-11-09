#ifndef SOLVER_SETUP_CHECKER_H
#define SOLVER_SETUP_CHECKER_H

#include <rml_model.h>

class SolverSetupChecker
{

    protected:

        //! Const reference to model.
        const RModel &rModel;

    private:

        //! Internal initialization function.
        void _init(const SolverSetupChecker *pSolverSetupChecker = nullptr);

    public:

        //! Constructor.
        explicit SolverSetupChecker(const RModel &rModel);

        //! Copy constructor.
        SolverSetupChecker(const SolverSetupChecker &solverSetupChecker);

        //! Destructor.
        ~SolverSetupChecker();

        //! Assignment operator.
        SolverSetupChecker &operator =(const SolverSetupChecker &solverSetupChecker);

        //! Perform check.
        void perform(QStringList &warnings,QStringList &errors) const;

        //! Check that problem tasks can be solved together and in given order.
        static void checkProblemTaskTree(const RProblemTaskItem &taskTree, QStringList &warnings, QStringList &errors);

    private:

        //! Append problem types of given task and of all tasks below it, in order of solving.
        static void collectProblemTypes(const RProblemTaskItem &taskItem, std::vector<RProblemType> &problemTypes);

        //! Check elements.
        void checkElements(QStringList &warnings,QStringList &errors) const;

        //! Check assigned materials.
        void checkMaterials(QStringList &warnings,QStringList &errors) const;

        //! Check boundary conditions.
        void checkBoundaryConditions(QStringList &warnings,QStringList &errors) const;

};

#endif // SOLVER_SETUP_CHECKER_H
