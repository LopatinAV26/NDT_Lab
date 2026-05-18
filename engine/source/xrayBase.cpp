#include "xrayBase.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>

#include "implot.h"

float XrayBase::CalculateIntensity(float intensity_0, float focusDistance_0, float focusDistance_1) const
{
    return intensity_0 / std::pow((focusDistance_0 / focusDistance_1), 2);
}

float XrayBase::ToMinute(float exposition, float mA) const
{
    return exposition / mA;
}

float XrayBase::ToSecond(float exposition, float mA) const
{
    return (exposition / mA) * 60;
}

void XrayBase::DrawThicknessMarkers(const std::vector<DiagramLineRef> &lines,
                                    float thickness,
                                    bool drawVerticalLine) const
{
    if (lines.empty())
        return;
    if (!std::isfinite(thickness))
        return;

    ImPlotSpec lineSpec;
    lineSpec.LineColor = ImVec4(1.0f, 1.0f, 1.0f, 0.35f);

    if (drawVerticalLine)
        ImPlot::PlotInfLines("##thickness-line", &thickness, 1, lineSpec);

    for (const auto &line : lines)
    {
        if (!line.x || !line.y)
            continue;

        const auto &xs = *line.x;
        const auto &ys = *line.y;

        if (xs.size() < 2 || xs.size() != ys.size())
            continue;

        for (size_t i = 0; i + 1 < xs.size(); ++i)
        {
            const float x0 = xs.at(i);
            const float x1 = xs.at(i + 1);
            const float minX = std::min(x0, x1);
            const float maxX = std::max(x0, x1);

            if (thickness < minX || thickness > maxX)
                continue;

            const float delta = x1 - x0;
            if (std::fabs(delta) <= std::numeric_limits<float>::epsilon())
                continue;

            const float t = std::clamp((thickness - x0) / delta, 0.0f, 1.0f);

            double px = {thickness};
            double py = ys.at(i) + t * (ys.at(i + 1) - ys.at(i));

            const ImVec4 fillColor = line.color;
            const ImVec4 outlineColor{0.05f, 0.05f, 0.05f, 1.0f};

            std::string markerLabel = "##thickness-marker-" + line.label + "-" + std::to_string(i);

            ImPlotSpec markerSpec;
            markerSpec.LineWeight = 2.0f;
            markerSpec.Marker = ImPlotMarker_Diamond;
            markerSpec.MarkerSize = 6.0f;
            markerSpec.MarkerLineColor = outlineColor;
            markerSpec.MarkerFillColor = fillColor;
            ImPlot::PlotScatter(markerLabel.c_str(), &px, &py, 1, markerSpec);

            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%s S=%.1f  T=%.1f", line.label.c_str(), px, py);
            ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true, "%s", buffer);

            break;
        }
    }
}
