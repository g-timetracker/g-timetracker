/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

import QtQuick 2.0
import QtQuick.Window 2.2

Canvas {
    id: chart

    property var chartData
    property string unitsName
    property int unitsValue
    property int padding: 16 * Screen.devicePixelRatio
    property color defaultBarColor: "#000000"
    property color axesColor: "#60000000"
    property color gridColor: "#1E000000"
    property color labelFontColor: "#DD000000"
    property real labelFontSize: 12 * Screen.devicePixelRatio
    property string labelFontFamily: "Roboto"
    property real legendIconSize: padding
    property color legendFontColor: labelFontColor
    property real legendFontSize: labelFontSize
    property string legendFontFamily: labelFontFamily
    property int axesWidth: 1 * Screen.devicePixelRatio
    property int gridWidth: 1 * Screen.devicePixelRatio
    property int gridCells: 10
    property int maxBarWidth: chart.width / 10

    function updateChart() {
        requestPaint()
    }

    QtObject {
        id: d

        property int chartAreaWidth: chart.width - labelsWidth - legendWidth
        property int chartAreaHeight: chart.height - padding * 2
        property var tickValues: []
        property var tickPositions: []
        property real labelsWidth
        property real legendWidth
        property real tickSize
        property real maxValue
        property real maxNormalized

        function _calcNiceNumber(number) {
            var exponent = Math.floor(Math.log(number) * Math.LOG10E)
            var fraction = number / Math.pow(10, exponent)

            var niceFraction
            // Nice numbers can be expressed as 1*10^n, 2*10^n or 5*10^n
            if (fraction < 1) {
                niceFraction = 1
            } else if (fraction < 2) {
                niceFraction = 2
            } else if (fraction < 5) {
                niceFraction = 5
            } else {
                niceFraction = 10
            }

            return niceFraction * Math.pow(10, exponent)
        }

        function _calcTickValues() {
            var unitsMax = chartData.max / unitsValue
            tickSize = _calcNiceNumber(unitsMax / gridCells)
            maxNormalized = Math.ceil(unitsMax / tickSize) * tickSize
            maxValue = maxNormalized * unitsValue
            var res = []
            for (var i = 0; i < gridCells - 1; i++) {
                res.push(tickSize * (i+1))
            }

            tickValues = res
        }

        function _calcTickPositions() {
            var res = []
            for (var i = 0; i < tickValues.length; i++) {
                res[i] = chartAreaHeight - Math.round(chartAreaHeight * tickValues[i] / maxNormalized) + padding + gridWidth / 2
            }

            tickPositions = res
        }

        function _drawLabels(ctx) {
            ctx.fillStyle = labelFontColor
            ctx.font = "%1pt %2".arg(labelFontSize).arg(labelFontFamily)
            ctx.textBaseline = "middle"
            var maxLabelWidth = 0
            for (var i = 0; i < tickValues.length; i++) {
                var labelText = qsTranslate("duration", chart.unitsName || "", "", tickValues[i])
                if (tickSize < 1.0) {
                    // Qt translator can handle plurals only for integers, so replace rounded value
                    labelText = labelText.replace(Math.floor(tickValues[i]),
                                                  tickValues[i].toLocaleString(Qt.locale(), "f", 1))
                }
                maxLabelWidth = Math.max(ctx.measureText(labelText).width, maxLabelWidth)
                ctx.fillText(labelText, padding, tickPositions[i])
            }
            labelsWidth = maxLabelWidth + padding * 2
        }

        function _drawLegend(ctx) {
            ctx.font = "%1pt %2".arg(legendFontSize).arg(legendFontFamily)
            ctx.textBaseline = "middle"
            var maxCategoryWidth = chartData.data.reduce(function(previousValue, currentValue) {
                return Math.max(previousValue, ctx.measureText(currentValue.label).width)
            }, 0)
            var rectWidth = legendIconSize
            var spacing = padding
            legendWidth = rectWidth + maxCategoryWidth + padding * 3
            var xOffset = chart.width - legendWidth + padding
            for (var i = 0; i < chartData.data.length; i++) {
                var yOffset = padding + (rectWidth + spacing) * i
                ctx.fillStyle = chartData.data[i].color || defaultBarColor
                ctx.fillRect(xOffset, yOffset, rectWidth, rectWidth)
                ctx.fillStyle = legendFontColor
                ctx.fillText(chartData.data[i].label, xOffset + rectWidth + padding, yOffset + rectWidth / 2)
            }
        }

        function _drawAxes(ctx) {
            ctx.beginPath()
            ctx.lineWidth = axesWidth
            ctx.strokeStyle = axesColor
            ctx.moveTo(labelsWidth - axesWidth / 2, padding)
            ctx.lineTo(labelsWidth - axesWidth / 2, chart.height - padding + axesWidth / 2)
            ctx.lineTo(chart.width - legendWidth, chart.height - padding + axesWidth / 2)
            ctx.stroke()
        }

        function _drawGrid(ctx) {
            ctx.beginPath()
            ctx.lineWidth = gridWidth
            ctx.strokeStyle = gridColor
            for (var i = 0; i < tickPositions.length; i++) {
                ctx.moveTo(labelsWidth, tickPositions[i])
                ctx.lineTo(chart.width - legendWidth, tickPositions[i])
            }
            ctx.stroke()
        }

        function _drawBars(ctx) {
            var xOffset = labelsWidth
            var yOffset = padding
            var spacing = chartAreaWidth / (chartData.data.length + 0.17) * 0.17   // 20% of bar width
            spacing = Math.floor(spacing) || 1
            var barWidth = Math.min((chartAreaWidth - spacing) / chartData.data.length - spacing, maxBarWidth)
            barWidth = Math.floor(barWidth) || 1
            var chartWidth = barWidth * chartData.data.length + spacing * (chartData.data.length + 1)
            if (chartWidth < chartAreaWidth) {  // Center chart horizontally
                xOffset += Math.floor((chartAreaWidth - chartWidth) / 2)
            }
            for (var i = 0; i < chartData.data.length; i++) {
                ctx.fillStyle = chartData.data[i].color || defaultBarColor
                var barHeight = chartAreaHeight * (chartData.data[i].value / maxValue)
                ctx.fillRect(xOffset + spacing * (i+1) + barWidth * i, yOffset + chartAreaHeight - barHeight, barWidth, barHeight)
            }
        }

        function drawChart(ctx) {
            _calcTickValues()
            _calcTickPositions()

            _drawLabels(ctx)

            _drawLegend(ctx)

            _drawAxes(ctx)

            _drawGrid(ctx)

            _drawBars(ctx)
        }
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()

        if (!chartData || !chartData.data || !chartData.data.length) {
            return
        }

        d.drawChart(ctx)
    }

    onChartDataChanged: requestPaint()
}
