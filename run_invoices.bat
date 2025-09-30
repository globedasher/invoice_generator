@echo off
echo Activating virtual environment...
call venv\Scripts\activate.bat

echo Installing/updating dependencies...
pip install -r requirements.txt

echo.
echo Generating Excel invoices...
python generate_invoices.py

echo.
echo Deactivating virtual environment...
if defined VIRTUAL_ENV call deactivate

echo.
echo ============================================
echo Excel file created with timestamp
echo.
echo To print invoices:
echo 1. Open the generated Excel file in Excel
echo 2. Press Ctrl+P to print
echo 3. Select "Print Entire Workbook"
echo 4. Choose your printer and click Print
echo ============================================
echo.
echo Press any key to exit...
pause >nul