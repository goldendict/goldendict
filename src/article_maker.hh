/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_MAKER_HH_INCLUDED__
#define __ARTICLE_MAKER_HH_INCLUDED__

#include <QObject>
#include <set>
#include <list>
#include "dictionary.hh"
#include "instances.hh"
#include "wordfinder.hh"

/// This class generates the article's body for the given lookup request
class ArticleMaker: public QObject
{
  Q_OBJECT // We make it QObject to use tr() conveniently

  std::vector< sptr< Dictionary::Class > > const & dictionaries;
  std::vector< Instances::Group > const & groups;

  QString displayStyle;

public:

  /// On construction, a reference to all dictionaries and a reference all
  /// groups' instances are to be passed. Those references are kept stored as
  /// references, and as such, any changes to them would reflect on the results
  /// of the inquiries, altthough those changes are perfectly legal.
  ArticleMaker( std::vector< sptr< Dictionary::Class > > const & dictionaries,
                std::vector< Instances::Group > const & groups,
                QString const & displayStyle );

  /// Sets the display style to use for any new requests. This affects the
  /// choice of the stylesheet file.
  void setDisplayStyle( QString const & );

  /// Looks up the given word within the given group, and creates a full html
  /// page text containing its definition.
  /// The result is returned as Dictionary::DataRequest just like dictionaries
  /// themselves do. The difference is that the result is a complete html page
  /// with all definitions from all the relevant dictionaries.
  sptr< Dictionary::DataRequest > makeDefinitionFor( QString const & word, unsigned groupId ) const;

  /// Makes up a text which states that no translation for the given word
  /// was found. Sometimes it's better to call this directly when it's already
  /// known that there's no translation.
  sptr< Dictionary::DataRequest > makeNotFoundTextFor( QString const & word, QString const & group ) const;

  /// Creates an 'untitled' page. The result is guaranteed to be instant.
  sptr< Dictionary::DataRequest > makeEmptyPage() const;

private:

  /// Makes everything up to and including the opening body tag.
  std::string makeHtmlHeader( QString const & word, QString const & icon ) const;

  /// Makes the html body for makeNotFoundTextFor()
  static std::string makeNotFoundBody( QString const & word, QString const & group );

  friend class ArticleRequest; // Allow it calling makeNotFoundBody()
};

/// The request specific to article maker. This should really be private,
/// but we need it to be handled by moc.
class ArticleRequest: public Dictionary::DataRequest
{
  Q_OBJECT

  QString word, group;
  std::vector< sptr< Dictionary::Class > > const & activeDicts;
  
  std::set< gd::wstring > alts; // Accumulated main forms
  std::list< sptr< Dictionary::WordSearchRequest > > altSearches;
  bool altsDone, bodyDone;
  std::list< sptr< Dictionary::DataRequest > > bodyRequests;
  bool foundAnyDefinitions;
  bool closePrevSpan; // Indicates whether the last opened article span is to
                      // be closed after the article ends.
  sptr< WordFinder > stemmedWordFinder; // Used when there're no results

public:

  ArticleRequest( QString const & word, QString const & group,
                  std::vector< sptr< Dictionary::Class > > const & activeDicts,
                  std::string const & header );

  virtual void cancel()
  { finish(); } // Add our own requests cancellation here

private slots:

  void altSearchFinished();
  void bodyFinished();
  void stemmedSearchFinished();
};


#endif
