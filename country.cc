/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "country.hh"
#include "folding.hh"
#include <QtCore>
#include "wstring_qt.hh"

namespace Country {

namespace
{
  struct Database: public QMap< QString, QString >
  {
    static Database const & instance()
    { static const Database db; return db; }

  private:

    Database();

    void addCountry( QString const & country, QString const & code )
    {
      (*this)[ gd::toQString( Folding::apply( gd::toWString( country ) ) ) ] = code.toLower();
    }
  };

  Database::Database()
  {
    addCountry( "Afghanistan", "AF" );
    addCountry( "Aland Islands", "AX" );
    addCountry( "Albania", "AL" );
    addCountry( "Algeria", "DZ" );
    addCountry( "American Samoa", "AS" );
    addCountry( "Andorra", "AD" );
    addCountry( "Angola", "AO" );
    addCountry( "Anguilla", "AI" );
    addCountry( "Antarctica", "AQ" );
    addCountry( "Antigua and Barbuda", "AG" );
    addCountry( "Argentina", "AR" );
    addCountry( "Armenia", "AM" );
    addCountry( "Aruba", "AW" );
    addCountry( "Australia", "AU" );
    addCountry( "Austria", "AT" );
    addCountry( "Azerbaijan", "AZ" );
    addCountry( "Bahamas", "BS" );
    addCountry( "Bahrain", "BH" );
    addCountry( "Bangladesh", "BD" );
    addCountry( "Barbados", "BB" );
    addCountry( "Belarus", "BY" );
    addCountry( "Belgium", "BE" );
    addCountry( "Belize", "BZ" );
    addCountry( "Benin", "BJ" );
    addCountry( "Bermuda", "BM" );
    addCountry( "Bhutan", "BT" );
    addCountry( "Bolivia, Plurinational State of", "BO" );
    addCountry( "Bolivia", "BO" );
    addCountry( "Bosnia and Herzegovina", "BA" );
    addCountry( "Botswana", "BW" );
    addCountry( "Bouvet Island", "BV" );
    addCountry( "Brazil", "BR" );
    addCountry( "British Indian Ocean Territory", "IO" );
    addCountry( "Brunei Darussalam", "BN" );
    addCountry( "Bulgaria", "BG" );
    addCountry( "Burkina Faso", "BF" );
    addCountry( "Burundi", "BI" );
    addCountry( "Cambodia", "KH" );
    addCountry( "Cameroon", "CM" );
    addCountry( "Canada", "CA" );
    addCountry( "Cape Verde", "CV" );
    addCountry( "Cayman Islands", "KY" );
    addCountry( "Central African Republic", "CF" );
    addCountry( "Chad", "TD" );
    addCountry( "Chile", "CL" );
    addCountry( "China", "CN" );
    addCountry( "Christmas Island", "CX" );
    addCountry( "Cocos (Keeling) Islands", "CC" );
    addCountry( "Colombia", "CO" );
    addCountry( "Comoros", "KM" );
    addCountry( "Congo", "CG" );
    addCountry( "Congo, the Democratic Republic of the", "CD" );
    addCountry( "Cook Islands", "CK" );
    addCountry( "Costa Rica", "CR" );
    addCountry( "Cote d'Ivoire", "CI" );
    addCountry( "Croatia", "HR" );
    addCountry( "Cuba", "CU" );
    addCountry( "Cyprus", "CY" );
    addCountry( "Czech Republic", "CZ" );
    addCountry( "Denmark", "DK" );
    addCountry( "Djibouti", "DJ" );
    addCountry( "Dominica", "DM" );
    addCountry( "Dominican Republic", "DO" );
    addCountry( "Ecuador", "EC" );
    addCountry( "Egypt", "EG" );
    addCountry( "El Salvador", "SV" );
    addCountry( "Equatorial Guinea", "GQ" );
    addCountry( "Eritrea", "ER" );
    addCountry( "Estonia", "EE" );
    addCountry( "Ethiopia", "ET" );
    addCountry( "Falkland Islands (Malvinas)", "FK" );
    addCountry( "Faroe Islands", "FO" );
    addCountry( "Fiji", "FJ" );
    addCountry( "Finland", "FI" );
    addCountry( "France", "FR" );
    addCountry( "French Guiana", "GF" );
    addCountry( "French Polynesia", "PF" );
    addCountry( "French Southern Territories", "TF" );
    addCountry( "Gabon", "GA" );
    addCountry( "Gambia", "GM" );
    addCountry( "Georgia", "GE" );
    addCountry( "Germany", "DE" );
    addCountry( "Ghana", "GH" );
    addCountry( "Gibraltar", "GI" );
    addCountry( "Greece", "GR" );
    addCountry( "Greenland", "GL" );
    addCountry( "Grenada", "GD" );
    addCountry( "Guadeloupe", "GP" );
    addCountry( "Guam", "GU" );
    addCountry( "Guatemala", "GT" );
    addCountry( "Guernsey", "GG" );
    addCountry( "Guinea", "GN" );
    addCountry( "Guinea-Bissau", "GW" );
    addCountry( "Guyana", "GY" );
    addCountry( "Haiti", "HT" );
    addCountry( "Heard Island and McDonald Islands", "HM" );
    addCountry( "Holy See (Vatican City State)", "VA" );
    addCountry( "Honduras", "HN" );
    addCountry( "Hong Kong", "HK" );
    addCountry( "Hungary", "HU" );
    addCountry( "Iceland", "IS" );
    addCountry( "India", "IN" );
    addCountry( "Indonesia", "ID" );
    addCountry( "Iran, Islamic Republic of", "IR" );
    addCountry( "Iran", "IR" );
    addCountry( "Iraq", "IQ" );
    addCountry( "Ireland", "IE" );
    addCountry( "Isle of Man", "IM" );
    addCountry( "Israel", "IL" );
    addCountry( "Italy", "IT" );
    addCountry( "Jamaica", "JM" );
    addCountry( "Japan", "JP" );
    addCountry( "Jersey", "JE" );
    addCountry( "Jordan", "JO" );
    addCountry( "Kazakhstan", "KZ" );
    addCountry( "Kenya", "KE" );
    addCountry( "Kiribati", "KI" );
    addCountry( "Korea, Democratic People's Republic of", "KP" );
    addCountry( "Korea, Republic of", "KR" );
    addCountry( "Korea", "KR" );
    addCountry( "Kuwait", "KW" );
    addCountry( "Kyrgyzstan", "KG" );
    addCountry( "Lao People's Democratic Republic", "LA" );
    addCountry( "Latvia", "LV" );
    addCountry( "Lebanon", "LB" );
    addCountry( "Lesotho", "LS" );
    addCountry( "Liberia", "LR" );
    addCountry( "Libyan Arab Jamahiriya", "LY" );
    addCountry( "Liechtenstein", "LI" );
    addCountry( "Lithuania", "LT" );
    addCountry( "Luxembourg", "LU" );
    addCountry( "Macao", "MO" );
    addCountry( "Macedonia, the former Yugoslav Republic of", "MK" );
    addCountry( "Macedonia", "MK" );
    addCountry( "Madagascar", "MG" );
    addCountry( "Malawi", "MW" );
    addCountry( "Malaysia", "MY" );
    addCountry( "Maldives", "MV" );
    addCountry( "Mali", "ML" );
    addCountry( "Malta", "MT" );
    addCountry( "Marshall Islands", "MH" );
    addCountry( "Martinique", "MQ" );
    addCountry( "Mauritania", "MR" );
    addCountry( "Mauritius", "MU" );
    addCountry( "Mayotte", "YT" );
    addCountry( "Mexico", "MX" );
    addCountry( "Micronesia, Federated States of", "FM" );
    addCountry( "Micronesia", "FM" );
    addCountry( "Moldova, Republic of", "MD" );
    addCountry( "Moldova", "MD" );
    addCountry( "Monaco", "MC" );
    addCountry( "Mongolia", "MN" );
    addCountry( "Montenegro", "ME" );
    addCountry( "Montserrat", "MS" );
    addCountry( "Morocco", "MA" );
    addCountry( "Mozambique", "MZ" );
    addCountry( "Myanmar", "MM" );
    addCountry( "Namibia", "NA" );
    addCountry( "Nauru", "NR" );
    addCountry( "Nepal", "NP" );
    addCountry( "Netherlands", "NL" );
    addCountry( "Netherlands Antilles", "AN" );
    addCountry( "New Caledonia", "NC" );
    addCountry( "New Zealand", "NZ" );
    addCountry( "Nicaragua", "NI" );
    addCountry( "Niger", "NE" );
    addCountry( "Nigeria", "NG" );
    addCountry( "Niue", "NU" );
    addCountry( "Norfolk Island", "NF" );
    addCountry( "Northern Mariana Islands", "MP" );
    addCountry( "Norway", "NO" );
    addCountry( "Oman", "OM" );
    addCountry( "Pakistan", "PK" );
    addCountry( "Palau", "PW" );
    addCountry( "Palestinian Territory, Occupied", "PS" );
    addCountry( "Panama", "PA" );
    addCountry( "Papua New Guinea", "PG" );
    addCountry( "Paraguay", "PY" );
    addCountry( "Peru", "PE" );
    addCountry( "Philippines", "PH" );
    addCountry( "Pitcairn", "PN" );
    addCountry( "Poland", "PL" );
    addCountry( "Portugal", "PT" );
    addCountry( "Puerto Rico", "PR" );
    addCountry( "Qatar", "QA" );
    addCountry( "Reunion", "RE" );
    addCountry( "Romania", "RO" );
    addCountry( "Russian Federation", "RU" );
    addCountry( "Russia", "RU" );
    addCountry( "Rwanda", "RW" );
    addCountry( "Saint Barthélemy", "BL" );
    addCountry( "Saint Helena, Ascension and Tristan da Cunha", "SH" );
    addCountry( "Saint Kitts and Nevis", "KN" );
    addCountry( "Saint Lucia", "LC" );
    addCountry( "Saint Martin (French part)", "MF" );
    addCountry( "Saint Pierre and Miquelon", "PM" );
    addCountry( "Saint Vincent and the Grenadines", "VC" );
    addCountry( "Samoa", "WS" );
    addCountry( "San Marino", "SM" );
    addCountry( "Sao Tome and Principe", "ST" );
    addCountry( "Saudi Arabia", "SA" );
    addCountry( "Senegal", "SN" );
    addCountry( "Serbia", "RS" );
    addCountry( "Seychelles", "SC" );
    addCountry( "Sierra Leone", "SL" );
    addCountry( "Singapore", "SG" );
    addCountry( "Slovakia", "SK" );
    addCountry( "Slovenia", "SI" );
    addCountry( "Solomon Islands", "SB" );
    addCountry( "Somalia", "SO" );
    addCountry( "South Africa", "ZA" );
    addCountry( "South Georgia and the South Sandwich Islands", "GS" );
    addCountry( "Spain", "ES" );
    addCountry( "Sri Lanka", "LK" );
    addCountry( "Sudan", "SD" );
    addCountry( "Suriname", "SR" );
    addCountry( "Svalbard and Jan Mayen", "SJ" );
    addCountry( "Swaziland", "SZ" );
    addCountry( "Sweden", "SE" );
    addCountry( "Switzerland", "CH" );
    addCountry( "Syrian Arab Republic", "SY" );
    addCountry( "Taiwan, Province of China", "TW" );
    addCountry( "Tajikistan", "TJ" );
    addCountry( "Tanzania, United Republic of", "TZ" );
    addCountry( "Tanzania", "TZ" );
    addCountry( "Thailand", "TH" );
    addCountry( "Timor-Leste", "TL" );
    addCountry( "Togo", "TG" );
    addCountry( "Tokelau", "TK" );
    addCountry( "Tonga", "TO" );
    addCountry( "Trinidad and Tobago", "TT" );
    addCountry( "Tunisia", "TN" );
    addCountry( "Turkey", "TR" );
    addCountry( "Turkmenistan", "TM" );
    addCountry( "Turks and Caicos Islands", "TC" );
    addCountry( "Tuvalu", "TV" );
    addCountry( "Uganda", "UG" );
    addCountry( "Ukraine", "UA" );
    addCountry( "United Arab Emirates", "AE" );
    addCountry( "United Kingdom", "GB" );
    addCountry( "United States", "US" );
    addCountry( "United States Minor Outlying Islands", "UM" );
    addCountry( "Uruguay", "UY" );
    addCountry( "Uzbekistan", "UZ" );
    addCountry( "Vanuatu", "VU" );
    addCountry( "Venezuela, Bolivarian Republic of", "VE" );
    addCountry( "Venezuela", "VE" );
    addCountry( "Viet Nam", "VN" );
    addCountry( "Virgin Islands, British", "VG" );
    addCountry( "Virgin Islands, U.S.", "VI" );
    addCountry( "Wallis and Futuna", "WF" );
    addCountry( "Western Sahara", "EH" );
    addCountry( "Yemen", "YE" );
    addCountry( "Zambia", "ZM" );
    addCountry( "Zimbabwe", "ZW" );
  }
}

QString englishNametoIso2( QString const & name )
{
  Database::const_iterator i = Database::instance().find( gd::toQString( Folding::apply( gd::toWString( name ) ) ) );

  if ( i == Database::instance().end() )
    return QString();
  else
    return i.value();
}

}
