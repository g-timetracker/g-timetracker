import QtQuick 2.0

Canvas {
    id: chart

    property var chartData
    property string units
    property int padding: 10
    property color defaultBarColor: "#000000"
    property color axesColor: "#607D8B"
    property color gridColor: "#20607D8B"
    property color labelFontColor: "#263238"
    property real labelFontSize: 8
    property string labelFontFamily: "sans-serif"
    property real legendIconSize: padding
    property color legendFontColor: labelFontColor
    property real legendFontSize: labelFontSize
    property string legendFontFamily: labelFontFamily
    property int axesWidth: 1
    property int gridWidth: 1
    property int gridCells: 10
    property int maxBarWidth: chart.width / 10

    QtObject {
        id: d

        property int chartAreaWidth: chart.width - labelsWidth - legendWidth
        property int chartAreaHeight: chart.height - padding * 2
        property var tickValues: []
        property var tickPositions: []
        property real labelsWidth
        property real legendWidth
        property real maxValue

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
            var tickSize = _calcNiceNumber(chartData.max / gridCells)
            maxValue = Math.ceil(chartData.max / tickSize) * tickSize
            var res = []
            for (var i = 0; i < gridCells - 1; i++) {
                res.push(tickSize * (i+1))
            }

            tickValues = res
        }

        function _calcTickPositions() {
            var res = []
            for (var i = 0; i < tickValues.length; i++) {
                res[i] = chartAreaHeight - Math.round(chartAreaHeight * tickValues[i] / maxValue) + padding + gridWidth / 2
            }

            tickPositions = res
        }

        function _drawLabels(ctx) {
            ctx.fillStyle = labelFontColor
            ctx.font = "%1pt %2".arg(labelFontSize).arg(labelFontFamily)
            ctx.textBaseline = "middle"
            var maxLabelWidth = 0
            for (var i = 0; i < tickValues.length; i++) {
                var labelText = "%1%2".arg(tickValues[i]).arg(chart.units ? " " + chart.units : "")
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
            var barWidth = Math.min((chartAreaWidth - spacing) / chartData.data.length - spacing, maxBarWidth)
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
