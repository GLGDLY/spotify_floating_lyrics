#ifndef LYRICS_DEF
#define LYRICS_DEF
#include "./thirdparty/cpp-httplib/httplib.h"
#include "app.h"
#include "json.h"
#include "widget.h"

#include <QThread>
#include <Qstring>
#include <ctime>
#include <stdint.h>


class LyricsThread : public QThread {
	Q_OBJECT
public:
	LyricsThread(Widget* widget, App* app);

	void run(void);

private:
	Widget* p_widget;
	App* p_app;
	httplib::Client open_cli, open_api_cli, auth_cli, lyrics_api_cli;
	QString auth_access_token, refresh_token;
	time_t auth_expires = 0;
	QJsonArray curr_lyrics;
	QString last_track_id;
	QString verify_code = "";
	JSON lyrics_data;

	void set_msgbox(QString title, QString text);
	void get_auth_verify_code(void);
	void get_auth_access_token(void);
	void get_lyrics(QString track_id, uint8_t retry = 0);
	void get_playing(uint8_t retry = 0);
};
#endif
