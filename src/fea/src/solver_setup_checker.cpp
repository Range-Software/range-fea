#include "solver_setup_checker.h"

void SolverSetupChecker::_init(const SolverSetupChecker *pSolverSetupChecker)
{
    if (pSolverSetupChecker)
    {

    }
}

SolverSetupChecker::SolverSetupChecker(const RModel &rModel)
    : rModel(rModel)
{
    this->_init();
}

SolverSetupChecker::SolverSetupChecker(const SolverSetupChecker &solverSetupChecker)
    : rModel(solverSetupChecker.rModel)
{
    this->_init(&solverSetupChecker);
}

SolverSetupChecker::~SolverSetupChecker()
{

}

SolverSetupChecker &SolverSetupChecker::operator =(const SolverSetupChecker &solverSetupChecker)
{
    this->_init(&solverSetupChecker);
    return (*this);
}

void SolverSetupChecker::perform(QStringList &warnings, QStringList &errors) const
{
    warnings.clear();
    errors.clear();

    this->checkElements(warnings,errors);
    this->checkMaterials(warnings,errors);
    this->checkBoundaryConditions(warnings,errors);
}

void SolverSetupChecker::checkElements(QStringList &warnings, QStringList &errors) const
{
    if (this->rModel.getNElements() == 0)
    {
        errors.append("Empty model.");
        return;
    }

    if (this->rModel.getNVolumes() == 0)
    {
        warnings.append(QObject::tr("Model does not contain any volume elements."));
    }
}

void SolverSetupChecker::checkMaterials(QStringList &warnings, QStringList &errors) const
{
    RProblemTypeMask problemTypeMask = this->rModel.getProblemTaskTree().getProblemTypeMask();
    QList<RMaterialProperty::Type> materialProperties = RMaterialProperty::getTypes(problemTypeMask);

    QStringList matErrors;
    // Check if entities have material group assigned.
    for (uint i=0;i<this->rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = this->rModel.getElementGroupPtr(i);

        if (pElementGroup->getMaterial().size() == 0)
        {
            QString errorMessage = QObject::tr("Entity") + " <b>" + pElementGroup->getName() + "</b> " + QObject::tr("has no material assigned.");
            matErrors.append(errorMessage);
        }
    }

    if (uint(matErrors.size()) == this->rModel.getNElementGroups())
    {
        errors.append(QObject::tr("Model has no material assigned."));
        return;
    }
    if (matErrors.size() > 10)
    {
        QString errorMessage = QObject::tr("Multiple") + " " + QLocale().toString(matErrors.size()) + " " + QObject::tr("entities have no material assigned.");
        errors.append(errorMessage);
        return;
    }
    if (matErrors.size() > 0)
    {
        errors = matErrors;
        return;
    }

    // Check if entities have material properties needed for starting the solver.
    for (uint i=0;i<this->rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = this->rModel.getElementGroupPtr(i);

        if (!pElementGroup->getMaterial().hasProperties(materialProperties))
        {
            QString errorMessage = QObject::tr("Entity") + " <b>" + pElementGroup->getName() + "</b> " + QObject::tr("has material assigned which is missing required properties.");
            warnings.append(errorMessage);
        }
    }
}

void SolverSetupChecker::checkBoundaryConditions(QStringList &warnings, QStringList &errors) const
{
    bool hasExplicit = false;
    bool hasImplicit = false;

    std::vector<RProblemType> problemTypes = RProblem::getTypes(this->rModel.getProblemTaskTree().getProblemTypeMask());

    for (uint i=0;i<this->rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = this->rModel.getElementGroupPtr(i);

        for (uint j=0;j<pElementGroup->getNBoundaryConditions();j++)
        {
            if (pElementGroup->getBoundaryCondition(j).getExplicit())
            {
                hasExplicit = true;
            }
            else
            {
                hasImplicit = true;
            }
        }
    }

    if (!hasExplicit && !hasImplicit)
    {
        errors.append(QObject::tr("No boundary condition assigned."));
        return;
    }

    for (uint i=0;i<problemTypes.size();i++)
    {
        if (problemTypes[i] == R_PROBLEM_MESH)
        {
            continue;
        }
        hasExplicit = hasImplicit = false;

        std::vector<RBoundaryConditionType> conditionTypes = RBoundaryCondition::getTypes(problemTypes[i]);

        for (uint j=0;j<conditionTypes.size();j++)
        {
            if ((RBoundaryCondition::getExplicit(conditionTypes[j]) && hasExplicit) ||
                (!RBoundaryCondition::getExplicit(conditionTypes[j]) && hasImplicit))
            {
                continue;
            }
            for (uint k=0;k<this->rModel.getNElementGroups();k++)
            {
                const RElementGroup *pElementGroup = this->rModel.getElementGroupPtr(k);

                if (pElementGroup->hasBoundaryCondition(conditionTypes[j]))
                {
                    if (RBoundaryCondition::getExplicit(conditionTypes[j]))
                    {
                        hasExplicit = true;
                    }
                    else
                    {
                        hasImplicit = true;
                    }
                    break;
                }
            }
            if (hasExplicit && hasImplicit)
            {
                break;
            }
        }

        if (!hasExplicit)
        {
            warnings.append("<b>" + RProblem::getName(problemTypes[i]) + ":</b> " + QObject::tr("No explicit boundary condition assigned."));
        }
        if (!hasImplicit)
        {
            warnings.append("<b>" + RProblem::getName(problemTypes[i]) + ":</b> " + QObject::tr("No implicit boundary condition assigned."));
        }
    }
}
