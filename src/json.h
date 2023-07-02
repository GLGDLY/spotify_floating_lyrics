#ifndef JSON_DEF
#define JSON_DEF

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <qjsondocument.h>
#include <qjsonvalue.h>


class JSON {
public:
	JSON(void) = default;
	JSON(QFile file);
	JSON(QString json_str);
	JSON(std::string json_str);
	JSON(QByteArray json_bytes);

	const QJsonParseError get_error(void);

	void from_json(const QString json_str);
	QString to_json(void);

	QJsonValue get(const QString key, const QJsonValue default_value = QJsonValue::Undefined) const;
	void set(const QString key, const QJsonValue value);
	QJsonValue operator[](const QString key) const;

private:
	QJsonDocument json_doc;
	QJsonObject json_obj;
	QJsonParseError json_error;
};

#endif
