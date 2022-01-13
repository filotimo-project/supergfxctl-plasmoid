/*
    SPDX-FileCopyrightText: 2021 Janghyub Seo <jhyub06@gmail.com>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "supergfxctl.h"
#include <KLocalizedString>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

QString vendorToName(GfxVendor vendor) {
    switch (vendor) {
        case GfxVendor::DEDICATED:
            return {"dedicated"};
        case GfxVendor::INTEGRATED:
            return {"integrated"};
        case GfxVendor::COMPUTE:
            return {"compute"};
        case GfxVendor::VFIO:
            return {"vfio"};
        case GfxVendor::EGPU:
            return {"eGPU"};
        case GfxVendor::HYBRID:
            return {"hybrid"};
        default:
            return {""};
    }
}

SuperGfxCtl::SuperGfxCtl(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args) {
    // open json
    QFile file("/etc/supergfxd.conf");
    file.open(QIODevice::ReadOnly);
    QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();
    isVfioEnabled = json.value("gfx_vfio_enable").toBool();
    file.close();
    // run timer
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SuperGfxCtl::getState);
    timer->setInterval(1000);
    this->getState();
    timer->start();

    connect(timeoutTimer, &QTimer::timeout, this, &SuperGfxCtl::reduceTimer);
    timeoutTimer->setInterval(1000);
    timer->start();
}

SuperGfxCtl::~SuperGfxCtl() {
}

QString SuperGfxCtl::vendorName() {
    return vendorToName(vendor);
}

QString SuperGfxCtl::powerName() {
    switch (power) {
        case GfxPower::ACTIVE:
            return {"active"};
        case GfxPower::SUSPENDED:
            return {"suspended"};
        case GfxPower::OFF:
            return {"off"};
        case GfxPower::UNKNOWN:
            return {"unknown"};
        default: //whatever
            return {"unknown"};
    }
}

QString SuperGfxCtl::iconName() {
    switch (vendor) {
        case GfxVendor::DEDICATED:
            return {"supergfxctl-plasmoid-gpu-dedicated"};
        case GfxVendor::INTEGRATED:
            if (power == GfxPower::ACTIVE) return {"supergfxctl-plasmoid-gpu-integrated-active"};
            else return {"supergfxctl-plasmoid-gpu-integrated"};
        case GfxVendor::COMPUTE:
            if (power == GfxPower::ACTIVE) return {"supergfxctl-plasmoid-gpu-compute-active"};
            else return {"supergfxctl-plasmoid-gpu-compute"};
        case GfxVendor::VFIO:
            if (power == GfxPower::ACTIVE) return {"supergfxctl-plasmoid-gpu-vfio-active"};
            else return {"supergfxctl-plasmoid-gpu-vfio"};
        case GfxVendor::EGPU:
            if (power == GfxPower::ACTIVE) return {"supergfxctl-plasmoid-gpu-egpu-active"};
            else return {"supergfxctl-plasmoid-gpu-egpu"};
        case GfxVendor::HYBRID:
            if (power == GfxPower::ACTIVE) return {"supergfxctl-plasmoid-gpu-hybrid-active"};
            else return {"supergfxctl-plasmoid-gpu-hybrid"};
    }
    return {""};
}

QString SuperGfxCtl::actionName() {
    if (action == GfxAction::REBOOT)
        return {"Reboot"};
    if (action == GfxAction::LOGOUT)
        return {"Logout"};
    return {""};
}

VendorList *SuperGfxCtl::vendorList() {
    auto list = new VendorList();

    enum class Requirement {
        NONE,
        VFIO,
        INTEGRATED,
    };

    enum class Section {
        ACTIVE,
        AVAILABLE,
        UNAVAILABLE,
    };

    Requirement requirement;
    Section section;

    auto dedicated = new QObject(list);
    requirement = Requirement::NONE;
    if (vendor == GfxVendor::DEDICATED) section = Section::ACTIVE;
    else section = Section::AVAILABLE;
    dedicated->setProperty("name", "Dedicated");
    dedicated->setProperty("iconName", "supergfxctl-plasmoid-gpu-dedicated");
    dedicated->setProperty("requirement", static_cast<int>(requirement));
    dedicated->setProperty("section", static_cast<int>(section));
    dedicated->setProperty("gfxIndex", 0);
    list->append(dedicated);

    auto integrated = new QObject(list);
    requirement = Requirement::NONE;
    if (vendor == GfxVendor::INTEGRATED) section = Section::ACTIVE;
    else section = Section::AVAILABLE;
    integrated->setProperty("name", "Integrated");
    integrated->setProperty("iconName", "supergfxctl-plasmoid-gpu-integrated-active");
    integrated->setProperty("requirement", static_cast<int>(requirement));
    integrated->setProperty("section", static_cast<int>(section));
    integrated->setProperty("gfxIndex", 1);
    list->append(integrated);

    auto compute = new QObject(list);
    switch (vendor) {
        case GfxVendor::COMPUTE:
            section = Section::ACTIVE;
            requirement = Requirement::NONE;
            break;
        case GfxVendor::INTEGRATED:
        case GfxVendor::VFIO:
            section = Section::AVAILABLE;
            requirement = Requirement::NONE;
            break;
        default:
            section = Section::UNAVAILABLE;
            requirement = Requirement::INTEGRATED;
    }
    compute->setProperty("name", "Compute");
    compute->setProperty("iconName", "supergfxctl-plasmoid-gpu-compute-active");
    compute->setProperty("requirement", static_cast<int>(requirement));
    compute->setProperty("section", static_cast<int>(section));
    compute->setProperty("gfxIndex", 2);
    list->append(compute);

    auto vfio = new QObject(list);
    switch (vendor) {
        case GfxVendor::VFIO:
            section = Section::ACTIVE;
            requirement = Requirement::NONE;
            break;
        case GfxVendor::INTEGRATED:
        case GfxVendor::COMPUTE:
            if (isVfioEnabled) {
                section = Section::AVAILABLE;
                requirement = Requirement::NONE;
            } else {
                section = Section::UNAVAILABLE;
                requirement = Requirement::VFIO;
            }
            break;
        default:
            section = Section::UNAVAILABLE;
            if (isVfioEnabled) requirement = Requirement::INTEGRATED;
            else requirement = Requirement::VFIO;
    }
    vfio->setProperty("name", "vfio");
    vfio->setProperty("iconName", "supergfxctl-plasmoid-gpu-vfio-active");
    vfio->setProperty("requirement", static_cast<int>(requirement));
    vfio->setProperty("section", static_cast<int>(section));
    vfio->setProperty("gfxIndex", 3);
    list->append(vfio);

    auto hybrid = new QObject(list);
    requirement = Requirement::NONE;
    if (vendor == GfxVendor::HYBRID) section = Section::ACTIVE;
    else section = Section::AVAILABLE;
    hybrid->setProperty("name", "Hybrid");
    hybrid->setProperty("iconName", "supergfxctl-plasmoid-gpu-hybrid-active");
    hybrid->setProperty("requirement", static_cast<int>(requirement));
    hybrid->setProperty("section", static_cast<int>(section));
    hybrid->setProperty("gfxIndex", 5);
    list->append(hybrid);

    list->orderSections();

    if (currentList != nullptr) oldList = currentList;
    currentList = list;
    delete oldList;

    return list;
}

void SuperGfxCtl::revertVendor() {
    setVendor(vendor);
}

void SuperGfxCtl::setVendor(int gfxIndex) {
    setVendor(static_cast<GfxVendor>(gfxIndex));
}

void SuperGfxCtl::setVendor(GfxVendor vendor) {
    mLoadingGfxIdx = static_cast<int>(vendor);
    emit loadingChanged();
    QDBusConnection bus = QDBusConnection::systemBus();
    auto *interface = new QDBusInterface("org.supergfxctl.Daemon",
                                         "/org/supergfxctl/Gfx",
                                         "org.supergfxctl.Daemon",
                                         bus,
                                         this);
    auto pcall = interface->asyncCall("SetVendor", static_cast<quint32>(vendor));
    auto *watcher = new QDBusPendingCallWatcher(pcall, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher * )), this,
            SLOT(finishSetVendorCall(QDBusPendingCallWatcher * )));
    delete interface;
}

void SuperGfxCtl::finishSetVendorCall(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<quint32> reply = *watcher;
    if (reply.isValid()) {
        mErrorMessage = "";
        auto newAction = static_cast<GfxAction>(reply.value());
        if (action != newAction) {
            if (newAction == GfxAction::LOGOUT) {
                timeoutTimer->stop();
                mTimeout = 180;
                timeoutTimer->start();
                emit timeoutChanged();
            } else if (action == GfxAction::LOGOUT) { // if current logout action disappears
                timeoutTimer->stop();
                mTimeout = 0;
                emit timeoutChanged();
            }
            action = newAction;
            emit actionChanged();
        }
    }
    if (reply.isError()) {
        mErrorMessage = reply.error().message();
    }
    mLoadingGfxIdx = -1;
    emit loadingChanged();
    delete watcher;
}

int SuperGfxCtl::loadingGfxIdx() {
    return mLoadingGfxIdx;
}

bool SuperGfxCtl::isSelectEnabled() {
    return action != GfxAction::REBOOT && action != GfxAction::LOGOUT;
}

int SuperGfxCtl::timeout() {
    return mTimeout;
}

void SuperGfxCtl::reduceTimer() {
    if (mTimeout > 0) {
        mTimeout--;
        if (mTimeout == 0) {
            action = GfxAction::NONE;
            actionChanged();
        }
        timeoutChanged();
    }
}

QString SuperGfxCtl::errorMessage() {
    return mErrorMessage;
}

void SuperGfxCtl::getState() {
    QDBusConnection bus = QDBusConnection::systemBus();
    auto *interface = new QDBusInterface("org.supergfxctl.Daemon",
                                         "/org/supergfxctl/Gfx",
                                         "org.supergfxctl.Daemon",
                                         bus,
                                         this);
    auto pcall1 = interface->asyncCall("Vendor");
    auto *watcher1 = new QDBusPendingCallWatcher(pcall1, this);
    auto pcall2 = interface->asyncCall("Power");
    auto *watcher2 = new QDBusPendingCallWatcher(pcall2, this);
    connect(watcher1, SIGNAL(finished(QDBusPendingCallWatcher * )), this,
            SLOT(finishGetVendorCall(QDBusPendingCallWatcher * )));
    connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher * )), this,
            SLOT(finishGetPowerCall(QDBusPendingCallWatcher * )));
    delete interface;
}

void SuperGfxCtl::finishGetVendorCall(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<quint32> reply = *watcher;
    if (reply.isValid()) {
        auto newVendor = static_cast<GfxVendor>(reply.value());
        if (vendor != newVendor) {
            vendor = newVendor;
            emit stateChanged();
        }
    }
    delete watcher;
}

void SuperGfxCtl::finishGetPowerCall(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<quint32> reply = *watcher;
    if (reply.isValid()) {
        auto newPower = static_cast<GfxPower>(reply.value());
        if (power != newPower) {
            power = newPower;
            emit stateChanged();
        }
    }
    delete watcher;
}

K_PLUGIN_CLASS_WITH_JSON(SuperGfxCtl, "metadata.json")

#include "supergfxctl.moc"
