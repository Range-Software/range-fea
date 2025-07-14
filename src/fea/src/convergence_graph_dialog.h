#ifndef CONVERGENCE_GRAPH_DIALOG_H
#define CONVERGENCE_GRAPH_DIALOG_H

#include <QDialog>

class ConvergenceGraphDialog : public QDialog
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ConvergenceGraphDialog(const QStringList &convergenceFileNames, QWidget *parent = nullptr);

    private:

        //! Find problem ID or matrix solver ID in convergence log filename and convert it to name.
        static QString getNameFromID(const QString &fileName, const QString &fallbackName);

};

#endif // CONVERGENCE_GRAPH_DIALOG_H
