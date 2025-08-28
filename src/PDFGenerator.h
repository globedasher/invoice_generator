#pragma once
#include <QObject>
#include <QString>
#include <QPainter>
#include <QPrinter>
#include <QFont>
#include <QPixmap>
#include "InvoiceData.h"

class PDFGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit PDFGenerator(QObject* parent = nullptr);
    
    void setTemplate(const InvoiceTemplate& templateData);
    void setHighlightRules(const QList<HighlightRule>& rules);
    bool generatePDF(const QList<Order>& orders, const QString& outputPath);
    
private:
    void drawInvoice(QPainter& painter, const Order& order, const QRect& pageRect);
    void drawHeader(QPainter& painter, const Order& order, const QRect& pageRect);
    void drawBillingInfo(QPainter& painter, const Order& order, const QRect& pageRect);
    int drawLineItems(QPainter& painter, const Order& order, const QRect& pageRect);  // Returns currentY after last item
    int drawTotals(QPainter& painter, const Order& order, const QRect& pageRect, int startY);  // Returns currentY after totals
    void drawFooter(QPainter& painter, const Order& order, const QRect& pageRect, int startY);
    
    QFont m_titleFont;
    QFont m_headerFont;
    QFont m_bodyFont;
    QFont m_smallFont;
    
    InvoiceTemplate m_template;
    QPixmap m_logo;
    QList<HighlightRule> m_highlightRules;
};