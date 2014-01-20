/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#pragma once
#include <QtGui/QDialog>
#include "ui_configdialog.h"

class ConfigDialog : public QDialog, public Ui::ConfigDialog {
	Q_OBJECT

public:
	ConfigDialog();
	~ConfigDialog();

	void SetupUi();

protected slots:
	void accept();
	void reject();

};
