#include "ui/main/RealtimePanel.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace gpm {
namespace {

QLabel* label(const QString& text, const QString& objectName = {}) {
    auto* l = new QLabel(text);
    if (!objectName.isEmpty()) l->setObjectName(objectName);
    l->setWordWrap(false);
    return l;
}

QFrame* frame(const QString& objectName = "metricCard") {
    auto* f = new QFrame;
    f->setObjectName(objectName);
    return f;
}

void appendPoint(QLineSeries* series, int x, double value) {
    if (!series) return;
    series->append(x, value);
    if (series->count() > 240) series->remove(0);
}

} // namespace

RealtimePanel::RealtimePanel(QWidget* parent) : QWidget(parent) {
    buildUi();
}

RealtimePanel::~RealtimePanel() = default;

void RealtimePanel::buildUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    auto* root = new QVBoxLayout(content);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    auto* fpsRow = new QGridLayout;
    fpsRow->setHorizontalSpacing(10);
    fpsRow->addWidget(buildFpsCard("fps", QStringLiteral("帧率"), "#22d3ee"), 0, 0);
    fpsRow->addWidget(buildFpsCard("frame", QStringLiteral("帧时间"), "#4ade80"), 0, 1);
    fpsRow->addWidget(buildFpsCard("low", QStringLiteral("1% / 0.1% Low"), "#fbbf24"), 0, 2);
    root->addLayout(fpsRow);

    auto* hwRow = new QGridLayout;
    hwRow->setHorizontalSpacing(10);
    hwRow->setVerticalSpacing(10);
    hwRow->addWidget(buildHardwareCard("cpu", QStringLiteral("CPU"), "#60a5fa"), 0, 0);
    hwRow->addWidget(buildHardwareCard("gpu", QStringLiteral("GPU"), "#4ade80"), 0, 1);
    hwRow->addWidget(buildHardwareCard("ram", QStringLiteral("RAM"), "#fbbf24"), 0, 2);
    hwRow->addWidget(buildHardwareCard("vram", QStringLiteral("VRAM"), "#22d3ee"), 0, 3);
    root->addLayout(hwRow);

    auto* chartRow = new QGridLayout;
    chartRow->setHorizontalSpacing(10);
    chartRow->addWidget(buildChart(QStringLiteral("FPS / 1% low / 帧时间"),
                                   &m_fpsSeries, &m_lowSeries, &m_frameSeries,
                                   { "FPS", "1% low", "Frame ms x10" },
                                   { "#22d3ee", "#fbbf24", "#8b8b94" },
                                   &m_fpsAxisX, &m_fpsAxisY), 0, 0);
    chartRow->addWidget(buildChart(QStringLiteral("CPU / GPU / 温度"),
                                   &m_cpuSeries, &m_gpuSeries, &m_tempSeries,
                                   { "CPU %", "GPU %", "GPU C" },
                                   { "#60a5fa", "#4ade80", "#f87171" },
                                   &m_hwAxisX, &m_hwAxisY), 0, 1);
    root->addLayout(chartRow);

    auto* netRow = new QGridLayout;
    netRow->setHorizontalSpacing(10);
    netRow->addWidget(buildNetworkCard("ping", QStringLiteral("网络延迟")), 0, 0);
    netRow->addWidget(buildNetworkCard("down", QStringLiteral("下载")), 0, 1);
    netRow->addWidget(buildNetworkCard("up", QStringLiteral("上传")), 0, 2);
    root->addLayout(netRow);
    root->addStretch(1);

    scroll->setWidget(content);
    outer->addWidget(scroll);
}

QWidget* RealtimePanel::buildFpsCard(const QString& key, const QString& title, const QString& color) {
    auto* card = frame();
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(4);

    auto* titleLabel = label(title, "cardTitle");
    auto* valueLabel = label(QStringLiteral("N/A"), "fpsValue");
    valueLabel->setStyleSheet(QStringLiteral("color:%1;").arg(color));
    auto* subLabel = label(QStringLiteral("等待采样"), "mutedText");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(subLabel);
    m_values.insert(key, valueLabel);
    m_subValues.insert(key, subLabel);
    return card;
}

QWidget* RealtimePanel::buildHardwareCard(const QString& prefix, const QString& title, const QString& color) {
    auto* card = frame();
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    auto* titleLabel = label(title, "cardTitle");
    titleLabel->setStyleSheet(QStringLiteral("color:%1;").arg(color));
    auto* usage = label(QStringLiteral("N/A"), "hwUsage");
    usage->setStyleSheet(QStringLiteral("color:%1;").arg(color));
    m_values.insert(prefix + ".usage", usage);
    layout->addWidget(titleLabel);
    layout->addWidget(usage);

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(4);
    const QStringList metrics = { "temp", "freq", "volt", "power" };
    const QStringList names = { QStringLiteral("温度"), QStringLiteral("频率"), QStringLiteral("电压"), QStringLiteral("功耗") };
    for (int i = 0; i < metrics.size(); ++i) {
        grid->addWidget(label(names[i], "metricName"), i / 2, (i % 2) * 2);
        auto* v = label(QStringLiteral("N/A"), "metricValue");
        m_values.insert(prefix + "." + metrics[i], v);
        grid->addWidget(v, i / 2, (i % 2) * 2 + 1);
    }
    layout->addLayout(grid);
    return card;
}

QWidget* RealtimePanel::buildNetworkCard(const QString& key, const QString& title) {
    auto* card = frame();
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->addWidget(label(title, "cardTitle"));
    auto* value = label(QStringLiteral("N/A"), "netValue");
    m_values.insert("net." + key, value);
    layout->addWidget(value);
    return card;
}

QChartView* RealtimePanel::buildChart(const QString& title,
                                      QLineSeries** a,
                                      QLineSeries** b,
                                      QLineSeries** c,
                                      const QStringList& names,
                                      const QStringList& colors,
                                      QValueAxis** axisX,
                                      QValueAxis** axisY) {
    auto* chart = new QChart;
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::NoAnimation);
    chart->setBackgroundVisible(false);
    chart->legend()->setVisible(true);
    chart->legend()->setLabelColor(QColor("#8b8b94"));
    chart->setTitleBrush(QBrush(QColor("#e4e4e7")));

    QLineSeries* series[3] = { new QLineSeries, new QLineSeries, new QLineSeries };
    for (int i = 0; i < 3; ++i) {
        series[i]->setName(names.value(i));
        QPen pen(QColor(colors.value(i, "#22d3ee")));
        pen.setWidth(i == 2 ? 1 : 2);
        series[i]->setPen(pen);
        chart->addSeries(series[i]);
    }

    auto* x = new QValueAxis;
    x->setRange(0, 240);
    x->setLabelsVisible(false);
    x->setGridLineColor(QColor("#35353a"));
    auto* y = new QValueAxis;
    y->setRange(0, 160);
    y->setLabelFormat("%.0f");
    y->setLabelsColor(QColor("#8b8b94"));
    y->setGridLineColor(QColor("#35353a"));
    chart->addAxis(x, Qt::AlignBottom);
    chart->addAxis(y, Qt::AlignLeft);
    for (auto* s : series) {
        s->attachAxis(x);
        s->attachAxis(y);
    }

    *a = series[0];
    *b = series[1];
    *c = series[2];
    *axisX = x;
    *axisY = y;
    if (!m_fpsChart) m_fpsChart = chart;
    else m_hwChart = chart;

    auto* view = new QChartView(chart);
    view->setObjectName("chartView");
    view->setRenderHint(QPainter::Antialiasing, false);
    view->setMinimumHeight(180);
    return view;
}

void RealtimePanel::setText(const QString& key, const QString& value) {
    if (auto* label = m_values.value(key, nullptr)) label->setText(value);
}

QString RealtimePanel::pct(std::optional<double> value) {
    return value ? QString::asprintf("%.0f%%", *value) : QStringLiteral("N/A");
}

QString RealtimePanel::number(std::optional<double> value, const char* format, const QString& suffix) {
    return value ? QString::asprintf(format, *value) + suffix : QStringLiteral("N/A");
}

void RealtimePanel::updateValues(const SampleBlock& b) {
    setText("fps", number(b.fps, "%.0f"));
    setText("frame", number(b.frameTimeMs, "%.1f", QStringLiteral(" ms")));
    const QString low = b.oneLowPct || b.pointOneLowPct
        ? QStringLiteral("%1 / %2").arg(number(b.oneLowPct, "%.0f"), number(b.pointOneLowPct, "%.0f"))
        : QStringLiteral("N/A");
    setText("low", low);

    if (auto* sub = m_subValues.value("fps", nullptr)) sub->setText(QStringLiteral("均值由 1B 报告计算"));
    if (auto* sub = m_subValues.value("frame", nullptr)) sub->setText(QStringLiteral("越低越稳定"));
    if (auto* sub = m_subValues.value("low", nullptr)) sub->setText(QStringLiteral("最慢帧百分位"));

    setText("cpu.usage", pct(b.cpuUsagePct));
    setText("gpu.usage", pct(b.gpuUsagePct));
    setText("ram.usage", b.ramUsedMb && b.ramTotalMb
        ? QStringLiteral("%1 / %2 GB").arg(QString::asprintf("%.1f", *b.ramUsedMb / 1024.0),
                                           QString::asprintf("%.0f", *b.ramTotalMb / 1024.0))
        : pct(b.ramUsagePct));
    setText("vram.usage", b.vramUsedMb && b.vramTotalMb
        ? QStringLiteral("%1 / %2 GB").arg(QString::asprintf("%.1f", *b.vramUsedMb / 1024.0),
                                           QString::asprintf("%.0f", *b.vramTotalMb / 1024.0))
        : pct(b.vramUsagePct));

    setText("cpu.temp", number(b.cpuTempC, "%.0f", QStringLiteral(" C")));
    setText("gpu.temp", number(b.gpuTempC, "%.0f", QStringLiteral(" C")));
    setText("ram.temp", QStringLiteral("N/A"));
    setText("vram.temp", QStringLiteral("N/A"));

    setText("cpu.freq", number(b.cpuFreqMhz, "%.0f", QStringLiteral(" MHz")));
    setText("gpu.freq", number(b.gpuFreqMhz, "%.0f", QStringLiteral(" MHz")));
    setText("ram.freq", number(b.ramFreqMhz, "%.0f", QStringLiteral(" MHz")));
    setText("vram.freq", QStringLiteral("N/A"));

    setText("cpu.volt", number(b.cpuVoltageV, "%.2f", QStringLiteral(" V")));
    setText("gpu.volt", number(b.gpuVoltageV, "%.2f", QStringLiteral(" V")));
    setText("ram.volt", QStringLiteral("N/A"));
    setText("vram.volt", QStringLiteral("N/A"));

    setText("cpu.power", number(b.cpuPowerW, "%.0f", QStringLiteral(" W")));
    setText("gpu.power", number(b.gpuPowerW, "%.0f", QStringLiteral(" W")));
    setText("ram.power", QStringLiteral("N/A"));
    setText("vram.power", QStringLiteral("N/A"));

    setText("net.ping", number(b.netLatencyMs, "%.0f", QStringLiteral(" ms")));
    setText("net.down", number(b.netRxKbps, "%.0f", QStringLiteral(" KB/s")));
    setText("net.up", number(b.netTxKbps, "%.0f", QStringLiteral(" KB/s")));

    appendPoint(m_fpsSeries, m_xIndex, b.fps.value_or(0));
    appendPoint(m_lowSeries, m_xIndex, b.oneLowPct.value_or(0));
    appendPoint(m_frameSeries, m_xIndex, b.frameTimeMs.value_or(0) * 10.0);
    appendPoint(m_cpuSeries, m_xIndex, b.cpuUsagePct.value_or(0));
    appendPoint(m_gpuSeries, m_xIndex, b.gpuUsagePct.value_or(0));
    appendPoint(m_tempSeries, m_xIndex, b.gpuTempC.value_or(0));

    const int start = qMax(0, m_xIndex - 240);
    if (m_fpsAxisX) m_fpsAxisX->setRange(start, qMax(240, m_xIndex));
    if (m_hwAxisX) m_hwAxisX->setRange(start, qMax(240, m_xIndex));
    ++m_xIndex;
}

} // namespace gpm
