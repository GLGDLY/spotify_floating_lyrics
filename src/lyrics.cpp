#include "lyrics.h"

#include <QDebug>
#include <QMessageBox>
#include <stdlib.h>


const httplib::Headers general_header = {
	{"User-Agent",
	 "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 "
	 "Mobile Safari/537.36"},
	{"App-platform", "WebPlayer"},
	{"content-type", "text/html; charset=utf-8"},
};
const httplib::Headers auth_header = {
	{"Authorization", "Basic NmI0NTc0NzliYTg2NDg2MmI2ZGY0MjgzZDQ3NzRkMzM6ZWM1MGM4YzZlZWRlNDYzMTkxOTgyMWNiY2RmM2E4ZGI="},
	{"Content-Type", "application/x-www-form-urlencoded"},
	{"User-Agent",
	 "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 "
	 "Mobile Safari/537.36"},
	{"App-platform", "WebPlayer"},
};

/* --------------- public methods --------------- */

LyricsThread::LyricsThread(Widget* widget, App* app)
	: open_cli("https://open.spotify.com")
	, open_api_cli("https://api.spotify.com")
	, auth_cli("https://accounts.spotify.com")
	, lyrics_api_cli("https://spotify-lyric-api.herokuapp.com")
	, p_widget(widget)
	, p_app(app) {}

void LyricsThread::run(void) {
	this->get_auth_verify_code();
	this->get_auth_access_token();
	// get_access_token();
	// get_lyrics("5f8eCNwTlr0RJopE9vQ6mB");
	while (1) {
		this->msleep(500);
		this->get_playing();
	}
}

/* --------------- private methods --------------- */

void LyricsThread::set_msgbox(QString title, QString text) {
	qDebug() << title << ":" << text;
	QMessageBox message_box;
	message_box.critical(nullptr, title, text);
}

void LyricsThread::get_auth_verify_code(void) {
	const std::string auth_url =
		"https://accounts.spotify.com/en/"
		"authorize?response_type=code&client_id=6b457479ba864862b6df4283d4774d33&scope=user-read-playback-state&"
		"redirect_uri=http://localhost:8888/lyrics";
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	std::string cmd = "explorer \"" + auth_url + "\"";
	system(cmd.c_str());
#elif defined(__linux__) || defined(__linux) || defined(linux)
	std::string cmd = "xdg-open \"" + auth_url + "\"";
	system(cmd.c_str());
#elif defined(__APPLE__) || defined(__MACH__)
	std::string cmd = "open \"" + auth_url + "\"";
	system(cmd.c_str());
#else
	QMessageBox message_box(QMessageBox::NoIcon, "Auth", "copy the following url and open with browser:\n" + auth_url,
							QMessageBox::Ok);
	message_box.setTextInteractionFlags(Qt::TextSelectableByMouse);
#endif

	uint16_t timeout_timer = 0;
	while (!this->p_app->get_verify_code(this->verify_code)) {
		this->msleep(500);
		if (++timeout_timer >= 5 * 60) {
			this->set_msgbox("Error", "Fail on getting verify code, program will exit (timeout)");
			throw std::runtime_error("Fail on getting verify code, program will exit");
		}
	}
	qDebug() << "verify code: " << this->verify_code;
}

void LyricsThread::get_auth_access_token(void) {
	const std::string body = (this->refresh_token.isEmpty())
								 ? "code=" + this->verify_code.toStdString()
									   + "&grant_type=authorization_code&redirect_uri=http://localhost:8888/lyrics"
								 : "grant_type=refresh_token&refresh_token=" + this->refresh_token.toStdString();
	auth_cli.set_default_headers(auth_header);
	if (auto res = auth_cli.Post("/api/token", body, "application/x-www-form-urlencoded")) {
		// qDebug() << "Content-Type: " << QString::fromStdString(res.get_request_header_value("Content-Type"))
		// 		 << "\nauth: " << QString::fromStdString(res.get_request_header_value("Authorization"));
		if (res->status == 200) {
			qDebug() << QString::fromStdString(res->body);
			JSON d = res->body;
			if (auto token = d["access_token"].toString(); !token.isEmpty() && !token.isNull()) {
				this->auth_access_token = "";
				QTextStream out(&this->auth_access_token);
				out << "Bearer " << token;
				this->auth_expires = d["expires_in"].toInt() + time(nullptr);
				if (this->refresh_token.isEmpty()) {
					this->refresh_token = d["refresh_token"].toString();
				}
				qDebug() << "auth_access_token: " << this->auth_access_token << " << expires: " << this->auth_expires;
			} else {
				this->p_widget->SetErrMsgBox("Error",
											 QString("Fail on getting access token, program will exit (token: ")
												 + (token.isNull() ? QString("null") : token) + QString(")"));
			}

		} else {
			this->p_widget->SetErrMsgBox(
				"Error", QString("Fail on getting access token, program will exit (status code: ")
							 + QString::number(res->status) + QString(") >> ") + QString::fromStdString(res->body));
		}
	} else {
		this->p_widget->SetErrMsgBox("Error", "Fail on getting access token, program will exit (timeout)");
	}
}

// don't want to implement sp_dc cookies, direct use api from https://github.com/akashrchandran/spotify-lyrics-api
void LyricsThread::get_lyrics(QString track_id, uint8_t retry) {
	if (auto res = lyrics_api_cli.Get("/?trackid=" + track_id.toStdString())) {
		if (res->status == 200) {
			qDebug() << QString::fromStdString(res->body);
			JSON d = res->body;
			if (auto lyrics = d["lines"].toArray(); !lyrics.empty()) {
				this->curr_lyrics = lyrics;
				bool no_sync = true;
				for (QJsonValue raw_line : lyrics) {
					QJsonObject line = raw_line.toObject();
					auto words = line["words"].toString(), startTimeMs = line["startTimeMs"].toString();
					if (startTimeMs != "0") {
						no_sync = false;
					}
					qDebug() << startTimeMs << words;
				}
				if (no_sync) {
					this->curr_lyrics = QJsonArray();
				}
			} else {
				QJsonDocument doc;
				doc.setArray((lyrics.isEmpty() ? QJsonArray() : lyrics));
				this->p_widget->SetErrMsgBox("Error", QString("Fail on getting lyrics, program will exit (lyrics:")
														  + doc.toJson() + QString(")"));
			}

		} else {
			if (retry <= 3) {
				this->get_lyrics(track_id, ++retry);
				return;
			}
			qDebug() << "lyrics for this track is not available on spotify";
			this->curr_lyrics = QJsonArray();
		}
	} else {
		if (retry <= 3) {
			this->get_lyrics(track_id, ++retry);
			return;
		}
		this->p_widget->SetErrMsgBox("Error", "Fail on getting lyrics, program will exit (get request failed)");
	}
}

void LyricsThread::get_playing(uint8_t retry) {
	if (this->auth_access_token == nullptr || time(nullptr) > this->auth_expires) {
		this->get_auth_access_token();
	}
	const httplib::Headers playing_headers = {
		{"User-Agent",
		 "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) "
		 "Chrome/114.0.0.0 Mobile Safari/537.36"},
		{"App-platform", "WebPlayer"},
		{"Authorization", this->auth_access_token.toStdString()},
	};
	qDebug() << "fetching player status";
	if (auto res = open_api_cli.Get("/v1/me/player/currently-playing", playing_headers)) {
		qDebug() << "player status fetched (status code: " << res->status << ")";
		if (res->status == 200) {
			// qDebug() << "fetching player status body: " << QString::fromStdString(res->body);
			JSON d = res->body;
			if (auto is_playing = d["is_playing"]; is_playing.isBool() && is_playing.toBool()) {
				auto item = d["item"].toObject();
				QString track_name = item["name"].toString();
				QString track_id = item["id"].toString();
				int curr_ms = d["progress_ms"].toInt();
				qDebug() << "track_id: " << track_id;
				qDebug() << "curr_ms: " << QString::number(curr_ms);
				if (track_id != this->last_track_id) {
					this->last_track_id = track_id;
					qDebug() << "track changed, fetching lyrics";
					this->get_lyrics(track_id);
				}
				if (this->curr_lyrics.empty()) {
					this->p_widget->SetLyricsLabel(track_name + "(no lyrics available)");
					qDebug() << "no lyrics available";
					return;
				}
				QString last_words = "《" + track_name + "》"; // this->curr_lyrics[0]["words"];
				for (auto raw_line : this->curr_lyrics) {
					QJsonObject line = raw_line.toObject();
					QString words = line["words"].toString();
					int startTimeMs = atoi(line["startTimeMs"].toString().toStdString().c_str());
					if (startTimeMs >= curr_ms) {
						qDebug() << "outputing: " << last_words;
						this->p_widget->SetLyricsLabel(last_words);
						return;
					} else {
						last_words = words;
					}
				}
			} else {
				try {
					auto item = d["item"].toObject();
					QString track_name = item["name"].toString();
					this->p_widget->SetLyricsLabel("《" + track_name + "》");
					qDebug() << "no song playing";
				} catch (std::exception& e) {
					qDebug() << "no song playing";
					this->p_widget->SetLyricsLabel("no song playing");
					return;
				}
			}
		} else if (res->status == 204) {
			this->p_widget->SetLyricsLabel("no song playing");
			qDebug() << "no song playing";
			return;
		} else {
			if (retry <= 3) {
				this->get_auth_access_token();
				this->get_playing(++retry);
				return;
			}
			this->p_widget->SetErrMsgBox(
				"Error", QString("Fail on getting player status, program will exit (status code: ")
							 + QString::number(res->status) + QString(" ") + QString::fromStdString(res->body)
							 + QString(")"));
		}
	}
	// else {
	// 	this->p_widget->SetErrMsgBox("Error", "Fail on getting player status, program will exit (get request failed)");
	// }
}
