/*
 * hexdame: a draughts game played on a hexagonal grid.
 * Copyright (C) 2013  Samir Benmendil <samir.benmendil@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
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
 */

#ifndef APP_H
#define APP_H

#include <memory>
#include <QtCore>
#include <QtGui>
#include <log4cxx/logger.h>

class HexdameGame;

class App : public QApplication
{
    Q_OBJECT
public:
    App(int &argc, char **argv);
    ~App();

    App *INSTANCE();

    QString getProjectName();
    QString getProjectCodeName();
    QString getProjectVendorID();
    QString getProjectVendorName();
    QString getProjectID();
    int getProjectMajorVersion();
    int getProjectMinorVersion();
    int getProjectPatchVersion();
    QString getProjectVersion();
    QString getProjectCopyrightYears();
    QString getProjectInvocation();

public slots:
    void newGame();
    void setWhitePlayer(int);
    void setBlackPlayer(int);

private:
    void initGUI();
    void interactiveMain();
    void consoleMain();
    void loadActions();

    void printHelpMessage();
    void printVersionMessage();
    void printVersionTripletMessage();
    void printApplicationIdentifier();
    void setPreference(const std::string &key, const std::string &val);
    void unsetPreference(const std::string &key);
    void printPreference(const std::string &key)const;
    void printAllPreferences()const;
    void setLogLevel(const std::string &logger, const std::string &level);
    std::string getKeyName(const std::string &key)const;
    std::string getKeyRepr(const std::string &key)const;
    std::string convert(const QString &str)const;
    QString convert(const std::string &str)const;
    void loadStatusBar();


    static App *_instance;
    static log4cxx::LoggerPtr _logger;
    QString _invocation;
    bool _gui;
    bool _interactive;
    std::shared_ptr<QMainWindow> _mainwindow;
    HexdameGame *_game;
};

#endif
