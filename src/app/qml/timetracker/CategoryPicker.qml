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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import TimeLog 1.0
import "Texts.js" as Texts

ColumnLayout {
    property alias category: categoryModel.category

    spacing: 16

    TimeLogCategoryDepthModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

    Repeater {
        model: categoryModel

        ItemPositioner {
            LabelControl {
                Layout.fillWidth: true
                text: Texts.labelText(index == 0 ? qsTr("Category") : qsTr("Subcategory"))
            }

            ComboBoxControl {
                id: combobox

                property int subcategory: subcategoryIndex === undefined ? -1 : subcategoryIndex

                currentIndex: subcategory
                model: subcategories ? [ "" ].concat(subcategories) : []

                onCurrentIndexChanged: {
                    if (subcategory != currentIndex) {
                        subcategoryIndex = currentIndex
                    }
                }

                onSubcategoryChanged: {
                    if (currentIndex != subcategory) {
                        currentIndex = subcategory
                    }
                }
            }
        }
    }
}
