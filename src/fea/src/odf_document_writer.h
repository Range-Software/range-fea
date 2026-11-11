#ifndef ODF_DOCUMENT_WRITER_H
#define ODF_DOCUMENT_WRITER_H

#include <QImage>
#include <QMap>
#include <QPageLayout>
#include <QString>
#include <QStringList>

class QTextDocument;

//! Writes an OpenDocument text which looks like the printed document: page
//! size and margins, a title page, and a header and footer on every page.
//!
//! Qt's own ODF writer exports only the body of a QTextDocument, without page
//! setup, header or footer, and with its lengths taken as 96 dpi pixels. Its
//! output is used for the content and completed here.
class OdfDocumentWriter
{

    protected:

        //! Body of the document.
        QTextDocument *body;
        //! Title page, if any.
        QTextDocument *title;
        //! Distance of the title from the top of the body area in layout pixels.
        double titleOffset;
        //! Resolution the documents were laid out at.
        int layoutResolution;
        //! Page size and margins.
        QPageLayout pageLayout;
        //! Header logo.
        QImage headerLogo;
        //! Header logo size in layout pixels.
        double headerLogoSize;
        //! Header text lines.
        QStringList headerLines;
        //! Footer text: bold part and the rest.
        QString footerBoldText;
        QString footerText;
        //! Label put in front of the page number.
        QString pageLabel;
        //! Font size of the header and footer text in points.
        double textPointSize;
        //! Font family the text is drawn with.
        QString fontFamily;

    public:

        //! Constructor.
        OdfDocumentWriter(QTextDocument *body, int layoutResolution, const QPageLayout &pageLayout);

        //! Set title page, placed offset layout pixels below the top of the
        //! body area.
        void setTitle(QTextDocument *title, double offset);

        //! Set header shown on every page.
        void setHeader(const QImage &logo, double logoSize, const QStringList &lines);

        //! Set footer shown on every page but the title page.
        void setFooter(const QString &boldText, const QString &text, const QString &pageLabel);

        //! Set font size of the header and footer text.
        void setTextPointSize(double pointSize);

        //! Write the document. Throws RError on failure.
        void write(const QString &fileName) const;

    protected:

        //! Return document written by Qt's ODF writer, as the files of its package.
        static QMap<QString,QByteArray> writeQtOdf(QTextDocument *document);

        //! Return length in layout pixels as points.
        double toPoints(double pixels) const;

        //! Return lengths given by Qt as 96 dpi pixels converted to the layout
        //! resolution. Font sizes are in points already and stay.
        QString fixLengths(const QString &xml) const;

        //! Give each table of the document its width, column widths and
        //! alignment, measured from the laid out document.
        QString fixTables(const QString &content, QTextDocument *document, QString &newStyles) const;

        //! Replace font families by the font the text is drawn with, so that
        //! the document looks like the printed one in any application.
        QString fixFonts(const QString &xml) const;

        //! Return family of the font the text of the laid out document is
        //! really drawn with. A generic family like "Sans" is substituted, and
        //! what QFontInfo reports may differ from what the layout picked.
        static QString findFontFamily(QTextDocument *document);

        //! Return styles.xml with page layout, header and footer.
        QString buildStyles() const;

        //! Return part of xml between the end of the opening tag and the closing tag.
        static QString extractElement(const QString &xml, const QString &tag);

        //! Return xml with the automatic style names prefixed, so that styles of
        //! two documents do not clash.
        static QString prefixStyleNames(const QString &xml, const QString &prefix);

        //! Escape text for xml.
        static QString escape(const QString &text);

};

#endif // ODF_DOCUMENT_WRITER_H
