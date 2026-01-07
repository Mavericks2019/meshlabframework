#include "tab_manager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

TabManager::TabManager(QWidget* mainWindow) 
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , tabWidget(nullptr)
    , menuBar(nullptr)
    , controlContainer(nullptr)
    , basicGlWidget(nullptr)
    , cgalGlWidget(nullptr)
    , basicInfoLabel(nullptr)
    , cgalInfoLabel(nullptr)
{
}

TabManager::~TabManager() {
    // 清理已创建的tab
    QStringList tabNames = {"OpenMesh", "CGAL"};
    
    for (const QString& title : tabNames) {
        cleanupTab(title);
    }
}

void TabManager::initializeTabs() {
    // 创建自定义的标签页widget
    tabWidget = new UIUtils::CloseableTabWidget(mainWindow);
    
    // 创建菜单栏，传递createTab函数
    auto createTabFunc = [this](const QString& title, bool switchToTab) { 
        this->createTab(title, switchToTab); 
    };
    menuBar = UIUtils::createMenuBar(tabWidget, mainWindow, tabInfos, controlPanelMap, createTabFunc);
    
    // 创建控制面板容器
    controlContainer = new QWidget(mainWindow);
    QVBoxLayout *controlContainerLayout = new QVBoxLayout(controlContainer);
    controlContainerLayout->setAlignment(Qt::AlignTop);
    controlContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置控制面板固定宽度
    controlContainer->setFixedWidth(400);
    
    // 先连接信号
    connectSignals();
    
    // 创建第一个tab（OpenMesh），并切换到它
    createTab("OpenMesh", true);
    
    // 重要：手动触发第一次控制面板显示
    if (tabWidget->count() > 0) {
        QString currentTitle = tabWidget->tabText(0);
        
        // 显示当前tab对应的控制面板
        if (controlPanelMap.contains(currentTitle)) {
            QWidget* controlPanel = controlPanelMap[currentTitle];
            // 隐藏所有控制面板
            for (QWidget* panel : controlPanelMap.values()) {
                panel->setVisible(false);
            }
            // 显示当前控制面板
            controlPanel->setVisible(true);
        }
    }
}

void TabManager::createTab(const QString& title, bool switchToTab) {
    // 首先检查tab是否已经存在且显示在tab栏中
    QWidget* existingWidget = tabWidget->getWidgetByTitle(title);
    if (existingWidget && tabWidget->indexOf(existingWidget) >= 0) {
        // Tab已存在且在tab栏中显示
        if (switchToTab) {
            // 切换到该tab
            tabWidget->setCurrentWidget(existingWidget);
            
            // 更新窗口标题
            mainWindow->setWindowTitle("OBJ Viewer - " + title);
        }
        return;
    }
    
    // 如果Tab之前被创建过但被删除了，清理残留资源
    if (tabCreated.value(title, false)) {
        // 检查widget是否仍然存在但不在tab栏中
        if (existingWidget) {
            // widget存在但不在tab栏中，重新添加到tab栏
            tabWidget->addTab(existingWidget, title);
            
            if (switchToTab) {
                // 切换到新添加的tab
                int index = tabWidget->indexOf(existingWidget);
                if (index >= 0) {
                    tabWidget->setCurrentIndex(index);
                }
            }
            
            // 更新tabInfos
            for (int i = 0; i < tabInfos.size(); ++i) {
                if (tabInfos[i].title == title) {
                    tabInfos[i].widget = existingWidget;
                    tabInfos[i].isVisible = true;
                    break;
                }
            }
            
            return;
        } else {
            // 清理残留资源
            cleanupTab(title);
        }
    }
    
    // 根据标题创建相应的tab
    if (title == "OpenMesh") {
        createBasicTab();
    } else if (title == "CGAL") {
        createCGALTab();
    }
    
    // 标记为已创建
    tabCreated[title] = true;
    
    // 如果switchToTab为true，切换到新创建的tab
    if (switchToTab) {
        QWidget* widget = tabWidget->getWidgetByTitle(title);
        if (widget) {
            tabWidget->setCurrentWidget(widget);
            
            // 更新窗口标题
            mainWindow->setWindowTitle("OBJ Viewer - " + title);
        }
    }
}

void TabManager::cleanupTab(const QString& title) {
    // 清理tab资源
    if (title == "OpenMesh") {
        if (basicGlWidget) {
            delete basicGlWidget;
            basicGlWidget = nullptr;
        }
        if (basicInfoLabel) {
            delete basicInfoLabel;
            basicInfoLabel = nullptr;
        }
    } else if (title == "CGAL") {
        if (cgalGlWidget) {
            delete cgalGlWidget;
            cgalGlWidget = nullptr;
        }
        if (cgalInfoLabel) {
            delete cgalInfoLabel;
            cgalInfoLabel = nullptr;
        }
    }
    
    // 清理控制面板
    if (controlPanelMap.contains(title)) {
        QWidget* controlPanel = controlPanelMap[title];
        controlContainer->layout()->removeWidget(controlPanel);
        delete controlPanel;
        controlPanelMap.remove(title);
    }
    
    // 从tabInfos中移除对应的记录
    for (int i = tabInfos.size() - 1; i >= 0; --i) {
        if (tabInfos[i].title == title) {
            // 清理widget关联
            if (tabInfos[i].widget) {
                tabWidget->removeWidgetAssociations(tabInfos[i].widget);
            }
            tabInfos.removeAt(i);
        }
    }
    
    // 重置创建标记
    tabCreated[title] = false;
}

void TabManager::deleteTab(const QString& title) {
    // 查找对应的widget
    QWidget* widget = tabWidget->getWidgetByTitle(title);
    if (!widget) return;
    
    // 从tabWidget中移除
    int index = tabWidget->indexOf(widget);
    if (index >= 0) {
        tabWidget->removeTab(index);
    }
    
    // 清理widget关联
    tabWidget->removeWidgetAssociations(widget);
    
    // 从tabInfos中更新状态
    for (int i = 0; i < tabInfos.size(); ++i) {
        if (tabInfos[i].title == title) {
            tabInfos[i].widget = nullptr;
            tabInfos[i].isVisible = false;
            if (tabInfos[i].action) {
                tabInfos[i].action->setChecked(false);
            }
            break;
        }
    }
}

void TabManager::createBasicTab() {
    if (!basicGlWidget) {
        basicGlWidget = new BaseGLWidget;
    }
    
    QWidget* basicTab = ::createBasicTab(basicGlWidget);
    tabWidget->addTabWithTitle(basicTab, "OpenMesh");
    
    // 创建控制面板（如果不存在）
    if (!controlPanelMap.contains("OpenMesh")) {
        createControlPanel("OpenMesh");
    }
    
    // 更新tabInfos
    UIUtils::TabInfo info;
    info.name = "OpenMesh";
    info.title = "OpenMesh";
    info.widget = basicTab;
    info.controlPanel = controlPanelMap["OpenMesh"];
    info.isVisible = true;
    info.isDefault = true;
    info.originalIndex = 0;
    info.action = tabWidget->getActionForTitle("OpenMesh");
    
    // 替换或添加tab信息
    bool found = false;
    for (int i = 0; i < tabInfos.size(); ++i) {
        if (tabInfos[i].title == "OpenMesh") {
            tabInfos[i] = info;
            found = true;
            break;
        }
    }
    if (!found) {
        tabInfos.append(info);
    }
}

void TabManager::createCGALTab() {
    if (!cgalGlWidget) {
        cgalGlWidget = new CGALGLWidget;
    }
    
    QWidget* cgalTab = ::createCGALTab(cgalGlWidget);
    tabWidget->addTabWithTitle(cgalTab, "CGAL");
    
    // 创建控制面板（如果不存在）
    if (!controlPanelMap.contains("CGAL")) {
        createControlPanel("CGAL");
    }
    
    // 更新tabInfos
    UIUtils::TabInfo info;
    info.name = "CGAL";
    info.title = "CGAL";
    info.widget = cgalTab;
    info.controlPanel = controlPanelMap["CGAL"];
    info.isVisible = true;
    info.isDefault = false;
    info.originalIndex = 1;
    info.action = tabWidget->getActionForTitle("CGAL");
    
    // 替换或添加tab信息
    bool found = false;
    for (int i = 0; i < tabInfos.size(); ++i) {
        if (tabInfos[i].title == "CGAL") {
            tabInfos[i] = info;
            found = true;
            break;
        }
    }
    if (!found) {
        tabInfos.append(info);
    }
}

void TabManager::createControlPanel(const QString& title) {
    // 如果控制面板已经存在，直接返回
    if (controlPanelMap.contains(title)) {
        return;
    }
    
    QWidget* controlPanel = nullptr;
    
    if (title == "OpenMesh") {
        if (!basicGlWidget) basicGlWidget = new BaseGLWidget;
        
        QWidget *basicControlPanel = new QWidget;
        QVBoxLayout *basicControlLayout = new QVBoxLayout(basicControlPanel);
        basicControlLayout->setAlignment(Qt::AlignTop);
        basicControlLayout->addWidget(UIUtils::createColorSettingsGroup(basicGlWidget));
        basicControlLayout->addWidget(UIUtils::createModelInfoGroup(&basicInfoLabel));
        basicControlLayout->addWidget(createBasicControlPanel(basicGlWidget, basicInfoLabel, mainWindow));
        
        controlPanel = basicControlPanel;
    } else if (title == "CGAL") {
        if (!cgalGlWidget) cgalGlWidget = new CGALGLWidget;
        
        QWidget *cgalControlPanel = new QWidget;
        QVBoxLayout *cgalControlLayout = new QVBoxLayout(cgalControlPanel);
        cgalControlLayout->setAlignment(Qt::AlignTop);
        cgalControlLayout->addWidget(UIUtils::createColorSettingsGroup(cgalGlWidget));
        cgalControlLayout->addWidget(UIUtils::createModelInfoGroup(&cgalInfoLabel));
        cgalControlLayout->addWidget(createCGALControlPanel(cgalGlWidget, cgalInfoLabel, mainWindow));
        
        controlPanel = cgalControlPanel;
    }
    
    if (controlPanel) {
        // 添加到控制面板容器
        qobject_cast<QVBoxLayout*>(controlContainer->layout())->addWidget(controlPanel);
        
        // 添加到映射
        controlPanelMap[title] = controlPanel;
        
        // 初始隐藏
        controlPanel->setVisible(false);
    }
}

void TabManager::connectSignals() {
    // 连接tab关闭信号
    QObject::connect(tabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        if (tabWidget->count() <= 1) {
            QMessageBox::warning(nullptr, "Cannot Close Tab", 
                "You must have at least one tab open. Cannot close the last tab.");
            return;
        }
        
        // 获取要关闭的tab的widget和标题
        QWidget* widget = tabWidget->widget(index);
        QString tabTitle = tabWidget->tabText(index);
        
        // 删除tab
        deleteTab(tabTitle);
        
        // 切换到下一个tab
        if (tabWidget->count() > 0) {
            int newIndex = index > 0 ? index - 1 : 0;
            tabWidget->setCurrentIndex(newIndex);
            
            QWidget* currentWidget = tabWidget->widget(newIndex);
            QString currentTitle = tabWidget->tabText(newIndex);
            
            // 更新窗口标题
            mainWindow->setWindowTitle("OBJ Viewer - " + currentTitle);
            
            // 显示当前tab对应的控制面板
            if (controlPanelMap.contains(currentTitle)) {
                QWidget* controlPanel = controlPanelMap[currentTitle];
                // 隐藏所有控制面板
                for (QWidget* panel : controlPanelMap.values()) {
                    panel->setVisible(false);
                }
                // 显示当前控制面板
                controlPanel->setVisible(true);
            }
            
            // 更新菜单栏中对应action的选中状态
            for (int i = 0; i < tabInfos.size(); ++i) {
                if (tabInfos[i].widget == currentWidget && tabInfos[i].action) {
                    tabInfos[i].action->setChecked(true);
                    break;
                }
            }
        }
    });
    
    // 连接tab切换信号
    QObject::connect(tabWidget, &QTabWidget::currentChanged, [this](int index) {
        if (index >= 0) {
            QWidget* currentWidget = tabWidget->widget(index);
            QString currentTitle = tabWidget->tabText(index);
            
            // 更新窗口标题
            mainWindow->setWindowTitle("OBJ Viewer - " + currentTitle);
            
            // 更新菜单栏中对应action的选中状态
            for (int i = 0; i < tabInfos.size(); ++i) {
                if (tabInfos[i].widget == currentWidget && tabInfos[i].action) {
                    tabInfos[i].action->setChecked(true);
                    break;
                }
            }
            
            // 显示当前tab对应的控制面板
            if (controlPanelMap.contains(currentTitle)) {
                QWidget* controlPanel = controlPanelMap[currentTitle];
                // 隐藏所有控制面板
                for (QWidget* panel : controlPanelMap.values()) {
                    panel->setVisible(false);
                }
                // 显示当前控制面板
                controlPanel->setVisible(true);
            }
        }
    });
}