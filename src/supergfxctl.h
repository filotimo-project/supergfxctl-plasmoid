/*
    SPDX-FileCopyrightText: 2021 Janghyub Seo <jhyub06@gmail.com>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SUPERGFXCTL_H
#define SUPERGFXCTL_H


#include <Plasma/Applet>

enum class GfxState {
    NVIDIA,
    INTEGRATED,
    COMPUTE,
    VFIO,
    EGPU,
    HYBRID,
};

enum class GfxPower {
    ACTIVE,
    SUSPENDED,
    OFF,
    UNKNOWN,
};

enum class GfxAction {
    LOGOUT,
    REBOOT,
    INTEGRATED,
    NONE,
};

class SuperGFXCtl : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QString stateName READ gfxStateName NOTIFY gfxStateChanged)
    Q_PROPERTY(QString stateIconName READ gfxStateIconName NOTIFY gfxStateChanged)

public:
    SuperGFXCtl( QObject *parent, const QVariantList &args );
    ~SuperGFXCtl();
    QString gfxStateName();
    QString gfxStateIconName();

signals:
    void gfxStateChanged();

private:
    void gfxGet();
    GfxState state;
    GfxPower power;
    GfxAction lastAction;
};

#endif
