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

    private:

        //! Check elements.
        void checkElements(QStringList &warnings,QStringList &errors) const;

        //! Check assigned materials.
        void checkMaterials(QStringList &warnings,QStringList &errors) const;

        //! Check boundary conditions.
        void checkBoundaryConditions(QStringList &warnings,QStringList &errors) const;

};

#endif // SOLVER_SETUP_CHECKER_H
