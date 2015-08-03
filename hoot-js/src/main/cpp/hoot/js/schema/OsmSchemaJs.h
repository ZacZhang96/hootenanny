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
 * @copyright Copyright (C) 2014, 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */
#ifndef OSMSCHEMA_JS_H
#define OSMSCHEMA_JS_H

// node.js
// #include <nodejs/node.h>
#include <hoot/js/SystemNodeJs.h>

namespace hoot
{
using namespace node;
using namespace v8;

class OsmSchemaJs : public node::ObjectWrap
{
public:
 static void Init(v8::Handle<v8::Object> target);

private:
  OsmSchemaJs();
  ~OsmSchemaJs();

  static v8::Handle<v8::Value> getCategories(const v8::Arguments& args);
  static v8::Handle<v8::Value> isAncestor(const v8::Arguments& args);
  static v8::Handle<v8::Value> isArea(const v8::Arguments& args);
  static v8::Handle<v8::Value> isBuilding(const v8::Arguments& args);
  static v8::Handle<v8::Value> isLinear(const v8::Arguments& args);
  static v8::Handle<v8::Value> isLinearWaterway(const v8::Arguments& args);
  static v8::Handle<v8::Value> isPoi(const v8::Arguments& args);
  static v8::Handle<v8::Value> score(const v8::Arguments& args);

};


}


#endif // OSMSCHEMA_JS_H
