#pragma once
#include <QString>
#include <QList>
#include "InvoiceData.h"

class CSVParser {
public:
    static QList<Order> parseOrdersCSV(const QString& filePath);
    
private:
    static QString normalizeColumnName(const QString& name);
    static QStringList parseCSVLine(const QString& line);
    static double parseDouble(const QString& value);
    static int parseInt(const QString& value);
    static QDateTime parseDateTime(const QString& value);
};