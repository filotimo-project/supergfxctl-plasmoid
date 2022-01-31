#include "SuperGfxCtl.h"
#include "DaemonController.h"
#include "GfxModeCandidate.h"
#include <QtQml/QQmlEngine>

SuperGfxCtl::SuperGfxCtl(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args) {
    auto &ctl = DaemonController::from();
    connect(&ctl, &DaemonController::daemonOutdatedChanged, this, &SuperGfxCtl::daemonOutdatedChanged);
    connect(&ctl, &DaemonController::daemonFailingChanged, this, &SuperGfxCtl::daemonFailingChanged);
    connect(&ctl, &DaemonController::modeChanged, this, &SuperGfxCtl::modeChanged);
    connect(&ctl, &DaemonController::modeChanged, this, &SuperGfxCtl::iconNameChanged);
    connect(&ctl, &DaemonController::powerChanged, this, &SuperGfxCtl::powerChanged);
    connect(&ctl, &DaemonController::powerChanged, this, &SuperGfxCtl::iconNameChanged);
    connect(&ctl, &DaemonController::modeChanged, this, &SuperGfxCtl::candidateChanged);
    connect(&ctl, &DaemonController::powerChanged, this, &SuperGfxCtl::candidateChanged);
    connect(&ctl, &DaemonController::supportedChanged, this, &SuperGfxCtl::candidateChanged);
    connect(&ctl, &DaemonController::actionChanged, this, &SuperGfxCtl::expectionChanged);
    connect(&ctl, &DaemonController::actionChanged, this, [this] { m_realizing = -1; emit realizingChanged(); });
}

bool SuperGfxCtl::isDaemonOutdated() const {
    return DaemonController::from().isDaemonOutdated();
}

bool SuperGfxCtl::isDaemonFailing() const {
    return DaemonController::from().isDaemonFailing();
}

GfxMode *SuperGfxCtl::mode() const {
    auto ret = &GfxMode::current();
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

GfxPower *SuperGfxCtl::power() const {
    auto ret = &GfxPower::current();
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QString SuperGfxCtl::iconName() const {
    return GfxMode::current().iconName(GfxPower::current());
}

GfxModeCandidateList *SuperGfxCtl::candidates() {
    if (currentList != previousList && previousList != nullptr) {
        previousList->freeAll();
        delete previousList;
        previousList = currentList;
    }
    currentList = GfxModeCandidateList::from();
    if (previousList == nullptr) {
        previousList = currentList;
    }
    return currentList;
}

void SuperGfxCtl::realizeCandidate(int index) {
    m_realizing = index;
    emit realizingChanged();
    currentList->realize(index);
}

// Assumption: list is sorted, so [0] points to GfxModeCandidate(current, current)
void SuperGfxCtl::revert() {
    realizeCandidate(0);
}

GfxAction *SuperGfxCtl::expectedAction() const {
    auto ret = &GfxAction::current();
    QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
    return ret;
}

QString SuperGfxCtl::errorMsg() const {
    return DaemonController::from().errorMsg();
}

int SuperGfxCtl::realizing() const {
    return m_realizing;
}

K_PLUGIN_CLASS_WITH_JSON(SuperGfxCtl, "metadata.json")

#include "SuperGfxCtl.moc"
