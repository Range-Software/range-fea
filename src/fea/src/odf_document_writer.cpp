#include <algorithm>

#include <QAbstractTextDocumentLayout>
#include <QBuffer>
#include <QFontInfo>
#include <QGlyphRun>
#include <QRawFont>
#include <QTextLayout>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextFrame>
#include <QTextTable>
#include <QtCore/private/qzipreader_p.h>
#include <QtCore/private/qzipwriter_p.h>

#include <rbl_error.h>

#include "odf_document_writer.h"

namespace
{

//! Format a length in points.
QString points(double value)
{
    return QString::number(value,'f',2) + "pt";
}

//! Collect tables of a frame and its sub-frames in document order - the order
//! Qt's ODF writer writes them in.
void collectTables(QTextFrame *frame, QList<QTextTable*> &tables)
{
    for (QTextFrame::iterator it = frame->begin(); !it.atEnd(); ++it)
    {
        QTextFrame *childFrame = it.currentFrame();
        if (!childFrame)
        {
            continue;
        }
        if (QTextTable *table = qobject_cast<QTextTable*>(childFrame))
        {
            tables.append(table);
        }
        collectTables(childFrame,tables);
    }
}

}

OdfDocumentWriter::OdfDocumentWriter(QTextDocument *body, int layoutResolution, const QPageLayout &pageLayout)
    : body(body)
    , title(nullptr)
    , titleOffset(0.0)
    , layoutResolution(layoutResolution > 0 ? layoutResolution : 96)
    , pageLayout(pageLayout)
    , headerLogoSize(0.0)
    , textPointSize(10.0)
    , fontFamily(OdfDocumentWriter::findFontFamily(body))
{
}

QString OdfDocumentWriter::findFontFamily(QTextDocument *document)
{
    // Lay the document out, if it is not yet.
    document->documentLayout()->documentSize();
    for (QTextBlock block = document->begin(); block.isValid(); block = block.next())
    {
        if (!block.layout())
        {
            continue;
        }
        for (const QGlyphRun &glyphRun : block.layout()->glyphRuns())
        {
            const QString family(glyphRun.rawFont().familyName());
            if (!family.isEmpty())
            {
                return family;
            }
        }
    }
    return QFontInfo(document->defaultFont()).family();
}

void OdfDocumentWriter::setTitle(QTextDocument *title, double offset)
{
    this->title = title;
    this->titleOffset = std::max(offset,0.0);
}

void OdfDocumentWriter::setHeader(const QImage &logo, double logoSize, const QStringList &lines)
{
    this->headerLogo = logo;
    this->headerLogoSize = logoSize;
    this->headerLines = lines;
}

void OdfDocumentWriter::setFooter(const QString &boldText, const QString &text, const QString &pageLabel)
{
    this->footerBoldText = boldText;
    this->footerText = text;
    this->pageLabel = pageLabel;
}

void OdfDocumentWriter::setTextPointSize(double pointSize)
{
    this->textPointSize = pointSize;
}

void OdfDocumentWriter::write(const QString &fileName) const
{
    QMap<QString,QByteArray> package = OdfDocumentWriter::writeQtOdf(this->body);
    if (!package.contains("content.xml"))
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write the document content.");
    }

    QString content = QString::fromUtf8(package.value("content.xml"));
    QString automaticStyles = this->fixFonts(this->fixLengths(OdfDocumentWriter::extractElement(content,"office:automatic-styles")));
    // Lengths are also in the body, as the size of images.
    QString text = this->fixLengths(OdfDocumentWriter::extractElement(content,"office:text"));

    QString tableStyles;
    text = this->fixTables(text,this->body,tableStyles);
    automaticStyles += tableStyles;

    // Page styles: the title page has a header only, the following pages a
    // header and a footer with page numbers counted from the first of them.
    automaticStyles += "<style:style style:name=\"RTitleStart\" style:family=\"paragraph\" style:master-page-name=\"RTitlePage\">"
                       "<style:paragraph-properties fo:margin-top=\"" + points(this->toPoints(this->titleOffset)) + "\"/>"
                       "</style:style>";
    automaticStyles += "<style:style style:name=\"RBodyStart\" style:family=\"paragraph\" style:master-page-name=\"RBodyPage\">"
                       "<style:paragraph-properties style:page-number=\"1\"/>"
                       "</style:style>";

    QString titleText;
    if (this->title)
    {
        QMap<QString,QByteArray> titlePackage = OdfDocumentWriter::writeQtOdf(this->title);
        QString titleContent = OdfDocumentWriter::prefixStyleNames(QString::fromUtf8(titlePackage.value("content.xml")),"RTitle_");
        automaticStyles += this->fixFonts(this->fixLengths(OdfDocumentWriter::extractElement(titleContent,"office:automatic-styles")));
        titleText = "<text:p text:style-name=\"RTitleStart\"/>" + OdfDocumentWriter::extractElement(titleContent,"office:text");
    }

    // A paragraph with another master page starts a new page.
    text = titleText + "<text:p text:style-name=\"RBodyStart\"/>" + text;

    QString header = content.left(content.indexOf("<office:automatic-styles>"));
    content = header
            + "<office:automatic-styles>" + automaticStyles + "</office:automatic-styles>"
            + "<office:body><office:text>" + text + "</office:text></office:body>"
            + "</office:document-content>";

    package.insert("content.xml",content.toUtf8());
    package.insert("styles.xml",this->buildStyles().toUtf8());

    QByteArray logoData;
    if (!this->headerLogo.isNull())
    {
        QBuffer buffer(&logoData);
        buffer.open(QIODevice::WriteOnly);
        this->headerLogo.save(&buffer,"PNG");
        package.insert("Pictures/RLogo.png",logoData);
    }

    // Manifest of everything in the package.
    QString manifest = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">"
                       "<manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"/\"/>";
    for (auto it = package.constBegin(); it != package.constEnd(); ++it)
    {
        if (it.key() == "mimetype" || it.key().startsWith("META-INF/"))
        {
            continue;
        }
        QString mediaType = it.key().endsWith(".xml") ? "text/xml" : "image/png";
        manifest += "<manifest:file-entry manifest:media-type=\"" + mediaType + "\" manifest:full-path=\"" + escape(it.key()) + "\"/>";
    }
    manifest += "</manifest:manifest>";
    package.insert("META-INF/manifest.xml",manifest.toUtf8());

    // The mimetype has to come first and uncompressed.
    QZipWriter zipWriter(fileName);
    if (zipWriter.status() != QZipWriter::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to open file \'%s\'.",fileName.toUtf8().constData());
    }
    zipWriter.setCompressionPolicy(QZipWriter::NeverCompress);
    zipWriter.addFile("mimetype",QByteArray("application/vnd.oasis.opendocument.text"));
    zipWriter.setCompressionPolicy(QZipWriter::AutoCompress);
    for (auto it = package.constBegin(); it != package.constEnd(); ++it)
    {
        if (it.key() != "mimetype")
        {
            zipWriter.addFile(it.key(),it.value());
        }
    }
    zipWriter.close();
    if (zipWriter.status() != QZipWriter::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write to file \'%s\'.",fileName.toUtf8().constData());
    }
}

QMap<QString,QByteArray> OdfDocumentWriter::writeQtOdf(QTextDocument *document)
{
    QByteArray data;
    {
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        QTextDocumentWriter writer(&buffer,"ODF");
        if (!writer.write(document))
        {
            throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write the document.");
        }
    }

    QMap<QString,QByteArray> package;
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QZipReader zipReader(&buffer);
    for (const QZipReader::FileInfo &fileInfo : zipReader.fileInfoList())
    {
        if (fileInfo.isFile)
        {
            package.insert(fileInfo.filePath,zipReader.fileData(fileInfo.filePath));
        }
    }
    return package;
}

double OdfDocumentWriter::toPoints(double pixels) const
{
    return pixels * 72.0 / double(this->layoutResolution);
}

QString OdfDocumentWriter::fixLengths(const QString &xml) const
{
    // Qt converts pixels to points as if they were 96 dpi pixels.
    const double scale = 96.0 / double(this->layoutResolution);
    static const QRegularExpression attributeExpression("([a-z]+:[a-z-]+)=\"([^\"]*)\"");
    static const QRegularExpression lengthExpression("(-?[0-9]+(?:\\.[0-9]+)?)pt");

    QString result;
    qsizetype last = 0;
    QRegularExpressionMatchIterator it = attributeExpression.globalMatch(xml);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        if (match.captured(1) == "fo:font-size" || !match.captured(2).contains("pt"))
        {
            continue;
        }
        QString value = match.captured(2);
        QString scaled;
        qsizetype valueLast = 0;
        QRegularExpressionMatchIterator lengthIt = lengthExpression.globalMatch(value);
        while (lengthIt.hasNext())
        {
            QRegularExpressionMatch lengthMatch = lengthIt.next();
            scaled += value.mid(valueLast,lengthMatch.capturedStart() - valueLast);
            scaled += points(lengthMatch.captured(1).toDouble() * scale);
            valueLast = lengthMatch.capturedEnd();
        }
        scaled += value.mid(valueLast);

        result += xml.mid(last,match.capturedStart() - last);
        result += match.captured(1) + "=\"" + scaled + "\"";
        last = match.capturedEnd();
    }
    result += xml.mid(last);
    return result;
}

QString OdfDocumentWriter::fixTables(const QString &content, QTextDocument *document, QString &newStyles) const
{
    QList<QTextTable*> tables;
    collectTables(document->rootFrame(),tables);

    static const QRegularExpression tableExpression("<table:table table:style-name=\"([^\"]+)\">\\s*(<table:table-column[^>]*/>)");
    QList<QRegularExpressionMatch> matches;
    QRegularExpressionMatchIterator it = tableExpression.globalMatch(content);
    while (it.hasNext())
    {
        matches.append(it.next());
    }
    if (matches.size() != tables.size())
    {
        // Not the structure expected - better keep what Qt wrote.
        return content;
    }

    QAbstractTextDocumentLayout *layout = document->documentLayout();

    QString result;
    qsizetype last = 0;
    for (qsizetype i=0;i<matches.size();i++)
    {
        const QRegularExpressionMatch &match = matches.at(i);
        QTextTable *table = tables.at(i);
        const QTextTableFormat format = table->format();
        const double cellExtra = 2.0 * format.cellPadding() + format.border() + format.cellSpacing();

        // A cell block is laid out over the width of its column, padding excluded.
        std::vector<double> columnWidths(table->columns(),0.0);
        for (int r=0;r<table->rows();r++)
        {
            for (int c=0;c<table->columns();c++)
            {
                QTextTableCell cell = table->cellAt(r,c);
                if (!cell.isValid() || cell.columnSpan() != 1)
                {
                    continue;
                }
                QRectF rect = layout->blockBoundingRect(cell.firstCursorPosition().block());
                columnWidths[c] = std::max(columnWidths[c],rect.width() + cellExtra);
            }
        }
        // Other applications measure text a little differently, and with the
        // exact width a cell text would wrap.
        double tableWidth = 0.0;
        for (double &width : columnWidths)
        {
            width = width * 1.05 + double(this->layoutResolution) * 3.0 / 72.0;
            tableWidth += width;
        }

        QString align("left");
        if (format.alignment() & Qt::AlignHCenter)
        {
            align = "center";
        }
        else if (format.alignment() & Qt::AlignRight)
        {
            align = "right";
        }

        const QString styleName = QString("RTable%1").arg(i+1);
        newStyles += "<style:style style:name=\"" + styleName + "\" style:family=\"table\">"
                     "<style:table-properties style:width=\"" + points(this->toPoints(tableWidth)) + "\" table:align=\"" + align + "\""
                     + (align == "left" ? " fo:margin-left=\"" + points(this->toPoints(format.leftMargin())) + "\"" : QString())
                     + " table:border-model=\"collapsing\"/>"
                     "</style:style>";
        QString columns;
        for (int c=0;c<table->columns();c++)
        {
            const QString columnStyleName = QString("%1.C%2").arg(styleName).arg(c+1);
            newStyles += "<style:style style:name=\"" + columnStyleName + "\" style:family=\"table-column\">"
                         "<style:table-column-properties style:column-width=\"" + points(this->toPoints(columnWidths[c])) + "\"/>"
                         "</style:style>";
            columns += "<table:table-column table:style-name=\"" + columnStyleName + "\"/>";
        }

        result += content.mid(last,match.capturedStart() - last);
        result += "<table:table table:style-name=\"" + styleName + "\">" + columns;
        last = match.capturedEnd();
    }
    result += content.mid(last);
    return result;
}

QString OdfDocumentWriter::fixFonts(const QString &xml) const
{
    static const QRegularExpression fontExpression("fo:font-family=\"([^\"]*)\"");
    QString result;
    qsizetype last = 0;
    QRegularExpressionMatchIterator it = fontExpression.globalMatch(xml);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        result += xml.mid(last,match.capturedStart() - last);
        result += "fo:font-family=\"" + escape(this->fontFamily) + "\"";
        last = match.capturedEnd();
    }
    result += xml.mid(last);
    return result;
}

QString OdfDocumentWriter::buildStyles() const
{
    const QSizeF pageSize = this->pageLayout.pageSize().size(QPageSize::Millimeter);
    const QMarginsF margins = this->pageLayout.margins(QPageLayout::Millimeter);
    const QString fontFamily = escape(this->fontFamily);
    const QString textSize = points(this->textPointSize);
    const double logoPoints = this->toPoints(this->headerLogoSize);
    const double textWidth = (pageSize.width() - margins.left() - margins.right()) * 72.0 / 25.4;
    auto mm = [](double value) { return QString::number(value,'f',2) + "mm"; };

    QString xml;
    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<office:document-styles"
           " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
           " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
           " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
           " xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
           " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\""
           " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
           " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
           " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
           " office:version=\"1.2\">";

    // Paragraphs are spaced only by what the document itself sets, as when it
    // is printed.
    xml += "<office:styles>"
           "<style:default-style style:family=\"paragraph\">"
           "<style:paragraph-properties fo:margin-top=\"0pt\" fo:margin-bottom=\"0pt\" fo:line-height=\"100%\"/>"
           "<style:text-properties style:font-name=\"" + fontFamily + "\" fo:font-family=\"" + fontFamily + "\" fo:font-size=\"" + textSize + "\"/>"
           "</style:default-style>"
           "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
           "</office:styles>";

    xml += "<office:automatic-styles>"
           "<style:page-layout style:name=\"RPageLayout\">"
           "<style:page-layout-properties fo:page-width=\"" + mm(pageSize.width()) + "\" fo:page-height=\"" + mm(pageSize.height()) + "\""
           " fo:margin-top=\"" + mm(margins.top()) + "\" fo:margin-bottom=\"" + mm(margins.bottom()) + "\""
           " fo:margin-left=\"" + mm(margins.left()) + "\" fo:margin-right=\"" + mm(margins.right()) + "\""
           " style:print-orientation=\"portrait\"/>"
           "<style:header-style><style:header-footer-properties fo:min-height=\"0pt\" fo:margin-bottom=\"12pt\" fo:padding-bottom=\"4pt\" fo:border-bottom=\"0.5pt solid #000000\"/></style:header-style>"
           "<style:footer-style><style:header-footer-properties fo:min-height=\"0pt\" fo:margin-top=\"12pt\"/></style:footer-style>"
           "</style:page-layout>"
           "<style:style style:name=\"RHeaderText\" style:family=\"paragraph\">"
           "<style:text-properties fo:font-size=\"" + textSize + "\"/>"
           "</style:style>"
           "<style:style style:name=\"RFooterText\" style:family=\"paragraph\">"
           "<style:paragraph-properties fo:text-align=\"center\"/>"
           "<style:text-properties fo:font-size=\"" + textSize + "\"/>"
           "</style:style>"
           "<style:style style:name=\"RBold\" style:family=\"text\">"
           "<style:text-properties fo:font-weight=\"bold\"/>"
           "</style:style>"
           "<style:style style:name=\"RHeaderTable\" style:family=\"table\">"
           "<style:table-properties style:width=\"" + points(textWidth) + "\" table:align=\"left\"/>"
           "</style:style>"
           "<style:style style:name=\"RHeaderTable.A\" style:family=\"table-column\">"
           "<style:table-column-properties style:column-width=\"" + points(logoPoints + 12.0) + "\"/>"
           "</style:style>"
           "<style:style style:name=\"RHeaderTable.B\" style:family=\"table-column\">"
           "<style:table-column-properties style:column-width=\"" + points(textWidth - logoPoints - 12.0) + "\"/>"
           "</style:style>"
           "<style:style style:name=\"RHeaderTable.Cell\" style:family=\"table-cell\">"
           "<style:table-cell-properties fo:padding=\"2pt\" style:vertical-align=\"middle\"/>"
           "</style:style>"
           "</office:automatic-styles>";

    // Header - the logo next to the text lines, as on the printed page.
    QString headerContent;
    headerContent += "<table:table table:name=\"RHeader\" table:style-name=\"RHeaderTable\">"
                     "<table:table-column table:style-name=\"RHeaderTable.A\"/>"
                     "<table:table-column table:style-name=\"RHeaderTable.B\"/>"
                     "<table:table-row>"
                     "<table:table-cell table:style-name=\"RHeaderTable.Cell\"><text:p text:style-name=\"RHeaderText\">";
    if (!this->headerLogo.isNull())
    {
        headerContent += "<draw:frame draw:name=\"RLogo\" text:anchor-type=\"as-char\" svg:width=\"" + points(logoPoints) + "\" svg:height=\"" + points(logoPoints) + "\">"
                         "<draw:image xlink:href=\"Pictures/RLogo.png\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>"
                         "</draw:frame>";
    }
    headerContent += "</text:p></table:table-cell>"
                     "<table:table-cell table:style-name=\"RHeaderTable.Cell\">";
    for (const QString &line : this->headerLines)
    {
        headerContent += "<text:p text:style-name=\"RHeaderText\">" + escape(line) + "</text:p>";
    }
    headerContent += "</table:table-cell></table:table-row></table:table>";

    QString footerContent;
    footerContent += "<text:p text:style-name=\"RFooterText\"><text:span text:style-name=\"RBold\">" + escape(this->footerBoldText) + "</text:span>" + escape(this->footerText) + "</text:p>"
                     "<text:p text:style-name=\"RFooterText\">" + escape(this->pageLabel) + " <text:page-number text:select-page=\"current\">1</text:page-number></text:p>";

    xml += "<office:master-styles>"
           "<style:master-page style:name=\"RTitlePage\" style:page-layout-name=\"RPageLayout\" style:next-style-name=\"RBodyPage\">"
           "<style:header>" + headerContent + "</style:header>"
           "</style:master-page>"
           "<style:master-page style:name=\"RBodyPage\" style:page-layout-name=\"RPageLayout\">"
           "<style:header>" + headerContent + "</style:header>"
           "<style:footer>" + footerContent + "</style:footer>"
           "</style:master-page>"
           "<style:master-page style:name=\"Standard\" style:page-layout-name=\"RPageLayout\">"
           "<style:header>" + headerContent + "</style:header>"
           "<style:footer>" + footerContent + "</style:footer>"
           "</style:master-page>"
           "</office:master-styles>";

    xml += "</office:document-styles>";
    return xml;
}

QString OdfDocumentWriter::extractElement(const QString &xml, const QString &tag)
{
    qsizetype start = xml.indexOf("<" + tag);
    if (start < 0)
    {
        return QString();
    }
    start = xml.indexOf('>',start);
    if (start < 0 || xml.at(start-1) == '/')
    {
        return QString();
    }
    qsizetype end = xml.indexOf("</" + tag + ">",start);
    if (end < 0)
    {
        return QString();
    }
    return xml.mid(start + 1,end - start - 1);
}

QString OdfDocumentWriter::prefixStyleNames(const QString &xml, const QString &prefix)
{
    static const QRegularExpression nameExpression("((?:style:name|text:style-name|table:style-name|draw:style-name)=\")([^\"]+)\"");
    QString result;
    qsizetype last = 0;
    QRegularExpressionMatchIterator it = nameExpression.globalMatch(xml);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        result += xml.mid(last,match.capturedStart() - last);
        result += match.captured(1) + prefix + match.captured(2) + "\"";
        last = match.capturedEnd();
    }
    result += xml.mid(last);
    return result;
}

QString OdfDocumentWriter::escape(const QString &text)
{
    return text.toHtmlEscaped();
}
