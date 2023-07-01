#include "json.h"

#include <QDebug>

JSON::JSON(QFile file) {
	// QFile file(path);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString json_str = file.readAll();
	file.close();
	this->from_json(json_str);
}

JSON::JSON(QString json_str) { this->from_json(json_str); }

JSON::JSON(std::string json_str) {
	QString json_qstr = QString::fromStdString(json_str);
	this->from_json(json_qstr);
}

JSON::JSON(QByteArray json_bytes) {
	QString json_str = QString::fromUtf8(json_bytes);
	this->from_json(json_str);
}

const QJsonParseError JSON::get_error(void) { return this->json_error; }

void JSON::from_json(const QString json_str) {
	this->json_doc = QJsonDocument::fromJson(json_str.toUtf8(), &this->json_error);
	if (json_error.error != QJsonParseError::NoError) {
		qDebug() << "Json parse error!";
		return;
	}
	json_obj = json_doc.object();
}

QString JSON::to_json(void) {
	this->json_doc.setObject(this->json_obj);
	return this->json_doc.toJson();
}

QJsonValue JSON::get(const QString key, const QJsonValue default_value) const {
	if (this->json_obj.contains(key)) {
		return this->json_obj[key];
	} else {
		qDebug() << "JSON key not found!";
		return default_value;
	}
}

QJsonValue JSON::operator[](const QString key) const { return this->get(key); }
