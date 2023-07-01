#include "app.h"

#include <QDebug>
#include <qobjectdefs.h>

const QString default_http_resp =
	"<!DOCTYPE html><html><head><title>OK</title></head><body><h1>OK</h1></body><script>setTimeout(function() "
	"{window.close()}, 100);</script></html>";


App::App(void) {
	svr.Get("/lyrics", [&](const httplib::Request& req, httplib::Response& res) {
		qDebug() << "Got request: " << req.path.c_str();
		if (req.has_param("code")) {
			this->verify_code = QString::fromStdString(req.get_param_value("code"));
			this->verify_code_set = true;
		}
		res.set_content(default_http_resp.toStdString(), "text/html");
	});
}

void App::run(void) {
	qDebug() << "Starting server";
	svr.listen("localhost", 8888);
}

bool App::get_verify_code(QString& code) {
	if (this->verify_code_set) {
		code = this->verify_code;
		this->verify_code_set = false;
		return true;
	} else {
		return false;
	}
}