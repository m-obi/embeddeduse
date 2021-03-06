#include <QProcess>
#include <QQuickItem>
#include <QtDebug>
#include <QVariant>

#include "ApplicationSourceModel.h"

namespace {
const QString c_home{"black"};
}

ApplicationSourceModel::ApplicationSourceModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_appInfoColl{
          {QString{"orange"}, true, nullptr, nullptr},
          {QString{"lightgreen"}, true, nullptr, nullptr},
          {c_home, true, nullptr, nullptr},
          {QString{"cyan"}, true, nullptr, nullptr},
          {QString{"magenta"}, true, nullptr, nullptr},
          {QString{"yellow"}, false, nullptr, nullptr},
          {QString{"pink"}, false, nullptr, nullptr}
      }
{
}

QHash<int, QByteArray> ApplicationSourceModel::roleNames() const
{
    static const QHash<int, QByteArray> roleColl{
        { ROLE_COLOR, "color" },
        { ROLE_IS_RUNNING, "isRunning" },
        { ROLE_PROCESS_ID, "processId" },
        { ROLE_IS_HOME, "isHome" },
        { ROLE_APPLICATION_ITEM, "applicationItem" },
        { ROLE_IS_TOP, "isTop" },
    };
    return roleColl;
}

int ApplicationSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_appInfoColl.size();
}

QVariant ApplicationSourceModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        return {};
    }
    auto &appInfo = m_appInfoColl[row];
    switch (role) {
    case ROLE_COLOR:
        return appInfo.m_color;
    case ROLE_IS_RUNNING:
        return appInfo.m_process != nullptr && appInfo.m_process->state() == QProcess::Running;
    case ROLE_PROCESS_ID:
        return appInfo.m_process != nullptr ? appInfo.m_process->processId() : -1;
    case ROLE_IS_HOME:
        return appInfo.m_color == c_home;
    case ROLE_APPLICATION_ITEM: {
        QVariant v;
        v.setValue(static_cast<QObject *>(appInfo.m_item));
        return v;
    }
    case ROLE_IS_TOP:
        return appInfo.m_isTop;
    default:
        return {};
    }
}

bool ApplicationSourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (role) {
    case ROLE_IS_RUNNING:
        if (value.toBool()) {
            auto isRunning = startApplication(index.row());
            emit dataChanged(index, index);
            return isRunning;
        }
        // TODO: Stop application
        return false;
    default:
        return false;
    }
}

bool ApplicationSourceModel::startApplication(int row)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }
    auto &appInfo = m_appInfoColl[row];
    if (appInfo.m_process != nullptr) {
        return true; // Already running
    }
    auto cmd = QString{"../ClientApp/ClientApp -platform wayland %1"}.arg(appInfo.m_color);
    appInfo.m_process = new QProcess{this};
    appInfo.m_process->start(cmd);
    qInfo() << QString{"INFO: Started application %1 with process ID %2."}.arg(appInfo.m_color)
               .arg(appInfo.m_process->processId());
    return true;
}

void ApplicationSourceModel::insertApplicationItem(int processId, QQuickItem *item)
{
    auto row = indexOfProcess(processId);
    if (row != -1) {
        m_appInfoColl[row].m_item = item;
        emit dataChanged(index(row), index(row));
    }
}

int ApplicationSourceModel::indexOfProcess(int processId) const
{
    for (int i = 0; i < m_appInfoColl.size(); ++i) {
        if (m_appInfoColl[i].m_process != nullptr &&
                m_appInfoColl[i].m_process->processId() == processId) {
            return i;
        }
    }
    return -1;
}
