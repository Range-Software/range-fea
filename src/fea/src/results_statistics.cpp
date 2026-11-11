#include <cmath>
#include <limits>
#include <set>

#include <QLocale>

#include <rml_problem.h>
#include <rml_stress_setup.h>
#include <rml_support_reaction.h>

#include "results_statistics.h"
#include "model.h"

ResultsStatistics::ResultsStatistics(const Model &rModel)
    : hasResultsFlag(rModel.getNVariables() > 0)
{
    if (!this->hasResultsFlag)
    {
        return;
    }

    const RProblemTypeMask problemTypeMask = rModel.getProblemTaskTree().getProblemTypeMask();

    // State of the solution the results belong to.
    if (rModel.getTimeSolver().getEnabled())
    {
        this->solutionState.push_back({tr("Time step"),QLocale().toString(rModel.getTimeSolver().getCurrentTimeStep()+1),QString()});
        this->solutionState.push_back({tr("Computational time") + " [" + RVariable::getUnits(R_VARIABLE_TIME) + "]",
                                       ResultsStatistics::realToString(rModel.getTimeSolver().getCurrentTime()),QString()});
    }
    else if (problemTypeMask & R_PROBLEM_STRESS_MODAL)
    {
        this->solutionState.push_back({tr("Mode"),QLocale().toString(rModel.getProblemSetup().getModalSetup().getMode()+1),QString()});
        this->solutionState.push_back({tr("Frequency") + " [" + RVariable::getUnits(R_VARIABLE_FREQUENCY) + "]",
                                       ResultsStatistics::realToString(rModel.getProblemSetup().getModalSetup().getFrequency()),QString()});
    }

    // Entity each element belongs to, to tell where an element value occurs.
    std::vector<QString> elementEntityNames(rModel.getNElements());
    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }
        QString entityName = ResultsStatistics::entityTypeName(rModel.getEntityGroupType(i)) + " '" + pElementGroup->getName() + "'";
        for (uint j=0;j<pElementGroup->size();j++)
        {
            uint elementID = pElementGroup->get(j);
            if (elementID < elementEntityNames.size() && elementEntityNames[elementID].isEmpty())
            {
                elementEntityNames[elementID] = entityName;
            }
        }
    }

    // Results of each solved problem type, in the order they are solved.
    std::set<RVariableType> listedVariables;
    std::vector<RProblemType> problemTypes = RProblem::getTypes(problemTypeMask);
    RProblem::sortTypesByDependency(problemTypes);
    for (RProblemType problemType : problemTypes)
    {
        Section section;
        section.title = RProblem::getName(problemType);
        // Elements carrying body and boundary results of this problem type,
        // found when needed.
        std::vector<bool> resultElements[2];
        for (RVariableType variableType : RProblem::getComputedVariableTypes(RProblemTypeMask(problemType)))
        {
            if (listedVariables.count(variableType) > 0)
            {
                continue;
            }
            uint variablePosition = rModel.findVariable(variableType);
            if (variablePosition == RConstants::eod)
            {
                continue;
            }
            const int boundaryResult = ResultsStatistics::isBoundaryResult(variableType) ? 1 : 0;
            if (resultElements[boundaryResult].empty())
            {
                resultElements[boundaryResult] = ResultsStatistics::findResultElements(rModel,RProblemTypeMask(problemType),boundaryResult == 1);
            }
            Variable variable;
            if (ResultsStatistics::findVariableStatistics(rModel,rModel.getVariable(variablePosition),elementEntityNames,resultElements[boundaryResult],variable))
            {
                section.variables.push_back(variable);
            }
            listedVariables.insert(variableType);
        }
        if (problemType == R_PROBLEM_STRESS)
        {
            section.supportReactions = ResultsStatistics::findSupportReactions(rModel);
        }
        if (!section.variables.empty() || !section.supportReactions.rows.empty())
        {
            this->sections.push_back(section);
        }
    }

    // Anything stored which does not belong to a solved problem type. Which
    // elements carry it is not known, so all but point elements are taken.
    Section otherSection;
    otherSection.title = tr("Other results");
    const std::vector<bool> otherResultElements = ResultsStatistics::findResultElements(rModel,R_PROBLEM_NONE,true);
    for (uint i=0;i<rModel.getNVariables();i++)
    {
        const RVariable &rVariable = rModel.getVariable(i);
        if (listedVariables.count(rVariable.getType()) > 0)
        {
            continue;
        }
        Variable variable;
        if (ResultsStatistics::findVariableStatistics(rModel,rVariable,elementEntityNames,otherResultElements,variable))
        {
            otherSection.variables.push_back(variable);
        }
        listedVariables.insert(rVariable.getType());
    }
    if (!otherSection.variables.empty())
    {
        this->sections.push_back(otherSection);
    }
}

bool ResultsStatistics::hasResults() const
{
    return this->hasResultsFlag;
}

const std::vector<ResultsStatistics::Row> &ResultsStatistics::getSolutionState() const
{
    return this->solutionState;
}

const std::vector<ResultsStatistics::Section> &ResultsStatistics::getSections() const
{
    return this->sections;
}

QString ResultsStatistics::realToString(double value)
{
    // One format for all real values, so that they line up and compare
    // easily, whatever their magnitude.
    return QLocale().toString(value,'e',6);
}

QString ResultsStatistics::entityTypeName(REntityGroupType entityType)
{
    switch (entityType)
    {
        case R_ENTITY_GROUP_POINT:
            return tr("point");
        case R_ENTITY_GROUP_LINE:
            return tr("line");
        case R_ENTITY_GROUP_SURFACE:
            return tr("surface");
        case R_ENTITY_GROUP_VOLUME:
            return tr("volume");
        default:
            return REntityGroup::getTypeName(entityType);
    }
}

bool ResultsStatistics::isBoundaryResult(RVariableType variableType)
{
    // Computed on the boundary surfaces of a body, which have no thickness.
    return (variableType == R_VARIABLE_HEAT_RADIATION ||
            variableType == R_VARIABLE_HEAT_TRANSFER_COEFFICIENT);
}

std::vector<bool> ResultsStatistics::findResultElements(const Model &rModel, RProblemTypeMask problemTypeMask, bool boundaryResult)
{
    // Point elements carry no element values at all. Results of the body
    // follow the rules the solvers use to pick the elements they compute -
    // elsewhere an element only holds the value it was initialized with:
    //  - the entity has the material properties the problem type requires,
    //  - a surface has a thickness and a line a cross area. Without them
    //    they are the boundary of a body, like the surface of a solid.
    std::vector<bool> resultElements(rModel.getNElements(),false);
    const QList<RMaterialProperty::Type> requiredProperties = RMaterialProperty::getRequiredTypes(problemTypeMask);
    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }
        if (!boundaryResult)
        {
            if (!requiredProperties.isEmpty() && !pElementGroup->getMaterial().hasProperties(requiredProperties))
            {
                continue;
            }
            REntityGroupType entityType = rModel.getEntityGroupType(i);
            if (entityType == R_ENTITY_GROUP_SURFACE && !(rModel.getSurface(rModel.getEntityGroupPosition(i)).getThickness() > 0.0))
            {
                continue;
            }
            if (entityType == R_ENTITY_GROUP_LINE && !(rModel.getLine(rModel.getEntityGroupPosition(i)).getCrossArea() > 0.0))
            {
                continue;
            }
        }
        for (uint j=0;j<pElementGroup->size();j++)
        {
            uint elementID = pElementGroup->get(j);
            if (elementID < resultElements.size() && !R_ELEMENT_TYPE_IS_POINT(rModel.getElement(elementID).getType()))
            {
                resultElements[elementID] = true;
            }
        }
    }
    return resultElements;
}

bool ResultsStatistics::findVariableStatistics(const Model &rModel,
                                               const RVariable &rVariable,
                                               const std::vector<QString> &elementEntityNames,
                                               const std::vector<bool> &resultElements,
                                               Variable &variable)
{
    const bool isVector = (rVariable.getNVectors() > 1);
    const bool onNodes = (rVariable.getApplyType() == R_VARIABLE_APPLY_NODE);
    const bool onElements = (rVariable.getApplyType() == R_VARIABLE_APPLY_ELEMENT);

    uint nValues = rVariable.getNValues();
    if (onNodes)
    {
        nValues = std::min(nValues,rModel.getNNodes());
    }
    else if (onElements)
    {
        nValues = std::min(nValues,rModel.getNElements());
    }

    // Vectors are described by their magnitude.
    uint minPosition = RConstants::eod;
    uint maxPosition = RConstants::eod;
    double minValue = 0.0;
    double maxValue = 0.0;
    double sum = 0.0;
    uint nValid = 0;
    // Averages are weighted by the element size. Lengths, areas and volumes
    // can not be added together, so each element dimension has an average of
    // its own: 0 - line, 1 - surface, 2 - volume.
    double weightedSum[3] = { 0.0, 0.0, 0.0 };
    double weightSum[3] = { 0.0, 0.0, 0.0 };

    auto findValue = [&](uint position) -> double
    {
        if (!isVector)
        {
            return rVariable.getValue(0,position);
        }
        double value = 0.0;
        for (uint j=0;j<rVariable.getNVectors();j++)
        {
            double component = rVariable.getValue(j,position);
            value += component*component;
        }
        return std::sqrt(value);
    };

    // Size of an element and its dimension (0 - line, 1 - surface,
    // 2 - volume), or -1 when it has none.
    auto findElementSize = [&](const RElement &rElement, double &size) -> int
    {
        size = 0.0;
        int dimension = -1;
        if (R_ELEMENT_TYPE_IS_LINE(rElement.getType()))
        {
            dimension = rElement.findLength(rModel.getNodes(),size) ? 0 : -1;
        }
        else if (R_ELEMENT_TYPE_IS_SURFACE(rElement.getType()))
        {
            dimension = rElement.findArea(rModel.getNodes(),size) ? 1 : -1;
        }
        else if (R_ELEMENT_TYPE_IS_VOLUME(rElement.getType()))
        {
            dimension = rElement.findVolume(rModel.getNodes(),size) ? 2 : -1;
        }
        // The volume of a tetrahedron is signed by its node order.
        size = std::fabs(size);
        if (dimension < 0 || !std::isfinite(size) || size <= 0.0)
        {
            return -1;
        }
        return dimension;
    };

    for (uint i=0;i<nValues;i++)
    {
        if (onElements && (i >= resultElements.size() || !resultElements[i]))
        {
            continue;
        }
        double value = findValue(i);
        if (!std::isfinite(value))
        {
            continue;
        }
        if (nValid == 0 || value < minValue)
        {
            minValue = value;
            minPosition = i;
        }
        if (nValid == 0 || value > maxValue)
        {
            maxValue = value;
            maxPosition = i;
        }
        sum += value;
        nValid++;

        if (onElements)
        {
            double size = 0.0;
            int dimension = findElementSize(rModel.getElement(i),size);
            if (dimension >= 0)
            {
                weightedSum[dimension] += value*size;
                weightSum[dimension] += size;
            }
        }
    }

    if (onNodes)
    {
        // Node values are interpolated linearly over the elements, so the
        // integral over an element is its size times the mean of its node
        // values. Summed over the elements carrying the result this gives the
        // average over the body, whatever the mesh density. For a vector
        // magnitude, which is not linear within an element, it is a close
        // approximation.
        for (uint i=0;i<rModel.getNElements() && i<resultElements.size();i++)
        {
            if (!resultElements[i])
            {
                continue;
            }
            const RElement &rElement = rModel.getElement(i);
            double size = 0.0;
            int dimension = findElementSize(rElement,size);
            if (dimension < 0 || rElement.size() == 0)
            {
                continue;
            }
            double elementSum = 0.0;
            bool valid = true;
            for (uint j=0;j<rElement.size() && valid;j++)
            {
                uint nodeID = rElement.getNodeId(j);
                double value = (nodeID < nValues) ? findValue(nodeID) : std::numeric_limits<double>::quiet_NaN();
                valid = std::isfinite(value);
                elementSum += value;
            }
            if (!valid)
            {
                continue;
            }
            weightedSum[dimension] += size*elementSum/double(rElement.size());
            weightSum[dimension] += size;
        }
    }

    if (nValid == 0)
    {
        return false;
    }

    auto findLocation = [&](uint position) -> QString
    {
        double x = 0.0, y = 0.0, z = 0.0;
        QString location;
        if (onNodes)
        {
            const RNode &rNode = rModel.getNode(position);
            x = rNode.getX();
            y = rNode.getY();
            z = rNode.getZ();
            location = tr("Node") + " # " + QLocale().toString(position);
        }
        else if (onElements)
        {
            rModel.getElement(position).findCenter(rModel.getNodes(),x,y,z);
            location = tr("Element") + " # " + QLocale().toString(position);
            if (position < elementEntityNames.size() && !elementEntityNames[position].isEmpty())
            {
                location += ", " + elementEntityNames[position];
            }
        }
        else
        {
            return QString();
        }
        return location + " [" + ResultsStatistics::realToString(x) + " " + ResultsStatistics::realToString(y) + " " + ResultsStatistics::realToString(z) + "]";
    };

    variable.title = rVariable.getName();
    if (!rVariable.getUnits().isEmpty())
    {
        variable.title += " [" + rVariable.getUnits() + "]";
    }
    variable.rows.clear();

    if (minValue == maxValue)
    {
        // Same everywhere, as a stress component of a body held in that
        // direction - one row says it all.
        variable.rows.push_back({isVector ? tr("Constant magnitude") : tr("Constant value"),ResultsStatistics::realToString(minValue),QString()});
        return true;
    }
    variable.rows.push_back({isVector ? tr("Minimum magnitude") : tr("Minimum"),ResultsStatistics::realToString(minValue),findLocation(minPosition)});
    variable.rows.push_back({isVector ? tr("Maximum magnitude") : tr("Maximum"),ResultsStatistics::realToString(maxValue),findLocation(maxPosition)});
    const double plainAverage = sum/double(nValid);
    if (onNodes || onElements)
    {
        const QString weightedLabels[3][2] =
        {
            { tr("Length-weighted average"), tr("Length-weighted average magnitude") },
            { tr("Area-weighted average"),   tr("Area-weighted average magnitude")   },
            { tr("Volume-weighted average"), tr("Volume-weighted average magnitude") }
        };
        bool hasWeighted = false;
        for (int d=2;d>=0;d--)
        {
            if (weightSum[d] > 0.0)
            {
                variable.rows.push_back({weightedLabels[d][isVector ? 1 : 0],ResultsStatistics::realToString(weightedSum[d]/weightSum[d]),QString()});
                hasWeighted = true;
            }
        }
        if (!hasWeighted)
        {
            // No element with a size carries the result - every value counts
            // once, so a finely meshed region weighs more.
            if (onNodes)
            {
                variable.rows.push_back({isVector ? tr("Average magnitude of node values") : tr("Average of node values"),ResultsStatistics::realToString(plainAverage),QString()});
            }
            else
            {
                variable.rows.push_back({isVector ? tr("Average magnitude of element values") : tr("Average of element values"),ResultsStatistics::realToString(plainAverage),QString()});
            }
        }
    }
    else
    {
        variable.rows.push_back({isVector ? tr("Average magnitude") : tr("Average"),ResultsStatistics::realToString(plainAverage),QString()});
    }
    return true;
}

ResultsStatistics::SupportReactions ResultsStatistics::findSupportReactions(const Model &rModel)
{
    SupportReactions supportReactions;

    const std::vector<RSupportReaction> &rSupportReactions = rModel.getSupportReactions();
    if (rSupportReactions.empty())
    {
        return supportReactions;
    }

    const QString units(" [" + RVariable::getUnits(R_VARIABLE_FORCE) + "]");
    supportReactions.headers << tr("Support") << "Fx" + units << "Fy" + units << "Fz" + units << "|F|" + units;

    RR3Vector totalReaction(0.0,0.0,0.0);
    for (const RSupportReaction &rSupportReaction : rSupportReactions)
    {
        supportReactions.rows.push_back({rSupportReaction.getName() + " (" + ResultsStatistics::entityTypeName(rSupportReaction.getEntityType()) + ")",
                                         false,
                                         rSupportReaction.getForce()});
        for (uint i=0;i<3;i++)
        {
            totalReaction[i] += rSupportReaction.getForce()[i];
        }
    }
    const RR3Vector &appliedLoad = rModel.getAppliedLoad();
    RR3Vector imbalance(totalReaction[0]+appliedLoad[0],totalReaction[1]+appliedLoad[1],totalReaction[2]+appliedLoad[2]);

    supportReactions.rows.push_back({tr("Total reaction"),true,totalReaction});
    supportReactions.rows.push_back({tr("Total applied load"),true,appliedLoad});
    supportReactions.rows.push_back({tr("Imbalance"),true,imbalance});

    // In equilibrium the supports push back with exactly the applied load, so
    // the imbalance is zero. What remains is the residual left by the matrix
    // solver, which stops at its convergence tolerance.
    const bool isDynamic = (rModel.getTimeSolver().getEnabled() &&
                            rModel.getProblemSetup().getStressSetup().getTransientType() == R_STRESS_TRANSIENT_DYNAMIC);
    if (isDynamic)
    {
        supportReactions.notes << tr("Inertia and damping forces are not included.");
    }
    supportReactions.notes << tr("Imbalance is the sum of the total reaction and the total applied load, in equilibrium it is zero.");
    QString imbalanceNote = isDynamic
            ? tr("In a dynamic analysis it holds the inertia and damping forces as well.")
            : tr("What remains comes from the convergence tolerance of the matrix solver.");
    if (appliedLoad.length() > 0.0)
    {
        imbalanceNote += " " + tr("Here it is %1 % of the total applied load.").arg(QLocale().toString(100.0*imbalance.length()/appliedLoad.length(),'g',6));
    }
    supportReactions.notes << imbalanceNote;

    return supportReactions;
}
