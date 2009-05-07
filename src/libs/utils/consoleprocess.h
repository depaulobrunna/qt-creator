/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
**************************************************************************/

#ifndef CONSOLEPROCESS_H
#define CONSOLEPROCESS_H

#include "abstractprocess.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QProcess>

#include <QtNetwork/QLocalServer>

#ifdef Q_OS_WIN
#include <windows.h>
QT_BEGIN_NAMESPACE
class QWinEventNotifier;
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE
class QSettings;
class QTemporaryFile;
QT_END_NAMESPACE

namespace Core {
namespace Utils {

class QWORKBENCH_UTILS_EXPORT ConsoleProcess : public QObject, public AbstractProcess
{
    Q_OBJECT

public:
    enum Mode { Run, Debug, Suspend };
    ConsoleProcess(QObject *parent = 0);
    ~ConsoleProcess();

    bool start(const QString &program, const QStringList &args);
    void stop();

    void setMode(Mode m) { m_mode = m; }
    Mode mode() const { return m_mode; }

    bool isRunning() const; // This reflects the state of the console+stub
    qint64 applicationPID() const { return m_appPid; }
    int exitCode() const { return m_appCode; } // This will be the signal number if exitStatus == CrashExit
    QProcess::ExitStatus exitStatus() const { return m_appStatus; }

#ifdef Q_OS_UNIX
    void setSettings(QSettings *settings) { m_settings = settings; }
    static QString defaultTerminalEmulator();
    static QString terminalEmulator(const QSettings *settings);
    static void setTerminalEmulator(QSettings *settings, const QString &term);
#endif

signals:
    void processError(const QString &error);
    // These reflect the state of the actual client process
    void processStarted();
    void processStopped();

    // These reflect the state of the console+stub
    void wrapperStarted();
    void wrapperStopped();

private slots:
    void stubConnectionAvailable();
    void readStubOutput();
    void stubExited();
#ifdef Q_OS_WIN
    void inferiorExited();
#endif

private:
    static QString modeOption(Mode m);
    static QString msgCommChannelFailed(const QString &error);
    static QString msgPromptToClose();
    static QString msgCannotCreateTempFile(const QString &why);
    static QString msgCannotCreateTempDir(const QString & dir, const QString &why);
    static QString msgUnexpectedOutput();
    static QString msgCannotChangeToWorkDir(const QString & dir, const QString &why);
    static QString msgCannotExecute(const QString & p, const QString &why);

    QString stubServerListen();
    void stubServerShutdown();
#ifdef Q_OS_WIN
    void cleanupStub();
    void cleanupInferior();
#endif

    Mode m_mode;
    qint64 m_appPid;
    int m_appCode;
    QString m_executable;
    QProcess::ExitStatus m_appStatus;
    QLocalServer m_stubServer;
    QLocalSocket *m_stubSocket;
    QTemporaryFile *m_tempFile;
#ifdef Q_OS_WIN
    PROCESS_INFORMATION *m_pid;
    HANDLE m_hInferior;
    QWinEventNotifier *inferiorFinishedNotifier;
    QWinEventNotifier *processFinishedNotifier;
#else
    QProcess m_process;
    QByteArray m_stubServerDir;
    QSettings *m_settings;
#endif

};

} //namespace Utils
} //namespace Core

#endif
