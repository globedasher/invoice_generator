#include "HighlightManager.h"
#include <QTableWidgetItem>
#include <QColorDialog>
#include <QHeaderView>
#include <QDebug>

HighlightManager::HighlightManager(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_table(nullptr)
    , m_addButton(nullptr)
    , m_removeButton(nullptr)
{
    setupUI();
    
    // Add default rules matching the CSV data
    HighlightRule rule1;
    rule1.textMatch = "goldenrod";
    rule1.color = QColor(218, 165, 32); // Goldenrod color
    m_rules.append(rule1);
    
    HighlightRule rule2;
    rule2.textMatch = "huckleberry";
    rule2.color = QColor(138, 43, 226); // Blue violet (huckleberry color)
    m_rules.append(rule2);
    
    updateTable();
}

void HighlightManager::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Add instructions
    QLabel* instructions = new QLabel(
        "Configure color highlighting rules for line items.\n"
        "Enter text to match (case-insensitive) and choose colors.\n"
        "Example: \"bundle\" will highlight items containing \"bundle\" in the description."
    );
    instructions->setStyleSheet("background-color: #f0f0f0; padding: 10px; border: 1px solid #ccc;");
    m_mainLayout->addWidget(instructions);
    
    // Create table
    m_table = new QTableWidget(0, 2, this);
    m_table->setHorizontalHeaderLabels({"Text to Match", "Color"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setMinimumHeight(200);
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Make first column take most of the space
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    
    connect(m_table, &QTableWidget::cellChanged, this, &HighlightManager::onCellChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &HighlightManager::editColor);
    
    // Create buttons
    m_buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Add Rule", this);
    m_removeButton = new QPushButton("Remove Rule", this);
    
    connect(m_addButton, &QPushButton::clicked, this, &HighlightManager::addRule);
    connect(m_removeButton, &QPushButton::clicked, this, &HighlightManager::removeRule);
    
    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_removeButton);
    m_buttonLayout->addStretch();
    
    // Layout
    m_mainLayout->addWidget(m_table);
    m_mainLayout->addLayout(m_buttonLayout);
    
    setLayout(m_mainLayout);
}

void HighlightManager::setHighlightRules(const QList<HighlightRule>& rules) {
    m_rules = rules;
    updateTable();
}

QList<HighlightRule> HighlightManager::getHighlightRules() const {
    return m_rules;
}

QColor HighlightManager::getHighlightColor(const QString& description) const {
    QString lowerDesc = description.toLower();
    
    // Find first matching rule (case-insensitive partial matching)
    for (const auto& rule : m_rules) {
        if (lowerDesc.contains(rule.textMatch.toLower(), Qt::CaseInsensitive)) {
            return rule.color;
        }
    }
    
    return QColor(); // No match
}

void HighlightManager::addRule() {
    HighlightRule rule;
    rule.textMatch = "Enter text to match";
    rule.color = QColor(Qt::yellow);
    
    m_rules.append(rule);
    updateTable();
    emit highlightRulesChanged();
}

void HighlightManager::removeRule() {
    int row = m_table->currentRow();
    if (row >= 0 && row < m_rules.size()) {
        m_rules.removeAt(row);
        updateTable();
        emit highlightRulesChanged();
    }
}

void HighlightManager::editColor() {
    int row = m_table->currentRow();
    int col = m_table->currentColumn();
    
    if (row >= 0 && row < m_rules.size() && col == 1) {
        QColor color = QColorDialog::getColor(m_rules[row].color, this, "Choose Highlight Color");
        if (color.isValid()) {
            m_rules[row].color = color;
            updateTable();
            emit highlightRulesChanged();
        }
    }
}

void HighlightManager::onCellChanged(int row, int column) {
    if (row >= 0 && row < m_rules.size() && column == 0) {
        QTableWidgetItem* item = m_table->item(row, column);
        if (item) {
            m_rules[row].textMatch = item->text();
            emit highlightRulesChanged();
        }
    }
}

void HighlightManager::updateTable() {
    m_table->setRowCount(m_rules.size());
    
    for (int i = 0; i < m_rules.size(); i++) {
        // Text match column
        QTableWidgetItem* textItem = new QTableWidgetItem(m_rules[i].textMatch);
        m_table->setItem(i, 0, textItem);
        
        // Color column
        QTableWidgetItem* colorItem = new QTableWidgetItem(m_rules[i].color.name());
        colorItem->setBackground(QBrush(m_rules[i].color));
        colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, colorItem);
    }
}