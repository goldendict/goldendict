/* This file is (c) 2015 Zhe Wang <0x1997@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "chineseconversion.hh"
#include "ui_chineseconversion.h"

ChineseConversion::ChineseConversion(QWidget * parent, Config::Chinese const & cfg) :
  QGroupBox(parent),
  ui(new Ui::ChineseConversion)
{
  ui->setupUi( this );

  setChecked( cfg.enable );
  ui->enableSCToTWConversion->setChecked( cfg.enableSCToTWConversion );
  ui->enableSCToHKConversion->setChecked( cfg.enableSCToHKConversion );
  ui->enableTCToSCConversion->setChecked( cfg.enableTCToSCConversion );
}

ChineseConversion::~ChineseConversion()
{
  delete ui;
}

void ChineseConversion::getConfig( Config::Chinese & cfg ) const
{
  cfg.enable = isChecked();
  cfg.enableSCToTWConversion = ui->enableSCToTWConversion->isChecked();
  cfg.enableSCToHKConversion = ui->enableSCToHKConversion->isChecked();
  cfg.enableTCToSCConversion = ui->enableTCToSCConversion->isChecked();
}
