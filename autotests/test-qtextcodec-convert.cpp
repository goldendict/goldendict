#include <QTest>
#include <QDate>
#include "../iconv.hh"
#include <string>
#include "../wstring_qt.hh"

//used to test Iconv.cc
class testQTextCodec : public QObject
{
  Q_OBJECT
private slots:
  void testConvert();
  void testToWstring();
  void testToUtf8();
};

void testQTextCodec::testConvert()
{
  Iconv conv( "utf-8", Iconv::GdWchar );
  const char s[]  = { 0x61, 0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00 };
  void const * in = &s[ 0 ];
  size_t len      = 12;
  QString r       = conv.convert( in, len );
  QCOMPARE( r, "abc" );
}

void testQTextCodec::testToWstring()
{
  const char s[] = { 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0x00, 0x63 };
  gd::wstring r1 = Iconv::toWstring( "UTF-32BE", s, 12 );

  QCOMPARE( r1.size(), 3 );
  QCOMPARE( r1, U"abc" );
  char32_t * arr = (char32_t*)r1.c_str ();
  QCOMPARE( arr[ 0 ], 0x00000061 );
}

void testQTextCodec::testToUtf8()
{
  const char s[] = { 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0x00, 0x63 };
  std::string r1 = Iconv::toUtf8 ( "UTF-32BE", s, 12 );

  QCOMPARE( r1.size(), 3 );
  QCOMPARE( r1, u8"abc" );
  char * arr = (char*)r1.c_str ();
  QCOMPARE( arr[ 0 ], 0x61 );
}
QTEST_MAIN(testQTextCodec)
#include "test-qtextcodec-convert.moc"
