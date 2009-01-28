/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_MAKER_HH_INCLUDED__
#define __ARTICLE_MAKER_HH_INCLUDED__

#include "dictionary.hh"
#include "instances.hh"

/// This class generates the article's body for the given lookup request
class ArticleMaker
{
  std::vector< sptr< Dictionary::Class > > const & dictionaries;
  std::vector< Instances::Group > const & groups;

public:

  /// On construction, a reference to all dictionaries and a reference all
  /// groups' instances are to be passed. Those references are kept stored as
  /// references, and as such, any changes to them would reflect on the results
  /// of the inquiries, altthough those changes are perfectly legal.
  ArticleMaker( std::vector< sptr< Dictionary::Class > > const & dictionaries,
                std::vector< Instances::Group > const & groups );

  /// Looks up the given word within the given group, and creates a full html
  /// page text containing its definition.
  std::string makeDefinitionFor( QString const & word, QString const & group ) const;
};

#endif
