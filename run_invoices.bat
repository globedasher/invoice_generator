@echo off
echo Activating virtual environment...
call venv\Scripts\activate.bat

echo Installing/updating dependencies...
pip install -r requirements.txt

echo.
echo Generating invoices...
python generate_invoices.py

echo.
echo Deactivating virtual environment...
deactivate

echo.
echo Press any key to exit...
pause >nul