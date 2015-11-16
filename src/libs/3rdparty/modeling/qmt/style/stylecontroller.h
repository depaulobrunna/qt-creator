/***************************************************************************
**
** Copyright (C) 2015 Jochen Becher
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

#ifndef QMT_STYLECONTROLLER_H
#define QMT_STYLECONTROLLER_H

#include <QObject>

#include "styleengine.h"
#include "qmt/diagram/dobject.h"

#include <QScopedPointer>

namespace qmt {

class Style;
class ObjectVisuals;
class StyledObject;
class StyledRelation;
class DAnnotation;
class DBoundary;

class QMT_EXPORT StyleController : public QObject
{
    Q_OBJECT
    class Parameters;

public:
    explicit StyleController(QObject *parent = 0);
    ~StyleController();

    bool suppressGradients() const { return m_suppressGradients; }
    void setSuppressGradients(bool suppressGradients);

    const Style *adaptStyle(StyleEngine::ElementType elementType);
    const Style *adaptObjectStyle(StyleEngine::ElementType elementType, const ObjectVisuals &objectVisuals);
    const Style *adaptObjectStyle(const StyledObject &object);
    const Style *adaptRelationStyle(const StyledRelation &relation);
    const Style *adaptAnnotationStyle(const DAnnotation *annotation);
    const Style *adaptBoundaryStyle(const DBoundary *boundary);
    const Style *relationStarterStyle();

private:
    QScopedPointer<Style> m_defaultStyle;
    QScopedPointer<Style> m_relationStarterStyle;
    QScopedPointer<StyleEngine> m_defaultStyleEngine;
    bool m_suppressGradients;
};

} // namespace qmt

#endif // QMT_STYLECONTROLLER_H