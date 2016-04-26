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
#ifndef CHANGESETDERIVER_H
#define CHANGESETDERIVER_H

#include <hoot/core/io/ChangesetProvider.h>
#include <hoot/core/io/ElementInputStream.h>

namespace hoot
{

/**
 * Calculates the changeset difference between a source and target map
 */
class ChangesetDeriver : public ChangeSetProvider
{

public:

  ChangesetDeriver(ElementInputStreamPtr from, ElementInputStreamPtr to);

  /**
   * @see ChangeSetProvider
   */
  virtual boost::shared_ptr<OGRSpatialReference> getProjection() const;

  virtual ~ChangesetDeriver();

  /**
   * @see ChangeSetProvider
   */
  virtual void close();

  /**
   * @see ChangeSetProvider
   */
  virtual bool hasMoreChanges();

  /**
   * @see ChangeSetProvider
   */
  virtual Change readNextChange();

private:

  ElementInputStreamPtr _from;
  ElementInputStreamPtr _to;
  Change _next;
  ElementPtr _fromE, _toE;

  Change _nextChange();

};

typedef shared_ptr<ChangesetDeriver> ChangesetDeriverPtr;

}

#endif // CHANGESETDERIVER_H
