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
 * @copyright Copyright (C) 2015, 2016, 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */
#include "OsmXmlWriter.h"

// Boost
using namespace boost;

// Hoot
#include <hoot/core/OsmMap.h>
#include <hoot/core/elements/Node.h>
#include <hoot/core/elements/Relation.h>
#include <hoot/core/elements/Tags.h>
#include <hoot/core/elements/Way.h>
#include <hoot/core/index/OsmMapIndex.h>
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/util/Exception.h>
#include <hoot/core/util/Factory.h>
#include <hoot/core/util/MetadataTags.h>
#include <hoot/core/util/OsmUtils.h>
#include <hoot/core/visitors/CalculateMapBoundsVisitor.h>

// Qt
#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QXmlStreamWriter>

using namespace geos::geom;
using namespace std;

namespace hoot
{

unsigned int OsmXmlWriter::logWarnCount = 0;

HOOT_FACTORY_REGISTER(OsmMapWriter, OsmXmlWriter)

OsmXmlWriter::OsmXmlWriter() :
_formatXml(ConfigOptions().getOsmMapWriterFormatXml()),
_includeIds(false),
_includeDebug(ConfigOptions().getWriterIncludeDebugTags()),
_includePointInWays(false),
_includeCompatibilityTags(true),
_textStatus(ConfigOptions().getWriterTextStatus()),
_osmSchema(ConfigOptions().getOsmMapWriterSchema()),
_timestamp("1970-01-01T00:00:00Z"),
_precision(round(ConfigOptions().getWriterPrecision())),
_encodingErrorCount(0)
{
}

OsmXmlWriter::~OsmXmlWriter()
{
  close();
}

QString OsmXmlWriter::removeInvalidCharacters(const QString& s)
{
  // See #3553 for an explanation.

  //TODO: refactor this

  QString result;
  result.reserve(s.size());

  bool foundError = false;
  for (int i = 0; i < s.size(); i++)
  {
    QChar c = s[i];
    // See http://stackoverflow.com/questions/730133/invalid-characters-in-xml
    if (c < 0x20 && c != 0x9 && c != 0xA && c != 0xD)
    {
      foundError = true;
    }
    else
    {
      result.append(c);
    }
  }

  if (foundError)
  {
    _encodingErrorCount++;
    if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
    {
      LOG_WARN("Found an invalid character in string: '" << s << "'");
      LOG_WARN("  UCS-4 version of the string: " << s.toUcs4());
    }
    else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
    {
      LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
    }
    logWarnCount++;
  }

  return result;
}

void OsmXmlWriter::open(QString url)
{
  QFile* f = new QFile();
  _fp.reset(f);
  f->setFileName(url);
  if (!_fp->open(QIODevice::WriteOnly | QIODevice::Text))
  {
    throw Exception(QObject::tr("Error opening %1 for writing").arg(url));
  }

  _writer.reset(new QXmlStreamWriter(_fp.get()));
  _writer->setCodec("UTF-8");

  if (_formatXml)
  {
    _writer->setAutoFormatting(true);
  }

  _writer->writeStartDocument();

  _writer->writeStartElement("osm");
  _writer->writeAttribute("version", "0.6");
  _writer->writeAttribute("generator", "hootenanny");

   _bounds.init();
}

void OsmXmlWriter::close()
{
  if (_writer.get())
  {
    _writer->writeEndElement();
    _writer->writeEndDocument();
  }

  if (_fp.get())
  {
    _fp->close();
  }
}

void OsmXmlWriter::setIncludeCompatibilityTags(bool includeCompatibility)
{
  _includeCompatibilityTags = includeCompatibility;
}

QString OsmXmlWriter::toString(const ConstOsmMapPtr& map)
{
  OsmXmlWriter writer;
  // this will be deleted by the _fp auto_ptr
  QBuffer* buf = new QBuffer();
  writer._fp.reset(buf);
  if (!writer._fp->open(QIODevice::WriteOnly | QIODevice::Text))
  {
    throw InternalErrorException(QObject::tr("Error opening QBuffer for writing. Odd."));
  }
  writer.write(map);
  return QString::fromUtf8(buf->data(), buf->size());
}

QString OsmXmlWriter::_typeName(ElementType e)
{
  switch(e.getEnum())
  {
  case ElementType::Node:
    return "node";
  case ElementType::Way:
    return "way";
  case ElementType::Relation:
    return "relation";
  default:
    throw HootException("Unexpected element type.");
  }
}

void OsmXmlWriter::write(ConstOsmMapPtr map, const QString& path)
{
  open(path);
  write(map);
}

void OsmXmlWriter::write(ConstOsmMapPtr map)
{
  if (!_fp.get() || _fp->isWritable() == false)
  {
    throw HootException("Please open the file before attempting to write.");
  }

  //TODO: the coord sys and schema entries currently won't get written during streaming writing

  int epsg = map->getProjection()->GetEPSGGeogCS();
  if (epsg > -1)
  {
    _writer->writeAttribute("srs", QString("+epsg:%1").arg(epsg));
  }
  else
  {
    char *wkt;
    map->getProjection()->exportToWkt(&wkt);
    _writer->writeAttribute("srs", wkt);
    free(wkt);
  }

  if (_osmSchema != "")
  {
    _writer->writeAttribute("schema", _osmSchema);
  }

  const geos::geom::Envelope bounds = CalculateMapBoundsVisitor::getGeosBounds(map);
  _writeBounds(bounds);

  _writeNodes(map);
  _writeWays(map);
  _writeRelations(map);

  close();
}

void OsmXmlWriter::_writeMetadata(const Element *e)
{
  LOG_VART(e->getElementId());
  LOG_VART(e->getVersion());
  LOG_VART(e->getStatus());

  if (_includeCompatibilityTags)
  {
    _writer->writeAttribute("timestamp", OsmUtils::toTimeString(e->getTimestamp()));
    long version = e->getVersion();
    if (version == ElementData::VERSION_EMPTY)
    {
      version = 1;
    }
    _writer->writeAttribute("version", QString::number(version));
    LOG_VART(version);
  }
  else
  {
    //TODO: This comparison seems to be still unequal when I set an element's timestamp to
    //ElementData::TIMESTAMP_EMPTY.  See RemoveAttributeVisitor
    if (e->getTimestamp() != ElementData::TIMESTAMP_EMPTY)
    {
      _writer->writeAttribute("timestamp", OsmUtils::toTimeString(e->getTimestamp()));
    }
    if (e->getVersion() != ElementData::VERSION_EMPTY)
    {
      _writer->writeAttribute("version", QString::number(e->getVersion()));
    }
  }
  if (e->getChangeset() != ElementData::CHANGESET_EMPTY)
  {
    _writer->writeAttribute("changeset", QString::number(e->getChangeset()));
  }
  if (e->getUser() != ElementData::USER_EMPTY)
  {
    _writer->writeAttribute("user", e->getUser());
  }
  if (e->getUid() != ElementData::UID_EMPTY)
  {
    _writer->writeAttribute("uid", QString::number(e->getUid()));
  }
}

void OsmXmlWriter::_writeNodes(ConstOsmMapPtr map)
{
  QList<long> nids;
  const NodeMap& nodes = map->getNodes();
  for (NodeMap::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    nids.append(it->first);
  }

  // sort the values to give consistent results.
  qSort(nids.begin(), nids.end(), qLess<long>());
  for (int i = 0; i < nids.size(); i++)
  {
    writePartial(map->getNode(nids[i]));
  }
}

void OsmXmlWriter::_writeWays(ConstOsmMapPtr map)
{
  QList<long> wids;
  const WayMap& ways = map->getWays();
  for (WayMap::const_iterator it = ways.begin(); it != ways.end(); ++it)
  {
    wids.append(it->first);
  }

  // sort the values to give consistent results.
  qSort(wids.begin(), wids.end(), qLess<long>());
  for (int i = 0; i < wids.size(); i++)
  {
    //_writePartial(map->getWay(wids[i]));

    const Way* w = map->getWay(wids[i]).get();

    //TODO: duplicated code below here with _writePartial here b/c of _includePointInWays

    _writer->writeStartElement("way");
    _writer->writeAttribute("visible", "true");
    _writer->writeAttribute("id", QString::number(w->getId()));

    _writeMetadata(w);

    for (size_t j = 0; j < w->getNodeCount(); j++)
    {
      _writer->writeStartElement("nd");
      long nid = w->getNodeId(j);
      _writer->writeAttribute("ref", QString::number(w->getNodeId(j)));
      if (_includePointInWays)
      {
        ConstNodePtr n = map->getNode(nid);
        _writer->writeAttribute("x", QString::number(n->getX(), 'g', _precision));
        _writer->writeAttribute("y", QString::number(n->getY(), 'g', _precision));
      }
      _writer->writeEndElement();
    }

    const Tags& tags = w->getTags();

    for (Tags::const_iterator tit = tags.constBegin(); tit != tags.constEnd(); ++tit)
    {
      if (tit.key().isEmpty() == false && tit.value().isEmpty() == false)
      {
        _writer->writeStartElement("tag");
        _writer->writeAttribute("k", removeInvalidCharacters(tit.key()));

        if (tit.key() == MetadataTags::HootStatus() && w->getStatus() != Status::Invalid)
        {
          if (_textStatus)
          {
            _writer->writeAttribute("v", w->getStatus().toTextStatus());
          }
          else
          {
            _writer->writeAttribute("v", w->getStatus().toCompatString());
          }
        }
        else
        {
          _writer->writeAttribute("v", removeInvalidCharacters(tit.value()));
        }
        _writer->writeEndElement();
      }
    }

    // If we already have a "hoot:status" tag, make sure it contains the actual
    // status of the element. See writeNodes for more info
    if (! tags.contains(MetadataTags::HootStatus()))
    {
      if (_textStatus)
      {
        _writer->writeStartElement("tag");
        _writer->writeAttribute("k", MetadataTags::HootStatus());
        _writer->writeAttribute("v", w->getStatus().toTextStatus());
        _writer->writeEndElement();
      }
      else if (_includeDebug)
      {
        _writer->writeStartElement("tag");
        _writer->writeAttribute("k", MetadataTags::HootStatus());
        _writer->writeAttribute("v", w->getStatus().toCompatString());
        _writer->writeEndElement();
      }
    }

    if (w->hasCircularError() && ConfigOptions().getWriterIncludeCircularErrorTags())
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::ErrorCircular());
      _writer->writeAttribute("v", QString("%1").arg(w->getCircularError()));
      _writer->writeEndElement();
    }

    if (_includeDebug || _includeIds)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootId());
      _writer->writeAttribute("v", QString("%1").arg(w->getId()));
      _writer->writeEndElement();
    }

    _writer->writeEndElement();
  }
}

void OsmXmlWriter::_writeRelations(ConstOsmMapPtr map)
{
  QList<long> rids;
  const RelationMap& relations = map->getRelations();
  for (RelationMap::const_iterator it = relations.begin(); it != relations.end(); ++it)
  {
    rids.append(it->first);
  }

  // sort the values to give consistent results.
  qSort(rids.begin(), rids.end(), qLess<long>());
  for (int i = 0; i < rids.size(); i++)
  {
    writePartial(map->getRelation(rids[i]));
  }
}

void OsmXmlWriter::_writeBounds(const Envelope& bounds)
{
  _writer->writeStartElement("bounds");
  _writer->writeAttribute("minlat", QString::number(bounds.getMinY(), 'g', _precision));
  _writer->writeAttribute("minlon", QString::number(bounds.getMinX(), 'g', _precision));
  _writer->writeAttribute("maxlat", QString::number(bounds.getMaxY(), 'g', _precision));
  _writer->writeAttribute("maxlon", QString::number(bounds.getMaxX(), 'g', _precision));
  _writer->writeEndElement();
}

void OsmXmlWriter::writePartial(const ConstNodePtr& n)
{
  _writer->writeStartElement("node");
  _writer->writeAttribute("visible", "true");
  _writer->writeAttribute("id", QString::number(n->getId()));
  _writeMetadata(n.get());
  _writer->writeAttribute("lat", QString::number(n->getY(), 'f', _precision));
  _writer->writeAttribute("lon", QString::number(n->getX(), 'f', _precision));

  const Tags& tags = n->getTags();

  for (Tags::const_iterator it = tags.constBegin(); it != tags.constEnd(); ++it)
  {
    if (it.key().isEmpty() == false && it.value().isEmpty() == false)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", removeInvalidCharacters(it.key()));
      if (it.key() == MetadataTags::HootStatus() && n->getStatus() != Status::Invalid)
      {
        if (_textStatus)
        {
          _writer->writeAttribute("v", n->getStatus().toTextStatus());
        }
        else
        {
          _writer->writeAttribute("v", n->getStatus().toCompatString());
        }
      }
      else
      {
        _writer->writeAttribute("v", removeInvalidCharacters(it.value()));
      }
      _writer->writeEndElement();
    }
  }

  if (! tags.contains(MetadataTags::HootStatus()))
  {
    if (_textStatus && tags.getNonDebugCount() > 0)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      _writer->writeAttribute("v", n->getStatus().toTextStatus());
      _writer->writeEndElement();
    }
    else if (_includeDebug)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      if (_textStatus)
      {
        _writer->writeAttribute("v", n->getStatus().toTextStatus());
      }
      else
      {
        _writer->writeAttribute("v", n->getStatus().toCompatString());
      }
      _writer->writeEndElement();
    }
  }

  // turn this on when we start using node circularError.
  if (n->hasCircularError() && tags.getNonDebugCount() > 0 &&
      ConfigOptions().getWriterIncludeCircularErrorTags())
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::ErrorCircular());
    _writer->writeAttribute("v", QString("%1").arg(n->getCircularError()));
    _writer->writeEndElement();
  }

  if (_includeDebug || _includeIds)
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::HootId());
    _writer->writeAttribute("v", QString("%1").arg(n->getId()));
    _writer->writeEndElement();
  }
  _writer->writeEndElement();

  _bounds.expandToInclude(n->getX(), n->getY());
}

void OsmXmlWriter::writePartial(const ConstWayPtr& w)
{
  _writer->writeStartElement("way");
  _writer->writeAttribute("visible", "true");
  _writer->writeAttribute("id", QString::number(w->getId()));

  _writeMetadata(w.get());

  for (size_t j = 0; j < w->getNodeCount(); j++)
  {
    _writer->writeStartElement("nd");
    _writer->writeAttribute("ref", QString::number(w->getNodeId(j)));
    _writer->writeEndElement();
  }

  const Tags& tags = w->getTags();

  for (Tags::const_iterator tit = tags.constBegin(); tit != tags.constEnd(); ++tit)
  {
    if (tit.key().isEmpty() == false && tit.value().isEmpty() == false)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", removeInvalidCharacters(tit.key()));

      if (tit.key() == MetadataTags::HootStatus() && w->getStatus() != Status::Invalid)
      {
        if (_textStatus)
        {
          _writer->writeAttribute("v", w->getStatus().toTextStatus());
        }
        else
        {
          _writer->writeAttribute("v", w->getStatus().toCompatString());
        }
      }
      else
      {
        _writer->writeAttribute("v", removeInvalidCharacters(tit.value()));
      }
      _writer->writeEndElement();
    }
  }

  // Logic: If we already have a "hoot:status" tag, make sure it contains the actual
  // status of the element. See writeNodes for more info
  if (! tags.contains(MetadataTags::HootStatus()))
  {
    if (_textStatus)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      _writer->writeAttribute("v", w->getStatus().toTextStatus());
      _writer->writeEndElement();
    }
    else if (_includeDebug)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      _writer->writeAttribute("v", w->getStatus().toCompatString());
      _writer->writeEndElement();
    }
  }

  if (w->hasCircularError() && ConfigOptions().getWriterIncludeCircularErrorTags())
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::ErrorCircular());
    _writer->writeAttribute("v", QString("%1").arg(w->getCircularError()));
    _writer->writeEndElement();
  }

  if (_includeDebug || _includeIds)
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::HootId());
    _writer->writeAttribute("v", QString("%1").arg(w->getId()));
    _writer->writeEndElement();
  }

  _writer->writeEndElement();
}

void OsmXmlWriter::writePartial(const ConstRelationPtr& r)
{
  _writer->writeStartElement("relation");
  _writer->writeAttribute("visible", "true");
  _writer->writeAttribute("id", QString::number(r->getId()));

  _writeMetadata(r.get());

  const vector<RelationData::Entry>& members = r->getMembers();
  for (size_t j = 0; j < members.size(); j++)
  {
    const RelationData::Entry& e = members[j];
    _writer->writeStartElement("member");
    _writer->writeAttribute("type", _typeName(e.getElementId().getType()));
    _writer->writeAttribute("ref", QString::number(e.getElementId().getId()));
    _writer->writeAttribute("role", removeInvalidCharacters(e.role));
    _writer->writeEndElement();
  }

  const Tags& tags = r->getTags();

  for (Tags::const_iterator tit = tags.constBegin(); tit != tags.constEnd(); ++tit)
  {
    if (tit.key().isEmpty() == false && tit.value().isEmpty() == false)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", removeInvalidCharacters(tit.key()));

      // Does this need && r->getStatus() != Status::Invalid
      if (tit.key() == MetadataTags::HootStatus())
      {
        if (_textStatus)
        {
          _writer->writeAttribute("v", r->getStatus().toTextStatus());
        }
        else
        {
          _writer->writeAttribute("v", r->getStatus().toCompatString());
        }
      }
      else
      {
        _writer->writeAttribute("v", removeInvalidCharacters(tit.value()));
      }
      _writer->writeEndElement();
    }
  }

  if (r->getType() != "")
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", "type");
    _writer->writeAttribute("v", removeInvalidCharacters(r->getType()));
    _writer->writeEndElement();
  }

  if (r->hasCircularError() && ConfigOptions().getWriterIncludeCircularErrorTags())
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::ErrorCircular());
    _writer->writeAttribute("v", QString("%1").arg(r->getCircularError()));
    _writer->writeEndElement();
  }

  if (! tags.contains(MetadataTags::HootStatus()))
  {
    if (_textStatus)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      _writer->writeAttribute("v", r->getStatus().toTextStatus());
      _writer->writeEndElement();
    }
    else if (_includeDebug)
    {
      _writer->writeStartElement("tag");
      _writer->writeAttribute("k", MetadataTags::HootStatus());
      _writer->writeAttribute("v", r->getStatus().toCompatString());
      _writer->writeEndElement();
    }
  }

  if (_includeDebug || _includeIds)
  {
    _writer->writeStartElement("tag");
    _writer->writeAttribute("k", MetadataTags::HootId());
    _writer->writeAttribute("v", QString("%1").arg(r->getId()));
    _writer->writeEndElement();
  }

  _writer->writeEndElement();
}

void OsmXmlWriter::finalizePartial()
{
  _writeBounds(_bounds);    //bounds will be written at the end of output
  close();
}

}
