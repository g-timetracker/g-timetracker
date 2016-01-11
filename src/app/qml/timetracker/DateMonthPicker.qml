import QtQuick 2.0
import QtQuick.Controls 1.4

ItemPositioner {
    property var beginDate: new Date()
    property var endDate: new Date()

    spacing: 10

    ComboBox {
        id: periodSelector

        function calcModel() {
            var months = []
            for (var i = 0; i < 12; i++) {
                months.push(new Date(new Date().setMonth(i, 1)).toLocaleString(Qt.locale(), "MMMM"))
            }

            var result = new Array(12)
            var start = new Date().getMonth();
            for (var count = 0; count < 12; count++) {
                switch (count) {
                case 0:
                    result[count] = "current"
                    break
                case 1:
                    result[count] = "previous"
                    break
                default:
                    result[count] = months[(start - count + 12) % 12]
                    break
                }
            }

            return result
        }

        model: calcModel()

        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                endDate = new Date()
                beginDate = new Date(new Date(new Date(endDate).setDate(1)).setHours(0, 0, 0, 0))
                break
            default:
                endDate = new Date(new Date(new Date().setMonth(new Date().getMonth() - currentIndex + 1, 1)).setHours(0, 0, 0, 0) - 1000)
                beginDate = new Date(new Date(new Date(endDate).setDate(1)).setHours(0, 0, 0, 0))
                break
            }
        }
    }
}
