#include "PDFGenerator.h"
#include <QPainter>
#include <QPrinter>
#include <QDebug>
#include <QTextDocument>
#include <QTextOption>

PDFGenerator::PDFGenerator(QObject* parent)
    : QObject(parent)
    , m_titleFont("Arial", 16, QFont::Bold)  // Reduced for more space
    , m_headerFont("Arial", 10, QFont::Bold) // Reduced for table headers
    , m_bodyFont("Arial", 8)                 // Reduced for line items
    , m_smallFont("Arial", 7)                // Reduced for small text
{
}

void PDFGenerator::setTemplate(const InvoiceTemplate& templateData) {
    m_template = templateData;
    if (!m_template.logoPath.isEmpty()) {
        m_logo = QPixmap(m_template.logoPath);
        qDebug() << "PDFGenerator: Loaded logo from" << m_template.logoPath << "- size:" << m_logo.size() << "null:" << m_logo.isNull();
    } else {
        qDebug() << "PDFGenerator: No logo path provided";
    }
}

void PDFGenerator::setHighlightRules(const QList<HighlightRule>& rules) {
    m_highlightRules = rules;
}

bool PDFGenerator::generatePDF(const QList<Order>& orders, const QString& outputPath) {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize::Letter);
    printer.setPageMargins(QMarginsF(12.7, 12.7, 12.7, 12.7), QPageLayout::Millimeter); // 0.5 inch margins
    
    QPainter painter(&printer);
    if (!painter.isActive()) {
        qDebug() << "Failed to create PDF painter";
        return false;
    }
    
    // Get page dimensions in device units
    QRect pageRect = painter.viewport();
    qDebug() << "Page rect:" << pageRect << "DPI:" << printer.resolution();
    
    bool firstOrder = true;
    for (const auto& order : orders) {
        if (!firstOrder) {
            printer.newPage();
        }
        firstOrder = false;
        
        // Draw invoice with potential multi-page handling
        drawInvoice(painter, order, pageRect, printer);
    }
    
    return true;
}

void PDFGenerator::drawInvoice(QPainter& painter, const Order& order, const QRect& pageRect, QPrinter& printer) {
    // Check if this order needs multiple pages
    if (needsMultiplePages(order, pageRect)) {
        drawMultiPageInvoice(painter, order, pageRect, printer);
    } else {
        drawSinglePageInvoice(painter, order, pageRect);
    }
}

bool PDFGenerator::needsMultiplePages(const Order& order, const QRect& pageRect) const {
    int pageHeight = pageRect.height();
    double scaleY = pageHeight / 776.0;
    
    // Estimate space needed
    int headerSpace = 250 * scaleY;  // Header + billing info
    int footerSpace = 150 * scaleY;  // Totals + footer
    int availableForItems = pageHeight - headerSpace - footerSpace - 50 * scaleY; // Safety margin
    
    // Estimate line item space (normal sizing for readability)
    int itemHeight = 22 * scaleY;  // Normal row height for multi-page
    int estimatedItemsHeight = order.lineItems.size() * itemHeight;
    
    return estimatedItemsHeight > availableForItems;
}

void PDFGenerator::drawMultiPageInvoice(QPainter& painter, const Order& order, const QRect& pageRect, QPrinter& printer) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    double scaleY = pageHeight / 776.0;
    
    // Page 1: Logo, header, billing, and as many line items as fit
    drawLogo(painter, pageRect);
    drawHeader(painter, order, pageRect);
    drawBillingInfo(painter, order, pageRect);
    
    // Calculate available space for line items on first page  
    int headerEndY = 200 * scaleY;  // End of header/billing section (reduced from 280)
    int reservedSpace = 120 * scaleY; // Reserve space for totals/footer on last page
    int itemHeight = 22 * scaleY; // Normal height for readability
    
    int availableHeight = pageHeight - headerEndY - 30 * scaleY; // Reduced safety margin
    int itemsFirstPage = availableHeight / itemHeight;
    
    // Draw line items for first page
    int itemsDrawn = 0;
    int endY = drawLineItemsSubset(painter, order, pageRect, 0, itemsFirstPage, headerEndY);
    itemsDrawn += itemsFirstPage;
    
    // Continue with additional pages if needed
    while (itemsDrawn < order.lineItems.size()) {
        printer.newPage();
        
        // Draw logo and table headers on continuation page
        drawLogo(painter, pageRect);
        drawContinuationHeader(painter, order, pageRect);
        
        int continuationHeaderEnd = 80 * scaleY;  // Reduced from 120 to remove excessive spacing
        int availableOnContinuation = pageHeight - continuationHeaderEnd - 30 * scaleY;
        int itemsThisPage = qMin(availableOnContinuation / itemHeight, 
                                order.lineItems.size() - itemsDrawn);
        
        // Check if this is the last page - reserve space for totals/footer
        bool isLastPage = (itemsDrawn + itemsThisPage >= order.lineItems.size());
        if (isLastPage) {
            availableOnContinuation -= reservedSpace;
            itemsThisPage = qMin(availableOnContinuation / itemHeight, 
                                order.lineItems.size() - itemsDrawn);
        }
        
        endY = drawLineItemsSubset(painter, order, pageRect, itemsDrawn, itemsThisPage, continuationHeaderEnd);
        itemsDrawn += itemsThisPage;
        
        // If this is the last page, draw totals and footer
        if (itemsDrawn >= order.lineItems.size()) {
            int totalsStartY = endY + 20 * scaleY;
            int totalsEndY = drawTotals(painter, order, pageRect, totalsStartY);
            drawFooter(painter, order, pageRect, totalsEndY + 10 * scaleY);
        }
    }
}

void PDFGenerator::drawSinglePageInvoice(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Calculate positions as percentages of page size
    int margin = pageWidth * 0.05; // 5% margin
    
    // Draw logo at template position, maintaining proper aspect ratio
    if (!m_logo.isNull()) {
        // Use template positions scaled to PDF page size
        double scaleX = pageWidth / 700.0;  
        double scaleY = pageHeight / 776.0; 
        
        QRect templateRect(m_template.logoPosition.x() * scaleX, 
                          m_template.logoPosition.y() * scaleY,
                          m_template.logoPosition.width() * scaleX,
                          m_template.logoPosition.height() * scaleY);
        
        // Calculate scaled size that maintains aspect ratio and fits within template rect
        QSize logoSize = m_logo.size();
        QSize targetSize = templateRect.size();
        
        // Scale to fit within the target rectangle while maintaining aspect ratio
        QSize scaledSize = logoSize.scaled(targetSize, Qt::KeepAspectRatio);
        
        // Center the logo within the template rectangle
        QRect logoRect;
        logoRect.setSize(scaledSize);
        logoRect.moveCenter(templateRect.center());
        
        // Draw logo with proper aspect ratio
        painter.drawPixmap(logoRect, m_logo);
    }
    
    drawHeader(painter, order, pageRect);
    drawBillingInfo(painter, order, pageRect);
    int lineItemsEndY = drawLineItems(painter, order, pageRect);
    
    // Calculate safe positioning for totals and footer
    int reservedFooterSpace = 100; // Reserve space for totals + footer
    int maxContentY = pageHeight - reservedFooterSpace;
    
    // Position totals, ensuring they don't go off page
    int totalsStartY = qMin(lineItemsEndY + 20, maxContentY - 80);  // Ensure totals fit
    int totalsEndY = drawTotals(painter, order, pageRect, totalsStartY);
    
    // Position footer, ensuring it fits on page
    int footerStartY = qMin(totalsEndY + 10, maxContentY - 40);  // Reduced gap if needed, ensure footer fits
    drawFooter(painter, order, pageRect, footerStartY);
}

void PDFGenerator::drawLogo(QPainter& painter, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    if (!m_logo.isNull()) {
        double scaleX = pageWidth / 700.0;  
        double scaleY = pageHeight / 776.0; 
        
        QRect templateRect(m_template.logoPosition.x() * scaleX, 
                          m_template.logoPosition.y() * scaleY,
                          m_template.logoPosition.width() * scaleX,
                          m_template.logoPosition.height() * scaleY);
        
        QSize logoSize = m_logo.size();
        QSize targetSize = templateRect.size();
        QSize scaledSize = logoSize.scaled(targetSize, Qt::KeepAspectRatio);
        
        QRect logoRect;
        logoRect.setSize(scaledSize);
        logoRect.moveCenter(templateRect.center());
        
        painter.drawPixmap(logoRect, m_logo);
    }
}


void PDFGenerator::drawHeader(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use template positions scaled to PDF page size
    double scaleX = pageWidth / 700.0;  // Template preview width
    double scaleY = pageHeight / 776.0; // Template preview height
    
    // No title - config.xlsx template doesn't have \"INVOICE\" title
    
    painter.setFont(m_headerFont);
    
    // Draw "Order <number>" with proper spacing (matching template editor)
    QPoint orderPos(m_template.orderNumberPos.x() * scaleX, m_template.orderNumberPos.y() * scaleY);
    QString orderText = QString("Order %1").arg(order.orderId);  // Add space between Order and number
    painter.drawText(orderPos, orderText);
    
    // Date at template position  
    QPoint datePos(m_template.datePos.x() * scaleX, m_template.datePos.y() * scaleY);
    QString dateStr = order.createdAt.toString("MM/dd/yyyy");
    painter.drawText(datePos, QString("Date: %1").arg(dateStr));
}

void PDFGenerator::drawBillingInfo(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use template positions scaled to PDF page size
    double scaleX = pageWidth / 700.0;  
    double scaleY = pageHeight / 776.0; 
    
    // Draw billing exactly like template editor
    painter.setFont(m_bodyFont);  // Use body font for "Bill To:" to match template
    QPoint billingBasePos(m_template.billingNamePos.x() * scaleX, m_template.billingNamePos.y() * scaleY);
    painter.drawText(billingBasePos, "Bill To:");
    
    // Customer name - 20 pixel offset like template editor
    int yOffset = 20 * scaleY;
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), order.billingName);
    
    // Address line 1 - 15 pixel additional offset
    yOffset += 15 * scaleY;
    QString address = order.billingAddress1;
    if (!order.billingAddress2.isEmpty()) {
        address += ", " + order.billingAddress2;
    }
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), address);
    
    // City, State, ZIP - another 15 pixel offset
    yOffset += 15 * scaleY;
    QString cityStateZip = QString("%1, %2 %3")
                          .arg(order.billingCity)
                          .arg(order.billingProvince)
                          .arg(order.billingZip);
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), cityStateZip);
}

int PDFGenerator::drawLineItems(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use template positions scaled to PDF page size
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    // Table starts at template position (not hardcoded percentage)
    int tableStartY = m_template.tableStartPos.y() * scaleY;
    int lineHeight = m_template.rowHeight * scaleY;  // Use template row height
    
    painter.setFont(m_headerFont);
    
    // Define column positions using template layout (matching template editor - wider)
    int tableX = m_template.tableStartPos.x() * scaleX;
    int qtyX = tableX + 15 * scaleX;       // Offset like template editor
    int descX = tableX + 60 * scaleX;      // Match template editor
    int priceX = tableX + 450 * scaleX;    // Match template editor (moved right)
    int totalX = tableX + 550 * scaleX;    // Match template editor (moved right)
    
    // Draw table headers (matching template editor layout)
    painter.drawText(QPoint(qtyX, tableStartY), "Qty");
    painter.drawText(QPoint(descX, tableStartY), "Description"); 
    painter.drawText(QPoint(priceX, tableStartY), "Unit Price");
    painter.drawText(QPoint(totalX, tableStartY), "Line Total");
    
    // Draw line under headers (matching template editor)
    int headerLineY = tableStartY + 15 * scaleY;  // Fixed offset like template editor
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(tableX, headerLineY, tableX + 630 * scaleX, headerLineY);  // Match wider template width
    
    // Reset pen and font for content
    painter.setPen(QPen(Qt::black, 1));
    painter.setFont(m_bodyFont);
    
    int currentY = headerLineY + 10 * scaleY;
    
    // Calculate adaptive spacing and sizing based on content to fit on one page
    int numItems = order.lineItems.size();
    int dynamicSpacing;
    int adaptiveRowHeight = lineHeight;
    
    // Estimate total content height needed
    int headerHeight = 120 * scaleY;  // Approximate header + billing height
    int totalsHeight = 80 * scaleY;   // Totals section height
    int footerHeight = 80 * scaleY;   // Footer height
    int availableHeight = pageHeight - headerHeight - totalsHeight - footerHeight - 50 * scaleY; // 50px safety margin
    
    // Calculate estimated height per item (assuming some text wrapping)
    int estimatedItemHeight = adaptiveRowHeight + 4 * scaleY; // Base height + spacing
    int totalEstimatedHeight = numItems * estimatedItemHeight;
    
    // If content won't fit, make it more compact
    if (totalEstimatedHeight > availableHeight) {
        // Ultra-compact mode for large orders
        if (numItems > 40) {
            dynamicSpacing = 0;  // No spacing at all
            adaptiveRowHeight = qMax(10 * (int)scaleY, adaptiveRowHeight / 2);  // Half height, minimum 10px
        } else if (numItems > 30) {
            dynamicSpacing = 1 * scaleY;  // Minimal spacing
            adaptiveRowHeight = qMax(12 * (int)scaleY, adaptiveRowHeight * 2 / 3);  // 2/3 height
        } else if (numItems > 20) {
            dynamicSpacing = 2 * scaleY;  // Very tight spacing
            adaptiveRowHeight = qMax(14 * (int)scaleY, adaptiveRowHeight * 3 / 4);  // 3/4 height
        } else {
            dynamicSpacing = 3 * scaleY;  // Tight spacing
        }
    } else {
        // Normal spacing when content fits comfortably
        if (numItems > 10) {
            dynamicSpacing = 3 * scaleY;
        } else {
            dynamicSpacing = 4 * scaleY;
        }
    }
    
    // Draw line items (with dynamic height for wrapped text)
    for (const auto& item : order.lineItems) {
        // Calculate actual height needed for wrapped description text, but respect adaptive limits
        QRect descBounds(descX, 0, 380 * scaleX, 1000); // Wide width from updated template
        QRect actualDescRect = painter.boundingRect(descBounds, Qt::AlignLeft | Qt::TextWordWrap, item.description);
        int calculatedHeight = (int)(actualDescRect.height() + 2 * scaleY); // Reduced padding
        int actualRowHeight = qMax(adaptiveRowHeight, qMin(calculatedHeight, adaptiveRowHeight * 3)); // Cap max height to 3x adaptive
        
        // Check for highlighting (case insensitive)
        QColor bgColor;
        for (const auto& rule : m_highlightRules) {
            if (item.description.contains(rule.textMatch, Qt::CaseInsensitive)) {
                bgColor = rule.color;
                break;
            }
        }
        
        // Calculate consistent positioning for all elements
        int rowTopY = currentY - 5 * scaleY;  // Top of the row
        int verticalCenter = currentY + actualRowHeight / 2 - 5 * scaleY; // Center for single-line items
        
        // Draw highlight background if needed - perfectly aligned with text
        if (bgColor.isValid()) {
            QRect highlightRect(tableX - 3 * scaleX, rowTopY, 630 * scaleX, actualRowHeight);
            painter.fillRect(highlightRect, bgColor);
        }
        
        // Draw single-line items centered vertically
        painter.drawText(QPoint(qtyX, verticalCenter), QString::number(item.quantity));
        painter.drawText(QPoint(priceX, verticalCenter), QString("$%1").arg(item.unitPrice, 0, 'f', 2));
        painter.drawText(QPoint(totalX, verticalCenter), QString("$%1").arg(item.lineTotal(), 0, 'f', 2));
        
        // Description with word wrap - aligned with highlight
        QRect descRect(descX, rowTopY, 380 * scaleX, actualRowHeight);
        painter.drawText(descRect, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignTop, item.description);
        
        // Move to next row using actual calculated height and dynamic spacing
        currentY += actualRowHeight + dynamicSpacing;
    }
    
    // Return the Y position after the last line item
    return currentY;
}

int PDFGenerator::drawTotals(QPainter& painter, const Order& order, const QRect& pageRect, int startY) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use template positions scaled to PDF page size (align with line totals column)
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    // Get table start position to match template editor
    int tableStartX = m_template.tableStartPos.x() * scaleX;
    int totalsLabelX = tableStartX + 400 * scaleX; // Keep labels in original position
    int totalsValueX = tableStartX + 550 * scaleX; // Align values with "Line Total" column
    int totalsStartY = startY;
    int lineHeight = pageHeight * 0.025;
    
    painter.setFont(m_bodyFont);
    
    // Subtotal
    QRect subtotalLabelRect(totalsLabelX, totalsStartY, 130 * scaleX, lineHeight);
    QRect subtotalValueRect(totalsValueX, totalsStartY, 80 * scaleX, lineHeight);
    painter.drawText(subtotalLabelRect, Qt::AlignRight, "Subtotal:");
    painter.drawText(subtotalValueRect, Qt::AlignCenter, QString("$%1").arg(order.subtotal, 0, 'f', 2));
    
    // Tax
    int taxY = totalsStartY + lineHeight + 5;
    QRect taxLabelRect(totalsLabelX, taxY, 130 * scaleX, lineHeight);
    QRect taxValueRect(totalsValueX, taxY, 80 * scaleX, lineHeight);
    painter.drawText(taxLabelRect, Qt::AlignRight, "Tax:");
    painter.drawText(taxValueRect, Qt::AlignCenter, QString("$%1").arg(order.taxes, 0, 'f', 2));
    
    // Total (with heavier font and line above)
    int totalY = taxY + lineHeight + 10;
    
    // Draw line above total
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(totalsLabelX, totalY - 5, totalsValueX + 80 * scaleX, totalY - 5);
    painter.setPen(QPen(Qt::black, 1));
    
    painter.setFont(m_headerFont);
    QRect totalLabelRect(totalsLabelX, totalY, 130 * scaleX, lineHeight);
    QRect totalValueRect(totalsValueX, totalY, 80 * scaleX, lineHeight);
    painter.drawText(totalLabelRect, Qt::AlignRight, "Total:");
    painter.drawText(totalValueRect, Qt::AlignCenter, QString("$%1").arg(order.total, 0, 'f', 2));
    
    // Return Y position after totals
    return totalY + lineHeight;
}

void PDFGenerator::drawContinuationHeader(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    painter.setFont(m_headerFont);
    
    // Draw "Order <number> (continued)" at top of continuation page
    QPoint orderPos(m_template.orderNumberPos.x() * scaleX, 30 * scaleY);
    QString orderText = QString("Order %1 (continued)").arg(order.orderId);
    painter.drawText(orderPos, orderText);
    
    // Draw table headers
    int tableStartY = 40 * scaleY;  // Reduced from 60 to minimize spacing
    int tableX = m_template.tableStartPos.x() * scaleX;
    int qtyX = tableX + 15 * scaleX;
    int descX = tableX + 60 * scaleX;
    int priceX = tableX + 450 * scaleX;
    int totalX = tableX + 550 * scaleX;
    
    painter.drawText(QPoint(qtyX, tableStartY), "Qty");
    painter.drawText(QPoint(descX, tableStartY), "Description");
    painter.drawText(QPoint(priceX, tableStartY), "Unit Price");
    painter.drawText(QPoint(totalX, tableStartY), "Line Total");
    
    // Draw line under headers
    int headerLineY = tableStartY + 15 * scaleY;
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(tableX, headerLineY, tableX + 630 * scaleX, headerLineY);
    painter.setPen(QPen(Qt::black, 1));
}

int PDFGenerator::drawLineItemsSubset(QPainter& painter, const Order& order, const QRect& pageRect, int startIndex, int count, int startY) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    // Define column positions matching main drawLineItems
    int tableX = m_template.tableStartPos.x() * scaleX;
    int qtyX = tableX + 15 * scaleX;
    int descX = tableX + 60 * scaleX;
    int priceX = tableX + 450 * scaleX;
    int totalX = tableX + 550 * scaleX;
    
    painter.setFont(m_bodyFont);
    
    int currentY = startY + 10 * scaleY;  // Start below header line
    int itemHeight = 22 * scaleY;  // Normal height for multi-page readability
    
    // Draw the specified range of line items
    int endIndex = qMin(startIndex + count, order.lineItems.size());
    for (int i = startIndex; i < endIndex; i++) {
        const auto& item = order.lineItems[i];
        
        // Calculate actual height needed for description
        QRect descBounds(descX, 0, 380 * scaleX, 1000);
        QRect actualDescRect = painter.boundingRect(descBounds, Qt::AlignLeft | Qt::TextWordWrap, item.description);
        int calculatedHeight = actualDescRect.height() + 4 * scaleY;
        int actualRowHeight = qMax(itemHeight, calculatedHeight);
        
        // Check for highlighting
        QColor bgColor;
        for (const auto& rule : m_highlightRules) {
            if (item.description.contains(rule.textMatch, Qt::CaseInsensitive)) {
                bgColor = rule.color;
                break;
            }
        }
        
        int rowTopY = currentY - 5 * scaleY;
        int verticalCenter = currentY + actualRowHeight / 2 - 5 * scaleY;
        
        // Draw highlight background if needed
        if (bgColor.isValid()) {
            QRect highlightRect(tableX - 3 * scaleX, rowTopY, 630 * scaleX, actualRowHeight);
            painter.fillRect(highlightRect, bgColor);
        }
        
        // Draw line item data
        painter.drawText(QPoint(qtyX, verticalCenter), QString::number(item.quantity));
        painter.drawText(QPoint(priceX, verticalCenter), QString("$%1").arg(item.unitPrice, 0, 'f', 2));
        painter.drawText(QPoint(totalX, verticalCenter), QString("$%1").arg(item.lineTotal(), 0, 'f', 2));
        
        // Description with word wrap
        QRect descRect(descX, rowTopY, 380 * scaleX, actualRowHeight);
        painter.drawText(descRect, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignTop, item.description);
        
        currentY += actualRowHeight + 4 * scaleY;  // Normal spacing for multi-page
    }
    
    return currentY;
}

void PDFGenerator::drawFooter(QPainter& painter, const Order& order, const QRect& pageRect, int startY) {
    Q_UNUSED(order)  // Footer doesn't depend on order data
    
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use dynamic positioning, positioned to the left of totals
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    painter.setFont(m_bodyFont);
    
    // Position footer in left column (same as billing info alignment)
    int footerX = 50 * scaleX; // Left side, same as billing info
    int footerWidth = 380 * scaleX; // Width up to totals area
    
    int footerStartY = startY - 60 * scaleY; // Start at same level as totals
    int thankYouY = footerStartY;
    int policyY = footerStartY + 50 * scaleY;
    
    // Ensure footer stays within page bounds
    int maxFooterY = pageHeight - 90 * scaleY;
    if (thankYouY > maxFooterY - 80 * scaleY) {
        thankYouY = maxFooterY - 80 * scaleY;
        policyY = thankYouY + 50 * scaleY;
    }
    
    // Thank you message (left column)
    QRect thankYouRect(footerX, thankYouY, footerWidth, 40 * scaleY);
    painter.drawText(thankYouRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.thankYouText);
    
    // Policy text (left column)
    QRect policyRect(footerX, policyY, footerWidth, 80 * scaleY);
    painter.drawText(policyRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.policyText);
}