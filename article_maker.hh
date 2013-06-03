/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_MAKER_HH_INCLUDED__
#define __ARTICLE_MAKER_HH_INCLUDED__

#include <QObject>
#include <QMap>
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

  QString displayStyle, addonStyle;

  bool needExpandOptionalParts;
  bool collapseBigArticles;
  int articleLimitSize;

public:

  /// On construction, a reference to all dictionaries and a reference all
  /// groups' instances are to be passed. Those references are kept stored as
  /// references, and as such, any changes to them would reflect on the results
  /// of the inquiries, although those changes are perfectly legal.
  ArticleMaker( std::vector< sptr< Dictionary::Class > > const & dictionaries,
                std::vector< Instances::Group > const & groups,
                QString const & displayStyle,
                QString const & addonStyle);

  /// Sets the display style to use for any new requests. This affects the
  /// choice of the stylesheet file.
  void setDisplayStyle( QString const &, QString const & addonStyle );

  /// Looks up the given word within the given group, and creates a full html
  /// page text containing its definition.
  /// The result is returned as Dictionary::DataRequest just like dictionaries
  /// themselves do. The difference is that the result is a complete html page
  /// with all definitions from all the relevant dictionaries.
  /// Contexts is a map of context values to be passed to each dictionary, where
  /// the keys are dictionary ids.
  /// If mutedDicts is not empty, the search would be limited only to those
  /// dictionaries in group which aren't listed there.
  sptr< Dictionary::DataRequest > makeDefinitionFor( QString const & word, unsigned groupId,
                                                     QMap< QString, QString > const & contexts,
                                                     QSet< QString > const & mutedDicts =
                                                       QSet< QString >() ) const;

  /// Makes up a text which states that no translation for the given word
  /// was found. Sometimes it's better to call this directly when it's already
  /// known that there's no translation.
  sptr< Dictionary::DataRequest > makeNotFoundTextFor( QString const & word, QString const & group ) const;

  /// Creates an 'untitled' page. The result is guaranteed to be instant.
  sptr< Dictionary::DataRequest > makeEmptyPage() const;

  /// Create page with one picture
  sptr< Dictionary::DataRequest > makePicturePage( std::string const & url ) const;

  /// Set auto expanding optional parts of articles
  void setExpandOptionalParts( bool expand );

  /// Add base path to file path if it's relative and file not found
  /// Return true if path successfully adjusted
  static bool adjustFilePath( QString & fileName );

  /// Set collapse articles parameters
  void setCollapseParameters( bool autoCollapse, int articleSize );

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
  QMap< QString, QString > contexts;
  std::vector< sptr< Dictionary::Class > > activeDicts;
  
  std::set< gd::wstring > alts; // Accumulated main forms
  std::list< sptr< Dictionary::WordSearchRequest > > altSearches;
  bool altsDone, bodyDone;
  std::list< sptr< Dictionary::DataRequest > > bodyRequests;
  bool foundAnyDefinitions;
  bool closePrevSpan; // Indicates whether the last opened article span is to
                      // be closed after the article ends.
  sptr< WordFinder > stemmedWordFinder; // Used when there're no results

  /// A sequence of words and spacings between them, including the initial
  /// spacing before the first word and the final spacing after the last word.
  typedef QList< QString > Words;
  typedef QList< QString > Spacings;

  /// Splits the given string into words and spacings between them.
  QPair< Words, Spacings > splitIntoWords( QString const & );

  QPair< Words, Spacings > splittedWords;
  int currentSplittedWordStart;
  int currentSplittedWordEnd;
  QString currentSplittedWordCompound;
  QString lastGoodCompoundResult;
  bool firstCompoundWasFound;
  int articleSizeLimit;
  bool needExpandOptionalParts;

public:

  ArticleRequest( QString const & word, QString const & group,
                  QMap< QString, QString > const & contexts,
                  std::vector< sptr< Dictionary::Class > > const & activeDicts,
                  std::string const & header,
                  int sizeLimit, bool needExpandOptionalParts_ );

  virtual void cancel();
//  { finish(); } // Add our own requests cancellation here

private slots:

  void altSearchFinished();
  void bodyFinished();
  void stemmedSearchFinished();
  void individualWordFinished();

private:

  /// Appends the given string to 'data', with locking its mutex.
  void appendToData( std::string const & );

  /// Uses stemmedWordFinder to perform the next step of looking up word
  /// combinations.
  void compoundSearchNextStep( bool lastSearchSucceeded );

  /// Creates a single word out of the [currentSplittedWordStart..End] range.
  QString makeSplittedWordCompound();

  /// Makes an html link to the given word.
  std::string linkWord( QString const & );

  /// Escapes the spacing between the words to include in html.
  std::string escapeSpacing( QString const & );

  /// Find end of corresponding </div> tag
  int findEndOfCloseDiv( QString const &, int pos );
};


#endif
