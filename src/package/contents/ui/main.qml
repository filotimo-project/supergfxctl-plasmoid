/*
    SPDX-FileCopyrightText: 2021 Janghyub Seo <jhyub06@gmail.com>
    SPDX-License-Identifier: MPL-2.0
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

PlasmoidItem {
    id: mainWindow
    Plasmoid.icon: plasmoid.iconName
    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 10
    Binding {
        target: plasmoid
        property: "status"

        function isPlasmoidActive() {
            if (plasmoid.power.isActive) {
                if (plasmoid.isCharging) {
                    return plasmoid.configuration.statusActiveOnChargeDGPUActive === 0
                } else {
                    return plasmoid.configuration.statusActiveOnDischargeDGPUActive === 0
                }
            } else {
                if (plasmoid.isCharging) {
                    return plasmoid.configuration.statusActiveOnChargeDGPUInactive === 0
                } else {
                    return plasmoid.configuration.statusActiveOnDischargeDGPUInactive === 0
                }
            }
        }

        value: isPlasmoidActive() ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    }
    toolTipSubText: i18n("Graphics mode: %1, Discrete GPU power: %2", plasmoid.mode.name, plasmoid.power.name)
    compactRepresentation: MouseArea {
        property bool wasExpanded
        onPressed: wasExpanded = mainWindow.expanded
        onClicked: mainWindow.expanded = !wasExpanded
        hoverEnabled: true

        Kirigami.Icon {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse

            PlasmaComponents.BusyIndicator {
                anchors.centerIn: parent
                running: plasmoid.realizing != -1
                visible: running
            }

            /*
            PlasmaComponents.ProgressBar {
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                visible: plasmoid.nativeInterface.timeout > 0
                from: -60 // small trick to make the bar not disappear so fast. please tell me if this is resolution-dependent.
                to: 180
                value: plasmoid.nativeInterface.timeout
            }
            */
        }
    }
    fullRepresentation: PlasmaExtras.Representation
    {
        id: dialog
        Layout.minimumWidth: Kirigami.Units.iconSizes.medium * 10
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20
        header: PlasmaExtras.PlasmoidHeading {
            visible: !plasmoid.isDaemonFailing && !plasmoid.isDaemonOutdated
            ColumnLayout {
                RowLayout {
                    Kirigami.Icon {
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        source: plasmoid.power.iconName
                    }
                    PlasmaComponents.Label {
                        text: i18n("Discrete GPU power: %1", plasmoid.power.name)
                    }
                }
                PlasmaComponents.Label {
                    visible: plasmoid.errorMsg.length > 0
                    text: plasmoid.errorMsg
                    color: "red"
                    font.italic: true
                    clip: true
                }
                PlasmaComponents.Label {
                    visible: plasmoid.errorMsg.length > 0
                    text: i18n("Run journalctl -b -u supergfxd for more information.")
                    color: "red"
                    font.italic: true
                }
                RowLayout {
                    visible: plasmoid.isPending
                    PlasmaComponents.Label {
                        text: i18n("%1 is required to switch to %2.", plasmoid.pendingAction.name, plasmoid.pendingMode.name)
                        id: infoLabel
                    }
                    PlasmaComponents.Button {
                        text: i18n("Revert to %1", plasmoid.mode.name)
                        onClicked: plasmoid.revert()
                        icon.name: "edit-undo"
                    }
                }
            }
        }

        PlasmaComponents.ScrollView {
            anchors.fill: parent

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

            visible: !plasmoid.isDaemonFailing && !plasmoid.isDaemonOutdated

            contentItem: ListView {
                id: listView
                clip: true
                model: plasmoid.candidates
                boundsBehavior: Flickable.StopAtBounds
                currentIndex: -1
                topMargin: Kirigami.Units.smallSpacing * 2
                bottomMargin: Kirigami.Units.smallSpacing * 2
                leftMargin: Kirigami.Units.smallSpacing * 2
                rightMargin: Kirigami.Units.smallSpacing * 2
                spacing: Kirigami.Units.smallSpacing
                section.property: "section"
                section.delegate: Loader {
                    active: section != 0 && (section != 3 || plasmoid.configuration.showUnsupported)
                    height: active ? source.height : 0

                    sourceComponent: Item {
                        id: source
                        height: separatorLine.height + Kirigami.Units.smallSpacing
                        width: listView.width - Kirigami.Units.smallSpacing * 4
                        Kirigami.Separator {
                            id: separatorLine
                            width: source.width
                            anchors.top: parent.top
                        }
                    }
                }
                highlight: PlasmaExtras.Highlight { }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                // Was originally PlasmaExtras.ListItem
                // But it had a issue where its height is fixed to zero on initial load
                // Maybe change later?
                delegate: Item {
                    width: listView.width - Kirigami.Units.smallSpacing * 4
                    height: Kirigami.Units.gridUnit * 2
                    required property int section
                    required property string reason
                    required property string name
                    required property string icon
                    required property string buttonText
                    required property string buttonIcon
                    required property int index
                    visible: section != 3 || plasmoid.configuration.showUnsupported
                    MouseArea {
                        anchors {
                            fill: parent
                            leftMargin: Kirigami.Units.smallSpacing
                            rightMargin: Kirigami.Units.smallSpacing
                            topMargin: Kirigami.Units.smallSpacing
                            bottomMargin: Kirigami.Units.smallSpacing
                        }
                        hoverEnabled: true
                        onEntered: listView.currentIndex = index
                        Item {
                            anchors.fill: parent
                            Kirigami.Icon {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    left: parent.left
                                }
                                id: iconItem
                                height: Kirigami.Units.iconSizes.medium
                                width: Kirigami.Units.iconSizes.medium
                                source: icon
                                opacity: (section != 0 && section != 1) ? 0.6 : 1
                            }
                            Item {
                                anchors {
                                    verticalCenter: reason.length == 0 ? parent.verticalCenter : undefined
                                    top: reason.length == 0 ? undefined : parent.top
                                    bottom: reason.length == 0 ? undefined : parent.bottom
                                    left: iconItem.right
                                    right: button.left
                                    leftMargin: Kirigami.Units.smallSpacing
                                }
                                PlasmaComponents.Label {
                                    anchors {
                                        verticalCenter: reason.length == 0 ? parent.verticalCenter : undefined
                                        top: reason.length == 0 ? undefined : parent.top
                                        left: parent.left
                                    }
                                    id: nameLabel
                                    text: name
                                    font.bold: section == 0
                                    opacity: (section != 0 && section != 1) ? 0.6 : 1
                                }
                                PlasmaComponents.Label {
                                    anchors {
                                        top: reason.length == 0 ? undefined : nameLabel.bottom
                                        bottom: reason.length == 0 ? undefined : parent.bottom
                                        left: parent.left
                                    }
                                    id: descriptionLabel
                                    visible: reason.length != 0
                                    text: reason
                                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                    opacity: 0.6
                                }
                            }
                            PlasmaComponents.Button {
                                anchors {
                                    verticalCenter: iconItem.verticalCenter
                                    right: parent.right
                                }
                                id: button
                                flat: true
                                down: section == 0
                                enabled: section == 1 && plasmoid.realizing == -1
                                visible: plasmoid.realizing != index
                                onClicked: plasmoid.realizeCandidate(index)
                                icon.name: buttonIcon
                                text: buttonText
                            }
                            PlasmaComponents.BusyIndicator {
                                 anchors.centerIn: button
                                running: plasmoid.realizing == index
                                 visible: running
                            }
                        }
                    }
                }
            }
        }

        PlasmaExtras.PlaceholderMessage {
            anchors {
                centerIn: parent
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.gridUnit
            }

            visible: plasmoid.isDaemonOutdated

            text: i18n("supergfxd daemon is outdated.")
        }

        PlasmaExtras.PlaceholderMessage {
            anchors {
                centerIn: parent
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.gridUnit
            }

            visible: plasmoid.isDaemonFailing

            text: i18n("Can't connect to daemon.")
        }
    }
}
