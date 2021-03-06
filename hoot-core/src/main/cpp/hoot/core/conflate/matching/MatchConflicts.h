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
 * @copyright Copyright (C) 2015, 2017, 2018 DigitalGlobe (http://www.digitalglobe.com/)
 */
#ifndef MATCHCONFLICTS_H
#define MATCHCONFLICTS_H

// hoot
#include <hoot/core/OsmMap.h>

// Qt
#include <QMultiHash>

// Standard
#include <vector>

namespace hoot
{

class Match;

class MatchConflicts
{
public:
  typedef QMultiHash<size_t, size_t> ConflictMap;
  typedef std::multimap<ElementId, size_t> EidIndexMap;

  MatchConflicts(const ConstOsmMapPtr& map);

  EidIndexMap calculateEidIndexMap(const std::vector<const Match*>& matches) const;

  /**
   * Calculates all the conflicts between matches and puts the indexes to the conflicting pairs in
   * the provided conflicts set. conflicts is cleared before inserting conflicts.
   */
  void calculateMatchConflicts(const std::vector<const Match *> &matches, ConflictMap& conflicts);

  void calculateSubsetConflicts(const std::vector<const Match*>& matches, ConflictMap& conflicts,
                                const std::vector<int>& matchSet);

private:
  const ConstOsmMapPtr& _map;

};

}

#endif // MATCHCONFLICTS_H
