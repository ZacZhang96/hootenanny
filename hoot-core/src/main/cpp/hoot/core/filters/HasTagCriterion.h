/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015, 2016 DigitalGlobe (http://www.digitalglobe.com/)
 */
#ifndef HASTAGFILTER_H
#define HASTAGFILTER_H

// Hoot
#include "ElementCriterion.h"

// Qt
#include <QStringList>

namespace hoot
{

/**
 * Filters out everything except the specified status.
 */
class HasTagCriterion : public ElementCriterion
{
public:

  static string className() { return "hoot::HasTagCriterion"; }

  HasTagCriterion() {}
  HasTagCriterion(QString key);
  HasTagCriterion(QString key1, QString key2);
  HasTagCriterion(QString key1, QString key2, QString key3);

  void addKey(QString key);

  virtual bool isSatisfied(const shared_ptr<const Element> &e) const;

  virtual ElementCriterion* clone() { return new HasTagCriterion(_keys); }

protected:

  HasTagCriterion(QStringList keys);

private:

  QStringList _keys;
};

}

#endif // HASTAGFILTER_H
