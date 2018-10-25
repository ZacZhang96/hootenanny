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
 * @copyright Copyright (C) 2018 DigitalGlobe (http://www.digitalglobe.com/)
 */

#ifndef NON_ENGLISH_LANGUAGE_DETECTION_VISITOR_H
#define NON_ENGLISH_LANGUAGE_DETECTION_VISITOR_H

// hoot
#include <hoot/core/util/Configurable.h>
#include <hoot/core/visitors/ElementOsmMapVisitor.h>
#include <hoot/core/language/LanguageDetector.h>
#include <hoot/rnd/language/TranslationInfoProvider.h>

// Qt
#include <QMap>
#include <QSet>

namespace hoot
{

/**
 * Detects the source language of tag values
 *
 * Even though ToEnglishTranslationVisitor has the option of doing both translation and detection,
 * this class is useful if you're uncertain which source languages to declare for translation and
 * don't have the resources to stand up a translation server for every possible language.
 */
class NonEnglishLanguageDetectionVisitor : public ElementOsmMapVisitor, public Configurable
{

public:

  static std::string className() { return "hoot::NonEnglishLanguageDetectionVisitor"; }

  NonEnglishLanguageDetectionVisitor();
  virtual ~NonEnglishLanguageDetectionVisitor();

  virtual void visit(const boost::shared_ptr<Element>& e);

  virtual void setConfiguration(const Settings& conf);

  virtual QString getDescription() const
  { return "Detects source languages for selected tags"; }

  /**
   *
   *
   * @return
   */
  QString getLangCountsSortedByFrequency() const;

  /**
   *
   *
   * @return
   */
  QString getLangCountsSortedByLangName() const;

private:

  QMap<QString, int> _langNamesToCounts;
  QMap<QString, QString> _langCodesToLangs;
  QSet<QString> _tagKeys;
  bool _ignorePreTranslatedTags;
  bool _writeDetectedLangTags;
  bool _parseNames;

  QString _detectionSummary;
  bool _currentElementHasSuccessfulTagDetection;
  long _numTagDetectionsMade;
  long _numElementsWithSuccessfulTagDetection;
  long _numTotalElements;
  long _numProcessedTags;
  long _numProcessedElements;
  int _taskStatusUpdateInterval;

  boost::shared_ptr<TranslationInfoProvider> _infoClient;
  boost::shared_ptr<LanguageDetector> _langDetector;
};

}

#endif // NON_ENGLISH_LANGUAGE_DETECTION_VISITOR_H
