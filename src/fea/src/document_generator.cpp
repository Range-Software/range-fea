#include <QTextDocument>
#include <QTextCursor>
#include <QtPrintSupport/QPrinter>
#include <QTextDocumentWriter>
#include <QPainter>
#include <QPageSize>
#include <QMarginsF>
#include <QAbstractTextDocumentLayout>
#include <QPalette>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QSet>
#include <QTextBlock>

#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include "document_generator.h"
#include "odf_document_writer.h"

const QString DocumentGenerator::logoFileName(":/icons/logos/pixmaps/range-fea.svg");
const double DocumentGenerator::logoSize = 64.0;

namespace
{

//! Draw the part of the document within rect, black on white whatever the
//! application style is. QTextDocument::drawContents() takes the colors from
//! the application palette, whose text is light in a dark style and so
//! invisible on white paper.
void drawDocument(QPainter &painter, QTextDocument *document, const QRectF &rect)
{
    QPalette palette;
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Window, Qt::white);
    palette.setColor(QPalette::Link, QColor(0,0,238));
    palette.setColor(QPalette::LinkVisited, QColor(85,26,139));

    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = palette;

    painter.save();
    if (rect.isValid())
    {
        painter.setClipRect(rect);
        context.clip = rect;
    }
    document->documentLayout()->draw(&painter, context);
    painter.restore();
}

}

DocumentGenerator::DocumentGenerator()
    : enableHeaderCounters(true)
    , fontSizeScale(1.0)
    , docTitle(nullptr)
    , docHeader(nullptr)
    , docFooter(nullptr)
    , docBody(nullptr)
    , printer(nullptr)
    , layoutResolution(0)
{
    R_LOG_TRACE_IN;
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::setEnableHeaderCounters(bool enableHeaderCounters)
{
    R_LOG_TRACE_IN;
    this->enableHeaderCounters = enableHeaderCounters;
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::setFontSizeScale(double fontSizeScale)
{
    R_LOG_TRACE_IN;
    this->fontSizeScale = fontSizeScale;
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::addExportType(DocumentGenerator::Type type, const QString &fileName)
{
    R_LOG_TRACE_IN;
    const QString exportFileName(DocumentGenerator::ensureFileExtension(type,fileName));
    if (exportFileName != fileName)
    {
        RLogger::notice("File name \'%s\' was changed to \'%s\' to have the extension of its format.\n",
                        fileName.toUtf8().constData(),
                        exportFileName.toUtf8().constData());
    }
    this->exportTypes[type] = exportFileName;
    R_LOG_TRACE_OUT;
}

QStringList DocumentGenerator::getFileExtensions(DocumentGenerator::Type type)
{
    // The first one is the one given to a file without a proper extension.
    switch (type)
    {
        case PlainText: return {"txt"};
        case HTML:      return {"html","htm"};
        case Markdown:  return {"md","markdown"};
        case ODT:       return {"odt"};
        case PDF:       return {"pdf"};
    }
    return QStringList();
}

QString DocumentGenerator::ensureFileExtension(DocumentGenerator::Type type, const QString &fileName)
{
    const QStringList extensions(DocumentGenerator::getFileExtensions(type));
    if (fileName.isEmpty() || extensions.isEmpty())
    {
        return fileName;
    }
    const QFileInfo fileInfo(fileName);
    const QString suffix(fileInfo.suffix().toLower());
    if (extensions.contains(suffix))
    {
        return fileName;
    }
    // An OpenDocument text saved as .odf would open as an OpenDocument formula.
    if (type == ODT && suffix == "odf")
    {
        return fileName.left(fileName.length() - suffix.length()) + extensions.first();
    }
    return fileName + "." + extensions.first();
}

int DocumentGenerator::perform()
{
    R_LOG_TRACE_IN;
    RLogger::info("Generating document\n");
    RLogger::indent();

    this->docTitle = new QTextDocument;
    this->docHeader = new QTextDocument;
    this->docFooter = new QTextDocument;
    this->docBody = new QTextDocument;
    this->printer = new QPrinter(QPrinter::ScreenResolution);

    // The documents are laid out for the PDF device, so it is chosen before
    // anything is laid out - its resolution differs from the native one.
    this->printer->setOutputFormat(QPrinter::PdfFormat);
    this->printer->setPageSize(QPageSize(QPageSize::A4));
    this->printer->setPageMargins(QMarginsF(20.0,15.0,20.0,10.0),QPageLayout::Millimeter);

    int retVal = 0;

    try
    {
        this->generate();

        QMap<DocumentGenerator::Type,QString>::iterator iter;
        for (iter = this->exportTypes.begin(); iter != this->exportTypes.end(); ++iter)
        {
            RLogger::info("Exporting document to '%s'\n",iter.value().toUtf8().constData());
            switch (iter.key())
            {
                case DocumentGenerator::PlainText:
                {
                    this->exportToPlainText(iter.value());
                    break;
                }
                case DocumentGenerator::HTML:
                {
                    this->exportToHTML(iter.value());
                    break;
                }
                case DocumentGenerator::Markdown:
                {
                    this->exportToMarkdown(iter.value());
                    break;
                }
                case DocumentGenerator::ODT:
                {
                    this->exportToODT(iter.value());
                    break;
                }
                case DocumentGenerator::PDF:
                {
                    this->exportToPDF(iter.value());
                    break;
                }
            }
        }
        RLogger::notice("Documents have been generated.\n");
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to generate document: %s\n",error.getMessage().toUtf8().constData());
        retVal = 1;
    }

    delete this->docTitle;
    delete this->docHeader;
    delete this->docFooter;
    delete this->docBody;
    delete this->printer;

    RLogger::unindent();
    R_LOG_TRACE_RETURN(retVal);
}

void DocumentGenerator::generate()
{
    R_LOG_TRACE_IN;
    RLogger::info("Generating document.\n");
    RLogger::indent();

    QString style = QString("color: black; background-color: yellow;");
    this->docTitle->setDefaultStyleSheet(style);
    this->docHeader->setDefaultStyleSheet(style);
    this->docBody->setDefaultStyleSheet(style);
    this->docFooter->setDefaultStyleSheet(style);

    // Page size and fonts have to be laid out at one resolution. Without a
    // paint device the fonts follow the screen (96 dpi), while the native
    // printer reported its own resolution (144 dpi at 150 % display scaling)
    // for the page, which made the page too large for its text. The documents
    // are therefore laid out on the printer itself.
    this->layoutResolution = this->printer->resolution();
    for (QTextDocument *document : {this->docTitle,this->docHeader,this->docFooter,this->docBody})
    {
        document->documentLayout()->setPaintDevice(this->printer);
    }
    QRect pageRect(this->printer->pageLayout().paintRectPixels(this->layoutResolution));
    this->docHeader->setPageSize(pageRect.size());

    this->generateHeader();

    this->docFooter->setPageSize(pageRect.size());

    this->generateFooter();

    this->docTitle->setPageSize(pageRect.size());
    // Calculating the main document size for one page
    QSizeF centerSize(pageRect.width(),pageRect.height() - this->docHeader->size().toSize().height() - this->docFooter->size().toSize().height());
    this->docBody->setPageSize(centerSize);
    this->docBody->setIndentWidth(10.0);

    this->generateTitle();
    this->generateBody();

    RLogger::unindent();
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::generateHeader()
{
    R_LOG_TRACE_IN;
    QTextImageFormat imageFormat;
    imageFormat.setName(DocumentGenerator::logoFileName);
    imageFormat.setHeight(DocumentGenerator::logoSize);
    imageFormat.setWidth(DocumentGenerator::logoSize);

    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(0.0);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);

    QTextCursor cursor(this->docHeader);
    cursor.insertTable(1,2,tableFormat);

    cursor.insertImage(imageFormat,QTextFrameFormat::FloatLeft);
    cursor.movePosition(QTextCursor::NextCell);
    this->insertText(cursor,RVendor::name(),true);
    cursor.insertBlock(QTextBlockFormat(),QTextCharFormat());
    this->insertText(cursor,RVendor::description(),true);
    cursor.insertBlock(QTextBlockFormat(),QTextCharFormat());
    this->insertText(cursor,RVendor::title(),true);
    cursor.insertBlock(QTextBlockFormat(),QTextCharFormat());
    this->insertText(cursor,RVendor::www(),true);
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml("<hr noshade/>");
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::generateFooter()
{
    R_LOG_TRACE_IN;
    QTextCursor cursor(this->docFooter);
    cursor.insertHtml(QString("<center><b>") + RVendor::name() + QString("</b> - ") + RVendor::author().toHtmlEscaped() + QString(" &copy; ") + QString::number(RVendor::year()) + "</center>");
    cursor.insertBlock(QTextBlockFormat(),QTextCharFormat());
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::exportToPlainText(const QString &fileName) const
{
    R_LOG_TRACE_IN;
    QTextDocumentWriter writer(fileName);

    writer.setFormat(QString("plaintext").toUtf8());
    if (!writer.write(this->docBody))
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write to file \'%s\'.",fileName.toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::exportToHTML(const QString &fileName) const
{
    QTextDocumentWriter writer(fileName);

    writer.setFormat(QString("HTML").toUtf8());
    if (!writer.write(this->docBody))
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write to file \'%s\'.",fileName.toUtf8().constData());
    }
    this->exportImages(fileName);
}

void DocumentGenerator::exportToMarkdown(const QString &fileName) const
{
    QTextDocumentWriter writer(fileName);

    writer.setFormat(QString("markdown").toUtf8());
    if (!writer.write(this->docBody))
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write to file \'%s\'.",fileName.toUtf8().constData());
    }
    this->exportImages(fileName);
}

void DocumentGenerator::exportImages(const QString &fileName) const
{
    // HTML and Markdown only refer to images by their names, relative to the
    // document. Images which are not there yet are saved next to it.
    const QDir directory(QFileInfo(fileName).absoluteDir());
    QSet<QString> imageNames;
    for (QTextBlock block = this->docBody->begin(); block.isValid(); block = block.next())
    {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
        {
            const QTextCharFormat charFormat = it.fragment().charFormat();
            if (charFormat.isImageFormat())
            {
                imageNames.insert(charFormat.toImageFormat().name());
            }
        }
    }
    for (const QString &imageName : std::as_const(imageNames))
    {
        // Resources compiled into the application can not be referred to.
        if (imageName.isEmpty() || imageName.startsWith(':') || imageName.startsWith("qrc:") || QDir::isAbsolutePath(imageName))
        {
            continue;
        }
        const QString imageFileName(directory.filePath(imageName));
        if (QFileInfo::exists(imageFileName))
        {
            continue;
        }
        const QImage image = this->docBody->resource(QTextDocument::ImageResource,QUrl(imageName)).value<QImage>();
        if (image.isNull() || !image.save(imageFileName))
        {
            throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write image file \'%s\'.",imageFileName.toUtf8().constData());
        }
        RLogger::info("Image saved to '%s'\n",imageFileName.toUtf8().constData());
    }
}

void DocumentGenerator::exportToODT(const QString &fileName) const
{
    // Qt's ODF writer exports the body only, so the page setup, title page,
    // header and footer of the printed document are added by OdfDocumentWriter.
    OdfDocumentWriter writer(this->docBody,this->layoutResolution,this->printer->pageLayout());

    const double titleOffset = this->docTitle->pageSize().height()/2.0 - this->docTitle->size().height() - this->docHeader->size().height();
    writer.setTitle(this->docTitle,titleOffset);

    QImageReader logoReader(DocumentGenerator::logoFileName);
    logoReader.setScaledSize(QSize(256,256));
    writer.setHeader(logoReader.read(),DocumentGenerator::logoSize,{RVendor::name(),RVendor::description(),RVendor::title(),RVendor::www()});
    writer.setFooter(RVendor::name()," - " + RVendor::author() + " © " + QString::number(RVendor::year()),tr("Page"));
    writer.setTextPointSize(DocumentGenerator::getTextFormat().fontPointSize() * this->fontSizeScale);

    writer.write(fileName);
}

void DocumentGenerator::exportToPDF(const QString &fileName) const
{
    R_LOG_TRACE_IN;
    this->printer->setOutputFormat(QPrinter::PdfFormat);
    this->printer->setColorMode(QPrinter::Color);
    this->printer->setOutputFileName(fileName);

    // Setting up the rectangles for each section.
    QRect titleRect = QRect(QPoint(0,0), this->docTitle->size().toSize());
    QRect headerRect = QRect(QPoint(0,0), this->docHeader->size().toSize());
    QRect footerRect = QRect(QPoint(0,0), this->docFooter->size().toSize());
    QRect contentRect = QRect(QPoint(0,0), this->docBody->size().toSize());
    QRect currentRect = QRect(QPoint(0,0), this->docBody->pageSize().toSize());

    QPainter painter(this->printer);

    // The documents were laid out at layoutResolution, the PDF device has a
    // resolution of its own (on a scaled display 144 dpi against 96 dpi).
    // Without scaling the pages come out too large, cut on the right and with
    // the bottom of each page lost. All rectangles below are in layout units.
    const double scale = (this->layoutResolution > 0) ? double(this->printer->resolution()) / double(this->layoutResolution) : 1.0;
    auto resetTransform = [&painter,scale]()
    {
        painter.resetTransform();
        painter.scale(scale,scale);
    };

    // Drawing the title
    resetTransform();
    painter.translate(0, this->docTitle->pageSize().height()/2.0 - titleRect.height());
    drawDocument(painter, this->docTitle, titleRect);
    resetTransform();
    // Drawing the header on the top of the page
    drawDocument(painter, this->docHeader, headerRect);

    this->printer->newPage();

    // Loop if the current content rectangle intersects with the main content rectangle.
    uint nPages = 0;
    while (currentRect.intersects(contentRect))
    {
        // Resetting the painter matrix co ordinate system.
        resetTransform();
        // Applying negative translation of painter co-ordinate system by current main content rectangle top y coordinate.
        painter.translate(0, -currentRect.y());
        // Applying positive translation of painter co-ordinate system by header hight.
        painter.translate(0, headerRect.height());
        // Drawing the center content for current page.
        drawDocument(painter, this->docBody, currentRect);
        // Resetting the painter matrix co ordinate system.
        resetTransform();
        // Drawing the header on the top of the page
        drawDocument(painter, this->docHeader, headerRect);
        // Applying positive translation of painter co-ordinate system to draw the footer
        painter.translate(0, headerRect.height());
        painter.translate(0, this->docBody->pageSize().height());
        drawDocument(painter, this->docFooter, footerRect);

        // Translating the current rectangle to the area to be printed for the next page
        currentRect.translate(0, currentRect.height());

        painter.drawText(footerRect, Qt::AlignBottom | Qt::AlignCenter, QString("%1 %2").arg(tr("Page"),QString::number(nPages+1)));

        // Inserting a new page if there is till area left to be printed
        if (currentRect.intersects(contentRect))
        {
            this->printer->newPage();
        }
        nPages++;
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertH1(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(0);
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(10.0);
        blockFormat.setBottomMargin(5.0);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getH1Format();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    if (this->enableHeaderCounters)
    {
        cursor.insertText(QString::number(++this->headerCounters.nH1) + " " + text, textFormat);
    }
    else
    {
        cursor.insertText(text, textFormat);
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertH2(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(0);
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(10.0);
        blockFormat.setBottomMargin(5.0);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getH2Format();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    if (this->enableHeaderCounters)
    {
        cursor.insertText(QString::number(this->headerCounters.nH1) +
                          "." +
                          QString::number(++this->headerCounters.nH2) +
                          " " + text,
                          textFormat);
    }
    else
    {
        cursor.insertText(text, textFormat);
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertH3(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(1);
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(5.0);
        blockFormat.setBottomMargin(2.0);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getH3Format();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    if (this->enableHeaderCounters)
    {
        cursor.insertText(QString::number(this->headerCounters.nH1) +
                          "." +
                          QString::number(this->headerCounters.nH2) +
                          "." +
                          QString::number(++this->headerCounters.nH3) +
                          " " +
                          text,
                          textFormat);
    }
    else
    {
        cursor.insertText(text, textFormat);
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertH4(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(2);
        blockFormat.setAlignment(Qt::AlignLeft);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getH4Format();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    if (this->enableHeaderCounters)
    {
        cursor.insertText(QString::number(this->headerCounters.nH1) +
                          "." +
                          QString::number(this->headerCounters.nH2) +
                          "." +
                          QString::number(this->headerCounters.nH3) +
                          "." +
                          QString::number(++this->headerCounters.nH4) +
                          " " +
                          text,
                          textFormat);
    }
    else
    {
        cursor.insertText(text, textFormat);
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertH5(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setIndent(3);
        blockFormat.setAlignment(Qt::AlignLeft);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getH5Format();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    if (this->enableHeaderCounters)
    {
        cursor.insertText(QString::number(this->headerCounters.nH1) +
                          "." +
                          QString::number(this->headerCounters.nH2) +
                          "." +
                          QString::number(this->headerCounters.nH3) +
                          "." +
                          QString::number(this->headerCounters.nH4) +
                          "." +
                          QString::number(++this->headerCounters.nH5) +
                          " " +
                          text,
                          textFormat);
    }
    else
    {
        cursor.insertText(text, textFormat);
    }
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertText(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getTextFormat();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    cursor.insertText(text,textFormat);
    R_LOG_TRACE_OUT;
}

void DocumentGenerator::insertBold(QTextCursor &cursor, const QString &text, bool useCurrentBlockFormat)
{
    R_LOG_TRACE_IN;
    if (!useCurrentBlockFormat)
    {
        QTextBlockFormat blockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        cursor.insertBlock(blockFormat,QTextCharFormat());
    }
    QTextCharFormat textFormat = DocumentGenerator::getBoldFormat();
    textFormat.setFontPointSize(textFormat.fontPointSize() * this->fontSizeScale);
    cursor.insertText(text,textFormat);
    R_LOG_TRACE_OUT;
}

QTextTableFormat DocumentGenerator::getTableFormat()
{
    R_LOG_TRACE_IN;
    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(1);
    tableFormat.setCellSpacing(0);
    // No visible border, as printed. A thin border width with a solid style
    // came out as a grid in exported documents.
    tableFormat.setBorder(0.0);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setAlignment(Qt::AlignCenter);
    R_LOG_TRACE_RETURN(tableFormat);
}

QTextBlockFormat DocumentGenerator::getBlockFormat()
{
    R_LOG_TRACE_IN;
    QTextBlockFormat blockFormat;
    blockFormat.setIndent(0);
    blockFormat.setAlignment(Qt::AlignLeft);
    R_LOG_TRACE_RETURN(blockFormat);
}

QTextCharFormat DocumentGenerator::getH1Format()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(18.0);
    fmt.setFontCapitalization(QFont::AllUppercase);
    fmt.setFontWeight(QFont::Bold);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getH2Format()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(16.0);
    fmt.setFontWeight(QFont::Bold);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getH3Format()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(16.0);
    fmt.setFontWeight(QFont::Bold);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getH4Format()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(12.0);
    fmt.setFontWeight(QFont::Bold);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getH5Format()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(12.0);
    fmt.setFontUnderline(true);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getTextFormat()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(10.0);
    R_LOG_TRACE_RETURN(fmt);
}

QTextCharFormat DocumentGenerator::getBoldFormat()
{
    R_LOG_TRACE_IN;
    QTextCharFormat fmt;
    fmt.setFontPointSize(10.0);
    fmt.setFontWeight(QFont::Bold);
    R_LOG_TRACE_RETURN(fmt);
}
