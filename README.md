# Invoice Generator

A simple tool that creates professional Excel invoices from your order data. Just provide your orders in a CSV file and customize the invoice template - then easily export to PDF using Excel!

## 📋 What You Need

### Files Required:
- **orders.csv** - Your order data (exported from your store)
- **config.xlsx** - Your invoice template (customize with your logo and colors)

### Software Required:
- **Microsoft Excel** (recommended for PDF export)
- **Python 3.12+** (free download from python.org - check "Add to PATH" during installation)

## 🚀 Quick Start

### Step 1: Get Your Files Ready
1. Export your orders to a CSV file named `orders.csv`
2. Customize `config.xlsx` with your business logo and colors
3. Make sure both files are in the same folder as this program

### Step 2: Run the Program

**On Windows:**
- Double-click `run_invoices.bat`
- Wait for it to finish

**On Mac/Linux:**
```
source venv/bin/activate
python generate_invoices.py
```

### Step 3: Get Your Excel File
The program creates:
- **generated_invoices.xlsx** - All invoices in separate sheets

### Step 4: Create PDF (Manual)
1. Open `generated_invoices.xlsx` in Microsoft Excel
2. Go to **File > Export > Create PDF/XPS**
3. Click **Options** and select **"Entire workbook"**
4. Choose where to save and click **Publish**

✅ **Why manual export?** Excel's PDF export is much faster and more reliable than automated conversion, especially for large files (1000+ invoices).

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
- Reinstall Python and check "Add Python to PATH" during installation

**"File not found" error:**
- Make sure `orders.csv` and `config.xlsx` are in the same folder

**"Permission denied" on Excel file:**
- Close Excel completely and delete any existing `generated_invoices.xlsx` file

**Script hangs on "Saving Excel file":**
- Large files (1000+ invoices) can take 10-30 minutes - this is normal
- Check available RAM - you need at least 4GB free for large batches

**PDF export issues:**
- Use Excel's native export for best results
- For very large files, consider exporting in smaller batches
- "Print to PDF" may be faster than "Export as PDF" for some files

**Colors not working:**
- Check the Highlights sheet in config.xlsx
- Use specific words that appear in your product names
- Avoid common words like "of" or "bundle" that match everything

## 💡 Tips for Best Results

1. **Test with a few orders first** before running your full batch
2. **Keep backups** of your original files  
3. **Use specific words** for highlighting (e.g., "goldenrod" instead of "plant")
4. **Check your template** - what you see in Excel is what you get in the PDF
5. **For large batches (1000+ invoices)**: Ensure you have plenty of RAM and disk space
6. **Alternative PDF method**: Use Excel's "Print to PDF" if export is slow

---

*Need help? The program will show error messages to guide you if something goes wrong.*