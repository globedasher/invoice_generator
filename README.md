# Invoice Generator

A simple tool that creates professional Excel invoices from your order data. Just provide your orders in a CSV file and customize the invoice template - then easily export to PDF using Excel!

## 💾 Getting Started - Download the Program

### Download from GitHub:
1. **Click the green "Code" button** at the top of this page
2. **Select "Download ZIP"** from the dropdown menu
3. **Save the ZIP file** to your computer (usually goes to Downloads folder)
4. **Extract the ZIP file**:
   - Right-click the ZIP file and select "Extract All" (Windows)
   - Double-click the ZIP file (Mac)
   - Use your preferred extraction tool (Linux)
5. **Open the extracted folder** - you should see files like `generate_invoices.py` and `config.xlsx`

*💡 Tip: Create a dedicated folder like "My Invoice Generator" and extract everything there for easy access.*

## 📋 What You Need

### Files Required:
- **orders.csv** - Your order data (exported from your store)
- **config.xlsx** - Your invoice template (customize with your logo and colors)

### Software Required:
- **Microsoft Excel** (recommended for PDF export and viewing invoices)
- **Python 3.12+** (free download from [python.org](https://python.org/downloads/) - **IMPORTANT**: check "Add Python to PATH" during installation on Windows)

## 🚀 Quick Start

### Step 1: Get Your Files Ready
1. **Export your orders** to a CSV file named `orders.csv` (from your store/e-commerce platform)
2. **Customize `config.xlsx`** with your business logo and colors (already included in download)
3. **Place your `orders.csv` file** in the same folder where you extracted the program

### Step 2: Run the Program

**On Windows (Easiest Method):**
1. **Double-click `run_invoices.bat`** in the program folder
2. **Wait for it to finish** (a black window will appear and show progress)
3. **Press any key** when it says "Press any key to continue"

**On Mac/Linux:**
1. **Open Terminal** in the program folder
2. **Run these commands:**
   ```bash
   source venv/bin/activate
   python generate_invoices.py
   ```

### Step 3: Find Your Generated Invoices
The program creates a new Excel file:
- **generated_invoices_YYYYMMDD_HHMMSS.xlsx** - All invoices in separate sheets
- *Example: generated_invoices_20240315_143022.xlsx*
- **Look for this file** in the same folder where you ran the program

### Step 4: Print Your Invoices
1. **Double-click the generated Excel file** to open it in Microsoft Excel
2. **Press Ctrl+P** (Windows) or **Cmd+P** (Mac) to print
3. **Select "Print Entire Workbook"** in the print dialog
4. **Choose your printer** and click **Print**

**To Create a PDF Instead:**
- In the print dialog, choose **"Microsoft Print to PDF"** as your printer
- Click **Print** and save the PDF wherever you want

✅ **Why not automatic PDF?** Direct printing from Excel is much faster and more reliable than automated conversion, especially for large batches (1000+ invoices).

## 🎨 Customizing Your Invoices

### Adding Your Logo:
1. Open `config.xlsx` in Excel
2. Go to the "Invoice" sheet
3. Insert your logo image where you want it
4. Move and resize it as needed
5. Save the file

### Adding Colors to Products:
1. Open `config.xlsx` in Excel
2. Go to the "Highlights" sheet
3. Add rows with:
   - **Column A**: Text to match (like "huckleberry" or "tree")
   - **Column B**: Color name (like "yellow" or "red")
4. Products containing that text will be highlighted in that color

### Available Colors:
- **Basic**: red, green, blue, yellow, orange, purple, pink, brown, gray
- **Light**: light blue, light green, light yellow, light pink, light gray
- **Custom**: Use hex codes like #FF5733

## 📊 Order Data Format

Your `orders.csv` file should have these columns (names can vary):
- Order ID
- Order Date
- Customer Name
- Address information
- Product quantities, names, and prices

*The program is flexible with column names - "Order ID", "order_id", and "Order Number" all work.*

## ❓ Troubleshooting

**"Python is not recognized" (Windows):**
- Download Python again from [python.org](https://python.org/downloads/)
- During installation, **make sure to check "Add Python to PATH"**
- Restart your computer after installation

**"File not found" error:**
- Make sure your `orders.csv` file is in the same folder where you extracted the program
- The `config.xlsx` template should already be there from the download

**"Permission denied" on Excel file:**
- Close Excel completely and try running the program again
- Files are now timestamped to avoid conflicts with previously opened files

**Program seems to hang or takes a long time:**
- Large batches (1000+ invoices) can take 10-30 minutes - this is normal
- Make sure you have at least 4GB of free RAM available
- The black window will show progress messages - don't close it

**Printing issues:**
- Check printer settings before printing large batches
- For very large files (1000+ invoices), consider printing in smaller batches
- Preview a few pages first to ensure formatting looks correct

**Colors not working:**
- Check the Highlights sheet in config.xlsx
- Use specific words that appear in your product names
- Avoid common words like "of" or "bundle" that match everything

## 💡 Tips for Best Results

1. **Test with a few orders first** before running your full batch
2. **Keep backups** of your original files  
3. **Use specific words** for highlighting (e.g., "goldenrod" instead of "plant")
4. **Check your template** - what you see in Excel is what you get when printed
5. **For large batches (1000+ invoices)**: Ensure you have plenty of RAM and disk space
6. **Preview before printing**: Check a few pages in Excel to ensure formatting looks correct
7. **Timestamped files**: Each run creates a new file so you can keep multiple versions

---

*Need help? The program will show error messages to guide you if something goes wrong.*