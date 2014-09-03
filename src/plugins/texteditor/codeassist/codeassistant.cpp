/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "codeassistant.h"
#include "completionassistprovider.h"
#include "quickfixassistprovider.h"
#include "iassistprocessor.h"
#include "iassistproposal.h"
#include "iassistproposalwidget.h"
#include "iassistinterface.h"
#include "iassistproposalitem.h"
#include "runner.h"

#include <texteditor/basetexteditor.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/completionsettings.h>
#include <coreplugin/editormanager/editormanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/qtcassert.h>

#include <QKeyEvent>
#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QTimer>

using namespace TextEditor;
using namespace Internal;

namespace {

template <class T>
void filterEditorSpecificProviders(QList<T *> *providers, Core::Id editorId)
{
    typename QList<T *>::iterator it = providers->begin();
    while (it != providers->end()) {
        if ((*it)->supportsEditor(editorId))
            ++it;
        else
            it = providers->erase(it);
    }
}

} // Anonymous

namespace TextEditor {

class CodeAssistantPrivate : public QObject
{
    Q_OBJECT

public:
    CodeAssistantPrivate(CodeAssistant *assistant);
    virtual ~CodeAssistantPrivate();

    void configure(BaseTextEditorWidget *editorWidget);
    void reconfigure();
    bool isConfigured() const;

    void invoke(AssistKind kind, IAssistProvider *provider = 0);
    void process();
    void requestProposal(AssistReason reason, AssistKind kind, IAssistProvider *provider = 0);
    void cancelCurrentRequest();
    void invalidateCurrentRequestData();
    void displayProposal(IAssistProposal *newProposal, AssistReason reason);
    bool isDisplayingProposal() const;
    bool isWaitingForProposal() const;

    void notifyChange();
    bool hasContext() const;
    void destroyContext();

    CompletionAssistProvider *identifyActivationSequence();

    void stopAutomaticProposalTimer();
    void startAutomaticProposalTimer();

    virtual bool eventFilter(QObject *o, QEvent *e);

signals:
    void finished();

private slots:
    void finalizeRequest();
    void proposalComputed();
    void processProposalItem(IAssistProposalItem *proposalItem);
    void handlePrefixExpansion(const QString &newPrefix);
    void finalizeProposal();
    void automaticProposalTimeout();
    void updateCompletionSettings(const TextEditor::CompletionSettings &settings);
    void explicitlyAborted();
    void clearAbortedPosition();

private:
    CodeAssistant *m_q;
    BaseTextEditorWidget *m_editorWidget;
    CompletionAssistProvider *m_completionProvider;
    QList<QuickFixAssistProvider *> m_quickFixProviders;
    Internal::ProcessorRunner *m_requestRunner;
    IAssistProvider *m_requestProvider;
    AssistKind m_assistKind;
    IAssistProposalWidget *m_proposalWidget;
    QScopedPointer<IAssistProposal> m_proposal;
    bool m_receivedContentWhileWaiting;
    QTimer m_automaticProposalTimer;
    CompletionSettings m_settings;
    int m_abortedBasePosition;
    static const QChar m_null;
};

} // TextEditor

// --------------------
// CodeAssistantPrivate
// --------------------
const QChar CodeAssistantPrivate::m_null;

static const int AutomaticProposalTimerInterval = 400;

CodeAssistantPrivate::CodeAssistantPrivate(CodeAssistant *assistant)
    : m_q(assistant)
    , m_editorWidget(0)
    , m_completionProvider(0)
    , m_requestRunner(0)
    , m_requestProvider(0)
    , m_assistKind(TextEditor::Completion)
    , m_proposalWidget(0)
    , m_receivedContentWhileWaiting(false)
    , m_settings(TextEditorSettings::completionSettings())
    , m_abortedBasePosition(-1)
{
    m_automaticProposalTimer.setSingleShot(true);
    m_automaticProposalTimer.setInterval(AutomaticProposalTimerInterval);
    connect(&m_automaticProposalTimer, SIGNAL(timeout()), this, SLOT(automaticProposalTimeout()));

    connect(TextEditorSettings::instance(),
            SIGNAL(completionSettingsChanged(TextEditor::CompletionSettings)),
            this,
            SLOT(updateCompletionSettings(TextEditor::CompletionSettings)));
    connect(Core::EditorManager::instance(), SIGNAL(currentEditorChanged(Core::IEditor*)),
            this, SLOT(clearAbortedPosition()));
}

CodeAssistantPrivate::~CodeAssistantPrivate()
{
}

void CodeAssistantPrivate::configure(BaseTextEditorWidget *editorWidget)
{
    // @TODO: There's a list of providers but currently only the first one is used. Perhaps we
    // should implement a truly mechanism to support multiple providers for an editor (either
    // merging or not proposals) or just leave it as not extensible and store directly the one
    // completion and quick-fix provider (getting rid of the list).

    m_editorWidget = editorWidget;
    m_completionProvider = editorWidget->completionAssistProvider();
    m_quickFixProviders =
        ExtensionSystem::PluginManager::getObjects<QuickFixAssistProvider>();
    filterEditorSpecificProviders(&m_quickFixProviders, m_editorWidget->textDocument()->id());

    m_editorWidget->installEventFilter(this);
    connect(m_editorWidget->textDocument(),SIGNAL(mimeTypeChanged()),
            m_q, SLOT(reconfigure()));
}

void CodeAssistantPrivate::reconfigure()
{
    if (isConfigured())
        m_completionProvider = m_editorWidget->completionAssistProvider();
}

bool CodeAssistantPrivate::isConfigured() const
{
    return m_editorWidget != 0;
}

void CodeAssistantPrivate::invoke(AssistKind kind, IAssistProvider *provider)
{
    if (!isConfigured())
        return;

    stopAutomaticProposalTimer();

    if (isDisplayingProposal() && m_assistKind == kind && !m_proposal->isFragile()) {
        m_proposalWidget->setReason(ExplicitlyInvoked);
        m_proposalWidget->updateProposal(
                    m_editorWidget->textDocument()->textAt(
                        m_proposal->basePosition(),
                        m_editorWidget->position() - m_proposal->basePosition()));
    } else {
        destroyContext();
        requestProposal(ExplicitlyInvoked, kind, provider);
    }
}

void CodeAssistantPrivate::process()
{
    if (!isConfigured())
        return;

    stopAutomaticProposalTimer();

    if (m_assistKind == TextEditor::Completion) {
        if (m_settings.m_completionTrigger != ManualCompletion) {
            if (CompletionAssistProvider *provider = identifyActivationSequence()) {
                if (isWaitingForProposal())
                    cancelCurrentRequest();
                requestProposal(ActivationCharacter, Completion, provider);
                return;
            }
        }

        startAutomaticProposalTimer();
    } else {
        m_assistKind = TextEditor::Completion;
    }
}

void CodeAssistantPrivate::requestProposal(AssistReason reason,
                                           AssistKind kind,
                                           IAssistProvider *provider)
{
    QTC_ASSERT(!isWaitingForProposal(), return);

    if (m_editorWidget->hasBlockSelection())
        return; // TODO

    if (!provider) {
        if (kind == Completion)
            provider = m_completionProvider;
        else if (!m_quickFixProviders.isEmpty())
            provider = m_quickFixProviders.at(0);

        if (!provider)
            return;
    }

    m_assistKind = kind;
    IAssistProcessor *processor = provider->createProcessor();
    IAssistInterface *assistInterface = m_editorWidget->createAssistInterface(kind, reason);
    if (!assistInterface)
        return;

    if (provider->isAsynchronous()) {
        if (IAssistProposal *newProposal = processor->immediateProposal(assistInterface))
            displayProposal(newProposal, reason);

        m_requestProvider = provider;
        m_requestRunner = new ProcessorRunner;
        connect(m_requestRunner, SIGNAL(finished()), this, SLOT(proposalComputed()));
        connect(m_requestRunner, SIGNAL(finished()), this, SLOT(finalizeRequest()));
        connect(m_requestRunner, SIGNAL(finished()), this, SIGNAL(finished()));
        assistInterface->prepareForAsyncUse();
        m_requestRunner->setReason(reason);
        m_requestRunner->setProcessor(processor);
        m_requestRunner->setAssistInterface(assistInterface);
        m_requestRunner->start();
        return;
    }

    if (IAssistProposal *newProposal = processor->perform(assistInterface))
        displayProposal(newProposal, reason);
    delete processor;
}

void CodeAssistantPrivate::cancelCurrentRequest()
{
    m_requestRunner->setDiscardProposal(true);
    disconnect(m_requestRunner, SIGNAL(finished()), this, SLOT(proposalComputed()));
    invalidateCurrentRequestData();
}

void CodeAssistantPrivate::proposalComputed()
{
    // Since the request runner is a different thread, there's still a gap in which the queued
    // signal could be processed after an invalidation of the current request.
    if (m_requestRunner != sender())
        return;

    IAssistProposal *newProposal = m_requestRunner->proposal();
    AssistReason reason = m_requestRunner->reason();
    invalidateCurrentRequestData();
    displayProposal(newProposal, reason);
}

void CodeAssistantPrivate::displayProposal(IAssistProposal *newProposal, AssistReason reason)
{
    if (!newProposal)
        return;

    QScopedPointer<IAssistProposal> proposalCandidate(newProposal);

    if (isDisplayingProposal()) {
        if (!m_proposal->isFragile())
            return;
        destroyContext();
    }

    int basePosition = proposalCandidate->basePosition();
    if (m_editorWidget->position() < basePosition)
        return;

    if (m_abortedBasePosition == basePosition && reason != ExplicitlyInvoked)
        return;

    clearAbortedPosition();
    m_proposal.reset(proposalCandidate.take());

    if (m_proposal->isCorrective())
        m_proposal->makeCorrection(m_editorWidget);

    basePosition = m_proposal->basePosition();
    m_proposalWidget = m_proposal->createWidget();
    connect(m_proposalWidget, SIGNAL(destroyed()), this, SLOT(finalizeProposal()));
    connect(m_proposalWidget, SIGNAL(prefixExpanded(QString)),
            this, SLOT(handlePrefixExpansion(QString)));
    connect(m_proposalWidget, SIGNAL(proposalItemActivated(IAssistProposalItem*)),
            this, SLOT(processProposalItem(IAssistProposalItem*)));
    connect(m_proposalWidget, SIGNAL(explicitlyAborted()),
            this, SLOT(explicitlyAborted()));
    m_proposalWidget->setAssistant(m_q);
    m_proposalWidget->setReason(reason);
    m_proposalWidget->setKind(m_assistKind);
    m_proposalWidget->setUnderlyingWidget(m_editorWidget);
    m_proposalWidget->setModel(m_proposal->model());
    m_proposalWidget->setDisplayRect(m_editorWidget->cursorRect(basePosition));
    if (m_receivedContentWhileWaiting)
        m_proposalWidget->setIsSynchronized(false);
    else
        m_proposalWidget->setIsSynchronized(true);
    m_proposalWidget->showProposal(m_editorWidget->textDocument()->textAt(
                                       basePosition,
                                       m_editorWidget->position() - basePosition));
}

void CodeAssistantPrivate::processProposalItem(IAssistProposalItem *proposalItem)
{
    QTC_ASSERT(m_proposal, return);
    proposalItem->apply(m_editorWidget, m_proposal->basePosition());
    destroyContext();
    process();
}

void CodeAssistantPrivate::handlePrefixExpansion(const QString &newPrefix)
{
    QTC_ASSERT(m_proposal, return);
    const int currentPosition = m_editorWidget->position();
    m_editorWidget->setCursorPosition(m_proposal->basePosition());
    m_editorWidget->replace(currentPosition - m_proposal->basePosition(), newPrefix);
    notifyChange();
}

void CodeAssistantPrivate::finalizeRequest()
{
    if (ProcessorRunner *runner = qobject_cast<ProcessorRunner *>(sender()))
        delete runner;
}

void CodeAssistantPrivate::finalizeProposal()
{
    stopAutomaticProposalTimer();
    m_proposal.reset();
    m_proposalWidget = 0;
    if (m_receivedContentWhileWaiting)
        m_receivedContentWhileWaiting = false;
}

bool CodeAssistantPrivate::isDisplayingProposal() const
{
    return m_proposalWidget != 0;
}

bool CodeAssistantPrivate::isWaitingForProposal() const
{
    return m_requestRunner != 0;
}

void CodeAssistantPrivate::invalidateCurrentRequestData()
{
    m_requestRunner = 0;
    m_requestProvider = 0;
}

CompletionAssistProvider *CodeAssistantPrivate::identifyActivationSequence()
{
    if (!m_completionProvider)
        return 0;

    const int length = m_completionProvider->activationCharSequenceLength();
    if (length == 0)
        return 0;
    QString sequence = m_editorWidget->textDocument()->textAt(m_editorWidget->position() - length,
                                                            length);
    // In pretty much all cases the sequence will have the appropriate length. Only in the
    // case of typing the very first characters in the document for providers that request a
    // length greater than 1 (currently only C++, which specifies 3), the sequence needs to
    // be prepended so it has the expected length.
    const int lengthDiff = length - sequence.length();
    for (int j = 0; j < lengthDiff; ++j)
        sequence.prepend(m_null);
    return m_completionProvider->isActivationCharSequence(sequence) ? m_completionProvider : 0;
}

void CodeAssistantPrivate::notifyChange()
{
    stopAutomaticProposalTimer();

    if (isDisplayingProposal()) {
        QTC_ASSERT(m_proposal, return);
        if (m_editorWidget->position() < m_proposal->basePosition()) {
            destroyContext();
        } else {
            m_proposalWidget->updateProposal(
                m_editorWidget->textDocument()->textAt(m_proposal->basePosition(),
                                     m_editorWidget->position() - m_proposal->basePosition()));
            if (m_proposal->isFragile())
                startAutomaticProposalTimer();
        }
    }
}

bool CodeAssistantPrivate::hasContext() const
{
    return m_requestRunner || m_proposalWidget;
}

void CodeAssistantPrivate::destroyContext()
{
    stopAutomaticProposalTimer();

    if (isWaitingForProposal()) {
        cancelCurrentRequest();
    } else if (isDisplayingProposal()) {
        m_proposalWidget->closeProposal();
        disconnect(m_proposalWidget, SIGNAL(destroyed()), this, SLOT(finalizeProposal()));
        finalizeProposal();
    }
}

void CodeAssistantPrivate::startAutomaticProposalTimer()
{
    if (m_settings.m_completionTrigger == AutomaticCompletion)
        m_automaticProposalTimer.start();
}

void CodeAssistantPrivate::automaticProposalTimeout()
{
    if (isWaitingForProposal() || (isDisplayingProposal() && !m_proposal->isFragile()))
        return;

    requestProposal(IdleEditor, Completion);
}

void CodeAssistantPrivate::stopAutomaticProposalTimer()
{
    if (m_automaticProposalTimer.isActive())
        m_automaticProposalTimer.stop();
}

void CodeAssistantPrivate::updateCompletionSettings(const TextEditor::CompletionSettings &settings)
{
    m_settings = settings;
}

void CodeAssistantPrivate::explicitlyAborted()
{
    QTC_ASSERT(m_proposal, return);
    m_abortedBasePosition = m_proposal->basePosition();
}

void CodeAssistantPrivate::clearAbortedPosition()
{
    m_abortedBasePosition = -1;
}

bool CodeAssistantPrivate::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if (isWaitingForProposal()) {
        QEvent::Type type = e->type();
        if (type == QEvent::FocusOut) {
            destroyContext();
        } else if (type == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
            const QString &keyText = keyEvent->text();

            CompletionAssistProvider *completionProvider = 0;
            if ((keyText.isEmpty()
                 && keyEvent->key() != Qt::LeftArrow
                 && keyEvent->key() != Qt::RightArrow
                 && keyEvent->key() != Qt::Key_Shift)
                || (!keyText.isEmpty()
                    && (((completionProvider = dynamic_cast<CompletionAssistProvider *>(m_requestProvider))
                            ? !completionProvider->isContinuationChar(keyText.at(0))
                            : false)))) {
                destroyContext();
            } else if (!keyText.isEmpty() && !m_receivedContentWhileWaiting) {
                m_receivedContentWhileWaiting = true;
            }
        }
    }

    return false;
}

// -------------
// CodeAssistant
// -------------
CodeAssistant::CodeAssistant() : d(new CodeAssistantPrivate(this))
{
    connect(d, SIGNAL(finished()), SIGNAL(finished()));
}

CodeAssistant::~CodeAssistant()
{
    delete d;
}

void CodeAssistant::configure(BaseTextEditorWidget *editorWidget)
{
    d->configure(editorWidget);
}

void CodeAssistant::process()
{
    d->process();
}

void CodeAssistant::notifyChange()
{
    d->notifyChange();
}

bool CodeAssistant::hasContext() const
{
    return d->hasContext();
}

void CodeAssistant::destroyContext()
{
    d->destroyContext();
}

void CodeAssistant::invoke(AssistKind kind, IAssistProvider *provider)
{
    d->invoke(kind, provider);
}

void CodeAssistant::reconfigure()
{
    d->reconfigure();
}

#include "codeassistant.moc"
