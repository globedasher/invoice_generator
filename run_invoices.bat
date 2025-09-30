@echo off
echo Removing any existing output files...
del /f generated_invoices.xlsx 2>nul

echo.
echo Activating virtual environment...
call venv\Scripts\activate.bat

echo Installing/updating dependencies...
pip install -r requirements.txt

echo.
echo Generating Excel invoices...
python generate_invoices.py

echo.
echo Deactivating virtual environment...
deactivate

echo.
echo ============================================
echo Excel file created: generated_invoices.xlsx
echo.
echo To create PDF:
echo 1. Open generated_invoices.xlsx in Excel
echo 2. File ^> Export ^> Create PDF/XPS
echo 3. Click Options and select "Entire workbook"
echo 4. Click Publish
echo ============================================
echo.
echo Press any key to exit...
pause >nul