#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QScrollArea>
#include <QWidget>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QSplitter>
#include <QComboBox>
#include <QUrl>
// Note: QPdfView/QPdfDocument are Qt6 features, using alternative approach for Qt5
#include <QAction>

#include "CSVParser.h"
#include "TemplateEditor.h"
#include "HighlightManager.h"
#include "PDFGenerator.h"
#include "InvoiceData.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void openCSVFile();
    void generatePDF();
    void saveTemplate();
    void loadTemplate();
    void showAbout();
    void onOrderSelectionChanged();
    void sortOrders();
    void previewPDF();
    
private:
    void setupUI();
    void setupDataTab();
    void setupTemplateTab();
    void setupHighlightsTab();
    void setupPreviewTab();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void updateOrdersList();
    void updatePreview();
    void showOrder(const Order& order);
    void showPDFPreview(const QString& pdfPath);
    void drawInvoicePreview(QPainter& painter, const Order& order, const QRect& pageRect);
    
    // UI Components
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;
    
    // Data tab
    QWidget* m_dataTab;
    QVBoxLayout* m_dataLayout;
    QHBoxLayout* m_fileLayout;
    QLineEdit* m_csvPathEdit;
    QPushButton* m_browseButton;
    QListWidget* m_ordersList;
    QTextEdit* m_orderDetails;
    QPushButton* m_sortButton;
    QComboBox* m_sortComboBox;
    
    // Template tab
    QWidget* m_templateTab;
    TemplateEditor* m_templateEditor;
    
    // Highlights tab
    QWidget* m_highlightsTab;
    HighlightManager* m_highlightManager;
    
    // Preview tab
    QWidget* m_previewTab;
    QVBoxLayout* m_previewLayout;
    QScrollArea* m_previewScrollArea;
    QLabel* m_pdfPreviewLabel; // Using QLabel for simple PDF preview in Qt5
    QPushButton* m_previewButton;
    QPushButton* m_generateButton;
    
    // Actions
    QAction* m_openCSVAction;
    QAction* m_saveTemplateAction;
    QAction* m_loadTemplateAction;
    QAction* m_generatePDFAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    QAction* m_darkModeAction;
    
    // Data
    QList<Order> m_orders;
    InvoiceTemplate m_template;
    PDFGenerator* m_pdfGenerator;
    QString m_csvFilePath;
    QString m_lastPDFPath;
};