# Invoice Generator - Usage Guide

## Quick Start

1. **Launch Application**
   ```bash
   ./build/InvoiceGeneratorGUI
   ```

2. **The application now automatically adapts to your screen size:**
   - Uses 80% of screen width, 75% of screen height  
   - Centers itself on screen
   - Minimum size: 700x500 for usability
   - All panels are resizable with splitters

## Interface Overview

### 📊 **Orders Data Tab**
- **File Selection**: Browse for your CSV file
- **Responsive Layout**: 
  - Left panel (30%): Orders list
  - Right panel (70%): Order details
  - Drag the splitter to adjust proportions

### 🎨 **Template Editor Tab**  
- **Dynamic Preview**: Template scales automatically to fit window size
- **Responsive Layout**:
  - Left side (75%): Visual template editor
  - Right side (25%): Properties panel
- **Interactive Elements**: 
  - Click elements to select them
  - Drag elements to reposition
  - Use property spinboxes for precise positioning

### 🌈 **Line Highlighting Tab**
- **Responsive Table**: Automatically expands to fill available space
- **Default Rules**: Pre-loaded with "bundle" (yellow) and "tree" (light green)
- **Easy Editing**:
  - Edit text directly in table
  - Double-click color cells to change colors
  - Add/Remove buttons for managing rules

### 📄 **Preview & Generate Tab**
- **Responsive Preview Area**: Adapts to window size
- **External PDF Viewer**: Opens PDFs in your system's default viewer

## Responsive Features

### ✨ **Auto-Scaling**
- Template preview scales dynamically based on window size
- Minimum scale maintained for readability
- All layouts use splitters for user customization

### 🔧 **Flexible Sizing**
- Window automatically sizes to 80% of your screen
- All components have responsive minimum sizes
- Tables and lists expand to use available space

### 🌙 **Automatic Dark Mode**
- Automatically detects system dark mode settings
- Works with GNOME, KDE, and other desktop environments
- Complete dark theme coverage including:
  - Window borders and title bar
  - Menu bar and toolbar
  - Tab widget and splitters  
  - Template editor with appropriate text colors
- All components adapt seamlessly

## Tips for Small Screens

1. **Use Splitters**: Drag the dividers between panels to optimize space
2. **Maximize Template Editor**: Give more space to the preview area
3. **Minimize Properties Panel**: The right panel can be made quite narrow
4. **Use External Preview**: PDF preview opens in separate viewer to save space

## Keyboard Shortcuts

- **Ctrl+O**: Open CSV file
- **Ctrl+S**: Save template
- **Ctrl+G**: Generate PDF
- **Ctrl+Q**: Quit application

## Window Management

- **Minimum Size**: 700x500 pixels
- **Default Size**: 80% of screen dimensions
- **Position**: Automatically centered
- **Resizable**: All panels adapt to window resizing
- **Splitters**: Drag to customize panel proportions

The interface now works well on screens from 1024x768 up to large displays!