# Invoice Generator

A simple tool that creates professional PDF invoices from your order data. Just provide your orders in a CSV file and customize the invoice template - the rest is automatic!

## 📋 What You Need

### Files Required:
- **orders.csv** - Your order data (exported from your store)
- **config.xlsx** - Your invoice template (customize with your logo and colors)

### Software Required:
- **Windows**: Microsoft Excel (comes with Office)
- **Mac/Linux**: LibreOffice (free download)
- **Python** (free download from python.org)

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
python3 generate_invoices.py
```

### Step 3: Get Your Invoices
The program creates two files:
- **generated_invoices.xlsx** - Individual invoice sheets (for editing)
- **all_invoices.pdf** - All invoices in one PDF (for printing/emailing)

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

**"File not found" error:**
- Make sure `orders.csv` and `config.xlsx` are in the same folder

**PDF looks wrong:**
- Check that your Excel template is set up correctly
- Make sure column widths look good in Excel

**Colors not working:**
- Check the Highlights sheet in config.xlsx
- Use specific words that appear in your product names
- Avoid common words like "of" or "bundle" that match everything

**Windows: Excel not opening:**
- Make sure Microsoft Excel is installed
- The program will try LibreOffice as backup

## 💡 Tips for Best Results

1. **Test with a few orders first** before running your full batch
2. **Keep backups** of your original files
3. **Use specific words** for highlighting (e.g., "goldenrod" instead of "plant")
4. **Check your template** - what you see in Excel is what you get in the PDF

---

*Need help? The program will show error messages to guide you if something goes wrong.*