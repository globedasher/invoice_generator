#include "MainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_pdfGenerator(nullptr)
{
    setWindowTitle("Invoice Generator");
    setWindowIcon(QIcon(":/icons/invoice.png"));
    
    // Make window responsive to screen size
    QDesktopWidget desktop;
    QRect screenGeometry = desktop.screenGeometry();
    
    // Use 80% of screen width and 75% of screen height, with reasonable minimums
    int width = qMax(800, static_cast<int>(screenGeometry.width() * 0.8));
    int height = qMax(600, static_cast<int>(screenGeometry.height() * 0.75));
    
    resize(width, height);
    
    // Center the window on screen
    move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);
    
    // Set minimum size to ensure usability
    setMinimumSize(700, 500);
    
    m_pdfGenerator = new PDFGenerator(this);
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    
    // Set default template to exactly match config.xlsx Invoice sheet
    InvoiceTemplate defaultTemplate;
    defaultTemplate.logoPosition = QRect(590, 20, 100, 100);  // Far right corner
    defaultTemplate.orderNumberPos = QPoint(50, 30);          // Left side, aligned with billing
    defaultTemplate.datePos = QPoint(450, 120);               // Moved further right
    defaultTemplate.billingNamePos = QPoint(50, 70);          // B3 position
    defaultTemplate.tableStartPos = QPoint(50, 140);          // Moved up - right after billing info
    defaultTemplate.rowHeight = 14;                           // Ultra-compact line height
    defaultTemplate.columnWidths.quantity = 50;
    defaultTemplate.columnWidths.description = 280;
    defaultTemplate.columnWidths.unitPrice = 80;
    defaultTemplate.columnWidths.lineTotal = 80;
    defaultTemplate.subtotalPos = QPoint(480, 480);
    defaultTemplate.taxPos = QPoint(480, 500);
    defaultTemplate.totalPos = QPoint(480, 520);
    defaultTemplate.thankYouPos = QPoint(50, 680);   // Footer positioning - safe margin from bottom
    defaultTemplate.policyPos = QPoint(50, 710);     // Policy text below thank you
    defaultTemplate.thankYouText = "Thank you for your order.\nHappy planting!";
    defaultTemplate.policyText = "We do not offer refunds once you have left the premises. All sales are final. "
                                "Please check the contents of your order carefully to make sure there are no errors. "
                                "Staff are available to assist and correct errors.";
    
    m_template = defaultTemplate;
    m_templateEditor->setTemplate(m_template);
    m_pdfGenerator->setTemplate(m_template);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    
    // Data tab
    setupDataTab();
    m_tabWidget->addTab(m_dataTab, "Orders Data");
    
    // Template tab  
    setupTemplateTab();
    m_tabWidget->addTab(m_templateTab, "Template Editor");
    
    // Highlights tab
    setupHighlightsTab();
    m_tabWidget->addTab(m_highlightsTab, "Line Highlighting");
    
    // Preview tab
    setupPreviewTab();
    m_tabWidget->addTab(m_previewTab, "Preview & Generate");
    
    mainLayout->addWidget(m_tabWidget);
}

void MainWindow::setupDataTab() {
    m_dataTab = new QWidget();
    m_dataLayout = new QVBoxLayout(m_dataTab);
    
    // File selection
    m_fileLayout = new QHBoxLayout();
    m_fileLayout->addWidget(new QLabel("CSV File:"));
    
    m_csvPathEdit = new QLineEdit();
    m_csvPathEdit->setReadOnly(true);
    m_fileLayout->addWidget(m_csvPathEdit);
    
    m_browseButton = new QPushButton("Browse...");
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::openCSVFile);
    m_fileLayout->addWidget(m_browseButton);
    
    m_dataLayout->addLayout(m_fileLayout);
    
    // Orders list and details with responsive splitter
    QSplitter* contentSplitter = new QSplitter(Qt::Horizontal);
    
    // Orders list
    QWidget* ordersWidget = new QWidget();
    QVBoxLayout* ordersLayout = new QVBoxLayout(ordersWidget);
    ordersLayout->addWidget(new QLabel("Orders:"));
    
    // Sort controls
    QHBoxLayout* sortLayout = new QHBoxLayout();
    sortLayout->addWidget(new QLabel("Sort by:"));
    
    m_sortComboBox = new QComboBox();
    m_sortComboBox->addItems({"Order ID", "Date", "Customer Name", "Total Amount"});
    sortLayout->addWidget(m_sortComboBox);
    
    m_sortButton = new QPushButton("Sort");
    connect(m_sortButton, &QPushButton::clicked, this, &MainWindow::sortOrders);
    sortLayout->addWidget(m_sortButton);
    
    sortLayout->addStretch();
    ordersLayout->addLayout(sortLayout);
    
    m_ordersList = new QListWidget();
    connect(m_ordersList, &QListWidget::currentRowChanged, this, &MainWindow::onOrderSelectionChanged);
    ordersLayout->addWidget(m_ordersList);
    
    // Order details
    QWidget* detailsWidget = new QWidget();
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->addWidget(new QLabel("Order Details:"));
    
    m_orderDetails = new QTextEdit();
    m_orderDetails->setReadOnly(true);
    detailsLayout->addWidget(m_orderDetails);
    
    contentSplitter->addWidget(ordersWidget);
    contentSplitter->addWidget(detailsWidget);
    
    // Set initial splitter proportions (30% orders, 70% details)
    contentSplitter->setStretchFactor(0, 3);
    contentSplitter->setStretchFactor(1, 7);
    
    m_dataLayout->addWidget(contentSplitter, 1);
}

void MainWindow::setupTemplateTab() {
    m_templateTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_templateTab);
    
    m_templateEditor = new TemplateEditor();
    connect(m_templateEditor, &TemplateEditor::templateChanged, [this]() {
        m_template = m_templateEditor->getTemplate();
        m_pdfGenerator->setTemplate(m_template);
        qDebug() << "Template updated - new logo position:" << m_template.logoPosition;
    });
    
    layout->addWidget(m_templateEditor);
}

void MainWindow::setupHighlightsTab() {
    m_highlightsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_highlightsTab);
    
    m_highlightManager = new HighlightManager();
    connect(m_highlightManager, &HighlightManager::highlightRulesChanged, [this]() {
        m_pdfGenerator->setHighlightRules(m_highlightManager->getHighlightRules());
    });
    layout->addWidget(m_highlightManager);
    
    // Set initial highlight rules
    m_pdfGenerator->setHighlightRules(m_highlightManager->getHighlightRules());
}

void MainWindow::setupPreviewTab() {
    m_previewTab = new QWidget();
    m_previewLayout = new QVBoxLayout(m_previewTab);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_previewButton = new QPushButton("Preview PDF");
    connect(m_previewButton, &QPushButton::clicked, this, &MainWindow::previewPDF);
    buttonLayout->addWidget(m_previewButton);
    
    m_generateButton = new QPushButton("Generate PDF");
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::generatePDF);
    buttonLayout->addWidget(m_generateButton);
    
    buttonLayout->addStretch();
    m_previewLayout->addLayout(buttonLayout);
    
    // PDF preview area (simplified for Qt5)
    m_pdfPreviewLabel = new QLabel("PDF preview will be shown here after generation");
    m_pdfPreviewLabel->setAlignment(Qt::AlignCenter);
    m_pdfPreviewLabel->setStyleSheet("border: 2px dashed gray; min-height: 400px;");
    
    m_previewScrollArea = new QScrollArea();
    m_previewScrollArea->setWidget(m_pdfPreviewLabel);
    m_previewScrollArea->setWidgetResizable(true);
    
    m_previewLayout->addWidget(m_previewScrollArea, 1);
}

void MainWindow::setupMenuBar() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    m_openCSVAction = new QAction("&Open CSV...", this);
    m_openCSVAction->setShortcut(QKeySequence::Open);
    connect(m_openCSVAction, &QAction::triggered, this, &MainWindow::openCSVFile);
    fileMenu->addAction(m_openCSVAction);
    
    fileMenu->addSeparator();
    
    m_saveTemplateAction = new QAction("&Save Template...", this);
    m_saveTemplateAction->setShortcut(QKeySequence::Save);
    connect(m_saveTemplateAction, &QAction::triggered, this, &MainWindow::saveTemplate);
    fileMenu->addAction(m_saveTemplateAction);
    
    m_loadTemplateAction = new QAction("&Load Template...", this);
    connect(m_loadTemplateAction, &QAction::triggered, this, &MainWindow::loadTemplate);
    fileMenu->addAction(m_loadTemplateAction);
    
    fileMenu->addSeparator();
    
    m_generatePDFAction = new QAction("&Generate PDF...", this);
    m_generatePDFAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(m_generatePDFAction, &QAction::triggered, this, &MainWindow::generatePDF);
    fileMenu->addAction(m_generatePDFAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(m_exitAction);
    
    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    m_darkModeAction = new QAction("&Dark Mode (Follows System)", this);
    m_darkModeAction->setEnabled(false); // Just informational now
    viewMenu->addAction(m_darkModeAction);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    m_aboutAction = new QAction("&About", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar("Main");
    
    toolBar->addAction(m_openCSVAction);
    toolBar->addSeparator();
    toolBar->addAction(m_saveTemplateAction);
    toolBar->addAction(m_loadTemplateAction);
    toolBar->addSeparator();
    toolBar->addAction(m_generatePDFAction);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::openCSVFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Orders CSV File", "", "CSV Files (*.csv)");
        
    if (!fileName.isEmpty()) {
        m_csvFilePath = fileName;
        m_csvPathEdit->setText(fileName);
        
        // Parse CSV
        m_orders = CSVParser::parseOrdersCSV(fileName);
        
        if (m_orders.isEmpty()) {
            QMessageBox::warning(this, "Error", "No valid orders found in the CSV file.");
            return;
        }
        
        // Set default sort to Order ID and sort the data
        m_sortComboBox->setCurrentText("Order ID");
        sortOrders();
        
        updateOrdersList();
        statusBar()->showMessage(QString("Loaded %1 orders").arg(m_orders.size()));
        
        // Update preview with first order if available
        if (!m_orders.isEmpty()) {
            m_templateEditor->setPreviewOrder(m_orders.first());
        }
    }
}

void MainWindow::generatePDF() {
    if (m_orders.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please load a CSV file first.");
        return;
    }
    
    // Ensure orders are sorted by Order ID for PDF generation
    QList<Order> sortedOrders = m_orders;
    std::sort(sortedOrders.begin(), sortedOrders.end(), [](const Order& a, const Order& b) {
        return a.orderId < b.orderId;
    });
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save PDF As", "invoices.pdf", "PDF Files (*.pdf)");
        
    if (!fileName.isEmpty()) {
        QProgressDialog progress("Generating PDF...", "Cancel", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        
        // Update progress
        progress.setValue(25);
        QApplication::processEvents();
        
        bool success = m_pdfGenerator->generatePDF(sortedOrders, fileName);
        
        progress.setValue(100);
        
        if (success) {
            m_lastPDFPath = fileName;
            QMessageBox::information(this, "Success", 
                QString("PDF generated successfully: %1").arg(fileName));
                
            // Switch to preview tab and show success message
            m_tabWidget->setCurrentIndex(3);
            m_pdfPreviewLabel->setText(QString("PDF generated successfully:\n%1\n\nClick 'Preview PDF' to open in external viewer").arg(fileName));
        } else {
            QMessageBox::critical(this, "Error", "Failed to generate PDF.");
        }
    }
}

void MainWindow::previewPDF() {
    if (m_orders.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please load a CSV file first.");
        return;
    }
    
    // Generate temporary PDF for preview
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFile = QDir(tempDir).absoluteFilePath("invoice_preview.pdf");
    
    QProgressDialog progress("Generating preview...", "Cancel", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    progress.setValue(25);
    QApplication::processEvents();
    
    // Sort orders by Order ID for preview too
    QList<Order> sortedOrders = m_orders;
    std::sort(sortedOrders.begin(), sortedOrders.end(), [](const Order& a, const Order& b) {
        return a.orderId < b.orderId;
    });
    
    progress.setValue(50);
    QApplication::processEvents();
    
    bool success = m_pdfGenerator->generatePDF(sortedOrders, tempFile);
    
    progress.setValue(75);
    QApplication::processEvents();
    
    if (success) {
        // Show embedded preview using QPrinter to render first page as image
        showPDFPreview(tempFile);
        progress.setValue(100);
        statusBar()->showMessage("PDF preview generated successfully");
    } else {
        progress.setValue(100);
        QMessageBox::critical(this, "Error", "Failed to generate preview.");
    }
}

void MainWindow::saveTemplate() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Template", "template.json", "JSON Files (*.json)");
        
    if (!fileName.isEmpty()) {
        // Save template as JSON
        QJsonObject json;
        json["logoPath"] = m_template.logoPath;
        json["logoX"] = m_template.logoPosition.x();
        json["logoY"] = m_template.logoPosition.y();
        json["logoWidth"] = m_template.logoPosition.width();
        json["logoHeight"] = m_template.logoPosition.height();
        json["orderNumberX"] = m_template.orderNumberPos.x();
        json["orderNumberY"] = m_template.orderNumberPos.y();
        json["dateX"] = m_template.datePos.x();
        json["dateY"] = m_template.datePos.y();
        json["billingX"] = m_template.billingNamePos.x();
        json["billingY"] = m_template.billingNamePos.y();
        json["tableX"] = m_template.tableStartPos.x();
        json["tableY"] = m_template.tableStartPos.y();
        json["rowHeight"] = m_template.rowHeight;
        json["subtotalX"] = m_template.subtotalPos.x();
        json["subtotalY"] = m_template.subtotalPos.y();
        
        QJsonDocument doc(json);
        
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            QMessageBox::information(this, "Success", "Template saved successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Failed to save template.");
        }
    }
}

void MainWindow::loadTemplate() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Load Template", "", "JSON Files (*.json)");
        
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject json = doc.object();
            
            m_template.logoPath = json["logoPath"].toString();
            m_template.logoPosition = QRect(
                json["logoX"].toInt(),
                json["logoY"].toInt(),
                json["logoWidth"].toInt(),
                json["logoHeight"].toInt()
            );
            m_template.orderNumberPos = QPoint(json["orderNumberX"].toInt(), json["orderNumberY"].toInt());
            m_template.datePos = QPoint(json["dateX"].toInt(), json["dateY"].toInt());
            m_template.billingNamePos = QPoint(json["billingX"].toInt(), json["billingY"].toInt());
            m_template.tableStartPos = QPoint(json["tableX"].toInt(), json["tableY"].toInt());
            m_template.rowHeight = json["rowHeight"].toInt();
            m_template.subtotalPos = QPoint(json["subtotalX"].toInt(), json["subtotalY"].toInt());
            
            m_templateEditor->setTemplate(m_template);
            m_pdfGenerator->setTemplate(m_template);
            
            QMessageBox::information(this, "Success", "Template loaded successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Failed to load template.");
        }
    }
}


void MainWindow::showAbout() {
    QMessageBox::about(this, "About Invoice Generator",
        "Invoice Generator v1.0\n\n"
        "A C++ Qt application for generating professional PDF invoices "
        "from CSV order data with customizable templates and line highlighting.\n\n"
        "Features:\n"
        "• WYSIWYG template editor\n"
        "• Line item highlighting\n"
        "• CSV order import\n"
        "• PDF preview and generation\n"
        "• Automatic system dark mode detection\n"
        "• Responsive interface");
}

void MainWindow::onOrderSelectionChanged() {
    int row = m_ordersList->currentRow();
    if (row >= 0 && row < m_orders.size()) {
        showOrder(m_orders[row]);
        m_templateEditor->setPreviewOrder(m_orders[row]);
    }
}

void MainWindow::sortOrders() {
    if (m_orders.isEmpty()) return;
    
    QString sortBy = m_sortComboBox->currentText();
    
    std::sort(m_orders.begin(), m_orders.end(), [sortBy](const Order& a, const Order& b) {
        if (sortBy == "Order ID") {
            return a.orderId < b.orderId;
        } else if (sortBy == "Date") {
            return a.createdAt < b.createdAt;
        } else if (sortBy == "Customer Name") {
            return a.billingName.toLower() < b.billingName.toLower();
        } else if (sortBy == "Total Amount") {
            return a.total < b.total;
        }
        return false;
    });
    
    updateOrdersList();
}

void MainWindow::updateOrdersList() {
    m_ordersList->clear();
    
    for (const auto& order : m_orders) {
        QString itemText = QString("Order #%1 - %2 (%3 items) - $%4")
                          .arg(order.orderId)
                          .arg(order.billingName)
                          .arg(order.lineItems.size())
                          .arg(order.total, 0, 'f', 2);
        m_ordersList->addItem(itemText);
    }
    
    if (!m_orders.isEmpty()) {
        m_ordersList->setCurrentRow(0);
    }
}

void MainWindow::showOrder(const Order& order) {
    QString details;
    
    details += QString("Order ID: %1\n").arg(order.orderId);
    details += QString("Date: %1\n").arg(order.createdAt.toString("MM/dd/yyyy hh:mm"));
    details += QString("Customer: %1\n").arg(order.billingName);
    
    QString address = order.billingAddress1;
    if (!order.billingAddress2.isEmpty()) {
        address += ", " + order.billingAddress2;
    }
    details += QString("Address: %1\n").arg(address);
    details += QString("City: %1, %2 %3\n").arg(order.billingCity, order.billingProvince, order.billingZip);
    
    details += "\nLine Items:\n";
    for (const auto& item : order.lineItems) {
        details += QString("- %1x %2 @ $%3 = $%4\n")
                   .arg(item.quantity)
                   .arg(item.description)
                   .arg(item.unitPrice, 0, 'f', 2)
                   .arg(item.lineTotal(), 0, 'f', 2);
    }
    
    details += QString("\nSubtotal: $%1\n").arg(order.subtotal, 0, 'f', 2);
    details += QString("Tax: $%1\n").arg(order.taxes, 0, 'f', 2);
    details += QString("Total: $%1\n").arg(order.total, 0, 'f', 2);
    
    m_orderDetails->setText(details);
}

void MainWindow::showPDFPreview(const QString& pdfPath) {
    // Generate actual preview by rendering the first invoice using the same logic as PDF generator
    if (m_orders.isEmpty()) return;
    
    // Sort orders by Order ID to match PDF generation
    QList<Order> sortedOrders = m_orders;
    std::sort(sortedOrders.begin(), sortedOrders.end(), [](const Order& a, const Order& b) {
        return a.orderId < b.orderId;
    });
    
    // Create preview pixmap with letter size aspect ratio (8.5x11)
    QPixmap previewPixmap(850, 1100);  // 850x1100 maintains letter ratio
    previewPixmap.fill(Qt::white);
    
    QPainter previewPainter(&previewPixmap);
    if (!previewPainter.isActive()) {
        qDebug() << "Failed to create preview painter";
        return;
    }
    
    // Set up the painter similar to PDF generation
    QRect pageRect(0, 0, 850, 1100);
    
    // Use the first order for preview
    Order firstOrder = sortedOrders.first();
    
    // Draw the invoice using the same template and logic as PDF generator
    drawInvoicePreview(previewPainter, firstOrder, pageRect);
    
    previewPainter.end();
    
    // Scale pixmap to fit the label while maintaining aspect ratio
    QSize labelSize = m_previewScrollArea->size() - QSize(40, 60); // Account for margins and button
    QPixmap scaledPixmap = previewPixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    m_pdfPreviewLabel->setPixmap(scaledPixmap);
    m_pdfPreviewLabel->setText(""); // Clear any existing text
    
    // Add button to open in external viewer
    if (!m_previewScrollArea->findChild<QPushButton*>("openExternalButton")) {
        QPushButton* openButton = new QPushButton("Open in External Viewer", m_previewScrollArea);
        openButton->setObjectName("openExternalButton");
        openButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #4A90E2;"
            "   color: white;"
            "   border: none;"
            "   padding: 8px 16px;"
            "   border-radius: 4px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #357ABD;"
            "}"
        );
        openButton->move(10, 10);
        openButton->show();
        connect(openButton, &QPushButton::clicked, [pdfPath]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));
        });
    }
}

void MainWindow::drawInvoicePreview(QPainter& painter, const Order& order, const QRect& pageRect) {
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    
    // Calculate positions as percentages of page size (same as PDF generator)
    double scaleX = pageWidth / 700.0;  // Template preview width
    double scaleY = pageHeight / 776.0; // Template preview height
    
    // Set up fonts (same as PDF generator)
    QFont titleFont("Arial", 16, QFont::Bold);  // Reduced to match PDF generator
    QFont headerFont("Arial", 10, QFont::Bold); // Reduced to match PDF generator
    QFont bodyFont("Arial", 8);                 // Reduced to match PDF generator
    
    // Draw logo if available
    if (!m_template.logoPath.isEmpty()) {
        QPixmap logo(m_template.logoPath);
        if (!logo.isNull()) {
            QRect logoRect(m_template.logoPosition.x() * scaleX, 
                          m_template.logoPosition.y() * scaleY,
                          m_template.logoPosition.width() * scaleX,
                          m_template.logoPosition.height() * scaleY);
            painter.drawPixmap(logoRect, logo, logo.rect());
        }
    }
    
    // Draw header
    painter.setFont(headerFont);
    
    // Order number
    QPoint orderPos(m_template.orderNumberPos.x() * scaleX, m_template.orderNumberPos.y() * scaleY);
    QString orderText = QString("Order %1").arg(order.orderId);
    painter.drawText(orderPos, orderText);
    
    // Date
    QPoint datePos(m_template.datePos.x() * scaleX, m_template.datePos.y() * scaleY);
    QString dateStr = order.createdAt.toString("MM/dd/yyyy");
    painter.drawText(datePos, QString("Date: %1").arg(dateStr));
    
    // Draw billing info
    painter.setFont(bodyFont);
    QPoint billingBasePos(m_template.billingNamePos.x() * scaleX, m_template.billingNamePos.y() * scaleY);
    painter.drawText(billingBasePos, "Bill To:");
    
    // Customer name
    int yOffset = 20 * scaleY;
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), order.billingName);
    
    // Address
    yOffset += 15 * scaleY;
    QString address = order.billingAddress1;
    if (!order.billingAddress2.isEmpty()) {
        address += ", " + order.billingAddress2;
    }
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), address);
    
    // City, State, ZIP
    yOffset += 15 * scaleY;
    QString cityStateZip = QString("%1, %2 %3")
                          .arg(order.billingCity)
                          .arg(order.billingProvince)
                          .arg(order.billingZip);
    painter.drawText(QPoint(billingBasePos.x(), billingBasePos.y() + yOffset), cityStateZip);
    
    // Draw line items table
    int tableStartY = m_template.tableStartPos.y() * scaleY;
    int lineHeight = m_template.rowHeight * scaleY;
    
    painter.setFont(headerFont);
    
    // Column positions
    int tableX = m_template.tableStartPos.x() * scaleX;
    int qtyX = tableX + 15 * scaleX;
    int descX = tableX + 60 * scaleX;
    int priceX = tableX + 450 * scaleX;
    int totalX = tableX + 550 * scaleX;
    
    // Table headers
    painter.drawText(QPoint(qtyX, tableStartY), "Qty");
    painter.drawText(QPoint(descX, tableStartY), "Description");
    painter.drawText(QPoint(priceX, tableStartY), "Unit Price");
    painter.drawText(QPoint(totalX, tableStartY), "Line Total");
    
    // Header line
    int headerLineY = tableStartY + 15 * scaleY;
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(tableX, headerLineY, tableX + 630 * scaleX, headerLineY);
    painter.setPen(QPen(Qt::black, 1));
    
    // Line items
    painter.setFont(bodyFont);
    int currentY = headerLineY + 10 * scaleY;
    
    // Calculate adaptive spacing and sizing (same as PDF generator)
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
    
    for (const auto& item : order.lineItems) {
        // Calculate dynamic height for wrapped text
        QRect descBounds(descX, 0, 380 * scaleX, 1000);
        QRect actualDescRect = painter.boundingRect(descBounds, Qt::AlignLeft | Qt::TextWordWrap, item.description);
        int calculatedHeight = (int)(actualDescRect.height() + 2 * scaleY); // Reduced padding
        int actualRowHeight = qMax(adaptiveRowHeight, qMin(calculatedHeight, adaptiveRowHeight * 3)); // Cap max height to 3x adaptive
        
        // Check for highlighting
        QColor bgColor;
        // Simple highlighting check without accessing PDF generator's private rules
        if (item.description.contains("goldenrod", Qt::CaseInsensitive)) {
            bgColor = QColor("#DAA520");
        } else if (item.description.contains("huckleberry", Qt::CaseInsensitive)) {
            bgColor = QColor("#8A2BE2");
        }
        
        // Calculate consistent positioning for all elements (same as PDF generator)
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
        
        currentY += actualRowHeight + dynamicSpacing;
    }
    
    // Draw totals with page bounds checking
    int totalsX = pageWidth * 0.65;
    int totalsWidth = pageWidth * 0.3;
    int reservedFooterSpace = 100; // Reserve space for totals + footer
    int maxContentY = pageHeight - reservedFooterSpace;
    int totalsStartY = qMin((int)(currentY + 20 * scaleY), maxContentY - 80);  // Ensure totals fit
    int totalsLineHeight = pageHeight * 0.025;
    
    painter.setFont(bodyFont);
    
    // Subtotal
    QRect subtotalLabelRect(totalsX, totalsStartY, totalsWidth * 0.6, totalsLineHeight);
    QRect subtotalValueRect(totalsX + totalsWidth * 0.6, totalsStartY, totalsWidth * 0.4, totalsLineHeight);
    painter.drawText(subtotalLabelRect, Qt::AlignLeft, "Subtotal:");
    painter.drawText(subtotalValueRect, Qt::AlignRight, QString("$%1").arg(order.subtotal, 0, 'f', 2));
    
    // Tax
    int taxY = totalsStartY + totalsLineHeight + 5;
    QRect taxLabelRect(totalsX, taxY, totalsWidth * 0.6, totalsLineHeight);
    QRect taxValueRect(totalsX + totalsWidth * 0.6, taxY, totalsWidth * 0.4, totalsLineHeight);
    painter.drawText(taxLabelRect, Qt::AlignLeft, "Tax:");
    painter.drawText(taxValueRect, Qt::AlignRight, QString("$%1").arg(order.taxes, 0, 'f', 2));
    
    // Total
    int totalY = taxY + totalsLineHeight + 10;
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(totalsX, totalY - 5, totalsX + totalsWidth, totalY - 5);
    painter.setPen(QPen(Qt::black, 1));
    
    painter.setFont(headerFont);
    QRect totalLabelRect(totalsX, totalY, totalsWidth * 0.6, totalsLineHeight);
    QRect totalValueRect(totalsX + totalsWidth * 0.6, totalY, totalsWidth * 0.4, totalsLineHeight);
    painter.drawText(totalLabelRect, Qt::AlignLeft, "Total:");
    painter.drawText(totalValueRect, Qt::AlignRight, QString("$%1").arg(order.total, 0, 'f', 2));
    
    // Draw footer dynamically after totals
    painter.setFont(bodyFont);
    
    // Position footer with page bounds checking
    int footerStartY = qMin((int)(totalY + totalsLineHeight + 10 * scaleY), maxContentY - 40);  // Reduced gap if needed, ensure footer fits
    int thankYouY = footerStartY;
    int policyY = footerStartY + 25 * scaleY;  // Reduced spacing between footer elements
    
    // Final bounds check
    int maxFooterY = pageHeight - 20 * scaleY;  // 20px from bottom edge
    if (policyY + 20 * scaleY > maxFooterY) {
        thankYouY = maxFooterY - 45 * scaleY;  // Make room for both elements
        policyY = thankYouY + 25 * scaleY;
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