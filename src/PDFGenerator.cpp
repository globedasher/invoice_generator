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
    
    for (int i = 0; i < orders.size(); i++) {
        if (i > 0) {
            printer.newPage();
        }
        drawInvoice(painter, orders[i], pageRect);
    }
    
    return true;
}

void PDFGenerator::drawInvoice(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Calculate positions as percentages of page size
    int margin = pageWidth * 0.05; // 5% margin
    
    // Draw logo at template position (square, maintaining aspect ratio)
    if (!m_logo.isNull()) {
        // Use template positions scaled to PDF page size
        double scaleX = pageWidth / 700.0;  
        double scaleY = pageHeight / 776.0; 
        
        QRect logoRect(m_template.logoPosition.x() * scaleX, 
                      m_template.logoPosition.y() * scaleY,
                      m_template.logoPosition.width() * scaleX,
                      m_template.logoPosition.height() * scaleY);
        
        // Draw logo maintaining aspect ratio and preventing shrinking
        painter.drawPixmap(logoRect, m_logo, m_logo.rect());
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
    
    // Position totals dynamically after line items
    int totalsX = pageWidth * 0.65;
    int totalsWidth = pageWidth * 0.3;
    int totalsStartY = startY;  // Start at provided Y position
    int lineHeight = pageHeight * 0.025;
    
    painter.setFont(m_bodyFont);
    
    // Subtotal
    QRect subtotalLabelRect(totalsX, totalsStartY, totalsWidth * 0.6, lineHeight);
    QRect subtotalValueRect(totalsX + totalsWidth * 0.6, totalsStartY, totalsWidth * 0.4, lineHeight);
    painter.drawText(subtotalLabelRect, Qt::AlignLeft, "Subtotal:");
    painter.drawText(subtotalValueRect, Qt::AlignRight, QString("$%1").arg(order.subtotal, 0, 'f', 2));
    
    // Tax
    int taxY = totalsStartY + lineHeight + 5;
    QRect taxLabelRect(totalsX, taxY, totalsWidth * 0.6, lineHeight);
    QRect taxValueRect(totalsX + totalsWidth * 0.6, taxY, totalsWidth * 0.4, lineHeight);
    painter.drawText(taxLabelRect, Qt::AlignLeft, "Tax:");
    painter.drawText(taxValueRect, Qt::AlignRight, QString("$%1").arg(order.taxes, 0, 'f', 2));
    
    // Total (with heavier font and line above)
    int totalY = taxY + lineHeight + 10;
    
    // Draw line above total
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(totalsX, totalY - 5, totalsX + totalsWidth, totalY - 5);
    painter.setPen(QPen(Qt::black, 1));
    
    painter.setFont(m_headerFont);
    QRect totalLabelRect(totalsX, totalY, totalsWidth * 0.6, lineHeight);
    QRect totalValueRect(totalsX + totalsWidth * 0.6, totalY, totalsWidth * 0.4, lineHeight);
    painter.drawText(totalLabelRect, Qt::AlignLeft, "Total:");
    painter.drawText(totalValueRect, Qt::AlignRight, QString("$%1").arg(order.total, 0, 'f', 2));
    
    // Return Y position after totals
    return totalY + lineHeight;
}

void PDFGenerator::drawFooter(QPainter& painter, const Order& order, const QRect& pageRect, int startY) {
    Q_UNUSED(order)  // Footer doesn't depend on order data
    
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Use dynamic positioning instead of template positions
    double scaleX = pageWidth / 700.0;
    double scaleY = pageHeight / 776.0;
    
    painter.setFont(m_bodyFont);
    
    // Position footer dynamically after totals
    int thankYouY = startY;
    int policyY = startY + 30 * scaleY;
    
    // Ensure footer stays within page bounds (leave 30px margin from bottom)
    int maxFooterY = pageHeight - 90 * scaleY;  // 90px from bottom for both texts
    if (thankYouY > maxFooterY - 40 * scaleY) {
        thankYouY = maxFooterY - 40 * scaleY;
        policyY = thankYouY + 30 * scaleY;
    }
    
    // Thank you message with bounds checking
    QPoint thankYouPos(m_template.thankYouPos.x() * scaleX, thankYouY);
    QRect thankYouRect(thankYouPos.x(), thankYouPos.y(), 300 * scaleX, 40 * scaleY);
    painter.drawText(thankYouRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.thankYouText);
    
    // Policy text with bounds checking
    QPoint policyPos(m_template.policyPos.x() * scaleX, policyY);
    QRect policyRect(policyPos.x(), policyPos.y(), 500 * scaleX, 50 * scaleY);  // Reduced height
    painter.drawText(policyRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.policyText);
}