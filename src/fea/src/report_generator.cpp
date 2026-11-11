#include <QTextCursor>
#include <QFileInfo>
#include <QTextImageFormat>
#include <QUrl>

#include <rbl_logger.h>
#include <rml_problem_type.h>
#include <rml_problem.h>
#include <rml_element_group.h>
#include <rml_boundary_condition.h>
#include <rml_condition_component.h>
#include <rml_initial_condition.h>
#include <rml_environment_condition.h>
#include <rml_material.h>
#include <rml_material_property.h>
#include <rml_variable.h>
#include <rbl_statistics.h>

#include "report_generator.h"
#include "application.h"
#include "model.h"
#include "application_settings.h"
#include "session.h"

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
    cursor.insertText(tr("Author") + ": ",ReportGenerator::getTextFormat());
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

    ResultsStatistics resultsStatistics(rModel);

    if (!resultsStatistics.hasResults())
    {
        this->insertText(cursor,ResultsStatistics::tr("No results are available."));
    }
    else
    {
        // State of the solution the results belong to.
        for (const ResultsStatistics::Row &row : resultsStatistics.getSolutionState())
        {
            this->insertBold(cursor,row.label + ":");
            this->insertText(cursor," " + row.value,true);
        }

        QTextBlockFormat rightAlignment;
        rightAlignment.setAlignment(Qt::AlignRight);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(4);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0.0);
        tableFormat.setHeaderRowCount(0);
        tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tableFormat.setAlignment(Qt::AlignLeft);
        tableFormat.setLeftMargin(30.0);

        for (const ResultsStatistics::Section &section : resultsStatistics.getSections())
        {
            this->insertH2(cursor,section.title);

            for (const ResultsStatistics::Variable &variable : section.variables)
            {
                this->insertH4(cursor,variable.title);

                // Label, value and where the value occurs.
                cursor.insertTable(int(variable.rows.size()),3,tableFormat);
                for (const ResultsStatistics::Row &row : variable.rows)
                {
                    this->insertText(cursor,row.label,true);
                    cursor.movePosition(QTextCursor::NextCell);
                    cursor.setBlockFormat(rightAlignment);
                    this->insertText(cursor,row.value,true);
                    cursor.movePosition(QTextCursor::NextCell);
                    this->insertText(cursor,row.location,true);
                    cursor.movePosition(QTextCursor::NextCell);
                }
                cursor.movePosition(QTextCursor::NextBlock);
            }

            this->generateSupportReactions(cursor,section.supportReactions);
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

    this->insertH2(cursor,tr("Screenshot"));

    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignCenter);
    cursor.insertBlock(blockFormat);
    QString screenShotFileName;
    QImage modelScreenShotImage(this->takeModelScreenshot(screenShotFileName));
    if (!modelScreenShotImage.isNull())
    {
        QRect pageRect(this->printer->pageLayout().paintRectPixels(this->printer->resolution()));
        QImage image(modelScreenShotImage.scaledToWidth(qCeil(double(pageRect.width())*0.9),Qt::SmoothTransformation));
        // Named after the screenshot file, so that HTML and Markdown can refer
        // to it as a file next to the document.
        QTextImageFormat imageFormat;
        imageFormat.setName(QFileInfo(screenShotFileName).fileName());
        imageFormat.setWidth(image.width());
        imageFormat.setHeight(image.height());
        this->docBody->addResource(QTextDocument::ImageResource,QUrl(imageFormat.name()),image);
        cursor.insertImage(imageFormat);
    }
}

void ReportGenerator::generateSupportReactions(QTextCursor &cursor, const ResultsStatistics::SupportReactions &supportReactions)
{
    R_LOG_TRACE;

    if (supportReactions.rows.empty())
    {
        return;
    }

    this->insertH4(cursor,ResultsStatistics::tr("Support reactions"));

    QTextBlockFormat rightAlignment;
    rightAlignment.setAlignment(Qt::AlignRight);

    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(4);
    tableFormat.setCellSpacing(0);
    tableFormat.setBorder(0.0);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setAlignment(Qt::AlignLeft);
    tableFormat.setLeftMargin(30.0);

    cursor.insertTable(int(supportReactions.rows.size())+1,int(supportReactions.headers.size()),tableFormat);

    for (int i=0;i<supportReactions.headers.size();i++)
    {
        if (i > 0)
        {
            cursor.setBlockFormat(rightAlignment);
        }
        this->insertBold(cursor,supportReactions.headers.at(i),true);
        cursor.movePosition(QTextCursor::NextCell);
    }

    for (const ResultsStatistics::ForceRow &row : supportReactions.rows)
    {
        if (row.total)
        {
            this->insertBold(cursor,row.label,true);
        }
        else
        {
            this->insertText(cursor,row.label,true);
        }
        for (double value : {row.force[0],row.force[1],row.force[2],row.force.length()})
        {
            cursor.movePosition(QTextCursor::NextCell);
            cursor.setBlockFormat(rightAlignment);
            this->insertText(cursor,ResultsStatistics::realToString(value),true);
        }
        cursor.movePosition(QTextCursor::NextCell);
    }
    cursor.movePosition(QTextCursor::NextBlock);

    // What the imbalance means and how large it is.
    QTextBlockFormat noteFormat;
    noteFormat.setAlignment(Qt::AlignLeft);
    noteFormat.setLeftMargin(30.0);
    QTextCharFormat italicFormat = DocumentGenerator::getTextFormat();
    italicFormat.setFontPointSize(italicFormat.fontPointSize() * this->fontSizeScale);
    italicFormat.setFontItalic(true);
    for (const QString &note : supportReactions.notes)
    {
        cursor.insertBlock(noteFormat,QTextCharFormat());
        cursor.insertText(note,italicFormat);
    }
}

QImage ReportGenerator::takeModelScreenshot(QString &screenShotFile) const
{
    R_LOG_TRACE;

    const Model &rModel = Application::instance()->getSession()->getModel(this->modelID);
    screenShotFile = rModel.buildScreenShotFileName();

    Application::instance()->getSession()->setTakeScreenShot(this->modelID,screenShotFile,true);

    return QImage(screenShotFile);
}
