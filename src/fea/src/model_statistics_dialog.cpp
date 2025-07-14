#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include "model_statistics_dialog.h"
#include "application.h"

ModelStatisticsDialog::ModelStatisticsDialog(uint modelID, QWidget *parent)
    : QDialog(parent)
{
    QIcon closeIcon(":/icons/action/pixmaps/range-close.svg");

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

    content += ModelStatisticsDialog::generateTableRow(tr("Minimum"),QLocale().toString(nodeDistanceStatistics.getMin()));
    content += ModelStatisticsDialog::generateTableRow(tr("Maximum"),QLocale().toString(nodeDistanceStatistics.getMax()));
    content += ModelStatisticsDialog::generateTableRow(tr("Average"),QLocale().toString(nodeDistanceStatistics.getAvg()));
    content += ModelStatisticsDialog::generateTableRow(tr("Median"),QLocale().toString(nodeDistanceStatistics.getMed()));
    content += ModelStatisticsDialog::generateTableRow(tr("Percentile") +" 5",QLocale().toString(nodeDistanceStatistics.getP05()));
    content += ModelStatisticsDialog::generateTableRow(tr("Percentile") +" 95",QLocale().toString(nodeDistanceStatistics.getP95()));

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

    content += ModelStatisticsDialog::generateTableRow(tr("Line length"),QLocale().toString(lineLength));
    content += ModelStatisticsDialog::generateTableRow(tr("Surface area"),QLocale().toString(surfaceArea));
    content += ModelStatisticsDialog::generateTableRow(tr("Total volume"),QLocale().toString(volume));

    double xc(0.0), yc(0.0), zc(0.0);
    rModel.findNodeCenter(xc,yc,zc);
    content += ModelStatisticsDialog::generateTableRow(tr("Model center"),"["+ QLocale().toString(xc,'e') + " " + QLocale().toString(yc,'e') + " " + QLocale().toString(zc,'e') +"]");

    double xr[2], yr[2], zr[2];
    rModel.findNodeLimits(xr[0],xr[1],yr[0],yr[1],zr[0],zr[1]);
    content += ModelStatisticsDialog::generateTableRow(tr("Model limits"),"["+ QLocale().toString(xr[0],'e') + " " + QLocale().toString(yr[0],'e') + " " + QLocale().toString(zr[0],'e') +"] : ["+ QLocale().toString(xr[1],'e') + " " + QLocale().toString(yr[1],'e') + " " + QLocale().toString(zr[1],'e') +"]");
    content += ModelStatisticsDialog::generateTableRow(tr("Model size"),"["+ QLocale().toString(xr[1]-xr[0],'e') + " " + QLocale().toString(yr[1]-yr[0],'e') + " " + QLocale().toString(zr[1]-zr[0],'e') +"]");

    content += "</table>";

    textBrowser->insertHtml(content);
}

QString ModelStatisticsDialog::generateTableRow(const QString &label, const QString &value)
{
    return QString("<tr><th align=\"left\" nowrap>" + label + "</th><td align=\"right\"><pre>" + value + "</pre></td></tr>");
}
