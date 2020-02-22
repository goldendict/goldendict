/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "xdxf2html.hh"
#ifdef GD_PUGIXML_XSERIAL
#include "pugixml_Qt.h"
#else
#include <QtXml>
#endif
#include "gddebug.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include "folding.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "file.hh"
#include "filetype.hh"
#include "htmlescape.hh"
#include "qt4x5.hh"
#include <QDebug>
#include <QList>
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#endif

namespace Xdxf2Html {

#ifdef GD_PUGIXML_XSERIAL
static void fixLink( pugi::xml_node & el, string const & dictId, const char *attrName )
{
    QUrl url;
    pugi::xml_attribute attr = el.attribute(attrName);
    url.setScheme( "bres" );
    url.setHost( QString::fromStdString(dictId) );
    url.setPath( Qt4x5::Url::ensureLeadingSlash( attr.value() ) );

    attr.set_value( url.toEncoded().data() );
}
#else
static void fixLink( QDomElement & el, string const & dictId, const char *attrName )
{
    QUrl url;
    url.setScheme( "bres" );
    url.setHost( QString::fromStdString(dictId) );
    url.setPath( Qt4x5::Url::ensureLeadingSlash( el.attribute(attrName) ) );

    el.setAttribute( attrName, url.toEncoded().data() );
}
#endif

// converting a number into roman representation
string convertToRoman( int input, int lower_case )
{
    string romanvalue = "";
    if( input >= 4000 )
    {
        int x = ( input - input % 4000 ) / 1000;
        romanvalue = "(" + convertToRoman( x, lower_case ) + ")" ;
        input %= 4000;
    }

    const string roman[26] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I",
                               "m", "cm", "d", "cd", "c", "xc", "l", "xl", "x", "ix", "v", "iv", "i"};
    const int decimal[13] =  {1000,  900, 500,  400, 100,   90,  50,   40,  10,    9,   5,    4,   1};

    for( int i = 0; i < 13; i++ )
    {
        while( input >= decimal[ i ] )
        {
            input -= decimal[ i ];
            if ( lower_case == 1 )
                romanvalue += roman[ i + 13 ];
            else
                romanvalue += roman[ i ];
        }
    }
    return romanvalue;
}

#ifdef GD_PUGIXML_XSERIAL
class XdxfWalker : public pugi::xml_tree_walker {
private:
    DICT_TYPE type;
    map < string, string > const * pAbrv;
    Dictionary::Class const * dictPtr;
    IndexedZip * resourceZip;
    bool isLogicalFormat;

    QMap<int, int> defMaps;
    int nestingDepth;
public:
    QString headword;
public:
    XdxfWalker(const DICT_TYPE _type, map < string, string > const * _pAbrv,
               Dictionary::Class const * _dictPtr, IndexedZip * _resourceZip,
               bool _isLogicalFormat) : pugi::xml_tree_walker(),
        type(_type), pAbrv(_pAbrv), dictPtr(_dictPtr), resourceZip(_resourceZip),
        isLogicalFormat(_isLogicalFormat), nestingDepth(0)
    {
    }
    ~XdxfWalker()
    {
    }


    inline bool namecmp(const pugi::xml_node& node, const char *name, const Qt::CaseSensitivity c = Qt::CaseInsensitive)
    {
        return (QString(name).compare( node.name(), c ) == 0);
    }

    // Callback that is called when and traversal of node all it's childen ends
    void out(const pugi::xml_node& node)
    {
        if(isLogicalFormat && node.type() == pugi::node_element &&
                ( namecmp(node, "def") || node.attribute("def") ) )
        {
            if(defMaps.contains(nestingDepth+1))
                defMaps.remove(nestingDepth+1);
            --nestingDepth;
        }
    }

    // Callback that is called for each node traversed
    bool for_each( pugi::xml_node& el)
    {
        if( el.type() != pugi::node_element )
            return true;
        if(namecmp(el, "ex"))
        {
            QString author, source;
            author = el.attribute( "author" ).value();
            source = el.attribute( "source" ).value();
            bool elC = false;
            for( pugi::xml_node el2 = el.first_child(); el2; el2 = el2.next_sibling() )
            {
                if( namecmp(el2, "ex_orig") )
                {
                    el2.set_name("span");
                    el2.append_attribute("class").set_value("xdxf_ex_orig");
                }
                else if( namecmp(el2, "ex_tran") )
                {
                    el2.set_name("span");
                    el2.append_attribute("class").set_value("xdxf_ex_tran");
                }
                if(!elC)
                    elC = true;
            }

            if(  ( !author.isEmpty() || !source.isEmpty() ) && elC )
            {
                pugi::xml_node el2 = el.append_child( "span" );
                el2.append_attribute("class").set_value("xdxf_ex_source");
                QString text = author;
                if( !source.isEmpty() )
                {
                    if( !text.isEmpty() )
                        text += ", ";
                    text += source;
                }
                el2.append_child(pugi::node_pcdata).set_value(text.toUtf8().data());
            }

            el.set_name( "span" );
            el.append_attribute( "class" ).set_value( isLogicalFormat ? "xdxf_ex" : "xdxf_ex_old" );
        }
        else if(namecmp(el, "mrkd"))
        {
            el.set_name( "span" );
            el.append_attribute( "class" ).set_value( "xdxf_ex_markd" );
        }
        else if(namecmp(el, "k"))
        {
            if( type == STARDICT )
            {
                el.set_name( "span" );
                el.append_attribute( "class" ).set_value( "xdxf_k" );
            }
            else
            {
                headword = el.text().get();
                el.set_name( "div" );
                el.append_attribute( "class" ).set_value( "xdxf_headwords" );
                if( dictPtr->isFromLanguageRTL() != dictPtr->isToLanguageRTL() )
                    el.append_attribute( "dir" ).set_value( dictPtr->isFromLanguageRTL() ? "rtl" : "ltr" );
            }
        }
        else if(namecmp(el, "def"))
        {
            if( isLogicalFormat ) // in articles with visual format <def> tags do not effect the formatting.
            {
                {
                    ++nestingDepth;
                    if(!defMaps.contains(nestingDepth))
                        defMaps[nestingDepth] = 1;
                    else
                        defMaps[nestingDepth] = defMaps[nestingDepth] + 1;
                    el.append_attribute( "def" ).set_value( defMaps[nestingDepth] );
                }

                if( defMaps[nestingDepth] > 1 || el.parent().select_nodes("def").size() > 1 )
                {
                    QString numberText = ""; // the number to be inserted into the beginning of <def> (I,II,IV,1,2,3,a),b),c)...)
                    int depth = (nestingDepth - 1) % 3 + 1;
                    if( depth == 1 )
                        numberText = QString::fromStdString( convertToRoman(defMaps[nestingDepth],0) + ". " );
                    if( depth == 2 )
                        numberText = numberText.setNum( defMaps[nestingDepth] ) + ". ";
                    if( depth == 3 )
                        numberText = numberText.setNum( defMaps[nestingDepth] ) + ") ";
                    if( depth == 4 )
                        numberText = QString::fromStdString( convertToRoman(defMaps[nestingDepth],1) + ") " );
                    pugi::xml_node numberNode = el.prepend_child("span");
                    numberNode.append_attribute("class").set_value("xdxf_num");
                    numberNode.text().set(numberText.toUtf8().data());

                    pugi::xml_attribute attr = el.attribute( "cmt" );
                    if ( !attr.empty() )
                    {
                        pugi::xml_node cmtNode = el.insert_child_after("span", numberNode);
                        cmtNode.append_attribute( "class" ).set_value( "xdxf_co" );
                        numberNode.text().set(attr.value());
                    }
                }
                el.set_name( "span" );
                el.append_attribute( "class" ).set_value( "xdxf_def" );
            }
        }
        else if(namecmp(el, "opt"))
        {
            el.set_name( "span" );
            el.append_attribute("class").set_value("xdxf_opt");
        }
        else if(namecmp(el, "kref"))
        {
            el.set_name( "a" );
            el.append_attribute( "class" ).set_value( "xdxf_kref" );
            pugi::xml_attribute attr = el.attribute("idref");
            if ( !attr.empty() )
            {
                // todo implement support for referencing only specific parts of the article
                el.append_attribute("href").set_value(QString( "bword:" ).
                                                      append(el.text().get()).append("#").
                                                      append(el.attribute( "idref" ).value()).
                                                      toUtf8().data());
            }
            else
                el.append_attribute("href").set_value(QString( "bword:" ).
                                                      append(el.text().get()).
                                                      toUtf8().data());
            attr = el.attribute("kcmt");
            if ( !attr.empty() )
            {
                el.parent().insert_child_after(pugi::node_pcdata, el ).
                        set_value(QString(" ").append(attr.value()).toUtf8().data());
            }
        }
        else if(namecmp(el, "iref"))
        {
            el.set_name( "a" );
            pugi::xml_attribute attr = el.attribute("href");
            if ( attr.empty() )
                el.append_attribute("href").set_value(el.text().get());
            else if(strlen(attr.value()) == 0)
                attr.set_value(el.text().get());
        }
        else if(namecmp(el, "abr") || namecmp(el, "abbr"))
        {
            el.set_name( "span" );
            el.append_attribute("class").set_value("xdxf_abbr");
            if( type == XDXF && pAbrv != NULL )
            {
                string val = Utf8::encode( Folding::trimWhitespace( gd::toWString( QString::fromUtf8(el.text().get() ) ) ));

                // If we have such a key, display a title

                map< string, string >::const_iterator i = pAbrv->find( val );

                if ( i != pAbrv->end() )
                {
                    string title;

                    if ( Utf8::decode( i->second ).size() < 70 )
                    {
                        // Replace all spaces with non-breakable ones, since that's how Lingvo shows tooltips
                        title.reserve( i->second.size() );

                        for( char const * c = i->second.c_str(); *c; ++c )
                        {
                            if ( *c == ' ' || *c == '\t' )
                            {
                                // u00A0 in utf8
                                title.push_back( char(0xC2) );
                                title.push_back( char(0xA0) );
                            }
                            else
                                if( *c == '-' ) // Change minus to non-breaking hyphen (uE28091 in utf8)
                                {
                                    title.push_back( char(0xE2) );
                                    title.push_back( char(0x80) );
                                    title.push_back( char(0x91) );
                                }
                                else
                                    title.push_back( *c );
                        }
                    }
                    else
                        title = i->second;
                    el.append_attribute( "title" ).set_value( title.c_str() );
                }
            }
        }
        else if(namecmp(el, "dtrn"))
        {
            el.set_name( "span" );
            el.append_attribute("class").set_value("xdxf_dtrn");
        }
        else if(namecmp(el, "c"))
        {
            el.set_name( "span" );
            pugi::xml_attribute attr = el.attribute("c");
            if ( !attr.empty() )
            {
                el.append_attribute("style").set_value(QString("color:").append(attr.value()).toUtf8().data() );
                el.remove_attribute(attr);
            }
            else
                el.append_attribute("style").set_value("color:blue");
        }
        else if(namecmp(el, "co"))
        {
            el.set_name( "span" );
            el.append_attribute("class").set_value(isLogicalFormat ? "xdxf_co" : "xdxf_co_old");
        }
        else if(namecmp(el, "gr") || namecmp(el, "tense"))
        {
            el.set_name( "span" );
            el.append_attribute("class").set_value(isLogicalFormat ? "xdxf_gr" : "xdxf_gr_old");
        }
        else if(namecmp(el, "pos"))
        {
            el.set_name("span");
            el.append_attribute("class").set_value(isLogicalFormat ? "xdxf_gr" : "xdxf_gr_old");
        }
        else if(namecmp(el, "tr"))
        {
            el.set_name("span");
            el.append_attribute("class").set_value(isLogicalFormat ? "xdxf_tr" : "xdxf_tr_old");
        }
        else if(namecmp(el, "img"))
        {
            if ( !el.attribute("src").empty() )
            {
                fixLink( el, dictPtr->getId(), "src" );
            }

            if ( !el.attribute( "losrc" ).empty() )
            {
                fixLink( el, dictPtr->getId(), "losrc" );
            }

            if ( !el.attribute( "hisrc" ).empty() )
            {
                fixLink( el, dictPtr->getId(), "hisrc" );
            }
        }
        else if(namecmp(el, "rref"))
        {
            pugi::xml_attribute attr = el.attribute("start");
            if( dictPtr != NULL && attr.empty() )
            {
                string filename = Utf8::encode( gd::toWString( el.text().get() ) );
                if ( Filetype::isNameOfPicture( filename ) )
                {
                    pugi::xml_node p = el.parent();
                    if( !p.empty() )
                    {
                        QUrl url;
                        url.setScheme( "bres" );
                        url.setHost( QString::fromUtf8( dictPtr->getId().c_str() ) );
                        url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( filename.c_str() ) ) );

                        pugi::xml_node newcur = p.insert_child_after("img", el );
                        p.remove_child(el);
                        el = newcur;
                        el.append_attribute("src").set_value(url.toEncoded().data());
                        el.append_attribute("alt").set_value(Html::escape( filename ).c_str());
                        return true;
                    }
                }
                else if( Filetype::isNameOfSound( filename ) )
                {
                    pugi::xml_node p = el.parent();
                    if( !p.empty() )
                    {
                        bool search = false;
                        if( type == STARDICT )
                        {
                            string n = FsEncoding::dirname( dictPtr->getDictionaryFilenames()[ 0 ] ) +
                                    FsEncoding::separator() + string( "res" ) + FsEncoding::separator() +
                                    FsEncoding::encode( filename );
                            search = !File::exists( n ) &&
                                    ( !resourceZip ||
                                      !resourceZip->isOpen() ||
                                      !resourceZip->hasFile( Utf8::decode( filename ) ) );
                        }
                        else
                        {
                            string n = dictPtr->getDictionaryFilenames()[ 0 ] + ".files" +
                                    FsEncoding::separator() +
                                    FsEncoding::encode( filename );
                            search = !File::exists( n ) && !File::exists( FsEncoding::dirname( dictPtr->getDictionaryFilenames()[ 0 ] ) +
                                    FsEncoding::separator() +
                                    FsEncoding::encode( filename ) ) &&
                                    ( !resourceZip ||
                                      !resourceZip->isOpen() ||
                                      !resourceZip->hasFile( Utf8::decode( filename ) ) );
                        }

                        QUrl url;
                        url.setScheme( "gdau" );
                        url.setHost( QString::fromUtf8( search ? "search" : dictPtr->getId().c_str() ) );
                        url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( filename.c_str() ) ) );

                        pugi::xml_node np = p.insert_child_after( "script", el );
                        p.remove_child(el);
                        el = np;

                        el.append_attribute("type").set_value("text/javascript" );
                        el.text().set(makeAudioLinkScript( string( "\"" ) + url.toEncoded().data() + "\"",
                                                           dictPtr->getId() ).c_str());

                        np =  p.insert_child_after("span", el);
                        np.append_attribute( "class").set_value("xdxf_wav" );

                        np = np.append_child( "a" );
                        np.append_attribute( "href" ).set_value(url.toEncoded().data() );

                        np = np.append_child( "img" );
                        np.append_attribute( "src" ).set_value( "qrcx://localhost/icons/playsound.png" );
                        np.append_attribute( "border" ).set_value( "0" );
                        np.append_attribute( "align" ).set_value( "absmiddle" );
                        np.append_attribute( "alt" ).set_value( "Play" );
                        return true;
                    }
                }
            }

            // We don't really know how to handle this at the moment, so we'll just
            // convert it to a span and leave it as is for now.
            el.set_name( "span" );
            el.append_attribute( "class" ).set_value( "xdxf_rref" );
        }
        else
        {
        }
        return true;
    }
};
#else
QDomElement fakeElement( QDomDocument & dom )
{
    // Create element which will be removed after
    // We will insert it to empty elements to avoid output ones in <xxx/> form
    return dom.createElement( "b" );
}
#endif

string convert( string const & in, DICT_TYPE type, map < string, string > const * pAbrv,
                Dictionary::Class *dictPtr,  IndexedZip * resourceZip,
                bool isLogicalFormat, unsigned revisionNumber, QString * headword )
{
    //  DPRINTF( "Source>>>>>>>>>>: %s\n\n\n", in.c_str() );

    // Convert spaces after each end of line to &nbsp;s, and then each end of
    // line to a <br>

    QByteArray inConverted;
    inConverted.reserve(in.size() + 2048);
    if( type == XDXF )
    {
        inConverted = "<div class=\"xdxf\"";
        if( dictPtr->isToLanguageRTL() )
            inConverted.append(" dir=\"rtl\"");
        inConverted.append(">");
    }
    else
        inConverted.append("<div class=\"sdct_x\">");

    bool afterEol = false;

    for( string::const_iterator i = in.begin(), j = in.end(); i != j; ++i )
    {
        switch( *i )
        {
        case '\n':
            afterEol = true;
            if( !isLogicalFormat )
                inConverted.append( "<br/>" );
            break;

        case '\r':
            break;

        case ' ':
            if ( afterEol )
            {
                if( !isLogicalFormat )
                    inConverted.append( "&nbsp;" );
                break;
            }
            // Fall-through

        default:
            inConverted.push_back( *i );
            afterEol = false;
        }
    }

    // Strip "<nu />" tags - QDomDocument don't handle it correctly
    int n = 0;
    const QString nu_del("<nu />");
    while( ( n = inConverted.indexOf(nu_del, n) ) != -1 )
        inConverted.remove( n, nu_del.size() );

    // We build a dom representation of the given xml, then do some transforms
#ifdef GD_PUGIXML_XSERIAL
    pugi::xml_document dd;
    pugi::xml_parse_result xpr;
#else
    QDomDocument dd;

    QString errorStr;
    int errorLine, errorColumn;
#endif
    inConverted.append("</div>");
#ifdef GD_PUGIXML_XSERIAL
    if( !(xpr = dd.load_string(inConverted.data())) )
    {
        qWarning( "Xdxf2html error, xml parse failed: %s at %d,%d\n", inConverted.data(),  xpr.status,  xpr.description() );
#else
    if( !dd.setContent( inConverted, false, &errorStr, &errorLine, &errorColumn  ) )
    {
        qWarning( "Xdxf2html error, xml parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
#endif
        gdWarning( "The input was: %s\n", in.c_str() );

        return in;
    }
#ifdef GD_PUGIXML_XSERIAL
    XdxfWalker walker(type, pAbrv, dictPtr, resourceZip, isLogicalFormat);
    if(!dd.traverse(walker))
        return inConverted.data();
    if(headword)
        *headword = walker.headword;
    xml_writer_bytearray xwb(inConverted);
    dd.save(xwb, PUGIXML_TEXT(" "), pugi::format_indent | pugi::format_no_declaration | pugi::format_no_escapes, pugi::encoding_utf8);
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    return QString::fromUtf8(inConverted).remove('\n').remove( "<i/>"  ).toUtf8().data();
#else
    return QString::fromUtf8(inConverted).remove('\n').remove( "<i/>" ).toUtf8().data();
#endif
#else
    QDomNodeList nodes = dd.elementsByTagName( "ex" ); // Example

    while( nodes.size() )
    {
        QString author, source;
        QDomElement el = nodes.at( 0 ).toElement();

        author = el.attribute( "author", QString() );
        source = el.attribute( "source", QString() );

        if( el.hasChildNodes() )
        {
            QDomNodeList lst = el.childNodes();
            for( int i = 0; i < lst.count(); ++i )
            {
                QDomElement el2 = el.childNodes().at( i ).toElement();
                if( el2.tagName().compare( "ex_orig", Qt::CaseInsensitive ) == 0 )
                {
                    el2.setTagName( "span" );
                    el2.setAttribute( "class", "xdxf_ex_orig" );
                }
                else if( el2.tagName().compare( "ex_tran", Qt::CaseInsensitive ) == 0 )
                {
                    el2.setTagName( "span" );
                    el2.setAttribute( "class", "xdxf_ex_tran" );
                }
            }
        }
        if( ( !author.isEmpty() || !source.isEmpty() )
                && ( !el.text().isEmpty() || !el.childNodes().isEmpty() ) )
        {
            QDomElement el2 = dd.createElement( "span" );
            el2.setAttribute( "class", "xdxf_ex_source" );
            QString text = author;
            if( !source.isEmpty() )
            {
                if( !text.isEmpty() )
                    text += ", ";
                text += source;
            }
            QDomText txtNode = dd.createTextNode( text );
            el2.appendChild( txtNode );
            el.appendChild( el2 );
        }

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_ex" );
        else
            el.setAttribute( "class", "xdxf_ex_old" );
    }

    nodes = dd.elementsByTagName( "mrkd" ); // marked out words in translations/examples of usage

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_ex_markd" );
    }

    nodes = dd.elementsByTagName( "k" ); // Key

    if( headword )
        headword->clear();

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        if( type == STARDICT )
        {
            el.setTagName( "span" );
            el.setAttribute( "class", "xdxf_k" );
        }
        else
        {
            if( headword /*&& headword->isEmpty()*/ )
                *headword = el.text();

            el.setTagName( "div" );
            el.setAttribute( "class", "xdxf_headwords" );
            if( dictPtr->isFromLanguageRTL() != dictPtr->isToLanguageRTL() )
                el.setAttribute( "dir", dictPtr->isFromLanguageRTL() ? "rtl" : "ltr" );
        }
    }

    // processing of nested <def>s
    if( isLogicalFormat ) // in articles with visual format <def> tags do not effect the formatting.
    {
        nodes = dd.elementsByTagName( "def" );

        // this is a logical type of XDXF, so we need to render proper numbering
        // we will do it this way:

        // 1. we compute the maximum nesting depth of the article
        int maxNestingDepth = 1; // maximum nesting depth of the article
        for( int i = 0; i < nodes.size(); i++ )
        {
            QDomElement el = nodes.at( i ).toElement();
            QDomElement nestingNode = el;
            int nestingCount = 0;
            while ( nestingNode.parentNode().toElement().tagName() == "def" )
            {
                nestingCount++;
                nestingNode = nestingNode.parentNode().toElement();
            }
            if ( nestingCount > maxNestingDepth )
                maxNestingDepth = nestingCount;
        }
        // 2. in this loop we go layer-by-layer through all <def> and insert proper numbers according to its structure
        for( int j = maxNestingDepth; j > 0; j-- ) // j symbolizes special depth to be processed at this iteration
        {
            int siblingCount = 0; // this  that counts the number of among all siblings of this depth
            QString numberText = ""; // the number to be inserted into the beginning of <def> (I,II,IV,1,2,3,a),b),c)...)
            for( int i = 0; i < nodes.size(); i++ )
            {
                QDomElement el = nodes.at( i ).toElement();
                QDomElement nestingNode = el;
                // computing the depth @nestingDepth of a current node @el
                int nestingDepth = 0;
                while( nestingNode.parentNode().toElement().tagName() == "def" )
                {
                    nestingDepth++;
                    nestingNode=nestingNode.parentNode().toElement();
                }
                // we process nodes on of current depth @j
                // we do this in order not to break the numbering at this depth level
                if (nestingDepth == j)
                {
                    siblingCount++;
                    if( maxNestingDepth == 1 )
                    {
                        numberText = numberText.setNum( siblingCount ) + ". ";
                    }
                    else if( maxNestingDepth == 2 )
                    {
                        if( nestingDepth == 1 )
                            numberText = numberText.setNum( siblingCount ) + ". ";
                        if( nestingDepth == 2 )
                            numberText = numberText.setNum( siblingCount ) + ") ";
                    }
                    else
                    {
                        if( nestingDepth == 1 )
                            numberText = QString::fromStdString( convertToRoman(siblingCount,0) + ". " );
                        if( nestingDepth == 2 )
                            numberText = numberText.setNum( siblingCount ) + ". ";
                        if( nestingDepth == 3 )
                            numberText = numberText.setNum( siblingCount ) + ") ";
                        if( nestingDepth == 4 )
                            numberText = QString::fromStdString( convertToRoman(siblingCount,1) + ") " );
                    }
                    QDomElement numberNode = dd.createElement( "span" );
                    numberNode.setAttribute( "class", "xdxf_num" );
                    QDomText text_num = dd.createTextNode( numberText );
                    numberNode.appendChild( text_num );
                    el.insertBefore( numberNode, el.firstChild() );

                    if ( el.hasAttribute( "cmt" ) )
                    {
                        QDomElement cmtNode = dd.createElement( "span" );
                        cmtNode.setAttribute( "class", "xdxf_co" );
                        QDomText text_num = dd.createTextNode( el.attribute( "cmt" ) );
                        cmtNode.appendChild( text_num );
                        el.insertAfter( cmtNode, el.firstChild() );
                    }
                }
                else if( nestingDepth < j ) // if it goes one level up @siblingCount needs to be reset
                    siblingCount = 0;
            }
        }
        // we finally change all <def> tags into 'xdxf_def' <span>s
        while( nodes.size() )
        {
            QDomElement el = nodes.at( 0 ).toElement();
            el.setTagName( "span" );
            el.setAttribute( "class", "xdxf_def" );
        }
    }

    nodes = dd.elementsByTagName( "opt" ); // Optional headword part

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_opt" );
    }

    nodes = dd.elementsByTagName( "kref" ); // Reference to another word

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "a" );
        el.setAttribute( "href", QString( "bword:" ) + el.text() );
        el.setAttribute( "class", "xdxf_kref" );
        if ( el.hasAttribute( "idref" ) )
        {
            // todo implement support for referencing only specific parts of the article
            el.setAttribute( "href", QString( "bword:" ) + el.text() + "#" + el.attribute( "idref" ));
        }
        if ( el.hasAttribute( "kcmt" ) )
        {
            QDomText kcmtText = dd.createTextNode( " " + el.attribute( "kcmt" ) );
            el.parentNode().insertAfter( kcmtText, el );
        }
    }

    nodes = dd.elementsByTagName( "iref" ); // Reference to internet site

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        QString ref = el.attribute( "href" );
        if( ref.isEmpty() )
            ref = el.text();

        el.setAttribute( "href", ref );
        el.setTagName( "a" );
    }

    // Abbreviations
    if( revisionNumber < 29 )
        nodes = dd.elementsByTagName( "abr" );
    else
        nodes = dd.elementsByTagName( "abbr" );

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_abbr" );
        if( type == XDXF && pAbrv != NULL )
        {
            string val = Utf8::encode( Folding::trimWhitespace( gd::toWString( el.text() ) ) );

            // If we have such a key, display a title

            map< string, string >::const_iterator i = pAbrv->find( val );

            if ( i != pAbrv->end() )
            {
                string title;

                if ( Utf8::decode( i->second ).size() < 70 )
                {
                    // Replace all spaces with non-breakable ones, since that's how Lingvo shows tooltips
                    title.reserve( i->second.size() );

                    for( char const * c = i->second.c_str(); *c; ++c )
                    {
                        if ( *c == ' ' || *c == '\t' )
                        {
                            // u00A0 in utf8
                            title.push_back( char(0xC2) );
                            title.push_back( char(0xA0) );
                        }
                        else
                            if( *c == '-' ) // Change minus to non-breaking hyphen (uE28091 in utf8)
                            {
                                title.push_back( char(0xE2) );
                                title.push_back( char(0x80) );
                                title.push_back( char(0x91) );
                            }
                            else
                                title.push_back( *c );
                    }
                }
                else
                    title = i->second;
                el.setAttribute( "title", gd::toQString( Utf8::decode( title ) ) );
            }
        }
    }

    nodes = dd.elementsByTagName( "dtrn" ); // Direct translation

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_dtrn" );
    }

    nodes = dd.elementsByTagName( "c" ); // Color

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );

        if ( el.hasAttribute( "c" ) )
        {
            el.setAttribute( "style", "color:" + el.attribute( "c" ) );
            el.removeAttribute( "c" );
        }
        else
            el.setAttribute( "style", "color:blue" );
    }

    nodes = dd.elementsByTagName( "co" ); // Editorial comment

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_co" );
        else
            el.setAttribute( "class", "xdxf_co_old" );
    }

    /* grammar information */
    nodes = dd.elementsByTagName( "gr" ); // proper grammar tag
    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_gr" );
        else
            el.setAttribute( "class", "xdxf_gr_old" );
    }
    nodes = dd.elementsByTagName( "pos" ); // deprecated grammar tag
    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_gr" );
        else
            el.setAttribute( "class", "xdxf_gr_old" );
    }
    nodes = dd.elementsByTagName( "tense" ); // deprecated grammar tag
    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_gr" );
        else
            el.setAttribute( "class", "xdxf_gr_old" );
    }
    /* end of grammar generation */

    nodes = dd.elementsByTagName( "tr" ); // Transcription

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        el.setTagName( "span" );
        if( isLogicalFormat )
            el.setAttribute( "class", "xdxf_tr" );
        else
            el.setAttribute( "class", "xdxf_tr_old" );
    }

    // Ensure that ArticleNetworkAccessManager can deal with XDXF images.
    // We modify the URL by using the dictionary ID as the hostname.
    // This is necessary to determine from which dictionary a requested
    // image originates.
    nodes = dd.elementsByTagName( "img" );

    for( int i = 0; i < nodes.size(); i++ )
    {
        QDomElement el = nodes.at( i ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        if ( el.hasAttribute( "src" ) )
        {
            fixLink( el, dictPtr->getId(), "src" );
        }

        if ( el.hasAttribute( "losrc" ) )
        {
            fixLink( el, dictPtr->getId(), "losrc" );
        }

        if ( el.hasAttribute( "hisrc" ) )
        {
            fixLink( el, dictPtr->getId(), "hisrc" );
        }
    }

    nodes = dd.elementsByTagName( "rref" ); // Resource reference

    while( nodes.size() )
    {
        QDomElement el = nodes.at( 0 ).toElement();

        if( el.text().isEmpty() && el.childNodes().isEmpty() )
            el.appendChild( fakeElement( dd ) );

        //    if( type == XDXF && dictPtr != NULL && !el.hasAttribute( "start" ) )
        if( dictPtr != NULL && !el.hasAttribute( "start" ) )
        {
            string filename = Utf8::encode( gd::toWString( el.text() ) );

            if ( Filetype::isNameOfPicture( filename ) )
            {
                QUrl url;
                url.setScheme( "bres" );
                url.setHost( QString::fromUtf8( dictPtr->getId().c_str() ) );
                url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( filename.c_str() ) ) );

                QDomElement newEl = dd.createElement( "img" );
                newEl.setAttribute( "src", url.toEncoded().data() );
                newEl.setAttribute( "alt", Html::escape( filename ).c_str() );

                QDomNode parent = el.parentNode();
                if( !parent.isNull() )
                {
                    parent.replaceChild( newEl, el );
                    continue;
                }
            }
            else if( Filetype::isNameOfSound( filename ) )
            {

                QDomElement el_script = dd.createElement( "script" );
                QDomNode parent = el.parentNode();
                if( !parent.isNull() )
                {
                    bool search = false;
                    if( type == STARDICT )
                    {
                        string n = FsEncoding::dirname( dictPtr->getDictionaryFilenames()[ 0 ] ) +
                                FsEncoding::separator() + string( "res" ) + FsEncoding::separator() +
                                FsEncoding::encode( filename );
                        search = !File::exists( n ) &&
                                ( !resourceZip ||
                                  !resourceZip->isOpen() ||
                                  !resourceZip->hasFile( Utf8::decode( filename ) ) );
                    }
                    else
                    {
                        string n = dictPtr->getDictionaryFilenames()[ 0 ] + ".files" +
                                FsEncoding::separator() +
                                FsEncoding::encode( filename );
                        search = !File::exists( n ) && !File::exists( FsEncoding::dirname( dictPtr->getDictionaryFilenames()[ 0 ] ) +
                                FsEncoding::separator() +
                                FsEncoding::encode( filename ) ) &&
                                ( !resourceZip ||
                                  !resourceZip->isOpen() ||
                                  !resourceZip->hasFile( Utf8::decode( filename ) ) );
                    }


                    QUrl url;
                    url.setScheme( "gdau" );
                    url.setHost( QString::fromUtf8( search ? "search" : dictPtr->getId().c_str() ) );
                    url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( filename.c_str() ) ) );

                    el_script.setAttribute( "type", "text/javascript" );
                    parent.replaceChild( el_script, el );

                    QDomText el_txt = dd.createTextNode( makeAudioLinkScript( string( "\"" ) + url.toEncoded().data() + "\"",
                                                                              dictPtr->getId() ).c_str() );
                    el_script.appendChild( el_txt );

                    QDomElement el_span = dd.createElement( "span" );
                    el_span.setAttribute( "class", "xdxf_wav" );
                    parent.insertAfter( el_span, el_script );

                    QDomElement el_a = dd.createElement( "a" );
                    el_a.setAttribute( "href", url.toEncoded().data() );
                    el_span.appendChild( el_a );

                    QDomElement el_img = dd.createElement( "img");
                    el_img.setAttribute( "src", "qrcx://localhost/icons/playsound.png" );
                    el_img.setAttribute( "border", "0" );
                    el_img.setAttribute( "align", "absmiddle" );
                    el_img.setAttribute( "alt", "Play" );
                    el_a.appendChild( el_img );

                    continue;
                }
            }
        }

        // We don't really know how to handle this at the moment, so we'll just
        // convert it to a span and leave it as is for now.

        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_rref" );
    }

    //  GD_DPRINTF( "Result>>>>>>>>>>: %s\n\n\n", dd.toByteArray( 0 ).data() );

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    return dd.toString( 1 ).remove('\n').remove( QRegularExpression( "<(b|i)/>" ) ).toUtf8().data();
#else
    return dd.toString( 1 ).remove('\n').remove( QRegExp( "<(b|i)/>" ) ).toUtf8().data();
#endif
#endif
}

}

