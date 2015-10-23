.pragma library

.import "Util.js" as Util

var _colors = [
    {
        name: "Red",
        list: [
            { name: "red_50", value: "#FFEBEE" },
            { name: "red_100", value: "#FFCDD2" },
            { name: "red_200", value: "#EF9A9A" },
            { name: "red_300", value: "#E57373" },
            { name: "red_400", value: "#EF5350" },
            { name: "red_500", value: "#F44336" },
            { name: "red_600", value: "#E53935" },
            { name: "red_700", value: "#D32F2F" },
            { name: "red_800", value: "#C62828" },
            { name: "red_900", value: "#B71C1C" },
            { name: "red_A100", value: "#FF8A80" },
            { name: "red_A200", value: "#FF5252" },
            { name: "red_A400", value: "#FF1744" },
            { name: "red_A700", value: "#D50000" },
        ]
    },
    {
        name: "Pink",
        list: [
            { name: "pink_50", value: "#FCE4EC" },
            { name: "pink_100", value: "#F8BBD0" },
            { name: "pink_200", value: "#F48FB1" },
            { name: "pink_300", value: "#F06292" },
            { name: "pink_400", value: "#EC407A" },
            { name: "pink_500", value: "#E91E63" },
            { name: "pink_600", value: "#D81B60" },
            { name: "pink_700", value: "#C2185B" },
            { name: "pink_800", value: "#AD1457" },
            { name: "pink_900", value: "#880E4F" },
            { name: "pink_A100", value: "#FF80AB" },
            { name: "pink_A200", value: "#FF4081" },
            { name: "pink_A400", value: "#F50057" },
            { name: "pink_A700", value: "#C51162" },
        ]
    },
    {
        name: "Purple",
        list: [
            { name: "purple_50", value: "#F3E5F5" },
            { name: "purple_100", value: "#E1BEE7" },
            { name: "purple_200", value: "#CE93D8" },
            { name: "purple_300", value: "#BA68C8" },
            { name: "purple_400", value: "#AB47BC" },
            { name: "purple_500", value: "#9C27B0" },
            { name: "purple_600", value: "#8E24AA" },
            { name: "purple_700", value: "#7B1FA2" },
            { name: "purple_800", value: "#6A1B9A" },
            { name: "purple_900", value: "#4A148C" },
            { name: "purple_A100", value: "#EA80FC" },
            { name: "purple_A200", value: "#E040FB" },
            { name: "purple_A400", value: "#D500F9" },
            { name: "purple_A700", value: "#AA00FF" },
        ]
    },
    {
        name: "Deep Purple",
        list: [
            { name: "dark_purple_50", value: "#EDE7F6" },
            { name: "dark_purple_100", value: "#D1C4E9" },
            { name: "dark_purple_200", value: "#B39DDB" },
            { name: "dark_purple_300", value: "#9575CD" },
            { name: "dark_purple_400", value: "#7E57C2" },
            { name: "dark_purple_500", value: "#673AB7" },
            { name: "dark_purple_600", value: "#5E35B1" },
            { name: "dark_purple_700", value: "#512DA8" },
            { name: "dark_purple_800", value: "#4527A0" },
            { name: "dark_purple_900", value: "#311B92" },
            { name: "dark_purple_A100", value: "#B388FF" },
            { name: "dark_purple_A200", value: "#7C4DFF" },
            { name: "dark_purple_A400", value: "#651FFF" },
            { name: "dark_purple_A700", value: "#6200EA" },
        ]
    },
    {
        name: "Indigo",
        list: [
            { name: "indigo_50", value: "#E8EAF6" },
            { name: "indigo_100", value: "#C5CAE9" },
            { name: "indigo_200", value: "#9FA8DA" },
            { name: "indigo_300", value: "#7986CB" },
            { name: "indigo_400", value: "#5C6BC0" },
            { name: "indigo_500", value: "#3F51B5" },
            { name: "indigo_600", value: "#3949AB" },
            { name: "indigo_700", value: "#303F9F" },
            { name: "indigo_800", value: "#283593" },
            { name: "indigo_900", value: "#1A237E" },
            { name: "indigo_A100", value: "#8C9EFF" },
            { name: "indigo_A200", value: "#536DFE" },
            { name: "indigo_A400", value: "#3D5AFE" },
            { name: "indigo_A700", value: "#304FFE" },
        ]
    },
    {
        name: "Blue",
        list: [
            { name: "blue_50", value: "#E3F2FD" },
            { name: "blue_100", value: "#BBDEFB" },
            { name: "blue_200", value: "#90CAF9" },
            { name: "blue_300", value: "#64B5F6" },
            { name: "blue_400", value: "#42A5F5" },
            { name: "blue_500", value: "#2196F3" },
            { name: "blue_600", value: "#1E88E5" },
            { name: "blue_700", value: "#1976D2" },
            { name: "blue_800", value: "#1565C0" },
            { name: "blue_900", value: "#0D47A1" },
            { name: "blue_A100", value: "#82B1FF" },
            { name: "blue_A200", value: "#448AFF" },
            { name: "blue_A400", value: "#2979FF" },
            { name: "blue_A700", value: "#2962FF" },
        ]
    },
    {
        name: "Light Blue",
        list: [
            { name: "light_blue_50", value: "#E1F5FE" },
            { name: "light_blue_100", value: "#B3E5FC" },
            { name: "light_blue_200", value: "#81D4FA" },
            { name: "light_blue_300", value: "#4FC3F7" },
            { name: "light_blue_400", value: "#29B6F6" },
            { name: "light_blue_500", value: "#03A9F4" },
            { name: "light_blue_600", value: "#039BE5" },
            { name: "light_blue_700", value: "#0288D1" },
            { name: "light_blue_800", value: "#0277BD" },
            { name: "light_blue_900", value: "#01579B" },
            { name: "light_blue_A100", value: "#80D8FF" },
            { name: "light_blue_A200", value: "#40C4FF" },
            { name: "light_blue_A400", value: "#00B0FF" },
            { name: "light_blue_A700", value: "#0091EA" },
        ]
    },
    {
        name: "Cyan",
        list: [
            { name: "cyan_50", value: "#E0F7FA" },
            { name: "cyan_100", value: "#B2EBF2" },
            { name: "cyan_200", value: "#80DEEA" },
            { name: "cyan_300", value: "#4DD0E1" },
            { name: "cyan_400", value: "#26C6DA" },
            { name: "cyan_500", value: "#00BCD4" },
            { name: "cyan_600", value: "#00ACC1" },
            { name: "cyan_700", value: "#0097A7" },
            { name: "cyan_800", value: "#00838F" },
            { name: "cyan_900", value: "#006064" },
            { name: "cyan_A100", value: "#84FFFF" },
            { name: "cyan_A200", value: "#18FFFF" },
            { name: "cyan_A400", value: "#00E5FF" },
            { name: "cyan_A700", value: "#00B8D4" },
        ]
    },
    {
        name: "Teal",
        list: [
            { name: "teal_50", value: "#E0F2F1" },
            { name: "teal_100", value: "#B2DFDB" },
            { name: "teal_200", value: "#80CBC4" },
            { name: "teal_300", value: "#4DB6AC" },
            { name: "teal_400", value: "#26A69A" },
            { name: "teal_500", value: "#009688" },
            { name: "teal_600", value: "#00897B" },
            { name: "teal_700", value: "#00796B" },
            { name: "teal_800", value: "#00695C" },
            { name: "teal_900", value: "#004D40" },
            { name: "teal_A100", value: "#A7FFEB" },
            { name: "teal_A200", value: "#64FFDA" },
            { name: "teal_A400", value: "#1DE9B6" },
            { name: "teal_A700", value: "#00BFA5" },
        ]
    },
    {
        name: "Green",
        list: [
            { name: "green_50", value: "#E8F5E9" },
            { name: "green_100", value: "#C8E6C9" },
            { name: "green_200", value: "#A5D6A7" },
            { name: "green_300", value: "#81C784" },
            { name: "green_400", value: "#66BB6A" },
            { name: "green_500", value: "#4CAF50" },
            { name: "green_600", value: "#43A047" },
            { name: "green_700", value: "#388E3C" },
            { name: "green_800", value: "#2E7D32" },
            { name: "green_900", value: "#1B5E20" },
            { name: "green_A100", value: "#B9F6CA" },
            { name: "green_A200", value: "#69F0AE" },
            { name: "green_A400", value: "#00E676" },
            { name: "green_A700", value: "#00C853" },
        ]
    },
    {
        name: "Light Green",
        list: [
            { name: "light_green_50", value: "#F1F8E9" },
            { name: "light_green_100", value: "#DCEDC8" },
            { name: "light_green_200", value: "#C5E1A5" },
            { name: "light_green_300", value: "#AED581" },
            { name: "light_green_400", value: "#9CCC65" },
            { name: "light_green_500", value: "#8BC34A" },
            { name: "light_green_600", value: "#7CB342" },
            { name: "light_green_700", value: "#689F38" },
            { name: "light_green_800", value: "#558B2F" },
            { name: "light_green_900", value: "#33691E" },
            { name: "light_green_A100", value: "#CCFF90" },
            { name: "light_green_A200", value: "#B2FF59" },
            { name: "light_green_A400", value: "#76FF03" },
            { name: "light_green_A700", value: "#64DD17" },
        ]
    },
    {
        name: "Lime",
        list: [
            { name: "lime_50", value: "#F9FBE7" },
            { name: "lime_100", value: "#F0F4C3" },
            { name: "lime_200", value: "#E6EE9C" },
            { name: "lime_300", value: "#DCE775" },
            { name: "lime_400", value: "#D4E157" },
            { name: "lime_500", value: "#CDDC39" },
            { name: "lime_600", value: "#C0CA33" },
            { name: "lime_700", value: "#AFB42B" },
            { name: "lime_800", value: "#9E9D24" },
            { name: "lime_900", value: "#827717" },
            { name: "lime_A100", value: "#F4FF81" },
            { name: "lime_A200", value: "#EEFF41" },
            { name: "lime_A400", value: "#C6FF00" },
            { name: "lime_A700", value: "#AEEA00" },
        ]
    },
    {
        name: "Yellow",
        list: [
            { name: "yellow_50", value: "#FFFDE7" },
            { name: "yellow_100", value: "#FFF9C4" },
            { name: "yellow_200", value: "#FFF59D" },
            { name: "yellow_300", value: "#FFF176" },
            { name: "yellow_400", value: "#FFEE58" },
            { name: "yellow_500", value: "#FFEB3B" },
            { name: "yellow_600", value: "#FDD835" },
            { name: "yellow_700", value: "#FBC02D" },
            { name: "yellow_800", value: "#F9A825" },
            { name: "yellow_900", value: "#F57F17" },
            { name: "yellow_A100", value: "#FFFF8D" },
            { name: "yellow_A200", value: "#FFFF00" },
            { name: "yellow_A400", value: "#FFEA00" },
            { name: "yellow_A700", value: "#FFD600" },
        ]
    },
    {
        name: "Amber",
        list: [
            { name: "amber_50", value: "#FFF8E1" },
            { name: "amber_100", value: "#FFECB3" },
            { name: "amber_200", value: "#FFE082" },
            { name: "amber_300", value: "#FFD54F" },
            { name: "amber_400", value: "#FFCA28" },
            { name: "amber_500", value: "#FFC107" },
            { name: "amber_600", value: "#FFB300" },
            { name: "amber_700", value: "#FFA000" },
            { name: "amber_800", value: "#FF8F00" },
            { name: "amber_900", value: "#FF6F00" },
            { name: "amber_A100", value: "#FFE57F" },
            { name: "amber_A200", value: "#FFD740" },
            { name: "amber_A400", value: "#FFC400" },
            { name: "amber_A700", value: "#FFAB00" },
        ]
    },
    {
        name: "Orange",
        list: [
            { name: "orange_50", value: "#FFF3E0" },
            { name: "orange_100", value: "#FFE0B2" },
            { name: "orange_200", value: "#FFCC80" },
            { name: "orange_300", value: "#FFB74D" },
            { name: "orange_400", value: "#FFA726" },
            { name: "orange_500", value: "#FF9800" },
            { name: "orange_600", value: "#FB8C00" },
            { name: "orange_700", value: "#F57C00" },
            { name: "orange_800", value: "#EF6C00" },
            { name: "orange_900", value: "#E65100" },
            { name: "orange_A100", value: "#FFD180" },
            { name: "orange_A200", value: "#FFAB40" },
            { name: "orange_A400", value: "#FF9100" },
            { name: "orange_A700", value: "#FF6D00" },
        ]
    },
    {
        name: "Deep Orange",
        list: [
            { name: "deep_orange_50", value: "#FBE9E7" },
            { name: "deep_orange_100", value: "#FFCCBC" },
            { name: "deep_orange_200", value: "#FFAB91" },
            { name: "deep_orange_300", value: "#FF8A65" },
            { name: "deep_orange_400", value: "#FF7043" },
            { name: "deep_orange_500", value: "#FF5722" },
            { name: "deep_orange_600", value: "#F4511E" },
            { name: "deep_orange_700", value: "#E64A19" },
            { name: "deep_orange_800", value: "#D84315" },
            { name: "deep_orange_900", value: "#BF360C" },
            { name: "deep_orange_A100", value: "#FF9E80" },
            { name: "deep_orange_A200", value: "#FF6E40" },
            { name: "deep_orange_A400", value: "#FF3D00" },
            { name: "deep_orange_A700", value: "#DD2C00" },
        ]
    },
    {
        name: "Brown",
        list: [
            { name: "brown_50", value: "#EFEBE9" },
            { name: "brown_100", value: "#D7CCC8" },
            { name: "brown_200", value: "#BCAAA4" },
            { name: "brown_300", value: "#A1887F" },
            { name: "brown_400", value: "#8D6E63" },
            { name: "brown_500", value: "#795548" },
            { name: "brown_600", value: "#6D4C41" },
            { name: "brown_700", value: "#5D4037" },
            { name: "brown_800", value: "#4E342E" },
            { name: "brown_900", value: "#3E2723" },
        ]
    },
    {
        name: "Grey",
        list: [
            { name: "grey_50", value: "#FAFAFA" },
            { name: "grey_100", value: "#F5F5F5" },
            { name: "grey_200", value: "#EEEEEE" },
            { name: "grey_300", value: "#E0E0E0" },
            { name: "grey_400", value: "#BDBDBD" },
            { name: "grey_500", value: "#9E9E9E" },
            { name: "grey_600", value: "#757575" },
            { name: "grey_700", value: "#616161" },
            { name: "grey_800", value: "#424242" },
            { name: "grey_900", value: "#212121" },
        ]
    },
    {
        name: "Blue Gray",
        list: [
            { name: "blue_grey_50", value: "#ECEFF1" },
            { name: "blue_grey_100", value: "#CFD8DC" },
            { name: "blue_grey_200", value: "#B0BEC5" },
            { name: "blue_grey_300", value: "#90A4AE" },
            { name: "blue_grey_400", value: "#78909C" },
            { name: "blue_grey_500", value: "#607D8B" },
            { name: "blue_grey_600", value: "#546E7A" },
            { name: "blue_grey_700", value: "#455A64" },
            { name: "blue_grey_800", value: "#37474F" },
            { name: "blue_grey_900", value: "#263238" },
        ]
    }
]

var _minColorIndex = 3      // Colors lighter 300 could be hard to distinguish
var _maxColorIndex = 9      // Use only primary colors
var _startColorIndex = 5    // Start from 500 color
var _colorStep = 3          // Distance betwen next color and current

function colorForChartIndex(index, palettesOrder) {
    var paletteIndex = index % _colors.length
    var colorCount = index / _colors.length | 0
    var colorIndex = ((_startColorIndex - _minColorIndex) + _colorStep * colorCount)
                      % (_maxColorIndex - _minColorIndex + 1) + _minColorIndex

    return _colors[palettesOrder ? palettesOrder[paletteIndex] : paletteIndex].list[colorIndex].value
}

function randomPalettesOrder() {
    var order = []
    for (var i = 0; i < _colors.length; i++) {
        order.push(i)
    }

    return Util.shuffleArray(order)
}
