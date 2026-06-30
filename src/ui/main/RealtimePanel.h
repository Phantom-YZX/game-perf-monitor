#pragma once

#include "util/SampleBlock.h"

#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <optional>

class QChart;
class QChartView;
class QLabel;
class QLineSeries;
class QValueAxis;

namespace gpm {

class RealtimePanel : public QWidget {
    Q_OBJECT
public:
    explicit RealtimePanel(QWidget* parent = nullptr);
    ~RealtimePanel() override;

public slots:
    void updateValues(const SampleBlock& b);

private:
    void buildUi();
    QWidget* buildFpsCard(const QString& key, const QString& title, const QString& color);
    QWidget* buildHardwareCard(const QString& prefix, const QString& title, const QString& color);
    QWidget* buildNetworkCard(const QString& key, const QString& title);
    QChartView* buildChart(const QString& title,
                           QLineSeries** a,
                           QLineSeries** b,
                           QLineSeries** c,
                           const QStringList& names,
                           const QStringList& colors,
                           QValueAxis** axisX,
                           QValueAxis** axisY);
    void setText(const QString& key, const QString& value);
    static QString pct(std::optional<double> value);
    static QString number(std::optional<double> value, const char* format, const QString& suffix = {});

    QMap<QString, QLabel*> m_values;
    QMap<QString, QLabel*> m_subValues;

    QChart* m_fpsChart = nullptr;
    QChart* m_hwChart = nullptr;
    QLineSeries* m_fpsSeries = nullptr;
    QLineSeries* m_lowSeries = nullptr;
    QLineSeries* m_frameSeries = nullptr;
    QLineSeries* m_cpuSeries = nullptr;
    QLineSeries* m_gpuSeries = nullptr;
    QLineSeries* m_tempSeries = nullptr;
    QValueAxis* m_fpsAxisX = nullptr;
    QValueAxis* m_fpsAxisY = nullptr;
    QValueAxis* m_hwAxisX = nullptr;
    QValueAxis* m_hwAxisY = nullptr;

    int m_xIndex = 0;
};

} // namespace gpm
