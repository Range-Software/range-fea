#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include "model_statistics_dialog.h"
#include "application.h"
#include "model.h"
#include "results_statistics.h"
#include "session.h"
#include <rgl_icon.h>
#include <rbl_statistics.h>

ModelStatisticsDialog::ModelStatisticsDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
{
    QIcon closeIcon = RIcon::fromFile(":/icons/action/pixmaps/range-close.svg");

    this->setWindowTitle(tr("Model statistics"));
    this->resize(860,600);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QTextBrowser *textBrowser = new QTextBrowser;
    this->produceStatistics(modelID,textBrowser);
    textBrowser->setLineWrapMode(QTextEdit::NoWrap);
    QTextCursor textCursor = textBrowser->textCursor();
    textCursor.setPosition(0);
    textBrowser->setTextCursor(textCursor);
    mainLayout->addWidget(textBrowser);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *closeButton = new QPushButton(closeIcon, tr("Close"));
    buttonBox->addButton(closeButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);
}

int ModelStatisticsDialog::exec()
{
    int retVal = QDialog::exec();
    return retVal;
}

void ModelStatisticsDialog::produceStatistics(uint modelID, QTextBrowser *textBrowser)
{
    const Model &rModel = Application::instance()->getSession()->getModel(modelID);

    QString content;

    content += "<h1>" + rModel.getName() + "</h1>";
    content += "<table  border=\"0\">";
    content += ModelStatisticsDialog::generateTableRow(tr("Number of nodes"),QLocale().toString(rModel.getNNodes()));
    content += ModelStatisticsDialog::generateTableRow(tr("Number of elements"),QLocale().toString(rModel.getNElements()));

    RStatistics nodeDistanceStatistics = rModel.findNodeDistanceStatistics();

    content += "<tr><th align=\"center\" colspan=\"2\">"+ tr("Node distance statistics") +"</th></tr>";

    content += ModelStatisticsDialog::generateTableRow(tr("Minimum"),ResultsStatistics::realToString(nodeDistanceStatistics.getMin()));
    content += ModelStatisticsDialog::generateTableRow(tr("Maximum"),ResultsStatistics::realToString(nodeDistanceStatistics.getMax()));
    content += ModelStatisticsDialog::generateTableRow(tr("Average"),ResultsStatistics::realToString(nodeDistanceStatistics.getAvg()));
    content += ModelStatisticsDialog::generateTableRow(tr("Median"),ResultsStatistics::realToString(nodeDistanceStatistics.getMed()));
    content += ModelStatisticsDialog::generateTableRow(tr("Percentile") +" 5",ResultsStatistics::realToString(nodeDistanceStatistics.getP05()));
    content += ModelStatisticsDialog::generateTableRow(tr("Percentile") +" 95",ResultsStatistics::realToString(nodeDistanceStatistics.getP95()));

    content += "<tr><th align=\"center\" colspan=\"2\">"+ tr("Geometry statistics") +"</th></tr>";

    double lineLength(0.0);
    for (uint i=0;i<rModel.getNLines();i++)
    {
        lineLength += rModel.getLine(i).findLength(rModel.getNodes(),rModel.getElements());
    }
    double surfaceArea(0.0);
    for (uint i=0;i<rModel.getNSurfaces();i++)
    {
        surfaceArea += rModel.getSurface(i).findArea(rModel.getNodes(),rModel.getElements());
    }
    double volume(0.0);
    for (uint i=0;i<rModel.getNVolumes();i++)
    {
        volume += rModel.getVolume(i).findVolume(rModel.getNodes(),rModel.getElements());
    }

    content += ModelStatisticsDialog::generateTableRow(tr("Line length"),ResultsStatistics::realToString(lineLength));
    content += ModelStatisticsDialog::generateTableRow(tr("Surface area"),ResultsStatistics::realToString(surfaceArea));
    content += ModelStatisticsDialog::generateTableRow(tr("Total volume"),ResultsStatistics::realToString(volume));

    double xc(0.0), yc(0.0), zc(0.0);
    rModel.findNodeCenter(xc,yc,zc);
    content += ModelStatisticsDialog::generateTableRow(tr("Model center"),"["+ ResultsStatistics::realToString(xc) + " " + ResultsStatistics::realToString(yc) + " " + ResultsStatistics::realToString(zc) +"]");

    double xr[2], yr[2], zr[2];
    rModel.findNodeLimits(xr[0],xr[1],yr[0],yr[1],zr[0],zr[1]);
    content += ModelStatisticsDialog::generateTableRow(tr("Model limits"),"["+ ResultsStatistics::realToString(xr[0]) + " " + ResultsStatistics::realToString(yr[0]) + " " + ResultsStatistics::realToString(zr[0]) +"] : ["+ ResultsStatistics::realToString(xr[1]) + " " + ResultsStatistics::realToString(yr[1]) + " " + ResultsStatistics::realToString(zr[1]) +"]");
    content += ModelStatisticsDialog::generateTableRow(tr("Model size"),"["+ ResultsStatistics::realToString(xr[1]-xr[0]) + " " + ResultsStatistics::realToString(yr[1]-yr[0]) + " " + ResultsStatistics::realToString(zr[1]-zr[0]) +"]");

    content += "</table>";

    content += ModelStatisticsDialog::produceResults(rModel);

    textBrowser->insertHtml(content);
}

QString ModelStatisticsDialog::produceResults(const Model &rModel)
{
    QString content;

    content += "<h2>" + tr("Results") + "</h2>";

    ResultsStatistics resultsStatistics(rModel);

    if (!resultsStatistics.hasResults())
    {
        content += "<p>" + tr("No results are available.") + "</p>";
        return content;
    }

    // State of the solution the results belong to.
    content += "<table border=\"0\">";
    for (const ResultsStatistics::Row &row : resultsStatistics.getSolutionState())
    {
        content += ModelStatisticsDialog::generateTableRow(row.label.toHtmlEscaped(),row.value.toHtmlEscaped());
    }
    content += "</table>";

    for (const ResultsStatistics::Section &section : resultsStatistics.getSections())
    {
        content += "<h3>" + section.title.toHtmlEscaped() + "</h3>";
        if (!section.variables.empty())
        {
            content += "<table border=\"0\">";
            for (const ResultsStatistics::Variable &variable : section.variables)
            {
                content += "<tr><th align=\"left\" colspan=\"3\">" + variable.title.toHtmlEscaped() + "</th></tr>";
                for (const ResultsStatistics::Row &row : variable.rows)
                {
                    content += ModelStatisticsDialog::generateTableRow(row.label.toHtmlEscaped(),row.value.toHtmlEscaped(),row.location.toHtmlEscaped());
                }
            }
            content += "</table>";
        }
        content += ModelStatisticsDialog::produceSupportReactions(section.supportReactions);
    }

    return content;
}

QString ModelStatisticsDialog::produceSupportReactions(const ResultsStatistics::SupportReactions &supportReactions)
{
    if (supportReactions.rows.empty())
    {
        return QString();
    }

    // Same indent as the statistics rows of the results above.
    const QString indent("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
    // Values are right aligned, leading spaces keep the columns apart.
    const QString gap("&nbsp;&nbsp;&nbsp;&nbsp;");

    QString content;
    content += "<table border=\"0\">";
    content += "<tr><th align=\"left\" colspan=\"5\">" + tr("Support reactions") + "</th></tr>";
    content += "<tr>";
    for (int i=0;i<supportReactions.headers.size();i++)
    {
        if (i == 0)
        {
            content += "<td align=\"left\" nowrap>" + indent + "<b>" + supportReactions.headers.at(i).toHtmlEscaped() + "</b></td>";
        }
        else
        {
            content += "<td align=\"right\" nowrap><b>" + gap + supportReactions.headers.at(i).toHtmlEscaped() + "</b></td>";
        }
    }
    content += "</tr>";

    for (const ResultsStatistics::ForceRow &row : supportReactions.rows)
    {
        const QString label(row.label.toHtmlEscaped());
        content += "<tr><td align=\"left\" nowrap>" + indent + (row.total ? "<b>" + label + "</b>" : label) + "</td>";
        for (double value : {row.force[0],row.force[1],row.force[2],row.force.length()})
        {
            content += "<td align=\"right\" nowrap><pre>" + gap + ResultsStatistics::realToString(value) + "</pre></td>";
        }
        content += "</tr>";
    }
    content += "</table>";

    // The notes are kept out of the table, so that they do not widen its
    // columns.
    QStringList notes;
    for (const QString &note : supportReactions.notes)
    {
        notes << note.toHtmlEscaped();
    }
    content += "<p style=\"margin-left:32px\"><i>" + notes.join("<br/>") + "</i></p>";

    return content;
}

QString ModelStatisticsDialog::generateTableRow(const QString &label, const QString &value)
{
    return QString("<tr><th align=\"left\" nowrap>" + label + "</th><td align=\"right\"><pre>" + value + "</pre></td></tr>");
}

QString ModelStatisticsDialog::generateTableRow(const QString &label, const QString &value, const QString &location)
{
    // Indented and not bold, so that the rows read as belonging to the result
    // named in the bold row above them.
    return QString("<tr><td align=\"left\" nowrap>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + label + "</td><td align=\"right\"><pre>" + value + "</pre></td><td align=\"left\" nowrap>&nbsp;&nbsp;&nbsp;" + location + "</td></tr>");
}
