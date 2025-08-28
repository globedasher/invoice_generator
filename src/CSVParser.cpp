#include "CSVParser.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QHash>

QList<Order> CSVParser::parseOrdersCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open CSV file:" << filePath;
        return {};
    }

    QTextStream stream(&file);
    QList<Order> orders;
    QHash<QString, Order> orderMap; // Group by order ID
    
    // Read header
    if (stream.atEnd()) return {};
    QString headerLine = stream.readLine();
    QStringList headers = parseCSVLine(headerLine);
    
    // Normalize headers to lowercase for case-insensitive matching
    QStringList normalizedHeaders;
    for (const QString& header : headers) {
        normalizedHeaders.append(normalizeColumnName(header));
    }
    
    // Create column index mapping
    QHash<QString, int> columnMap;
    for (int i = 0; i < normalizedHeaders.size(); i++) {
        columnMap[normalizedHeaders[i]] = i;
    }
    
    // Read data rows
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList values = parseCSVLine(line);
        if (values.size() < headers.size()) {
            // Pad with empty strings if needed
            while (values.size() < headers.size()) {
                values.append("");
            }
        }
        
        // Extract order ID
        QString orderId = values.value(columnMap.value("order id", -1), "");
        if (orderId.isEmpty()) continue;
        
        // Get or create order
        if (!orderMap.contains(orderId)) {
            Order order;
            order.orderId = orderId;
            order.createdAt = parseDateTime(values.value(columnMap.value("created at", -1), ""));
            order.billingName = values.value(columnMap.value("billing name", -1), "");
            order.billingAddress1 = values.value(columnMap.value("billing address1", -1), "");
            order.billingAddress2 = values.value(columnMap.value("billing address2", -1), "");
            order.billingCity = values.value(columnMap.value("billing city", -1), "");
            order.billingProvince = values.value(columnMap.value("billing province", -1), "");
            order.billingZip = values.value(columnMap.value("billing zip", -1), "");
            order.billingCountry = values.value(columnMap.value("billing country", -1), "");
            order.subtotal = parseDouble(values.value(columnMap.value("subtotal", -1), "0"));
            order.shipping = parseDouble(values.value(columnMap.value("shipping", -1), "0"));
            order.taxes = parseDouble(values.value(columnMap.value("taxes", -1), "0"));
            order.total = parseDouble(values.value(columnMap.value("total", -1), "0"));
            
            orderMap[orderId] = order;
        }
        
        // Add line item if present
        QString itemName = values.value(columnMap.value("lineitem name", -1), "");
        if (!itemName.isEmpty()) {
            LineItem item;
            item.quantity = parseInt(values.value(columnMap.value("lineitem quantity", -1), "1"));
            item.description = itemName;
            item.unitPrice = parseDouble(values.value(columnMap.value("lineitem price", -1), "0"));
            item.sku = values.value(columnMap.value("lineitem sku", -1), "");
            
            orderMap[orderId].lineItems.append(item);
        }
    }
    
    // Convert map to list
    for (auto it = orderMap.begin(); it != orderMap.end(); ++it) {
        orders.append(it.value());
    }
    
    return orders;
}

QString CSVParser::normalizeColumnName(const QString& name) {
    return name.trimmed().toLower();
}

QStringList CSVParser::parseCSVLine(const QString& line) {
    QStringList result;
    QString current;
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); i++) {
        QChar c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.append(current.trimmed());
            current.clear();
        } else {
            current.append(c);
        }
    }
    
    result.append(current.trimmed());
    return result;
}

double CSVParser::parseDouble(const QString& value) {
    if (value.isEmpty()) return 0.0;
    return value.toDouble();
}

int CSVParser::parseInt(const QString& value) {
    if (value.isEmpty()) return 0;
    return value.toInt();
}

QDateTime CSVParser::parseDateTime(const QString& value) {
    if (value.isEmpty()) return QDateTime();
    
    // Try different formats
    QDateTime dt = QDateTime::fromString(value, "M/d/yyyy h:mm");
    if (!dt.isValid()) {
        dt = QDateTime::fromString(value, "M/d/yyyy");
    }
    if (!dt.isValid()) {
        dt = QDateTime::fromString(value, Qt::ISODate);
    }
    
    return dt;
}