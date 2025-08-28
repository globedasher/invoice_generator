#pragma once
#include <QString>
#include <QDateTime>
#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>

struct LineItem {
    int quantity;
    QString description;
    double unitPrice;
    QString sku;
    QColor highlightColor;
    
    double lineTotal() const { return quantity * unitPrice; }
};

struct Order {
    QString orderId;
    QDateTime createdAt;
    QString billingName;
    QString billingAddress1;
    QString billingAddress2;
    QString billingCity;
    QString billingProvince;
    QString billingZip;
    QString billingCountry;
    
    QList<LineItem> lineItems;
    double subtotal;
    double shipping;
    double taxes;
    double total;
    
    double calculateSubtotal() const {
        double sum = 0;
        for (const auto& item : lineItems) {
            sum += item.lineTotal();
        }
        return sum;
    }
};

struct HighlightRule {
    QString textMatch;
    QColor color;
};

struct InvoiceTemplate {
    QString logoPath;
    QRect logoPosition;
    QPoint orderNumberPos;
    QPoint datePos;
    QPoint billingNamePos;
    QPoint billingAddress1Pos;
    QPoint billingAddress2Pos;
    QPoint billingCityStateZipPos;
    
    // Table positions
    QPoint tableStartPos;
    int rowHeight;
    struct {
        int quantity;
        int description;
        int unitPrice;
        int lineTotal;
    } columnWidths;
    
    // Totals positions
    QPoint subtotalPos;
    QPoint taxPos;
    QPoint totalPos;
    
    // Footer positions (from config.xlsx A40 and B42)
    QPoint thankYouPos;
    QPoint policyPos;
    
    // Editable footer text
    QString thankYouText;
    QString policyText;
};