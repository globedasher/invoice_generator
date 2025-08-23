import pandas as pd
from reportlab.lib.pagesizes import LETTER
from reportlab.platypus import (
    SimpleDocTemplate, Table, TableStyle, Paragraph, Spacer, PageBreak, Image
)
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
import os

# --- CONFIG ---
ORDERS_CSV = "orders.csv"
CONFIG_XLSX = "config.xlsx"
OUTPUT_PDF = "invoices_with_highlights.pdf"

# --- Load Data ---
orders_df = pd.read_csv(ORDERS_CSV)
template_df = pd.read_excel(CONFIG_XLSX, sheet_name="Template", index_col=0)
highlight_df = pd.read_excel(CONFIG_XLSX, sheet_name="Highlighting")

# Extract template values
COMPANY_NAME = template_df.loc["CompanyName", "Value"]
COMPANY_ADDRESS = template_df.loc["CompanyAddress", "Value"]
COMPANY_EMAIL = template_df.loc["CompanyEmail", "Value"]
COMPANY_PHONE = template_df.loc["CompanyPhone", "Value"]
LOGO_PATH = template_df.loc["LogoPath", "Value"]
FOOTER_TEXT = template_df.loc["FooterText", "Value"]

# Create SKU -> color map
highlight_map = {row["SKU"]: row["Color"] for _, row in highlight_df.iterrows()}

# --- Clean Columns ---
orders_df.columns = [col.strip() for col in orders_df.columns]
grouped = orders_df.groupby("Order ID")

# --- ReportLab Styles ---
styles = getSampleStyleSheet()
item_style = ParagraphStyle(
    name='ItemStyle',
    fontName='Helvetica',
    fontSize=10,
    leading=12,
    wordWrap='CJK'
)

def add_page_number(canvas, doc):
    #page_num = canvas.getPageNumber()
    canvas.setFont("Helvetica", 9)
    #canvas.drawRightString(LETTER[0]-40, 30, f"Page {page_num}")
    canvas.drawString(40, 30, FOOTER_TEXT)

# --- Build PDF ---
doc = SimpleDocTemplate(OUTPUT_PDF, pagesize=LETTER)
elements = []

for i, (order_id, rows) in enumerate(grouped):
    # --- Company Header ---
    if os.path.exists(LOGO_PATH):
        elements.append(Image(LOGO_PATH, width=160, height=80))
    #elements.append(Paragraph(f"<b>{COMPANY_NAME}</b>", styles["Heading2"]))
    #elements.append(Paragraph(f"{COMPANY_ADDRESS}", styles["Normal"]))
    #elements.append(Paragraph(f"Email: {COMPANY_EMAIL} | Phone: {COMPANY_PHONE}", styles["Normal"]))
    #elements.append(Spacer(1, 12))

    # --- Invoice Header ---
    first_row = rows.iloc[0]
    billing_name = first_row.get("Billing Name", "")
    billing_address = ", ".join(
        str(first_row.get(x, "")) for x in ["Billing Address1", "Billing Address2", "Billing City", 
                                            "Billing Province", "Billing Country", "Billing Zip"]
        if str(first_row.get(x, "")).strip()
    )
    email = first_row.get("Email", "")

    elements.append(Paragraph(f"<b>Invoice for Order #{order_id}</b>", styles["Title"]))
    elements.append(Spacer(1, 12))
    elements.append(Paragraph(f"<b>Billing:</b> {billing_name}, {billing_address}", styles["Normal"]))
    elements.append(Paragraph(f"<b>Email:</b> {email}", styles["Normal"]))
    elements.append(Spacer(1, 12))

    # --- Line Items Table ---
    rows = rows.copy()
    rows["Line Total"] = rows["Lineitem price"] * rows["Lineitem quantity"]

    table_data = [["Item", "SKU", "Qty", "Price", "Line Total"]]
    table_style_cmds = [
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (2, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "TOP")
    ]

    for idx, r in rows.iterrows():
        item_paragraph = Paragraph(str(r.get("Lineitem name", "")), item_style)
        sku = r.get("Lineitem sku", "")
        row_data = [
            item_paragraph,
            sku,
            str(r.get("Lineitem quantity", "")),
            f"${r.get('Lineitem price', 0):.2f}",
            f"${r.get('Line Total', 0):.2f}"
        ]
        table_data.append(row_data)

        # Highlight SKU if in highlight_map
        if sku in highlight_map:
            try:
                bg_color = getattr(colors, highlight_map[sku].lower(), colors.yellow)
            except:
                bg_color = colors.yellow
            table_style_cmds.append(("BACKGROUND", (0, len(table_data)-1), (-1, len(table_data)-1), bg_color))

    # Totals
    subtotal = first_row.get("Subtotal", 0)
    shipping = first_row.get("Shipping", 0)
    taxes = first_row.get("Taxes", 0)
    total = first_row.get("Total", 0)

    table_data.extend([
        ["", "", "", "Subtotal:", f"${subtotal:.2f}"],
        ["", "", "", "Shipping:", f"${shipping:.2f}"],
        ["", "", "", "Taxes:", f"${taxes:.2f}"],
        ["", "", "", "Total:", f"${total:.2f}"]
    ])
    table_style_cmds.append(("FONTNAME", (3, -1), (-1, -1), "Helvetica-Bold"))

    table = Table(table_data, colWidths=[250, 80, 40, 60, 60])
    table.setStyle(TableStyle(table_style_cmds))

    elements.append(table)

    if i < len(grouped) - 1:
        elements.append(PageBreak())

#doc.build(elements, onFirstPage=add_page_number, onLaterPages=add_page_number)
doc.build(elements, onFirstPage=add_page_number, onLaterPages=add_page_number)
print(f"Created PDF with highlights: {OUTPUT_PDF}")

