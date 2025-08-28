#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include "MainWindow.h"

bool isSystemDarkMode() {
    // Check system dark mode on Linux
    QProcess process;
    process.start("gsettings", QStringList() << "get" << "org.gnome.desktop.interface" << "gtk-theme");
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();
    
    // If gsettings doesn't work, try checking Qt platform theme
    if (output.isEmpty()) {
        const char* qtTheme = qgetenv("QT_QPA_PLATFORMTHEME");
        if (qtTheme && QString(qtTheme).contains("dark")) {
            return true;
        }
    }
    
    return output.contains("dark", Qt::CaseInsensitive);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Invoice Generator");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Your Company");
    
    // Set a modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Detect system theme and apply if dark
    bool systemDarkMode = isSystemDarkMode();
    
    if (systemDarkMode) {
        // Apply comprehensive dark theme
        QPalette darkPalette;
        
        // Window and dialog backgrounds
        darkPalette.setColor(QPalette::Window, QColor(45, 45, 48));
        darkPalette.setColor(QPalette::WindowText, QColor(241, 241, 241));
        
        // Input field backgrounds
        darkPalette.setColor(QPalette::Base, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
        
        // Text colors
        darkPalette.setColor(QPalette::Text, QColor(241, 241, 241));
        darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        
        // Button colors
        darkPalette.setColor(QPalette::Button, QColor(60, 60, 60));
        darkPalette.setColor(QPalette::ButtonText, QColor(241, 241, 241));
        
        // Selection colors
        darkPalette.setColor(QPalette::Highlight, QColor(38, 79, 120));
        darkPalette.setColor(QPalette::HighlightedText, QColor(241, 241, 241));
        
        // Tooltip colors
        darkPalette.setColor(QPalette::ToolTipBase, QColor(90, 90, 90));
        darkPalette.setColor(QPalette::ToolTipText, QColor(241, 241, 241));
        
        // Links
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::LinkVisited, QColor(94, 94, 186));
        
        // Disabled colors
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
        
        app.setPalette(darkPalette);
        
        // Set additional style sheet for complete coverage
        app.setStyleSheet(
            "QMainWindow { background-color: rgb(45, 45, 48); }"
            "QMenuBar { background-color: rgb(60, 60, 60); color: rgb(241, 241, 241); }"
            "QMenuBar::item:selected { background-color: rgb(38, 79, 120); }"
            "QMenu { background-color: rgb(60, 60, 60); color: rgb(241, 241, 241); border: 1px solid rgb(90, 90, 90); }"
            "QMenu::item:selected { background-color: rgb(38, 79, 120); }"
            "QToolBar { background-color: rgb(60, 60, 60); border: none; }"
            "QStatusBar { background-color: rgb(60, 60, 60); color: rgb(241, 241, 241); }"
            "QTabWidget::pane { border: 1px solid rgb(90, 90, 90); background-color: rgb(45, 45, 48); }"
            "QTabBar::tab { background-color: rgb(60, 60, 60); color: rgb(241, 241, 241); padding: 8px; margin-right: 2px; }"
            "QTabBar::tab:selected { background-color: rgb(45, 45, 48); border-bottom: 2px solid rgb(38, 79, 120); }"
            "QSplitter::handle { background-color: rgb(90, 90, 90); }"
            "QSplitter::handle:horizontal { width: 3px; }"
            "QSplitter::handle:vertical { height: 3px; }"
        );
    }
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}