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
#include <QtCore/QSettings>
#include <QtGui/QDialog>
#include "configdialog.h"
#include "globals.h"

ConfigDialog::ConfigDialog() { }

ConfigDialog::~ConfigDialog() { }

void ConfigDialog::SetupUi() {
	setupUi(this);
	QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
	spinBox_locationTransmissionThreshold->setValue(cfg.value("locationTransmissionThreshold", DEFAULTCONFIG_LOCATIONTRANSMISSIONTHRESHOLD).toInt());
	spinBox_distanceTransmissionThreshold->setValue(cfg.value("distanceTransmissionThreshold", DEFAULTCONFIG_DISTANCETRANSMISSIONTHRESHOLD).toInt());
	spinBox_onlineStateTransmissionThreshold->setValue(cfg.value("onlineStateTransmissionThreshold", DEFAULTCONFIG_ONLINESTATETRANSMISSIONTHRESHOLD).toInt());
}

void ConfigDialog::accept() {
	Globals::locationTransmissionThreshold = spinBox_locationTransmissionThreshold->value();
	Globals::onlineStateTransmissionThreshold = spinBox_onlineStateTransmissionThreshold->value();

	QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
	cfg.setValue("locationTransmissionThreshold", spinBox_locationTransmissionThreshold->value());
	cfg.setValue("distanceTransmissionThreshold", spinBox_distanceTransmissionThreshold->value());
	cfg.setValue("onlineStateTransmissionThreshold", spinBox_onlineStateTransmissionThreshold->value());
	QDialog::accept();
}

void ConfigDialog::reject() {
	QDialog::reject();
}
