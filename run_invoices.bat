@echo off
echo Installing/updating dependencies...
pip install -r requirements.txt

echo.
echo Generating invoices...
python generate_invoices.py

echo.
echo Press any key to exit...
pause >nul