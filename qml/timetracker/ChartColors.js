.pragma library

.import "MaterialColors.js" as Colors

function addColors(data, colorProperty) {
    if (!data) {
        return
    }

    for (var i = 0; i < data.length; i++) {
        data[i][colorProperty] = Colors.colorForChartIndex(i)
    }
}
