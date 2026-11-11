#ifndef REPORT_DIALOG_H
#define REPORT_DIALOG_H

#include <QDialog>
#include <QGroupBox>

#include <rgl_file_chooser_button.h>

class ReportDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID for which report will be generated.
        uint modelID;
        //! HTML file enabled.
        static bool htmlFileEnabled;
        //! HTML group box.
        QGroupBox *htmlGroupBox;
        //! HTML file chooser.
        RFileChooserButton *htmlFileChooserButton;
        //! Markdown file enabled.
        static bool markdownFileEnabled;
        //! Markdown group box.
        QGroupBox *markdownGroupBox;
        //! Markdown file chooser.
        RFileChooserButton *markdownFileChooserButton;
        //! ODT file enabled.
        static bool odtFileEnabled;
        //! ODT group box.
        QGroupBox *odtGroupBox;
        //! ODT file chooser.
        RFileChooserButton *odtFileChooserButton;
        //! PDF file enabled.
        static bool pdfFileEnabled;
        //! PDF group box.
        QGroupBox *pdfGroupBox;
        //! PDF file chooser.
        RFileChooserButton *pdfFileChooserButton;

    public:

        //! Constructor.
        explicit ReportDialog(uint modelID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();
        
};

#endif // REPORT_DIALOG_H
