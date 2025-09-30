# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an Invoice Generator that creates professional PDF invoices from CSV order data using Excel templates. The system processes order data from Squarespace exports and generates individual invoices merged into a single multi-page PDF document.

## Development Environment

### Python Installation

**Windows:**
1. Download Python 3.12+ from https://python.org/downloads/
2. **IMPORTANT**: Check "Add Python to PATH" during installation
3. Verify installation: open Command Prompt and run `python --version`
4. If "python is not recognized", reinstall Python with PATH option checked

**Linux/Mac:**
- Python 3.12+ usually pre-installed
- Install via package manager if needed: `sudo apt install python3` (Ubuntu)

### Python Environment
- Python 3.12.3 or higher required
- Virtual environment located at `venv/`
- Activate with: `source venv/bin/activate` (Linux/Mac) or `venv\Scripts\activate.bat` (Windows)
- Required dependencies in `requirements.txt`

### System Dependencies
- **Windows**: LibreOffice at `C:\Program Files\LibreOffice\program\soffice.exe` - used for Excel to PDF conversion
- **Linux/Mac**: LibreOffice (`soffice`) at `/usr/bin/soffice` - used for Excel to PDF conversion
- Excel template file (`config.xlsx`) - contains invoice layout and styling

## Common Commands

### Setup and Installation

**Windows (First Time Setup):**
```batch
# Create virtual environment
python -m venv venv

# Activate virtual environment
venv\Scripts\activate.bat

# Install dependencies
pip install -r requirements.txt
```

**Linux/Mac:**
```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

**Adding New Dependencies:**
```bash
# After activating virtual environment
pip install package_name
pip freeze > requirements.txt
```

### Running the Invoice Generator

**Windows:**
```batch
# Run the batch file (easiest method)
run_invoices.bat

# Or manually activate and run
venv\Scripts\activate.bat
python generate_invoices.py
```

**Linux/Mac:**
```bash
# Activate virtual environment first
source venv/bin/activate

# Generate invoices from orders.csv using config.xlsx template
python generate_invoices.py
```

### Converting Excel to PDF

The script generates an Excel file (`generated_invoices.xlsx`) with all invoices. To create the final PDF:

**Method 1: Excel Built-in Export (Recommended)**
1. Open `generated_invoices.xlsx` in Microsoft Excel
2. Go to **File > Export > Create PDF/XPS**
3. Click **Options** and select **Entire workbook**
4. Choose output location and click **Publish**
5. Excel will create a multi-page PDF with all invoices

**Method 2: Print to PDF**
1. Open `generated_invoices.xlsx` in Microsoft Excel
2. Press **Ctrl+P** to open Print dialog
3. Select **Print Entire Workbook**
4. Choose **Microsoft Print to PDF** as printer
5. Click **Print** and save the PDF

**For Large Files (1000+ invoices):**
- Excel's export is much faster than LibreOffice (minutes vs hours)
- Uses less memory and is more reliable
- Maintains perfect formatting and print settings

### Development Testing
```bash
# Test with sample data
python3 generate_invoices.py

# Check LibreOffice availability
which soffice
soffice --version
```

## Architecture

### Core Components

**generate_invoices.py** - Main script with the following architecture:
- **Data Processing**: Reads CSV order data with pandas, normalizes column names
- **Template Management**: Loads Excel template using openpyxl, handles image copying
- **Invoice Generation**: Creates individual invoice worksheets by copying template
- **PDF Export**: Uses LibreOffice headless mode for Excel to PDF conversion

### Key Functions by Module

**Data Handling (`load_orders`)**: 
- Reads and normalizes CSV data from Squarespace exports
- Validates required columns for billing, line items, and totals

**Template Processing (`copy_images_from_template`)**:
- Safely copies logo and images from template to generated invoices
- Handles both file-based and binary image data

**Invoice Layout (`place_amount`, `fit_one_page`)**:
- Dynamically finds and populates total fields in Excel template
- Configures print settings for single-page invoices

**Highlighting System (`read_highlights`)**:
- Reads SKU-based color coding from "Highlighting"/"Highlights" sheet
- Applies background colors to line items based on SKU

### Data Flow
1. Parse CSV order data and validate required columns
2. Load Excel template and read highlighting rules  
3. For each unique order ID:
   - Copy template sheet and add images
   - Populate header (order #, date, billing address)
   - Add line items with quantities, descriptions, prices
   - Apply SKU-based highlighting if configured
   - Calculate and place totals (subtotal, tax, total)
   - Configure print settings for single page
4. Remove template sheets and save as Excel
5. Convert to PDF using LibreOffice headless mode

## File Structure

### Input Files
- `orders.csv` - Squarespace order export with normalized column names (case-insensitive)
- `config.xlsx` - Excel template with "Invoice" sheet and optional "Highlighting" sheet
- `logo.png` - Company logo embedded in template

### Output Files  
- `generated_invoices.xlsx` - Multi-sheet Excel with individual invoices (final output)
- PDF files are created manually using Excel's export feature (see conversion instructions above)

**Important Notes:**
- The `generated_invoices.xlsx` file may be quite large (50-500MB for many orders)
- If the script gets "Permission denied" errors, close Excel and delete any existing `generated_invoices.xlsx` file
- Use Excel's native "Export as PDF" feature for best performance with large files
- For 1000+ invoices, Excel export is much faster and more reliable than automated conversion

### Configuration
- **Template Layout**: Fixed cell positions (I1=order#, G5=date, B3-B5=billing, B9+=line items)
- **Print Setup**: Configured for single page portrait with 0.25" margins
- **Column Widths**: Optimized for readability (B=8, C=52, F=12, G=14)

## Template Requirements

The Excel template must contain:
- "Invoice" sheet with specific cell layout
- Logo/images in designated positions
- Predefined labels for totals section (rows 37-39)
- Optional "Highlighting" sheet for SKU color coding

## Error Handling

Common issues and solutions:

**Setup Issues:**
- "python is not recognized" (Windows): Reinstall Python with "Add to PATH" checked
- "pip is not recognized" (Windows): Virtual environment not activated properly

**Runtime Issues:**
- Missing CSV columns: Script validates and reports missing required fields
- Permission denied on Excel file: Close Excel and delete existing `generated_invoices.xlsx`
- Script hangs at "Saving Excel file": Large workbooks (50+ invoices) can take several minutes to save
- Template format: Must contain "Invoice" sheet with expected layout
- Image copying: Handles both file-path and binary image data safely

**Memory Issues:**
- High RAM usage (500MB+): Normal for large batches, script includes memory optimizations
- Out of memory errors: Try processing smaller batches or restart the system

**PDF Conversion Issues:**
- Use Excel's native export instead of automated conversion for best results
- For very large files (1000+ invoices), consider exporting in batches if Excel struggles
- Excel's "Print to PDF" option may be faster than "Export as PDF" for some files

## Dependencies

Key libraries:
- `pandas` - CSV data processing and validation
- `openpyxl` - Excel file manipulation and template handling  
- `subprocess` - LibreOffice PDF conversion integration
- Standard library: `tempfile`, `shutil`, `os` for file operations