#ifndef DOCUMENT_GENERATOR_H
#define DOCUMENT_GENERATOR_H

#include <QMap>
#include <QStringList>
#include <QTextDocument>
#include <QPrinter>
#include <QTextCharFormat>

#include <rbl_job.h>

class DocumentGenerator : public RJob
{
    Q_OBJECT

    public:

        enum Type
        {
            PlainText = 1 << 0,
            HTML      = 1 << 1,
            Markdown  = 1 << 2,
            ODT       = 1 << 3,
            PDF       = 1 << 4
        };

    protected:

        struct HeaderCounters
        {
            uint nH1;
            uint nH2;
            uint nH3;
            uint nH4;
            uint nH5;

            HeaderCounters() : nH1(0), nH2(0), nH3(0), nH4(0), nH5(0) { }
        };

        //! Header counters.
        HeaderCounters headerCounters;
        //! Enable header counters.
        bool enableHeaderCounters;
        //! Font size scale.
        double fontSizeScale;

        //! Export types.
        QMap<Type,QString> exportTypes;
        //! Document title.
        QTextDocument *docTitle;
        //! Document header.
        QTextDocument *docHeader;
        //! Document footer.
        QTextDocument *docFooter;
        //! Document body.
        QTextDocument *docBody;
        //! Printer.
        QPrinter *printer;
        //! Printer resolution the documents were laid out at. Painting to a
        //! device of another resolution has to be scaled by the ratio.
        int layoutResolution;

        //! Header logo.
        static const QString logoFileName;
        //! Size of the header logo in layout pixels.
        static const double logoSize;

    public:

        //! Constructor.
        explicit DocumentGenerator();

        //! Set enable header counters.
        void setEnableHeaderCounters(bool enableHeaderCounters);

        //! Set font size scale.
        void setFontSizeScale(double fontSizeScale);

        //! Add export type. The file name gets the extension of the format
        //! if it has none of its extensions.
        void addExportType(DocumentGenerator::Type type, const QString &fileName);

        //! Return file extensions of the format, the default one first.
        static QStringList getFileExtensions(DocumentGenerator::Type type);

        //! Return file name with an extension of the format: kept if it has
        //! one, .odf replaced by .odt, otherwise the default one appended.
        static QString ensureFileExtension(DocumentGenerator::Type type, const QString &fileName);

    protected:

        //! Overloaded run method.
        virtual int perform() override final;

        //! Generate document.
        void generate();

        //! Generate header.
        void generateHeader();

        //! Generate footer.
        void generateFooter();

        //! Generate title.
        virtual void generateTitle() = 0;

        //! Generate body.
        virtual void generateBody() = 0;

        //! Generate page number document.
        QTextDocument *generatePageNumber();

        //! Export to plain text file.
        void exportToPlainText(const QString &fileName) const;

        //! Export to HTML file.
        void exportToHTML(const QString &fileName) const;

        //! Export to Markdown file.
        void exportToMarkdown(const QString &fileName) const;

        //! Save the images of the body next to the exported file, where the
        //! file refers to them.
        void exportImages(const QString &fileName) const;

        //! Export to ODT file.
        void exportToODT(const QString &fileName) const;

        //! Export to PDF file.
        void exportToPDF(const QString &fileName) const;

        //! Insert H1 header at given cursor.
        void insertH1(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert H2 header at given cursor.
        void insertH2(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert H3 header at given cursor.
        void insertH3(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert H4 header at given cursor.
        void insertH4(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert H5 header at given cursor.
        void insertH5(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert text at given cursor.
        void insertText(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        //! Insert bold text at given cursor.
        void insertBold(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat = false);

        // Return default formats.

        static QTextTableFormat getTableFormat();
        static QTextBlockFormat getBlockFormat();
        static QTextCharFormat getH1Format();
        static QTextCharFormat getH2Format();
        static QTextCharFormat getH3Format();
        static QTextCharFormat getH4Format();
        static QTextCharFormat getH5Format();
        static QTextCharFormat getTextFormat();
        static QTextCharFormat getBoldFormat();

};

#endif // DOCUMENT_GENERATOR_H
