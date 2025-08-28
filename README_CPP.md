# Invoice Generator - C++ GUI Application

A professional C++ Qt application for generating PDF invoices from CSV order data with a user-friendly graphical interface.

## Features

- **WYSIWYG Template Editor**: Drag-and-drop interface to customize invoice layouts
- **CSV Order Import**: Parse Squarespace-style CSV exports with automatic field mapping
- **Line Item Highlighting**: Configure color-coding rules for different product types
- **PDF Generation**: High-quality PDF output using Qt's built-in printing system
- **Live Preview**: Real-time preview of template changes
- **Cross-Platform**: Works on Windows, macOS, and Linux

## Requirements

### System Dependencies
- Qt5 development libraries
- CMake 3.16 or higher
- C++17 compatible compiler

### Ubuntu/Debian Installation
```bash
sudo apt update
sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools cmake build-essential
```

### Other Platforms
- **Windows**: Install Qt5 from qt.io and Visual Studio/MinGW
- **macOS**: Install Qt5 via Homebrew (`brew install qt5`) and Xcode command line tools

## Building

1. Clone or extract the project
2. Run the build script:
   ```bash
   ./build.sh
   ```
   Or manually:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

## Usage

### 1. Launch the Application
```bash
./build/InvoiceGeneratorGUI
```

### 2. Load Order Data
- Go to the "Orders Data" tab
- Click "Browse..." to select your CSV file (should contain order data with billing info and line items)
- View loaded orders in the list and inspect details

### 3. Customize Template
- Switch to the "Template Editor" tab
- Click and drag elements (logo, order number, date, billing info, etc.) to reposition
- Use the Properties panel to fine-tune positions and sizes
- Browse for a logo image file
- Save/load template configurations as JSON files

### 4. Configure Line Highlighting
- Go to the "Line Highlighting" tab
- Add rules to color-code line items based on description text
- Set colors for different product categories (e.g., "bundle" → yellow, "tree" → green)

### 5. Generate PDF
- Switch to the "Preview & Generate" tab
- Click "Preview PDF" to test with external PDF viewer
- Click "Generate PDF" to create the final multi-page invoice document

## CSV Format

Expected CSV columns (case-insensitive):
- Order ID, Created At
- Billing Name, Billing Address1, Billing Address2, Billing City, Billing Province, Billing Zip
- Lineitem Quantity, Lineitem Name, Lineitem Price, Lineitem SKU
- Subtotal, Shipping, Taxes, Total

## Application Architecture

### Core Components

- **MainWindow**: Tabbed interface coordinating all components
- **CSVParser**: Robust CSV parsing with field normalization
- **TemplateEditor**: WYSIWYG drag-and-drop template designer
- **HighlightManager**: Color-coding rule configuration
- **PDFGenerator**: High-quality PDF output using QPrinter
- **InvoiceData**: Type-safe data structures for orders and templates

### Key Features

- **Drag-and-Drop Template Editing**: Click elements in the preview to select and drag them to new positions
- **Live Preview**: See changes immediately as you modify the template
- **Color Highlighting**: Automatic line item highlighting based on configurable text matching rules
- **Template Persistence**: Save and load custom templates as JSON files
- **External PDF Viewing**: Uses system default PDF viewer for preview (Qt5 compatible)

## File Locations

- **Executable**: `build/InvoiceGeneratorGUI`
- **Sample Data**: `orders.csv` (copied to build directory)
- **Templates**: Save/load as `.json` files anywhere
- **Generated PDFs**: Choose location when generating

## Troubleshooting

### Build Issues
- Ensure Qt5 development packages are installed
- Check CMake version (3.16+)
- Verify C++17 compiler support

### Runtime Issues
- Missing CSV columns: Check that your CSV has required fields
- PDF generation fails: Ensure write permissions to output directory
- Preview not working: Check that system has a default PDF viewer

## Customization

The application can be extended by:
- Adding new template elements in `TemplateEditor`
- Implementing additional CSV formats in `CSVParser`  
- Enhancing PDF styling in `PDFGenerator`
- Adding more highlighting options in `HighlightManager`

## Comparison to Python Version

This C++ version provides:
- **Better Performance**: Native compiled code
- **Professional GUI**: User-friendly drag-and-drop interface
- **No Dependencies**: Self-contained executable (with Qt libs)
- **Better UX**: Real-time preview and visual template editing
- **Cross-Platform**: Consistent experience across OS platforms

The Python version is better for:
- **Quick automation**: Command-line batch processing
- **Excel integration**: Direct Excel file manipulation
- **Simpler deployment**: Single script file