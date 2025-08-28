#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QTabWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QFileDialog>
#include <QSplitter>
#include "InvoiceData.h"

class TemplatePreview : public QWidget {
    Q_OBJECT
    
public:
    explicit TemplatePreview(QWidget* parent = nullptr);
    
    void setTemplate(const InvoiceTemplate& templ);
    InvoiceTemplate getTemplate() const { return m_template; }
    
    void setPreviewOrder(const Order& order) { m_previewOrder = order; update(); }
    
signals:
    void templateChanged();
    void elementSelected(const QString& elementName);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    
private:
    void drawTemplate(QPainter& painter);
    QString getElementAtPoint(const QPoint& point) const;
    
    InvoiceTemplate m_template;
    Order m_previewOrder;
    QString m_selectedElement;
    bool m_dragging;
    QPoint m_dragStart;
    QPixmap m_logo;
    
    // Scale factor for preview
    double m_scaleFactor;
};

class TemplateEditor : public QWidget {
    Q_OBJECT
    
public:
    explicit TemplateEditor(QWidget* parent = nullptr);
    
    void setTemplate(const InvoiceTemplate& templ);
    InvoiceTemplate getTemplate() const;
    
    void setPreviewOrder(const Order& order);
    
signals:
    void templateChanged();
    
private slots:
    void onLogoButtonClicked();
    void onElementSelected(const QString& elementName);
    void onPropertyChanged();
    void resetToDefaults();
    
private:
    void setupUI();
    void updatePropertyEditor(const QString& elementName);
    void applyPropertyChanges();
    void loadDefaultTemplate();
    void updatePageIndicator(const Order& order);
    
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    
    // Left side - preview
    QScrollArea* m_previewArea;
    TemplatePreview* m_preview;
    QLabel* m_pageIndicator;
    
    // Right side - properties with tabs
    QTabWidget* m_propertiesTab;
    
    // Logo tab
    QWidget* m_logoTab;
    QGroupBox* m_logoGroup;
    QLineEdit* m_logoPathEdit;
    QPushButton* m_logoButton;
    
    // Positioning tab
    QWidget* m_positionTab;
    QGroupBox* m_positionGroup;
    QGridLayout* m_positionLayout;
    QSpinBox* m_xSpinBox;
    QSpinBox* m_ySpinBox;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    
    // Footer tab
    QWidget* m_footerTab;
    QGroupBox* m_textGroup;
    QTextEdit* m_thankYouEdit;
    QTextEdit* m_policyEdit;
    
    QPushButton* m_resetButton;
    
    QString m_selectedElement;
    InvoiceTemplate m_template;
};