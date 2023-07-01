#include "app.h"
#include "lyrics.h"
#include "widget.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>
#include <fstream>


int main(int argc, char* argv[]) {
	try {
		/**
		 * Application
		 */
		QApplication a(argc, argv);
		Widget widget;

		QTranslator translator;
		const QStringList uiLanguages = QLocale::system().uiLanguages();
		for (const QString& locale : uiLanguages) {
			const QString baseName = "spotify_display_" + QLocale(locale).name();
			if (translator.load(":/i18n/" + baseName)) {
				a.installTranslator(&translator);
				break;
			}
		}
		widget.show();

		/**
		 * Thread
		 */
		App app;
		app.start();
		LyricsThread lyrics_thread(&widget, &app);
		lyrics_thread.start();

		qDebug() << "main thread id: " << QThread::currentThreadId();
		return a.exec();
	} catch (std::exception& e) {
		qDebug() << e.what();
		std::ofstream log("crash.log");
		if (log.is_open()) {
			log << e.what() << std::endl;
			log.close();
			qDebug() << "log file written";
		}
		return 1;
	} catch (...) {
		qDebug() << "unknown error";
		std::ofstream log("crash.log");
		if (log.is_open()) {
			log << "unknown error" << std::endl;
			log.close();
			qDebug() << "log file written";
		}
		return 1;
	}
}
