/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include <QtTest>
#include <timeline/timelinerenderer_p.h>

using namespace Timeline;

class DummyRenderer : public TimelineRenderer {
    friend class tst_TimelineRenderer;
};

class DummyModel : public TimelineModel {
public:
    DummyModel() : TimelineModel(0, QLatin1String("x")) {}

    void loadData()
    {
        setCollapsedRowCount(3);
        setExpandedRowCount(3);
        for (int i = 0; i < 10; ++i)
            insert(i, i, i);
        for (int i = 0; i < 10; ++i)
            insert(i, 10 - i, i);
        computeNesting();
        emit emptyChanged();
    }
};

class tst_TimelineRenderer : public QObject
{
    Q_OBJECT

private:
    void testMouseEvents(DummyRenderer *renderer, int x, int y);

private slots:
    void updatePaintNode();
    void mouseEvents();
};

void tst_TimelineRenderer::updatePaintNode()
{
    DummyRenderer renderer;
    QCOMPARE(renderer.updatePaintNode(0, 0), static_cast<QSGNode *>(0));
    DummyModel model;
    renderer.setModel(&model);
    QCOMPARE(renderer.updatePaintNode(0, 0), static_cast<QSGNode *>(0));
    model.loadData();
    QCOMPARE(renderer.updatePaintNode(0, 0), static_cast<QSGNode *>(0));
    TimelineZoomControl zoomer;
    renderer.setZoomer(&zoomer);
    QCOMPARE(renderer.updatePaintNode(0, 0), static_cast<QSGNode *>(0));
    zoomer.setTrace(0, 10);
    QSGNode *node = renderer.updatePaintNode(0, 0);
    QVERIFY(node != 0);
    QCOMPARE(renderer.updatePaintNode(node, 0), node);
    renderer.setModelDirty();
    QCOMPARE(renderer.updatePaintNode(node, 0), node);
    renderer.setRowHeightsDirty();
    QCOMPARE(renderer.updatePaintNode(node, 0), node);
    delete node;
}

void tst_TimelineRenderer::testMouseEvents(DummyRenderer *renderer, int x, int y)
{
    QMouseEvent event(QMouseEvent::MouseMove, QPointF(x - 1, y), Qt::NoButton,
                      Qt::NoButton, Qt::NoModifier);
    renderer->mouseMoveEvent(&event);

    QHoverEvent hover(QMouseEvent::HoverMove, QPointF(x, y), QPointF(x - 1, y));
    renderer->hoverMoveEvent(&hover);

    event = QMouseEvent(QMouseEvent::MouseButtonPress, QPointF(x, y), Qt::LeftButton,
                        Qt::LeftButton, Qt::NoModifier);
    renderer->mousePressEvent(&event);

    event = QMouseEvent(QMouseEvent::MouseButtonRelease, QPointF(x, y), Qt::LeftButton,
                        Qt::LeftButton, Qt::NoModifier);
    renderer->mouseReleaseEvent(&event);


}

void tst_TimelineRenderer::mouseEvents()
{
    DummyRenderer renderer;
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);
    testMouseEvents(&renderer, 1, 1); // make sure it doesn't crash without model and zoomer
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);
    renderer.setWidth(10);
    renderer.setHeight(90);
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);

    TimelineZoomControl zoomer;
    renderer.setZoomer(&zoomer);
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);

    DummyModel model;
    renderer.setModel(&model);
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);

    zoomer.setTrace(0, 10);
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);

    model.loadData();
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), 3);
    QCOMPARE(renderer.selectionLocked(), true);

    model.setExpanded(true);
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), 3);
    QCOMPARE(renderer.selectionLocked(), false); // toggle the locked status by clicking same item
    testMouseEvents(&renderer, 1, 1);
    QCOMPARE(renderer.selectedItem(), 3);
    QCOMPARE(renderer.selectionLocked(), true);
    testMouseEvents(&renderer, 1, 40);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), false); // unset locked by clicking empty space
    testMouseEvents(&renderer, 1, 400);
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), false);
    testMouseEvents(&renderer, 10, 1);
    QCOMPARE(renderer.selectedItem(), 19);
    QCOMPARE(renderer.selectionLocked(), true);

    renderer.selectNextFromSelectionId(4);
    QCOMPARE(renderer.selectedItem(), 8);
    QCOMPARE(renderer.selectionLocked(), true); // no toggling as we're clicking a different one
    renderer.selectPrevFromSelectionId(3);
    QCOMPARE(renderer.selectedItem(), 7);
    QCOMPARE(renderer.selectionLocked(), true);

    renderer.clearData();
    QCOMPARE(renderer.selectedItem(), -1);
    QCOMPARE(renderer.selectionLocked(), true);
}

QTEST_MAIN(tst_TimelineRenderer)

#include "tst_timelinerenderer.moc"