#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QHeaderView>
#include <QLabel>
#include "InvoiceData.h"

class HighlightManager : public QWidget {
    Q_OBJECT
    
public:
    explicit HighlightManager(QWidget* parent = nullptr);
    
    void setHighlightRules(const QList<HighlightRule>& rules);
    QList<HighlightRule> getHighlightRules() const;
    QColor getHighlightColor(const QString& description) const;
    
signals:
    void highlightRulesChanged();
    
private slots:
    void addRule();
    void removeRule();
    void editColor();
    void onCellChanged(int row, int column);
    
private:
    void setupUI();
    void updateTable();
    
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QTableWidget* m_table;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    
    QList<HighlightRule> m_rules;
};