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

#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <log4cxx/consoleappender.h>
#include <log4cxx/level.h>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>

#include "app.h"
#include "appinfo.h"
#include "hexdameview.h"
#include "hexdamegame.h"
#include "player.h"
#include "player/heuristic.h"

namespace
{
bool
matches_option(const QString &givenoption, const QString &expectedoption,
               int mindashes = 1, int maxdashes = 2)
{
    int dashes = 0;
    if (givenoption.length() > 0) {
        while ((dashes < givenoption.length()) && (givenoption[dashes] == '-')) {
            dashes++;
        }
    }
    if ((dashes < mindashes) || (dashes > maxdashes)) {
        return false;
    }
    QString substr = givenoption.right(givenoption.length() - dashes);
    return (expectedoption.compare(substr, Qt::CaseInsensitive) == 0);
}
}

inline std::ostream &
operator<<(std::ostream &out, const QString &str)
{
    QByteArray a = str.toUtf8();
    out << a.constData();
    return out;
}

App::App(int &argc, char **argv)
    : QApplication(argc, argv)
    , _invocation(argv[0])
    , _gui(false)
    , _interactive(false)
{
    // Enforce singleton property
    if (_instance) {
        throw std::runtime_error("Only one instance of App allowed.");
    }

    // Remember if we are done
    bool done = false;

    // Set the singleton instance to this
    _instance = this;

    // Set the application properties
    setApplicationName(APPLICATION_NAME);
    setApplicationVersion(APPLICATION_VERSION_STRING);
    setOrganizationName(APPLICATION_VENDOR_NAME);
    setOrganizationDomain(APPLICATION_VENDOR_URL);

    // Configure the logging mechanism
    log4cxx::LoggerPtr rootlogger = log4cxx::Logger::getRootLogger();
    rootlogger->addAppender(new log4cxx::ConsoleAppender(new log4cxx::PatternLayout("[%-5p] %m%n")));

    // Parse the commandline
    int idx = 1;
    while (idx < argc) {
        QString arg(argv[idx]);
        if (matches_option(arg, "help", 0) || matches_option(arg, "h") || matches_option(arg, "?", 0)) {
            printHelpMessage();
            std::exit(0);
        } else if (matches_option(arg, "version", 0)) {
            printVersionMessage();
            std::exit(0);
        } else if (matches_option(arg, "version-triplet")) {
            printVersionTripletMessage();
            std::exit(0);
        } else if (matches_option(arg, "prefset")) {
            // Verify that there is another argument
            if ((idx + 1) >= argc) {
                LOG4CXX_FATAL(_logger, "Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }

            // Increment the index
            idx++;

            // Get the next parameter
            std::string param(argv[idx]);

            // Determine if there is an equals sign
            // If there is, set the preference;
            // Otherwise, remove the preference
            size_t eqidx = param.find('=');
            if (eqidx != std::string::npos) {
                std::string key = param.substr(0, eqidx);
                std::string val = param.substr(eqidx + 1);
                setPreference(key, val);
            } else {
                unsetPreference(param);
            }
            done = true;
        } else if (matches_option(arg, "prefdel")) {
            // Verify that there is another argument
            if ((idx + 1) >= argc) {
                LOG4CXX_FATAL(_logger, "Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }

            // Increment the index
            idx++;

            // Get the next parameter
            std::string param(argv[idx]);

            // Remove the preference
            unsetPreference(param);
            done = true;
        } else if (matches_option(arg, "preflist")) {
            printAllPreferences();
            done = true;
        } else if (matches_option(arg, "prefget")) {
            // Verify that there is another argument
            if ((idx + 1) >= argc) {
                LOG4CXX_FATAL(_logger, "Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }

            // Increment the index
            idx++;

            // Get the next parameter
            std::string param(argv[idx]);

            // Print the preference
            printPreference(param);
            done = true;
        } else if (matches_option(arg, "loglevel")) {
            // Verify that there is another argument
            if ((idx + 1) >= argc) {
                LOG4CXX_FATAL(_logger, "Option \"" << arg << "\" requires a parameter.");
                std::exit(1);
            }

            // Increment the index
            idx++;

            // Get the next parameter
            std::string param(argv[idx]);

            // Determine if there is an equals sign and act accordingly
            size_t eqidx = param.find('=');
            if (eqidx != std::string::npos) {
                std::string logger = param.substr(0, eqidx);
                std::string level  = param.substr(eqidx + 1);
                setLogLevel(logger, level);
            } else {
                setLogLevel("", param);
            }
        } else if (matches_option(arg, "appid") || matches_option(arg, "application-identifier")) {
            printApplicationIdentifier();
            std::exit(0);
        } else if (matches_option(arg, "gui")) {
            if (_interactive) {
                LOG4CXX_FATAL(_logger, "Cannot specify both \"--gui\" and \"--interactive\" simultaneously.");
                std::exit(1);
            }
            if (_gui) {
                LOG4CXX_WARN(_logger, "Option \"" << arg << "\" already specified. Ignoring.");
            }
            _gui = true;
        } else if (matches_option(arg, "interactive")) {
            if (_gui) {
                LOG4CXX_FATAL(_logger, "Cannot specify both \"--gui\" and \"--interactive\" simultaneously.");
                std::exit(1);
            }
            if (_interactive) {
                LOG4CXX_WARN(_logger, "Option \"" << arg << "\" already specified. Ignoring.");
            }
            _interactive = true;
        } else {
            LOG4CXX_WARN(_logger, "Unrecognized option: \"" << arg << "\". Ignoring");
        }
        idx++;
    }

    initGUI();
}

App::~App()
{
}

App *
App::INSTANCE()
{
    return _instance;
}

void
App::initGUI()
{
    // Construct the main window
    _mainwindow.reset(new QMainWindow);

    loadActions();
    loadStatusBar();

    // TODO test this on a non-tiling WM
    QDesktopWidget *desktopwidget = desktop();
    int preferredwidth = 1024;
    int preferredheight = 768;
    int leftmargin = (desktopwidget->width() - preferredwidth) / 2;
    int topmargin  = (desktopwidget->height() - preferredheight) / 2;
    _mainwindow->move(leftmargin, topmargin);

    // Display the main window
    _mainwindow->setVisible(true);

    newGame();
}

void
App::loadActions()
{
    QToolBar *toolbar = new QToolBar("maintoolbar", _mainwindow.get());
    _mainwindow->addToolBar(Qt::TopToolBarArea, toolbar);

    QMenu *menu = _mainwindow->menuBar()->addMenu(tr("&Game"));
    //FIXME why does this not use the theme icon?
    QAction *action = menu->addAction(QIcon::fromTheme("document-new"), tr("&New Game"));
    toolbar->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(newGame()));

    action = toolbar->addAction(tr("&Debug"));
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(setDebugMode(bool)));
}

void
App::loadStatusBar()
{
    //TODO maybe not use a statusbar for this
    QStatusBar *statusBar = _mainwindow->statusBar();
    QStringList players{"Human", "Random", "NegaMax", "NegaMaxWTt", "MTD-f"};

    _whiteCombo = new QComboBox();
    statusBar->addPermanentWidget(_whiteCombo);
    _whiteCombo->addItems(players);
    _whiteCombo->setCurrentIndex(4);
    connect(_whiteCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setWhitePlayer(int)));

    statusBar->addPermanentWidget(new QLabel("White"));

    statusBar->showMessage("Test");

    statusBar->addPermanentWidget(new QLabel("Black"));

    _blackCombo = new QComboBox();
    statusBar->addPermanentWidget(_blackCombo);
    _blackCombo->addItems(players);
    _blackCombo->setCurrentIndex(4);
    connect(_blackCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setBlackPlayer(int)));
}

void
App::newGame()
{
    if (_game) delete _game;

    _game = new HexdameGame(this);
    _gameView = new HexdameView(_game);

    connect(_game, SIGNAL(gameOver()), this, SLOT(gameOver()));
    setBlackPlayer(_blackCombo->currentIndex());
    setWhitePlayer(_whiteCombo->currentIndex());

    _mainwindow->setCentralWidget(_gameView);

    _game->startNextTurn();
}

void
App::gameOver()
{
    _mainwindow->statusBar()->showMessage("Game Over");
}

void
App::setBlackPlayer(int idx)
{
    HumanPlayer *player = new HumanPlayer(_game, Black);
    switch (idx) {
        case 0:
            connect(_gameView, SIGNAL(playerMoved(Coord,Coord)), player, SLOT(moved(Coord,Coord)));
            _game->setBlackPlayer(player);
            break;
        case 1:
            _game->setBlackPlayer(new RandomPlayer(_game, Black));
            break;
        case 2:
            _game->setBlackPlayer(new NegaMaxPlayer(_game, Black, new SomeHeuristic()));
            break;
        case 3:
            _game->setBlackPlayer(new NegaMaxPlayerWTt(_game, Black, new SomeHeuristic()));
            break;
        case 4:
            _game->setBlackPlayer(new MTDfPlayer(_game, Black, new SomeHeuristic()));
            break;
    }
}

void
App::setWhitePlayer(int idx)
{
    HumanPlayer *player = new HumanPlayer(_game, White);
    switch (idx) {
        case 0:
            connect(_gameView, SIGNAL(playerMoved(Coord,Coord)), player, SLOT(moved(Coord,Coord)));
            _game->setWhitePlayer(player);
            break;
        case 1:
            _game->setWhitePlayer(new RandomPlayer(_game, White));
            break;
        case 2:
            _game->setWhitePlayer(new NegaMaxPlayer(_game, White, new SomeHeuristic()));
            break;
        case 3:
            _game->setWhitePlayer(new NegaMaxPlayerWTt(_game, White, new SomeHeuristic()));
            break;
        case 4:
            _game->setWhitePlayer(new MTDfPlayer(_game, White, new SomeHeuristic()));
            break;
    }
}

void
App::setDebugMode(bool debug)
{
    _game->setDebugMode(debug);
}

void
App::printHelpMessage()
{
    //TODO replace with some options of my own
    std::cout << "Usage: " << getProjectInvocation() << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    --help                       Displays this help message." << std::endl;
    std::cout << "    --version                    Prints the program version." << std::endl;
    std::cout << "    --version-triplet            Prints the undecorated program version." << std::endl;
    std::cout << "    --appid                      Prints the unique application identifier." << std::endl;
    std::cout << "    --prefset <key>=<val>        Sets the given preference." << std::endl;
    std::cout << "    --prefdel <key>              Unsets the given preference." << std::endl;
    std::cout << "    --prefget <key>              Prints the given preference." << std::endl;
    std::cout << "    --preflist                   Lists all preferences that are set." << std::endl;
    std::cout << "    --loglevel <level>           Sets the current logging level." << std::endl;
    std::cout << "    --loglevel <logger>=<level>  Sets the logging level for the given logger." << std::endl;
    std::cout << "    --gui                        Run in graphical user interface mode." << std::endl;
    std::cout << "    --interactive                Run in interactive commandline mode." << std::endl;
    std::cout << "Log Levels:" << std::endl;
    std::cout << "    all" << std::endl;
    std::cout << "    trace" << std::endl;
    std::cout << "    debug" << std::endl;
    std::cout << "    info" << std::endl;
    std::cout << "    warn" << std::endl;
    std::cout << "    error" << std::endl;
    std::cout << "    fatal" << std::endl;
    std::cout << "    off" << std::endl;
}

void
App::printVersionMessage()
{
    std::cout << getProjectName() << " v" << getProjectVersion() << std::endl;
    std::cout << getProjectVendorName() << "; Copyright (C) " << getProjectCopyrightYears();
}

void
App::printVersionTripletMessage()
{
    std::cout << getProjectVersion() << std::endl;
}

void
App::printApplicationIdentifier()
{
    std::cout << getProjectID() << std::endl;
}

QString
App::getProjectName()
{
    return APPLICATION_NAME;
}


QString
App::getProjectCodeName()
{
    return APPLICATION_CODENAME;
}

QString
App::getProjectVendorID()
{
    return APPLICATION_VENDOR_ID;
}

QString
App::getProjectVendorName()
{
    return APPLICATION_VENDOR_NAME;
}

QString
App::getProjectID()
{
    return APPLICATION_ID;
}

int
App::getProjectMajorVersion()
{
    return APPLICATION_VERSION_MAJOR;
}

int
App::getProjectMinorVersion()
{
    return APPLICATION_VERSION_MINOR;
}

int
App::getProjectPatchVersion()
{
    return APPLICATION_VERSION_PATCH;
}

QString
App::getProjectVersion()
{
    return APPLICATION_VERSION_STRING;
}

QString
App::getProjectCopyrightYears()
{
    return APPLICATION_COPYRIGHT_YEARS;
}

QString
App::getProjectInvocation()
{
    return _invocation;
}

std::string
App::getKeyName(const std::string &key)const
{
    std::string result(key);
    for (size_t i = 0; i < result.size(); i++) {
        if ((result[i] == '/') || (result[i] == '\\')) {
            result[i] = '.';
        }
    }
    return result;
}

std::string
App::getKeyRepr(const std::string &key)const
{
    std::string result(key);
    for (size_t i = 0; i < result.size(); i++) {
        if ((result[i] == '/') || (result[i] == '\\')) {
            result[i] = '/';
        }
    }
    return result;
}

void
App::setPreference(const std::string &key, const std::string &val)
{
    QSettings settings;
    std::string keyrep(getKeyRepr(key));
    QString qkeyrep(keyrep.c_str());
    QString qval(val.c_str());
    settings.setValue(qkeyrep, qval);
    settings.sync();
}

void
App::unsetPreference(const std::string &key)
{
    QSettings settings;
    std::string keyrep(getKeyRepr(key));
    QString qkeyrep(keyrep.c_str());
    settings.beginGroup(qkeyrep);
    if ((settings.childGroups().length() != 0) || (settings.childKeys().length() != 0)) {
        settings.setValue("", "");
    } else {
        settings.remove("");
    }
    settings.endGroup();
    settings.sync();
}

void
App::printPreference(const std::string &key)const
{
    QSettings settings;
    std::string keyrep(getKeyRepr(key));
    QString qkeyrep(keyrep.c_str());
    QString result = "undefined";
    if (settings.contains(qkeyrep)) {
        result = settings.value(qkeyrep, QString("undefined")).toString();
    }
    std::cout << result << std::endl;
}

void
App::printAllPreferences()const
{
    QSettings settings;
    QStringList keys = settings.allKeys();
    for (QStringList::const_iterator it = keys.begin(); it != keys.end(); ++it) {
        QString qkeystr = *it;
        QString qvalstr = settings.value(qkeystr).toString();

        if (! qvalstr.isEmpty()) {
            std::string key = getKeyName(convert(qkeystr));
            std::cout << key << "=" << qvalstr << std::endl;
        }
    }
}

void
App::setLogLevel(const std::string &logger, const std::string &level)
{
    log4cxx::LoggerPtr loggerptr = ((logger == "") ? (log4cxx::Logger::getRootLogger()) : (log4cxx::Logger::getLogger(logger)));
    std::string lowercaselevel(level);
    for (size_t i = 0; i < lowercaselevel.size(); i++) {
        lowercaselevel[i] = std::tolower(lowercaselevel[i]);
    }

    if (lowercaselevel == "all") {
        loggerptr->setLevel(log4cxx::Level::getAll());
    } else if (lowercaselevel == "trace") {
        loggerptr->setLevel(log4cxx::Level::getTrace());
    } else if (lowercaselevel == "debug") {
        loggerptr->setLevel(log4cxx::Level::getDebug());
    } else if (lowercaselevel == "info") {
        loggerptr->setLevel(log4cxx::Level::getInfo());
    } else if (lowercaselevel == "warn") {
        loggerptr->setLevel(log4cxx::Level::getWarn());
    } else if (lowercaselevel == "error") {
        loggerptr->setLevel(log4cxx::Level::getError());
    } else if (lowercaselevel == "fatal") {
        loggerptr->setLevel(log4cxx::Level::getFatal());
    } else if ((lowercaselevel == "off")  || (lowercaselevel == "none")) {
        loggerptr->setLevel(log4cxx::Level::getOff());
    } else {
        LOG4CXX_FATAL(_logger, "Unrecognized logging level: \"" << level << "\".");
        std::exit(1);
    }
}

std::string
App::convert(const QString &str)const
{
    QByteArray data = str.toUtf8();
    std::string result(data.constData());
    return result;
}

QString
App::convert(const std::string &str)const
{
    QString result(str.c_str());
    return result;
}

App *
App::_instance = 0;

log4cxx::LoggerPtr
App::_logger = log4cxx::Logger::getLogger("App");
