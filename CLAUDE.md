# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an Invoice Generator that creates professional PDF invoices from CSV order data using Excel templates. The system processes order data from Squarespace exports and generates individual invoices merged into a single multi-page PDF document.

## Development Environment

### Python Environment
- Python 3.12.3
- Virtual environment located at `venv/`
- Activate with: `source venv/bin/activate`
- Required dependencies in `requirements.txt`

### System Dependencies
- LibreOffice (`soffice`) at `/usr/bin/soffice` - used for Excel to PDF conversion
- Excel template file (`config.xlsx`) - contains invoice layout and styling

## Common Commands

### Setup and Installation
```bash
# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Install new packages (after activation)
pip install package_name
pip freeze > requirements.txt
```

### Running the Invoice Generator
```bash
# Generate invoices from orders.csv using config.xlsx template
python3 generate_invoices.py

# Or with virtual environment activated
python generate_invoices.py
```

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
- `generated_invoices.xlsx` - Multi-sheet Excel with individual invoices
- `all_invoices.pdf` - Single PDF with all invoices for printing/distribution

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
- Missing CSV columns: Script validates and reports missing required fields
- LibreOffice not found: Ensure `/usr/bin/soffice` is available  
- Template format: Must contain "Invoice" sheet with expected layout
- Image copying: Handles both file-path and binary image data safely

## Dependencies

Key libraries:
- `pandas` - CSV data processing and validation
- `openpyxl` - Excel file manipulation and template handling  
- `subprocess` - LibreOffice PDF conversion integration
- Standard library: `tempfile`, `shutil`, `os` for file operations