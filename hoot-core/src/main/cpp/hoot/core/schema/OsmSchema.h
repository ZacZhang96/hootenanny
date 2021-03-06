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
 * @copyright Copyright (C) 2015, 2016, 2017, 2018 DigitalGlobe (http://www.digitalglobe.com/)
 */

#ifndef OSMSCHEMA_H
#define OSMSCHEMA_H

// hoot
#include <hoot/core/elements/Element.h>
#include <hoot/core/schema/SchemaVertex.h>

// Qt
#include <QSet>
#include <QString>

// Standard
#include <stdint.h>
#include <vector>

namespace hoot
{

class Tags;

enum EdgeType
{
  CanHave,
  IsA,
  SimilarTo,
  ParentOf,
  AssociatedWith,
  CompoundComponent,
};

struct OsmSchemaCategory {
  enum Type
  {
    Empty =           0x00,
    Poi =             0x01,
    Building =        0x02,
    Transportation =  0x04,
    Use =             0x08,
    Name =            0x10,
    PseudoName =      0x20,
    HgisPoi =         0x40, // Human Geography POI. See ticket #6853 for a definition of a "HGIS POI"
    Multiuse =        0x80,
    All = Poi | Building | Transportation | Use | Name | HgisPoi | Multiuse
  };

  OsmSchemaCategory() : _type(Empty) {}
  OsmSchemaCategory(Type t) : _type(t) {}

  static OsmSchemaCategory building() { return OsmSchemaCategory(Building); }
  static OsmSchemaCategory hgisPoi() { return OsmSchemaCategory(HgisPoi); }
  static OsmSchemaCategory poi() { return OsmSchemaCategory(Poi); }
  static OsmSchemaCategory transportation() { return OsmSchemaCategory(Transportation); }
  static OsmSchemaCategory use() { return OsmSchemaCategory(Use); }
  static OsmSchemaCategory name() { return OsmSchemaCategory(Name); }
  static OsmSchemaCategory pseudoName() { return OsmSchemaCategory(PseudoName); }
  static OsmSchemaCategory multiUse() { return OsmSchemaCategory(Multiuse); }

  bool operator==(OsmSchemaCategory t) const { return t._type == _type; }
  bool operator!=(OsmSchemaCategory t) const { return t._type != _type; }

  Type getEnum() const { return _type; }

  static OsmSchemaCategory fromString(const QString& s)
  {
    if (s == "poi")
    {
      return Poi;
    }
    else if (s == "building")
    {
      return Building;
    }
    else if (s == "transportation")
    {
      return Transportation;
    }
    else if (s == "use")
    {
      return Use;
    }
    else if (s == "name")
    {
      return Name;
    }
    else if (s == "pseudoname")
    {
      return PseudoName;
    }
    else if (s == "hgispoi")
    {
      return HgisPoi;
    }
    else if (s == "multiuse")
    {
      return Multiuse;
    }
    else if (s == "")
    {
      return Empty;
    }
    else
    {
      throw IllegalArgumentException("Unknown category. " + s);
    }
  }

  static OsmSchemaCategory fromStringList(const QStringList& s);

  /**
   * Returns true if all of the categories in c are in this. If this is empty then it always returns
   * true.
   */
  bool contains(const OsmSchemaCategory& c) const
  {
    return (getEnum() & c.getEnum()) == c.getEnum();
  }

  /**
   * Returns true if any of the categories in c are the same as the categories in this. If one or
   * both are empty then it returns false.
   */
  bool intersects(const OsmSchemaCategory& c) const
  {
    return (getEnum() & c.getEnum()) != Empty;
  }

  QString toString() const { return toStringList().join(","); }

  QStringList toStringList() const
  {
    QStringList result;

    if (_type & Poi)
    {
      result << "poi";
    }
    if (_type & Building)
    {
      result << "building";
    }
    if (_type & Transportation)
    {
      result << "transportation";
    }
    if (_type & Use)
    {
      result << "use";
    }
    if (_type & Name)
    {
      result << "name";
    }
    if (_type & PseudoName)
    {
      result << "pseudoname";
    }
    if (_type & HgisPoi)
    {
      result << "hgispoi";
    }
    if (_type & Multiuse)
    {
      result << "multiuse";
    }

    return result;
  }

private:

  Type _type;
};

inline OsmSchemaCategory operator&(const OsmSchemaCategory t1, const OsmSchemaCategory t2)
{
  return OsmSchemaCategory((OsmSchemaCategory::Type)(t1.getEnum() & t2.getEnum()));
}

inline OsmSchemaCategory operator|(const OsmSchemaCategory t1, const OsmSchemaCategory t2)
{
  return OsmSchemaCategory((OsmSchemaCategory::Type)(t1.getEnum() | t2.getEnum()));
}

// explicitly put these types in their own name scope. Use with OsmGeometries::Node, etc.
struct OsmGeometries
{
  enum Type : uint16_t
  {
    Empty =       0x00,
    Node =        0x01,
    Area =        0x02,
    LineString =  0x04,
    ClosedWay =   0x08,
    Way =         Area | LineString | ClosedWay,
    Relation =    0x10,
    All =         Relation | Way | Node
  };

  static Type fromString(const QString& s);
};

struct TagEdge
{
public:

  TagEdge()
  {
    show = true;
  }

  enum EdgeType type;
  double similarToWeight;
  double averageWeight;
  bool show;
  double w1;
  double w2;
};

class OsmSchemaData;
class Relation;
class Way;

/**
 * This class is reentrant, but not thread safe.
 */
class OsmSchema
{
public:

  OsmSchema();

  virtual ~OsmSchema();

  void addAssociatedWith(QString name1, QString name2);

  void addIsA(QString name1, QString name2);

  void addSimilarTo(QString name1, QString name2, double weight, bool oneway = false);

  QString average(const QString& kvp1, double w1, const QString& kvp2, double w2, double& best);

  QString average(const QString& kvp1, const QString& kvp2, double& best);

  /**
   * ONLY FOR UNIT TESTING. Be a good neighbor and call loadDefault() when you're done.
   */
  void createTestingGraph();

  /**
   * Searches for the first common ancestor between two key value pairs. If there is no common
   * ancestor then an empty TagVertex is returned.
   */
  const SchemaVertex& getFirstCommonAncestor(const QString& kvp1, const QString& kvp2);

  std::vector<SchemaVertex> getAssociatedTags(QString name);

  OsmSchemaCategory getCategories(const Tags& t) const;
  OsmSchemaCategory getCategories(const QString& k, const QString& v) const;
  OsmSchemaCategory getCategories(const QString& kvp) const;

  std::vector<SchemaVertex> getAllTags();

  QSet<QString> getAllTagKeys();

  bool hasTagKey(const QString key);

  std::vector<SchemaVertex> getChildTags(QString name);

  static OsmSchema& getInstance();

  double getIsACost() const;

  /**
   * Returns all tags that have a similar score >= minimumScore.
   *
   * minimumScore must be > 0.
   */
  std::vector<SchemaVertex> getSimilarTags(QString name, double minimumScore);

  std::vector<SchemaVertex> getTagByCategory(OsmSchemaCategory c) const;

  /**
   * Returns the tag vertex for a given kvp. If the vertex is compound then an empty vertex will
   * be returned.
   */
  const SchemaVertex& getTagVertex(const QString& kvp) const;

  /**
   * Returns all schema vertices that are represented in the set of tags. This will return both
   * compound vertices and tag vertices.
   */
  std::vector<SchemaVertex> getSchemaVertices(const Tags& tags) const;

  /**
   * Returns all the schema vertices in the set of tags that do not also have parent vertices in the
   * set of tags. E.g. returns railway_platform, but not public_transit=platform.
   *
   * "Unique" may not be the best modifier in the method name, but "WithParentTagsRemoved" seemed
   * a bit verbose. Open to suggestions. -JRS
   */
  std::vector<SchemaVertex> getUniqueSchemaVertices(const Tags& tags) const;

  /**
   * Returns true if at least one tag in the set of specified tags is part of the specified
   * category.
   */
  bool hasCategory(const Tags& t, const QString& category) const;

  /**
   * Returns true if the specified kvp is part of the specified category.
   */
  bool hasCategory(const QString& kvp, const QString& category) const;

  bool isAncestor(const QString& childKvp, const QString& parentKvp);

  bool isArea(const Tags& t, const ElementType& type) const;
  bool isArea(const ConstElementPtr& e) const;

  bool isAreaForStats(const Tags& t, const ElementType& type) const;
  bool isAreaForStats(const ConstElementPtr& e) const;

  bool allowsFor(const Tags& t, const ElementType& type, OsmGeometries::Type geometries);
  bool allowsFor(const ConstElementPtr& e, OsmGeometries::Type geometries);

  bool isNonBuildingArea(const ConstElementPtr& e) const;

  bool isBuilding(const Tags& t, const ElementType& type) const;
  bool isBuilding(const ConstElementPtr& e) const;

  /**
   * Returns true if this is a building:part. This is mutually exclusive with isBuilding.
   */
  bool isBuildingPart(const Tags& t, const ElementType& type) const;
  bool isBuildingPart(const ConstElementPtr& e) const;

  /**
   * Determines whether the element passed in is a polygon under the POI to Polygon conflation
   * definition
   *
   * @param e element to determine type of
   * @param tagIgnoreList if the input feature contains any of the tags in this list this method
   * will always return false
   * @return true if the element meets the specified criteria; false otherwise
   */
  bool isPoiPolygonPoly(const ConstElementPtr& e,
                        const QStringList tagIgnoreList = QStringList()) const;

  /**
   * Determines whether the element passed in is a POI under the POI to Polygon conflation
   * definition
   *
   * @param e element to determine type of
   * @param tagIgnoreList if the input feature contains any of the tags in this list this method
   * will always return false
   * @return true if the element meets the specified criteria; false otherwise
   */
  bool isPoiPolygonPoi(const ConstElementPtr& e,
                       const QStringList tagIgnoreList = QStringList()) const;

  /**
   * Returns true if this is a geometry collection.
   */
  bool isCollection(const Element& e) const;

  /**
   * Returns true if this is a POI as defined by the Tampa DG group.
   */
  bool isHgisPoi(const Element& e) const;

  /**
   * Returns true if the element is a highway type (e.g. road, primary, path, etc.)
   *
   * This is not an exhaustive list, be sure and check the function to make sure it will do what
   * you expect in your instance.
   */
  bool isLinearHighway(const Tags& t, const ElementType& type) const;

  /**
   * Returns true if the element is a linear object (e.g. road, etc.)
   *
   * This is not an exhaustive list, be sure and check the function to make sure it will do what
   * you expect in your instance.
   */
  bool isLinear(const Element& e) const;

  /**
   * Returns true if the specified element is a linear waterway.
   */
  bool isLinearWaterway(const Element& e);

  /**
   * Returns true if the specified element is a power utility line.
   */
  bool isPowerLine(const Element& e) const;

  /**
   * Returns true if the element is a roundabout
   *
   * This is not an exhaustive check, feel free to add more criteria
   */
  bool isRoundabout(const Tags& tags, const ElementType& type) const;

  /**
   * Returns true if the specified element is a multi-use building.
   */
  bool isMultiUseBuilding(const Element& e) const;

  /**
   * Returns true if the specified element is multi-use.
   */
  bool isMultiUse(const Element& e) const;

  /**
   * Returns true if this is a list of values. Right now this just looks for a semicolon in value,
   * but in the future the list of valid list keys may be stored in the schema file.
   */
  bool isList(const QString& key, const QString& value) const;

  /**
   * Returns true if the kvp contains metadata about the feature as opposed to real information
   * about the features.
   *
   * Tags such as "hoot:status", "source", "uuid" are all metadata.
   *
   * Tags such as "name", "highway" and "height" are not metadata.
   */
  bool isMetaData(const QString& key, const QString& value);

  bool isMultiLineString(const Relation& r) const;

  /**
   * Returns true if this is a unidirectional way (e.g. oneway=true)
   */
  bool isOneWay(const Element &e) const;

  bool isPoi(const Element& e) const;

  /**
   * Determines whether an element has a name
   *
   * @param element the element to examine
   * @return true if the element has a name tag; false otherwie
   */
  bool hasName(const Element& element) const;

  /**
   * Returns true if the specified element is a railway.
   */
  bool isRailway(const Element& e);

  /**
   * Returns true if this is a reversed unidirectional way. (E.g. oneway=reverse)
   */
  bool isReversed(const Element& e) const;

  /**
   * Return true if this tag can contain free-form text.
   */
  bool isTextTag(const QString& key) { return getTagVertex(key).valueType == Text; }

  /**
   * Loads the default configuration. This should only be used by unit tests.
   */
  void loadDefault();

  double score(const QString& kvp1, const QString& kvp2);
  double score(const SchemaVertex& v1, const SchemaVertex& v2);

  /**
   * @brief scoreOneWay Returns a oneway score. E.g. highway=primary is similar to highway=road,
   *  but a highway=road isn't necessarily similar to a highway=primary (so it gets a low score).
   */
  double scoreOneWay(const QString& kvp1, const QString& kvp2);

  /**
   * Sets the cost when traversing up the tree to a parent node. This is useful for strict score
   * checking rather than equivalent tags.
   */
  void setIsACost(double cost);

  QString toGraphvizString() const;

  static QString toKvp(const QString& key, const QString& value);

  void update();

  void updateOrCreateVertex(const SchemaVertex& tv);

  /**
   * Determines tags are contained in a list of tags
   *
   * @param tags tags to examine
   * @param tagList string tag list to check against
   * @return true if any tag in a set of input tags is contained in a string list of tags; false
   * otherwise
   * @todo If we came up with a way to support key=* wildcards in the Tags class, then we could
   * probably get rid of this method altogether and just check against the tags whereever this
   * method is being called.
   */
  bool containsTagFromList(const Tags& tags, const QStringList tagList) const;

private:

  // the templates we're including take a crazy long time to include, so I'm isolating the
  // implementation.
  OsmSchemaData* d;
  static boost::shared_ptr<OsmSchema> _theInstance;
  SchemaVertex _empty;

  /// Provide caching for isMetaData
  QHash<QString, bool> _metadataKey;

  QSet<QString> _allTagKeysCache;
};

}

#endif // OSMSCHEMA_H
