#include "TemplateEditor.h"
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QApplication>

// TemplatePreview implementation
TemplatePreview::TemplatePreview(QWidget* parent)
    : QWidget(parent)
    , m_dragging(false)
    , m_scaleFactor(0.4)
{
    // Make size responsive
    setMinimumSize(400, 500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    
    // Set default template to exactly match config.xlsx Invoice sheet layout
    // Based on Python code: B2=order#, G5=date, B3-B5=billing, B8+=line items, H37-39=totals
    
    m_template.logoPosition = QRect(590, 20, 100, 100);  // Far right for 700px width
    
    // Header positions - align order and billing on left side
    m_template.orderNumberPos = QPoint(50, 30);    // Move order to left side
    m_template.datePos = QPoint(450, 120);         // Move date farther right
    
    // Billing info (Excel B3-B5) - align with order
    m_template.billingNamePos = QPoint(50, 70);    // B3 position, aligned with order
    
    // Table starts right after billing info
    m_template.tableStartPos = QPoint(50, 140);   // Moved up - right after billing info 
    m_template.rowHeight = 14;  // Ultra-compact rows for large orders
    
    // Column layout: B=Qty, C=Description, F=Unit Price, G=Line Total (wider template)
    m_template.columnWidths.quantity = 50;        // Column B
    m_template.columnWidths.description = 380;    // Column C to F span (wider)
    m_template.columnWidths.unitPrice = 80;       // Column F  
    m_template.columnWidths.lineTotal = 80;       // Column G
    
    // Totals at Excel H36-H39 (adjusted for wider template)
    m_template.subtotalPos = QPoint(580, 480);    // H37 position (moved right)
    m_template.taxPos = QPoint(580, 500);         // H38 position (moved right)
    m_template.totalPos = QPoint(580, 520);       // H39 position (moved right)
    
    // Footer positions - safe margin from bottom
    m_template.thankYouPos = QPoint(50, 680);     // Footer positioning
    m_template.policyPos = QPoint(50, 710);       // Policy text below thank you
    
    // Default footer text from config.xlsx
    m_template.thankYouText = "Thank you for your order.\nHappy planting!";
    m_template.policyText = "We do not offer refunds once you have left the premises. All sales are final. "
                           "Please check the contents of your order carefully to make sure there are no errors. "
                           "Staff are available to assist and correct errors.";
}

void TemplatePreview::setTemplate(const InvoiceTemplate& templ) {
    m_template = templ;
    
    if (!m_template.logoPath.isEmpty()) {
        m_logo = QPixmap(m_template.logoPath);
    }
    
    update();
    emit templateChanged();
}

void TemplatePreview::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // Use appropriate background color based on current palette
    QColor backgroundColor = palette().color(QPalette::Base);
    if (backgroundColor.lightness() < 128) {
        // Dark mode - use a lighter background for the "paper"
        painter.fillRect(rect(), QColor(60, 60, 60));
    } else {
        // Light mode - use white
        painter.fillRect(rect(), Qt::white);
    }
    
    // Calculate dynamic scale factor based on widget size  
    double scaleX = (double)width() / 750.0;  // Based on template width (now 700px)
    double scaleY = (double)height() / 850.0; // Based on template height
    m_scaleFactor = qMin(scaleX, scaleY) * 0.9; // Leave some margin
    
    // Ensure minimum scale for readability
    m_scaleFactor = qMax(0.2, m_scaleFactor);
    
    // Apply scale factor
    painter.scale(m_scaleFactor, m_scaleFactor);
    
    drawTemplate(painter);
}

void TemplatePreview::drawTemplate(QPainter& painter) {
    // Determine if we're in dark mode
    bool isDarkMode = palette().color(QPalette::Base).lightness() < 128;
    
    // Use appropriate colors for dark/light mode
    QColor textColor = isDarkMode ? Qt::white : Qt::black;
    QColor borderColor = isDarkMode ? QColor(200, 200, 200) : Qt::black;
    
    // Draw page border (standard 8.5x11 letter size ratio)
    painter.setPen(QPen(borderColor, 2));
    painter.drawRect(10, 10, 700, 776); // Wider template for better layout
    
    // Set text color for all text
    painter.setPen(textColor);
    
    // Draw logo placeholder
    QRect logoRect = m_template.logoPosition;
    if (!m_logo.isNull()) {
        painter.drawPixmap(logoRect, m_logo);
    } else {
        painter.setPen(QPen(borderColor, 1, Qt::DashLine));
        painter.drawRect(logoRect);
        painter.setPen(textColor);
        painter.drawText(logoRect, Qt::AlignCenter, "Logo");
    }
    
    painter.setPen(textColor);
    
    // No title - config.xlsx template doesn't have "INVOICE" title
    
    // Draw order info
    QFont headerFont("Arial", 10);  // Reduced to match PDF generator
    painter.setFont(headerFont);
    
    // Draw "Order <number>" with proper spacing on left side
    QString orderNum = m_previewOrder.orderId.isEmpty() ? "250054" : m_previewOrder.orderId;
    QString orderText = QString("Order %1").arg(orderNum);  // Add space between Order and number
    painter.drawText(m_template.orderNumberPos, orderText);
    
    QString dateText = "Date: " + (m_previewOrder.createdAt.isValid() ? m_previewOrder.createdAt.toString("MM/dd/yyyy") : "01/01/2024");
    painter.drawText(m_template.datePos, dateText);
    
    // Draw billing info
    QFont bodyFont("Arial", 8);  // Reduced to match PDF generator
    painter.setFont(bodyFont);
    
    painter.drawText(m_template.billingNamePos, "Bill To:");
    
    int yOffset = 20;
    QString name = m_previewOrder.billingName.isEmpty() ? "John Doe" : m_previewOrder.billingName;
    painter.drawText(m_template.billingNamePos + QPoint(0, yOffset), name);
    
    yOffset += 15;
    QString address = m_previewOrder.billingAddress1.isEmpty() ? "123 Main St" : m_previewOrder.billingAddress1;
    painter.drawText(m_template.billingNamePos + QPoint(0, yOffset), address);
    
    yOffset += 15;
    QString cityState = m_previewOrder.billingCity.isEmpty() ? "City, ST 12345" : 
                       QString("%1, %2 %3").arg(m_previewOrder.billingCity, m_previewOrder.billingProvince, m_previewOrder.billingZip);
    painter.drawText(m_template.billingNamePos + QPoint(0, yOffset), cityState);
    
    // Draw table header (within page bounds)
    int tableY = m_template.tableStartPos.y();
    int tableX = m_template.tableStartPos.x();
    
    painter.setFont(QFont("Arial", 8, QFont::Bold));  // Reduced table header font
    painter.drawText(QPoint(tableX, tableY), "Qty");
    painter.drawText(QPoint(tableX + 60, tableY), "Description");
    painter.drawText(QPoint(tableX + 450, tableY), "Unit Price");
    painter.drawText(QPoint(tableX + 550, tableY), "Line Total");
    
    // Draw line under header (stay within page - wider now)
    tableY += 15;
    painter.drawLine(tableX, tableY, tableX + 630, tableY);
    
    // Draw sample line items
    painter.setFont(bodyFont);
    tableY += 10;
    
    if (m_previewOrder.lineItems.isEmpty()) {
        // Sample data matching CSV examples (stay within page bounds)
        painter.drawText(QPoint(tableX + 15, tableY), "3");
        painter.drawText(QPoint(tableX + 60, tableY), "Canada Goldenrod - Solidago lepida, bundle of 5");
        painter.drawText(QPoint(tableX + 450, tableY), "$10.00");
        painter.drawText(QPoint(tableX + 550, tableY), "$30.00");
        tableY += m_template.rowHeight;
        
        painter.drawText(QPoint(tableX + 15, tableY), "1");
        painter.drawText(QPoint(tableX + 60, tableY), "Evergreen Huckleberry - Vaccinium ovatum, bundle of 5");
        painter.drawText(QPoint(tableX + 450, tableY), "$43.00");
        painter.drawText(QPoint(tableX + 550, tableY), "$43.00");
    } else {
        for (const auto& item : m_previewOrder.lineItems) {
            painter.drawText(QPoint(tableX + 15, tableY), QString::number(item.quantity));
            
            // Keep description within page bounds (wider column now)
            QRect descRect(tableX + 60, tableY - 10, 380, m_template.rowHeight);
            painter.drawText(descRect, Qt::AlignLeft | Qt::TextWordWrap, item.description);
            
            painter.drawText(QPoint(tableX + 450, tableY), QString("$%1").arg(item.unitPrice, 0, 'f', 2));
            painter.drawText(QPoint(tableX + 550, tableY), QString("$%1").arg(item.lineTotal(), 0, 'f', 2));
            
            tableY += m_template.rowHeight;
        }
    }
    
    // Draw totals and footer dynamically positioned after line items (matching PDF generator behavior)
    painter.setFont(bodyFont);
    
    double subtotal = m_previewOrder.subtotal > 0 ? m_previewOrder.subtotal : 73.00;
    double tax = m_previewOrder.taxes > 0 ? m_previewOrder.taxes : 4.82;
    double total = m_previewOrder.total > 0 ? m_previewOrder.total : 77.82;
    
    // Position totals dynamically after line items (add some spacing)
    int totalsStartY = tableY + 20; // 20px spacing after line items
    int totalsX = 450; // Right-aligned position (matching template column layout)
    int lineSpacing = 20;
    
    // Subtotal
    QRect subtotalLabelRect(totalsX - 80, totalsStartY, 70, 20);
    QRect subtotalValueRect(totalsX, totalsStartY, 80, 20);
    painter.drawText(subtotalLabelRect, Qt::AlignLeft, "Subtotal:");
    painter.drawText(subtotalValueRect, Qt::AlignRight, QString("$%1").arg(subtotal, 0, 'f', 2));
    
    // Tax
    int taxY = totalsStartY + lineSpacing;
    QRect taxLabelRect(totalsX - 80, taxY, 70, 20);
    QRect taxValueRect(totalsX, taxY, 80, 20);
    painter.drawText(taxLabelRect, Qt::AlignLeft, "Tax:");
    painter.drawText(taxValueRect, Qt::AlignRight, QString("$%1").arg(tax, 0, 'f', 2));
    
    // Total (with bold font and line above)
    int totalY = taxY + lineSpacing + 5;
    
    // Draw line above total
    painter.setPen(QPen(borderColor, 2));
    painter.drawLine(totalsX - 80, totalY - 3, totalsX + 80, totalY - 3);
    painter.setPen(textColor);
    
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    QRect totalLabelRect(totalsX - 80, totalY, 70, 20);
    QRect totalValueRect(totalsX, totalY, 80, 20);
    painter.drawText(totalLabelRect, Qt::AlignLeft, "Total:");
    painter.drawText(totalValueRect, Qt::AlignRight, QString("$%1").arg(total, 0, 'f', 2));
    
    // Draw footer text dynamically positioned after totals (matching PDF generator behavior)
    painter.setFont(bodyFont);
    
    int footerStartY = totalY + 40; // 40px spacing after totals
    
    // Thank you message (editable)
    QRect thankYouRect(50, footerStartY, 300, 40);
    painter.drawText(thankYouRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.thankYouText);
    
    // Policy text (editable)
    QRect policyRect(50, footerStartY + 50, 500, 60);
    painter.drawText(policyRect, Qt::AlignLeft | Qt::TextWordWrap, m_template.policyText);
    
    // Highlight selected element
    if (!m_selectedElement.isEmpty()) {
        QColor highlightColor = isDarkMode ? QColor(100, 150, 255) : Qt::blue;
        painter.setPen(QPen(highlightColor, 2));
        painter.setBrush(Qt::NoBrush);
        
        if (m_selectedElement == "logo") {
            painter.drawRect(m_template.logoPosition);
        } else if (m_selectedElement == "orderNumber") {
            painter.drawRect(m_template.orderNumberPos.x() - 5, m_template.orderNumberPos.y() - 15, 150, 20);
        } else if (m_selectedElement == "date") {
            painter.drawRect(m_template.datePos.x() - 5, m_template.datePos.y() - 15, 120, 20);
        } else if (m_selectedElement == "billing") {
            painter.drawRect(m_template.billingNamePos.x() - 5, m_template.billingNamePos.y() - 15, 200, 60);
        } else if (m_selectedElement == "table") {
            painter.drawRect(m_template.tableStartPos.x() - 5, m_template.tableStartPos.y() - 15, 500, 20);
        } else if (m_selectedElement == "totals") {
            // Highlight dynamic totals area
            int lineItemsEndY = m_template.tableStartPos.y() + 15;
            if (m_previewOrder.lineItems.isEmpty()) {
                lineItemsEndY += 2 * m_template.rowHeight;
            } else {
                lineItemsEndY += m_previewOrder.lineItems.size() * m_template.rowHeight;
            }
            int totalsStartY = lineItemsEndY + 20;
            painter.drawRect(450 - 80, totalsStartY - 12, 160, 80);
        } else if (m_selectedElement == "footer") {
            // Highlight dynamic footer area
            int lineItemsEndY = m_template.tableStartPos.y() + 15;
            if (m_previewOrder.lineItems.isEmpty()) {
                lineItemsEndY += 2 * m_template.rowHeight;
            } else {
                lineItemsEndY += m_previewOrder.lineItems.size() * m_template.rowHeight;
            }
            int footerStartY = lineItemsEndY + 20 + 80;
            painter.drawRect(50 - 5, footerStartY - 12, 500, 120);
        }
    }
}

void TemplatePreview::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Convert screen coordinates to template coordinates
        QPoint screenPos = event->pos();
        QPoint templatePoint = QPoint(screenPos.x() / m_scaleFactor, screenPos.y() / m_scaleFactor);
        m_selectedElement = getElementAtPoint(templatePoint);
        m_dragging = !m_selectedElement.isEmpty();
        m_dragStart = screenPos;
        
        // Debug coordinate conversion issues - elements jumping when clicked
        // qDebug() << "Click: screen=" << screenPos << "template=" << templatePoint << "scaleFactor=" << m_scaleFactor;
        
        emit elementSelected(m_selectedElement);
        update();
    }
}

void TemplatePreview::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && !m_selectedElement.isEmpty()) {
        // Convert screen pixel movement to template coordinate movement
        QPoint screenDelta = QPoint(event->pos().x() - m_dragStart.x(), 
                                   event->pos().y() - m_dragStart.y());
        QPoint delta = QPoint(screenDelta.x() / m_scaleFactor, screenDelta.y() / m_scaleFactor);
        
        if (m_selectedElement == "logo") {
            // Only move the logo, preserve original size exactly
            QRect oldRect = m_template.logoPosition;
            QPoint newTopLeft = m_template.logoPosition.topLeft() + delta;
            QSize originalSize = m_template.logoPosition.size();  // Should be 100x100
            m_template.logoPosition = QRect(newTopLeft, originalSize);
            // Logo movement debug removed - positioning is now working correctly
        } else if (m_selectedElement == "orderNumber") {
            m_template.orderNumberPos += delta;
        } else if (m_selectedElement == "date") {
            m_template.datePos += delta;
        } else if (m_selectedElement == "billing") {
            m_template.billingNamePos += delta;
        } else if (m_selectedElement == "table") {
            m_template.tableStartPos += delta;
        } else if (m_selectedElement == "totals") {
            m_template.subtotalPos += delta;
            m_template.taxPos += delta;
            m_template.totalPos += delta;
        }
        
        // Update drag start to current position for next move event
        m_dragStart = event->pos();
        update();
        emit templateChanged();
    }
}

void TemplatePreview::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    if (m_dragging) {
        m_dragging = false;
        // Final template change signal after drag is complete
        emit templateChanged();
    }
}

QString TemplatePreview::getElementAtPoint(const QPoint& point) const {
    // Check logo
    if (m_template.logoPosition.contains(point)) {
        return "logo";
    }
    
    // Check order number (point is text baseline, so offset up by font height)
    QRect orderRect(m_template.orderNumberPos.x() - 5, m_template.orderNumberPos.y() - 12, 150, 16);
    if (orderRect.contains(point)) {
        return "orderNumber";
    }
    
    // Check date (point is text baseline, so offset up by font height)
    QRect dateRect(m_template.datePos.x() - 5, m_template.datePos.y() - 12, 120, 16);
    if (dateRect.contains(point)) {
        return "date";
    }
    
    // Check billing area (includes "Bill To:" label and address lines)
    QRect billingRect(m_template.billingNamePos.x() - 5, m_template.billingNamePos.y() - 12, 200, 60);
    if (billingRect.contains(point)) {
        return "billing";
    }
    
    // Check table area (table headers are drawn at tableStartPos)
    QRect tableRect(m_template.tableStartPos.x() - 5, m_template.tableStartPos.y() - 12, 500, 16);
    if (tableRect.contains(point)) {
        return "table";
    }
    
    // Calculate dynamic totals position (matching drawTemplate logic)
    int lineItemsEndY = m_template.tableStartPos.y() + 15; // Header
    if (m_previewOrder.lineItems.isEmpty()) {
        lineItemsEndY += 2 * m_template.rowHeight; // Sample items
    } else {
        lineItemsEndY += m_previewOrder.lineItems.size() * m_template.rowHeight;
    }
    
    int totalsStartY = lineItemsEndY + 20;
    int totalsX = 450;
    
    // Check totals area (dynamic positioning)
    QRect totalsRect(totalsX - 80, totalsStartY - 12, 160, 80); // Cover all 3 total lines
    if (totalsRect.contains(point)) {
        return "totals";
    }
    
    // Check footer area (dynamic positioning after totals)
    int footerStartY = totalsStartY + 80; // After totals section
    QRect footerRect(50, footerStartY - 12, 500, 120); // Cover both footer sections
    if (footerRect.contains(point)) {
        return "footer";
    }
    
    return "";
}

// TemplateEditor implementation
TemplateEditor::TemplateEditor(QWidget* parent)
    : QWidget(parent)
    , m_selectedElement("")
{
    setupUI();
}

void TemplateEditor::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Create splitter for responsive layout
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    
    // Preview area
    m_preview = new TemplatePreview();
    m_previewArea = new QScrollArea();
    m_previewArea->setWidget(m_preview);
    m_previewArea->setWidgetResizable(true);
    m_previewArea->setMinimumSize(350, 400);
    
    connect(m_preview, &TemplatePreview::templateChanged, this, &TemplateEditor::templateChanged);
    connect(m_preview, &TemplatePreview::elementSelected, this, &TemplateEditor::onElementSelected);
    
    // Properties panel
    // Logo group
    m_logoGroup = new QGroupBox("Logo");
    QVBoxLayout* logoLayout = new QVBoxLayout(m_logoGroup);
    
    m_logoPathEdit = new QLineEdit();
    m_logoButton = new QPushButton("Browse...");
    connect(m_logoButton, &QPushButton::clicked, this, &TemplateEditor::onLogoButtonClicked);
    
    logoLayout->addWidget(new QLabel("Logo File:"));
    logoLayout->addWidget(m_logoPathEdit);
    logoLayout->addWidget(m_logoButton);
    
    // Position group
    m_positionGroup = new QGroupBox("Position & Size");
    m_positionLayout = new QGridLayout(m_positionGroup);
    
    m_xSpinBox = new QSpinBox();
    m_xSpinBox->setRange(0, 1000);
    m_ySpinBox = new QSpinBox();
    m_ySpinBox->setRange(0, 1000);
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(10, 500);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(10, 500);
    
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TemplateEditor::onPropertyChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TemplateEditor::onPropertyChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TemplateEditor::onPropertyChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TemplateEditor::onPropertyChanged);
    
    m_positionLayout->addWidget(new QLabel("X:"), 0, 0);
    m_positionLayout->addWidget(m_xSpinBox, 0, 1);
    m_positionLayout->addWidget(new QLabel("Y:"), 1, 0);
    m_positionLayout->addWidget(m_ySpinBox, 1, 1);
    m_positionLayout->addWidget(new QLabel("Width:"), 2, 0);
    m_positionLayout->addWidget(m_widthSpinBox, 2, 1);
    m_positionLayout->addWidget(new QLabel("Height:"), 3, 0);
    m_positionLayout->addWidget(m_heightSpinBox, 3, 1);
    
    // Text editing group with improved styling
    m_textGroup = new QGroupBox("Footer Text");
    m_textGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 8px; } QGroupBox::title { margin-left: 8px; }");
    QVBoxLayout* textLayout = new QVBoxLayout(m_textGroup);
    textLayout->setSpacing(8);
    
    // Thank You Message section
    QLabel* thankYouLabel = new QLabel("Thank You Message:");
    thankYouLabel->setStyleSheet("font-weight: normal; color: #666;");
    textLayout->addWidget(thankYouLabel);
    
    m_thankYouEdit = new QTextEdit();
    m_thankYouEdit->setFixedHeight(75);
    m_thankYouEdit->setStyleSheet(
        "QTextEdit {"
        "    border: 2px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    background-color: white;"
        "    color: black;"
        "    font-family: Arial;"
        "    font-size: 11px;"
        "}"
        "QTextEdit:focus {"
        "    border-color: #4A90E2;"
        "}"
    );
    m_thankYouEdit->setPlainText(m_template.thankYouText);
    connect(m_thankYouEdit, &QTextEdit::textChanged, this, &TemplateEditor::onPropertyChanged);
    textLayout->addWidget(m_thankYouEdit);
    
    // Policy Text section
    QLabel* policyLabel = new QLabel("Policy Text:");
    policyLabel->setStyleSheet("font-weight: normal; color: #666; margin-top: 8px;");
    textLayout->addWidget(policyLabel);
    
    m_policyEdit = new QTextEdit();
    m_policyEdit->setFixedHeight(95);
    m_policyEdit->setStyleSheet(
        "QTextEdit {"
        "    border: 2px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    background-color: white;"
        "    color: black;"
        "    font-family: Arial;"
        "    font-size: 11px;"
        "}"
        "QTextEdit:focus {"
        "    border-color: #4A90E2;"
        "}"
    );
    m_policyEdit->setPlainText(m_template.policyText);
    connect(m_policyEdit, &QTextEdit::textChanged, this, &TemplateEditor::onPropertyChanged);
    textLayout->addWidget(m_policyEdit);
    
    // Reset button
    m_resetButton = new QPushButton("Reset to Defaults");
    connect(m_resetButton, &QPushButton::clicked, this, &TemplateEditor::resetToDefaults);
    
    // Create tabbed properties panel
    m_propertiesTab = new QTabWidget();
    m_propertiesTab->setMinimumWidth(200);
    m_propertiesTab->setMaximumWidth(320);
    
    // Logo tab
    m_logoTab = new QWidget();
    QVBoxLayout* logoTabLayout = new QVBoxLayout(m_logoTab);
    logoTabLayout->addWidget(m_logoGroup);
    logoTabLayout->addStretch();
    m_propertiesTab->addTab(m_logoTab, "Logo");
    
    // Positioning tab
    m_positionTab = new QWidget();
    QVBoxLayout* positionTabLayout = new QVBoxLayout(m_positionTab);
    positionTabLayout->addWidget(m_positionGroup);
    positionTabLayout->addStretch();
    m_propertiesTab->addTab(m_positionTab, "Position");
    
    // Footer tab
    m_footerTab = new QWidget();
    QVBoxLayout* footerTabLayout = new QVBoxLayout(m_footerTab);
    footerTabLayout->addWidget(m_textGroup);
    
    // Add reset button to footer tab since it's most commonly used
    footerTabLayout->addWidget(m_resetButton);
    footerTabLayout->addStretch();
    m_propertiesTab->addTab(m_footerTab, "Footer");
    
    // Add widgets to splitter
    splitter->addWidget(m_previewArea);
    splitter->addWidget(m_propertiesTab);
    
    // Set splitter proportions (75% preview, 25% properties)
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    // Layout main content
    m_mainLayout->addWidget(splitter);
    
    // Initially disable position controls
    m_positionGroup->setEnabled(false);
}

void TemplateEditor::setTemplate(const InvoiceTemplate& templ) {
    m_template = templ;
    m_preview->setTemplate(templ);
    m_logoPathEdit->setText(templ.logoPath);
    
    // Update text fields
    m_thankYouEdit->setPlainText(templ.thankYouText);
    m_policyEdit->setPlainText(templ.policyText);
}

InvoiceTemplate TemplateEditor::getTemplate() const {
    return m_preview->getTemplate();
}

void TemplateEditor::setPreviewOrder(const Order& order) {
    m_preview->setPreviewOrder(order);
}

void TemplateEditor::onLogoButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, 
        "Select Logo Image", "", "Image Files (*.png *.jpg *.jpeg *.bmp)");
    
    if (!fileName.isEmpty()) {
        m_logoPathEdit->setText(fileName);
        m_template.logoPath = fileName;
        m_preview->setTemplate(m_template);
        emit templateChanged();
    }
}

void TemplateEditor::onElementSelected(const QString& elementName) {
    m_selectedElement = elementName;
    updatePropertyEditor(elementName);
}

void TemplateEditor::onPropertyChanged() {
    applyPropertyChanges();
}

void TemplateEditor::updatePropertyEditor(const QString& elementName) {
    m_positionGroup->setEnabled(!elementName.isEmpty());
    
    if (elementName.isEmpty()) return;
    
    m_positionGroup->setTitle(QString("Position & Size - %1").arg(elementName));
    
    // Temporarily block signals to prevent triggering property changes during update
    m_xSpinBox->blockSignals(true);
    m_ySpinBox->blockSignals(true);
    m_widthSpinBox->blockSignals(true);
    m_heightSpinBox->blockSignals(true);
    
    if (elementName == "logo") {
        m_xSpinBox->setValue(m_template.logoPosition.x());
        m_ySpinBox->setValue(m_template.logoPosition.y());
        m_widthSpinBox->setValue(m_template.logoPosition.width());
        m_heightSpinBox->setValue(m_template.logoPosition.height());
        m_widthSpinBox->setEnabled(true);
        m_heightSpinBox->setEnabled(true);
    } else if (elementName == "orderNumber") {
        m_xSpinBox->setValue(m_template.orderNumberPos.x());
        m_ySpinBox->setValue(m_template.orderNumberPos.y());
        m_widthSpinBox->setEnabled(false);
        m_heightSpinBox->setEnabled(false);
    } else if (elementName == "date") {
        m_xSpinBox->setValue(m_template.datePos.x());
        m_ySpinBox->setValue(m_template.datePos.y());
        m_widthSpinBox->setEnabled(false);
        m_heightSpinBox->setEnabled(false);
    } else if (elementName == "billing") {
        m_xSpinBox->setValue(m_template.billingNamePos.x());
        m_ySpinBox->setValue(m_template.billingNamePos.y());
        m_widthSpinBox->setEnabled(false);
        m_heightSpinBox->setEnabled(false);
    } else if (elementName == "table") {
        m_xSpinBox->setValue(m_template.tableStartPos.x());
        m_ySpinBox->setValue(m_template.tableStartPos.y());
        m_widthSpinBox->setEnabled(false);
        m_heightSpinBox->setEnabled(false);
    } else if (elementName == "totals") {
        m_xSpinBox->setValue(m_template.subtotalPos.x());
        m_ySpinBox->setValue(m_template.subtotalPos.y());
        m_widthSpinBox->setEnabled(false);
        m_heightSpinBox->setEnabled(false);
    }
    
    // Re-enable signals after updating values
    m_xSpinBox->blockSignals(false);
    m_ySpinBox->blockSignals(false);
    m_widthSpinBox->blockSignals(false);
    m_heightSpinBox->blockSignals(false);
}

void TemplateEditor::applyPropertyChanges() {
    if (m_selectedElement.isEmpty()) return;
    
    // Get current template to avoid overwrites
    InvoiceTemplate currentTemplate = m_preview->getTemplate();
    
    if (m_selectedElement == "logo") {
        // Only update position and size if they've actually changed
        int newX = m_xSpinBox->value();
        int newY = m_ySpinBox->value();
        int newW = m_widthSpinBox->value();
        int newH = m_heightSpinBox->value();
        
        if (newX != currentTemplate.logoPosition.x() || 
            newY != currentTemplate.logoPosition.y() ||
            newW != currentTemplate.logoPosition.width() ||
            newH != currentTemplate.logoPosition.height()) {
            currentTemplate.logoPosition = QRect(newX, newY, newW, newH);
        }
    } else if (m_selectedElement == "orderNumber") {
        QPoint newPos(m_xSpinBox->value(), m_ySpinBox->value());
        if (newPos != currentTemplate.orderNumberPos) {
            currentTemplate.orderNumberPos = newPos;
        }
    } else if (m_selectedElement == "date") {
        QPoint newPos(m_xSpinBox->value(), m_ySpinBox->value());
        if (newPos != currentTemplate.datePos) {
            currentTemplate.datePos = newPos;
        }
    } else if (m_selectedElement == "billing") {
        QPoint newPos(m_xSpinBox->value(), m_ySpinBox->value());
        if (newPos != currentTemplate.billingNamePos) {
            currentTemplate.billingNamePos = newPos;
        }
    } else if (m_selectedElement == "table") {
        QPoint newPos(m_xSpinBox->value(), m_ySpinBox->value());
        if (newPos != currentTemplate.tableStartPos) {
            currentTemplate.tableStartPos = newPos;
        }
    } else if (m_selectedElement == "totals") {
        QPoint newPos(m_xSpinBox->value(), m_ySpinBox->value());
        QPoint delta = newPos - currentTemplate.subtotalPos;
        if (delta != QPoint(0, 0)) {
            currentTemplate.subtotalPos += delta;
            currentTemplate.taxPos += delta;
            currentTemplate.totalPos += delta;
        }
    }
    
    // Always update text fields and logo path (not position-dependent)
    currentTemplate.logoPath = m_logoPathEdit->text();
    currentTemplate.thankYouText = m_thankYouEdit->toPlainText();
    currentTemplate.policyText = m_policyEdit->toPlainText();
    
    m_template = currentTemplate;
    m_preview->setTemplate(currentTemplate);
    emit templateChanged();
}

void TemplateEditor::resetToDefaults() {
    InvoiceTemplate defaultTemplate;
    defaultTemplate.logoPath = m_template.logoPath; // Keep current logo path
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
    
    setTemplate(defaultTemplate);
    emit templateChanged();
}