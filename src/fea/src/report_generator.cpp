#include <QTextCursor>
#include <QStyleFactory>

#include <rbl_logger.h>

#include "report_generator.h"
#include "application.h"

ReportGenerator::ReportGenerator(uint modelID)
    : modelID(modelID)
{
    R_LOG_TRACE;
    this->setEnableHeaderCounters(false);
}

void ReportGenerator::generateTitle()
{
    R_LOG_TRACE;
    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    QTextCursor cursor(this->docTitle);

    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignCenter);

    cursor.insertBlock(blockFormat);
    cursor.insertText("Analysis Report",ReportGenerator::getH1Format());
    cursor.insertBlock();
    cursor.insertText(rModel.getName(),ReportGenerator::getH2Format());
    cursor.insertBlock(blockFormat);
    std::vector<RProblemType> problemTypes = RProblem::getTypes(rModel.getProblemTaskTree().getProblemTypeMask());
    QString problemTypeList;
    for (uint i=0;i<problemTypes.size();i++)
    {
        if (i > 0)
        {
            problemTypeList += ", ";
        }
        problemTypeList += RProblem::getName(problemTypes[i]);
    }
    cursor.insertText(problemTypeList,ReportGenerator::getH3Format());
    cursor.insertBlock(blockFormat);
    cursor.insertBlock(blockFormat);
    cursor.insertText(Application::instance()->getApplicationSettings()->getUserFullName(),ReportGenerator::getH4Format());
    cursor.insertBlock(blockFormat);
    cursor.insertText(Application::instance()->getApplicationSettings()->getUserEmail(),ReportGenerator::getTextFormat());
    cursor.insertBlock(blockFormat);
    cursor.insertBlock(blockFormat);
    cursor.insertText(QDateTime::currentDateTimeUtc().toLocalTime().toString("MMMM d, yyyy"),ReportGenerator::getTextFormat());
}

void ReportGenerator::generateBody()
{
    R_LOG_TRACE;
    this->generateModelChapter();
    this->generateProblemChapter();
    this->generateResultsChapter();
}

void ReportGenerator::generateModelChapter()
{
    R_LOG_TRACE;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    QTextCursor cursor(this->docBody);
    cursor.movePosition(QTextCursor::End);

    this->insertH1(cursor,tr("Model"));
    this->insertH2(cursor,tr("Mesh details"));

    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(1);
    tableFormat.setCellSpacing(0);
    tableFormat.setBorder(1.0);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setAlignment(Qt::AlignCenter);

    cursor.insertTable(6, 2, tableFormat);

    this->insertText(cursor,tr("Number of nodes") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNNodes()),true);

    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,tr("Number of elements") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNElements()),true);

    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,tr("Number of point entities") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNPoints()),true);

    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,tr("Number of line entities") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNLines()),true);

    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,tr("Number of surface entities") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNSurfaces()),true);

    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,tr("Number of volume entities") + ":",true);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,QLocale().toString(rModel.getNVolumes()),true);
}

void ReportGenerator::generateProblemChapter()
{
    R_LOG_TRACE;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    QTextCursor cursor(this->docBody);
    cursor.movePosition(QTextCursor::End);

    this->insertH1(cursor,tr("Problem"));

//    RProblemTypeMask problemTypeMask = rModel.getProblemTaskTree().getProblemTypeMask();
//    if (problemTypeMask & R_PROBLEM_RADIATIVE_HEAT)
//    {
//        this->insertH2(cursor,tr("Radiation setup"));
//        const RRadiationSetup &radiationSetup = rModel.getProblemSetup().getRadiationSetup();
//        this->insertBold(cursor,tr("Resolution:"));
//        this->insertText(cursor,RRadiationSetup::getResolutionText(radiationSetup.getResolution()),true);
//    }

//    this->insertH2(cursor,tr("Time solver setup"));
//    const RTimeSolver &timeSolver = rModel.getTimeSolver();

    this->insertH2(cursor,tr("Boundary conditions"));

    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }

        uint nbc = pElementGroup->getNBoundaryConditions();
        if (nbc == 0)
        {
            continue;
        }

        this->insertH3(cursor,pElementGroup->getName());

        for (uint j=0;j<nbc;j++)
        {
            const RBoundaryCondition &bc = pElementGroup->getBoundaryCondition(j);

            this->insertH4(cursor,RBoundaryCondition::getName(bc.getType()));

            for (uint k=0;k<bc.size();k++)
            {
                const RConditionComponent &cc = bc.getComponent(k);

                cursor.insertTable(cc.size()+1,2,ReportGenerator::getTableFormat());

                this->insertBold(cursor,tr(cc.getKeyName().toUtf8().constData()) + " [" + cc.getKeyUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                this->insertBold(cursor,tr(cc.getValueName().toUtf8().constData()) + " [" + cc.getUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                for (uint l=0;l<cc.size();l++)
                {
                    this->insertText(cursor,QLocale().toString(cc.getKey(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);

                    this->insertText(cursor,QLocale().toString(cc.getValue(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);
                }

                cursor.movePosition(QTextCursor::NextBlock);
            }

        }
    }

    this->insertH2(cursor,tr("Initial conditions"));

    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }

        uint nic = pElementGroup->getNInitialConditions();
        if (nic == 0)
        {
            continue;
        }

        this->insertH3(cursor,pElementGroup->getName());

        for (uint j=0;j<nic;j++)
        {
            const RInitialCondition &ic = pElementGroup->getInitialCondition(j);

            this->insertH4(cursor,RInitialCondition::getName(ic.getType()));

            for (uint k=0;k<ic.size();k++)
            {
                const RConditionComponent &cc = ic.getComponent(k);

                cursor.insertTable(cc.size()+1,2,ReportGenerator::getTableFormat());

                this->insertBold(cursor,tr(cc.getKeyName().toUtf8().constData()) + " [" + cc.getKeyUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                this->insertBold(cursor,tr(cc.getValueName().toUtf8().constData()) + " [" + cc.getUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                for (uint l=0;l<cc.size();l++)
                {
                    this->insertText(cursor,QLocale().toString(cc.getKey(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);

                    this->insertText(cursor,QLocale().toString(cc.getValue(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);
                }

                cursor.movePosition(QTextCursor::NextBlock);
            }

        }
    }

    this->insertH2(cursor,tr("Environment conditions"));

    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }

        uint nec = pElementGroup->getNEnvironmentConditions();
        if (nec == 0)
        {
            continue;
        }

        this->insertH3(cursor,pElementGroup->getName());

        for (uint j=0;j<nec;j++)
        {
            const REnvironmentCondition &ec = pElementGroup->getEnvironmentCondition(j);

            this->insertH4(cursor,REnvironmentCondition::getName(ec.getType()));

            for (uint k=0;k<ec.size();k++)
            {
                const RConditionComponent &cc = ec.getComponent(k);

                cursor.insertTable(cc.size()+1,2,ReportGenerator::getTableFormat());

                this->insertBold(cursor,tr(cc.getKeyName().toUtf8().constData()) + " [" + cc.getKeyUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                this->insertBold(cursor,tr(cc.getValueName().toUtf8().constData()) + " [" + cc.getUnits() + "]",true);
                cursor.movePosition(QTextCursor::NextCell);

                for (uint l=0;l<cc.size();l++)
                {
                    this->insertText(cursor,QLocale().toString(cc.getKey(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);

                    this->insertText(cursor,QLocale().toString(cc.getValue(l)),true);
                    cursor.movePosition(QTextCursor::NextCell);
                }

                cursor.movePosition(QTextCursor::NextBlock);
            }
        }
    }

    this->insertH2(cursor,tr("Materials"));

    for (uint i=0;i<rModel.getNElementGroups();i++)
    {
        const RElementGroup *pElementGroup = rModel.getElementGroupPtr(i);
        if (!pElementGroup)
        {
            continue;
        }

        this->insertH3(cursor,pElementGroup->getName());

        const RMaterial &material = pElementGroup->getMaterial();

        this->insertH4(cursor,material.getName());

        for (uint k=0;k<material.size();k++)
        {
            const RMaterialProperty &materialProperty = material.get(k);

            this->insertH5(cursor,tr(materialProperty.getValueName().toUtf8().constData()));

            cursor.insertTable(materialProperty.size()+1,2,ReportGenerator::getTableFormat());

            this->insertBold(cursor,tr(materialProperty.getKeyName().toUtf8().constData()) + " [" + materialProperty.getKeyUnits() + "]",true);
            cursor.movePosition(QTextCursor::NextCell);

            this->insertBold(cursor,tr(materialProperty.getValueName().toUtf8().constData()) + " [" + materialProperty.getUnits() + "]",true);
            cursor.movePosition(QTextCursor::NextCell);

            for (uint l=0;l<materialProperty.size();l++)
            {
                this->insertText(cursor,QLocale().toString(materialProperty.getKey(l)),true);
                cursor.movePosition(QTextCursor::NextCell);

                this->insertText(cursor,QLocale().toString(materialProperty.getValue(l)),true);
                cursor.movePosition(QTextCursor::NextCell);
            }

            cursor.movePosition(QTextCursor::NextBlock);
        }
    }
}

void ReportGenerator::generateResultsChapter()
{
    R_LOG_TRACE;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);

    QTextCursor cursor(this->docBody);
    cursor.movePosition(QTextCursor::End);

    this->insertH1(cursor,tr("Results"));

    this->insertBold(cursor,tr("Time-solver") + ":");
    this->insertText(cursor," " + tr(rModel.getTimeSolver().getEnabled() ? "enabled" : "disabled"),true);

    if (rModel.getTimeSolver().getEnabled())
    {
        this->insertBold(cursor,tr("Current time step") + ":");
        this->insertText(cursor," " + QLocale().toString(rModel.getTimeSolver().getCurrentTimeStep()+1),true);
        this->insertBold(cursor,tr("Current computational time") + ":");
        this->insertText(cursor," " + QLocale().toString(rModel.getTimeSolver().getCurrentTime()) + " [" + RVariable::getUnits(R_VARIABLE_TIME) + "]",true);
    }
    else
    {
        if (rModel.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS_MODAL)
        {
            this->insertBold(cursor,tr("Current mode") + ":");
            this->insertText(cursor," " + QLocale().toString(rModel.getProblemSetup().getModalSetup().getMode()+1),true);
            this->insertBold(cursor,tr("Current frequency") + ":");
            this->insertText(cursor," " + QLocale().toString(rModel.getProblemSetup().getModalSetup().getFrequency()) + " [" + RVariable::getUnits(R_VARIABLE_FREQUENCY) + "]",true);
        }
    }

//    const RMonitoringPointManager &monitoringPointManager = rModel.getMonitoringPointManager();

//    if (monitoringPointManager.size() > 0)
//    {
//        this->insertH2(cursor,tr("Monitoring points"));

//        for (uint i=0;i<monitoringPointManager.size();i++)
//        {
//            const RMonitoringPoint &monitoringPoint = monitoringPointManager.at(i);
////            monitoringPoint.g
//        }
//    }

    this->insertH2(cursor,tr("Variables"));

    for (uint i=0;i<rModel.getNVariables();i++)
    {
        const RVariable &rVariable = rModel.getVariable(i);
        this->insertH3(cursor,rVariable.getName());

        QTextBlockFormat blockFormat;
        blockFormat.setIndent(2);

        QTextBlockFormat rightAlignment;
        rightAlignment.setAlignment(Qt::AlignRight);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(1);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0.0);
        tableFormat.setHeaderRowCount(0);
        tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tableFormat.setAlignment(Qt::AlignCenter);

        RStatistics stat(rVariable.getValues(),100,true);

        cursor.insertTable(8,2,tableFormat);

        this->insertBold(cursor,tr("Variable type") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,tr(rVariable.getNVectors() == 1 ? "Scalar" : "Vector"),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Units") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,rVariable.getUnits(),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Minimum") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getMin()),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Maximum") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getMax()),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Average") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getAvg()),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Median") + ":",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getMed()),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Percentile") + " 5:",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getP05()),true);
        cursor.movePosition(QTextCursor::NextCell);

        this->insertBold(cursor,tr("Percentile") + " 95:",true);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(rightAlignment);
        this->insertText(cursor,QLocale().toString(stat.getP95()),true);
        cursor.movePosition(QTextCursor::NextCell);

        cursor.movePosition(QTextCursor::NextBlock);
    }

    this->insertH2(cursor,tr("Screenshot"));

    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignCenter);
    cursor.insertBlock(blockFormat);
    QImage modelScreenShotImage(this->takeModelScreenshot());
    QRect pageRect(this->printer->pageLayout().paintRectPixels(this->printer->resolution()));
    cursor.insertImage(modelScreenShotImage.scaledToWidth(qCeil(double(pageRect.width())*0.9),Qt::SmoothTransformation));
}

QImage ReportGenerator::takeModelScreenshot() const
{
    R_LOG_TRACE;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
    QString screenShotFile(rModel.buildScreenShotFileName());

    Application::instance()->getSession()->setTakeScreenShot(this->modelID,screenShotFile,true);

    return QImage(screenShotFile);
}
