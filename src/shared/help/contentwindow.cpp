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

#include "contentwindow.h"

#include <centralwidget.h>
#include <helpviewer.h>
#include <localhelpmanager.h>
#include <openpagesmanager.h>

#include <utils/navigationtreeview.h>

#include <QLayout>
#include <QFocusEvent>
#include <QMenu>

#include <QHelpEngine>
#include <QHelpContentModel>

using namespace Help::Internal;

ContentWindow::ContentWindow()
    : m_contentWidget(0)
    , m_expandDepth(-2)
{
    m_contentModel = (&LocalHelpManager::helpEngine())->contentModel();
    m_contentWidget = new Utils::NavigationTreeView;
    m_contentWidget->setModel(m_contentModel);
    m_contentWidget->setActivationMode(Utils::SingleClickActivation);
    m_contentWidget->installEventFilter(this);
    m_contentWidget->viewport()->installEventFilter(this);
    m_contentWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    setFocusProxy(m_contentWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_contentWidget);

    connect(m_contentWidget, &QWidget::customContextMenuRequested,
            this, &ContentWindow::showContextMenu);
    connect(m_contentWidget, &QTreeView::activated,
            this, &ContentWindow::itemActivated);

    connect(m_contentModel, &QHelpContentModel::contentsCreated,
            this, &ContentWindow::expandTOC);
}

ContentWindow::~ContentWindow()
{
}

void ContentWindow::expandTOC()
{
    if (m_expandDepth > -2) {
        expandToDepth(m_expandDepth);
        m_expandDepth = -2;
    }
}

void ContentWindow::expandToDepth(int depth)
{
    m_expandDepth = depth;
    if (depth == -1)
        m_contentWidget->expandAll();
    else
        m_contentWidget->expandToDepth(depth);
}

bool ContentWindow::eventFilter(QObject *o, QEvent *e)
{
    if (m_contentWidget && o == m_contentWidget->viewport()
        && e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QItemSelectionModel *sm = m_contentWidget->selectionModel();
        if (!sm)
            return QWidget::eventFilter(o, e);

        Qt::MouseButtons button = me->button();
        const QModelIndex &index = m_contentWidget->indexAt(me->pos());

        if (index.isValid() && sm->isSelected(index)) {
            if ((button == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier))
                    || (button == Qt::MidButton)) {
                QHelpContentItem *itm = m_contentModel->contentItemAt(index);
                if (itm)
                    emit linkActivated(itm->url(), true/*newPage*/);
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void ContentWindow::showContextMenu(const QPoint &pos)
{
    if (!m_contentWidget->indexAt(pos).isValid())
        return;

    QHelpContentModel *contentModel =
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    QHelpContentItem *itm =
        contentModel->contentItemAt(m_contentWidget->currentIndex());

    QMenu menu;
    QAction *curTab = menu.addAction(tr("Open Link"));
    QAction *newTab = menu.addAction(tr("Open Link as New Page"));
    if (!HelpViewer::canOpenPage(itm->url().path()))
        newTab->setEnabled(false);

    menu.move(m_contentWidget->mapToGlobal(pos));

    QAction *action = menu.exec();
    if (curTab == action)
        emit linkActivated(itm->url(), false/*newPage*/);
    else if (newTab == action)
        emit linkActivated(itm->url(), true/*newPage*/);
}

void ContentWindow::itemActivated(const QModelIndex &index)
{
    if (QHelpContentItem *itm = m_contentModel->contentItemAt(index))
        emit linkActivated(itm->url(), false/*newPage*/);
}
