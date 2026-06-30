#pragma once

#include "util/SampleBlock.h"

#include <QList>
#include <QMainWindow>

class QLabel;
class QPushButton;
class QStackedWidget;
class QWidget;

namespace gpm {

class HudWindow;
class RealtimePanel;
class SettingsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void setHudWindow(HudWindow* hud);

public slots:
    void onUiTick();

private slots:
    void onHudToggle(bool checked);
    void onHudPositionChanged();

private:
    void buildUi();
    QWidget* buildSidebar();
    QWidget* buildTopBar();
    QWidget* buildOverlayPage();
    QWidget* buildProfilesPage();
    QWidget* buildReportsPage();
    void switchPage(int index);
    void loadQss();

    QStackedWidget* m_stack = nullptr;
    QWidget* m_sidebar = nullptr;
    QWidget* m_topBar = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_gameInfo = nullptr;
    QList<QPushButton*> m_navButtons;

    RealtimePanel* m_realtime = nullptr;
    QWidget* m_overlay = nullptr;
    QWidget* m_profiles = nullptr;
    SettingsPage* m_settings = nullptr;
    QWidget* m_reports = nullptr;
    HudWindow* m_hud = nullptr;
};

} // namespace gpm
